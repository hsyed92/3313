#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  int pid;
  char *p1;
  char *p2;

  printf(1, "=== budget test start ===\n");

  // Budget is total allowed sz; after exec, sz is already program + stack pages.
  // Allow exactly two more heap pages beyond the current break.
  {
    uint base = (uint)sbrk(0);
    uint cap = base + 2 * 4096;
    if(setbudget((int)cap) < 0){
      printf(1, "setbudget failed (try cap=%d, initial sz issue)\n", cap);
      exit();
    }
    printf(1, "budget set to %d bytes (current sz %d + 2 pages)\n", cap, base);
  }

  // First page should succeed
  p1 = sbrk(4096);
  if((int)p1 == -1){
    printf(1, "first sbrk failed unexpectedly\n");
    exit();
  }
  printf(1, "first page request succeeded\n");

  // Second page should also succeed
  p2 = sbrk(4096);
  if((int)p2 == -1){
    printf(1, "second sbrk failed unexpectedly\n");
    exit();
  }
  printf(1, "second page request succeeded\n");

  // Third page should fail
  if((int)sbrk(4096) != -1){
    printf(1, "ERROR: third sbrk should have failed\n");
  } else {
    printf(1, "third page request correctly denied\n");
  }

  // Fork should fail once process is at budget
  pid = fork();
  if(pid < 0){
    printf(1, "fork correctly denied at budget limit\n");
  } else if(pid == 0){
    printf(1, "ERROR: child was created when fork should fail\n");
    exit();
  } else {
    wait();
  }

  printf(1, "=== budget test end ===\n");
  exit();
}