#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int mode;

  if(argc < 2){
    printf(1, "Usage: suspend [0 or 1]\n");
    exit();
  }

  mode = atoi(argv[1]);
  seteco(mode);
  
  printf(1, "Eco mode set to %d\n", mode);
  exit();
}