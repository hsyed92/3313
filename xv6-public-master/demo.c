// Integrated demo: runs lazy allocation, quotas, shared-memory IPC, and eco mode.
#include "types.h"
#include "stat.h"
#include "user.h"

static void
runprog(char *path, char **argv)
{
  int pid;

  pid = fork();
  if(pid < 0){
    printf(1, "demo: fork failed before %s\n", path);
    return;
  }
  if(pid == 0){
    exec(path, argv);
    printf(1, "demo: exec %s failed\n", path);
    exit();
  }
  wait();
}

int
main(void)
{
  char *av_lazy[] = { "lazy_test", 0 };
  char *av_budget[] = { "budget_test", 0 };
  char *av_shm[] = { "shmtest", 0 };
  char *av_eco_on[] = { "suspend", "1", 0 };
  char *av_eco_off[] = { "suspend", "0", 0 };

  printf(1, "\n");
  printf(1, "========== SE 3313 Group 6: Resource-Aware xv6 Demo ==========\n");
  printf(1, "Feature order: (1) quotas, (2) lazy alloc, (3) zero-copy IPC, (4) deep sleep\n\n");

  printf(1, "--- Feature 1: Resource quotas / energy budgets (budget_test) ---\n");
  runprog("budget_test", av_budget);

  printf(1, "\n--- Feature 2: Lazy page allocation (lazy_test) ---\n");
  runprog("lazy_test", av_lazy);

  printf(1, "\n--- Feature 3: Zero-copy IPC benchmark (shmtest) ---\n");
  runprog("shmtest", av_shm);

  printf(1, "\n--- Feature 4: Eco / deep-sleep mode (suspend 1 | suspend 0) ---\n");
  runprog("suspend", av_eco_on);
  printf(1, "Eco mode ON: idle CPUs execute HLT in the scheduler (see proc.c).\n");
  printf(1, "Sleeping 30 ticks as a short idle window, then disabling eco mode...\n");
  sleep(30);
  runprog("suspend", av_eco_off);

  printf(1, "\n========== Demo complete. Shell ready. ==========\n\n");
  exit();
}
