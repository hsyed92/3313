#include "types.h"
#include "stat.h"
#include "user.h"

// Use a moderate payload so the timing difference is visible
#define SHMKEY      7
#define CHUNK_SIZE  1024
#define ITERATIONS  2000

// Shared state exchanged between the parent and child
struct shm_region {
  int checksum;
  char payload[CHUNK_SIZE];
};

// Read exactly n bytes from fd unless EOF/error happens first.
static int
read_all(int fd, char *buf, int n)
{
  int cc;
  int total;

  total = 0;
  while(total < n){
    cc = read(fd, buf + total, n - total);
    if(cc < 1)
      return -1;
    total += cc;
  }
  return total;
}

// Write exactly n bytes to fd.
static int
write_all(int fd, char *buf, int n)
{
  int cc;
  int total;

  total = 0;
  while(total < n){
    cc = write(fd, buf + total, n - total);
    if(cc < 1)
      return -1;
    total += cc;
  }
  return total;
}

// Fill a buffer with a deterministic pattern so both sides touch real data.
static void
fill_payload(char *buf, int seed)
{
  int i;

  for(i = 0; i < CHUNK_SIZE; i++)
    buf[i] = 'A' + ((i + seed) % 26);
}

// Measure the cost of copying data through a pipe
static int
run_pipe_benchmark(void)
{
  int p[2];
  int pid;
  int start;
  int end;
  int i;
  int checksum;
  char buf[CHUNK_SIZE];
  char sink[CHUNK_SIZE];

  checksum = 0;
  // Create the pipe used for the copy-based benchmark
  if(pipe(p) < 0){
    printf(1, "pipe benchmark: pipe failed\n");
    return -1;
  }

  // Fork a reader so the parent can stream payloads through the pipe
  pid = fork();
  if(pid < 0){
    printf(1, "pipe benchmark: fork failed\n");
    close(p[0]);
    close(p[1]);
    return -1;
  }

  if(pid == 0){
    // Child consumes data from the pipe
    close(p[1]);
    for(i = 0; i < ITERATIONS; i++){
      if(read_all(p[0], sink, CHUNK_SIZE) != CHUNK_SIZE){
        printf(1, "pipe benchmark: short read\n");
        close(p[0]);
        exit();
      }
      checksum += sink[i % CHUNK_SIZE];
    }
    close(p[0]);
    if(checksum == -1)
      printf(1, "pipe checksum %d\n", checksum);
    exit();
  }

  close(p[0]);

  // Time how long repeated pipe writes take
  start = uptime();
  for(i = 0; i < ITERATIONS; i++){
    fill_payload(buf, i);
    if(write_all(p[1], buf, CHUNK_SIZE) != CHUNK_SIZE){
      printf(1, "pipe benchmark: short write\n");
      close(p[1]);
      wait();
      return -1;
    }
  }
  close(p[1]);
  wait();
  end = uptime();
  return end - start;
}

// Measure the cost of exchanging data through shared memory
static int
run_shared_memory_benchmark(int *checksum_out)
{
  int notify[2];
  int ack[2];
  int addr;
  int pid;
  int start;
  int end;
  int i;
  char token;
  volatile struct shm_region *region;

  // Attach to the shared page used for zero-copy transfers
  addr = shmget(SHMKEY);
  if(addr < 0){
    printf(1, "shared benchmark: shmget failed\n");
    return -1;
  }
  region = (volatile struct shm_region*)addr;
  region->checksum = 0;

  // Small pipes are only used here for synchronization
  if(pipe(notify) < 0 || pipe(ack) < 0){
    printf(1, "shared benchmark: pipe setup failed\n");
    shmclose(SHMKEY);
    return -1;
  }

  pid = fork();
  if(pid < 0){
    printf(1, "shared benchmark: fork failed\n");
    close(notify[0]);
    close(notify[1]);
    close(ack[0]);
    close(ack[1]);
    shmclose(SHMKEY);
    return -1;
  }

  if(pid == 0){
    // Child waits for notifications and reads from the shared page
    close(notify[1]);
    close(ack[0]);

    addr = shmget(SHMKEY);
    if(addr < 0){
      printf(1, "shared benchmark: child shmget failed\n");
      close(notify[0]);
      close(ack[1]);
      exit();
    }
    region = (volatile struct shm_region*)addr;

    // Wait for each notification, then consume the shared payload
    for(i = 0; i < ITERATIONS; i++){
      if(read(notify[0], &token, 1) != 1){
        printf(1, "shared benchmark: notify read failed\n");
        close(notify[0]);
        close(ack[1]);
        shmclose(SHMKEY);
        exit();
      }
      region->checksum += region->payload[i % CHUNK_SIZE];
      if(write(ack[1], "a", 1) != 1){
        printf(1, "shared benchmark: ack write failed\n");
        close(notify[0]);
        close(ack[1]);
        shmclose(SHMKEY);
        exit();
      }
    }

    close(notify[0]);
    close(ack[1]);
    shmclose(SHMKEY);
    exit();
  }

  close(notify[0]);
  close(ack[1]);

  // Parent writes each payload into shared memory and signals the child
  start = uptime();
  for(i = 0; i < ITERATIONS; i++){
    // Write directly into the shared page before notifying the reader
    fill_payload((char*)region->payload, i);
    if(write(notify[1], "n", 1) != 1 || read(ack[0], &token, 1) != 1){
      printf(1, "shared benchmark: sync failed\n");
      close(notify[1]);
      close(ack[0]);
      wait();
      shmclose(SHMKEY);
      return -1;
    }
  }
  end = uptime();

  // Clean up and collect the checksum produced by the child
  close(notify[1]);
  close(ack[0]);
  wait();
  *checksum_out = region->checksum;
  shmclose(SHMKEY);
  return end - start;
}

// Run both IPC methods and print the timing comparison
int
main(void)
{
  int pipe_ticks;
  int shm_ticks;
  int checksum;

  checksum = 0;
  printf(1, "=== Zero-Copy IPC / Shared Memory Benchmark ===\n");
  printf(1, "Transferring %d chunks of %d bytes\n", ITERATIONS, CHUNK_SIZE);

  pipe_ticks = run_pipe_benchmark();
  shm_ticks = run_shared_memory_benchmark(&checksum);

  if(pipe_ticks < 0 || shm_ticks < 0){
    printf(1, "benchmark failed\n");
    exit();
  }

  // Print the timing results for both IPC methods
  printf(1, "pipe transfer time:        %d ticks\n", pipe_ticks);
  printf(1, "shared-memory time:        %d ticks\n", shm_ticks);
  printf(1, "shared-memory checksum:    %d\n", checksum);

  if(pipe_ticks > shm_ticks)
    printf(1, "shared memory saved %d ticks\n", pipe_ticks - shm_ticks);
  else
    printf(1, "shared memory was not faster in this run\n");

  exit();
}
