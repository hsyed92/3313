#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "Starting lazy allocation test...\n");
  
  uint initial_sz = (uint)sbrk(0);
  printf(1, "Initial footprint: %d bytes\n", initial_sz);
  
  // Ask for 100 pages. With lazy allocation, this shouldn't cost physical memory yet.
  int alloc_size = 4096 * 100;
  char *heap = sbrk(alloc_size);
  
  if (heap == (char*)-1) {
    printf(1, "sbrk failed\n");
    exit();
  }
  
  uint new_sz = (uint)sbrk(0);
  printf(1, "Footprint after sbrk: %d bytes (Diff: %d bytes)\n", new_sz, new_sz - initial_sz);
  printf(1, "Checking memory access triggering lazy load...\n");
  
  // Write to just the first and last page to trigger page faults
  heap[0] = 'K';
  heap[alloc_size - 1] = 'H';
  
  if (heap[0] == 'K' && heap[alloc_size - 1] == 'H') {
    printf(1, "Success! Lazily allocated memory works on demand.\n");
  } else {
    printf(1, "Error validating lazy memory contents.\n");
  }
  
  exit();
}
