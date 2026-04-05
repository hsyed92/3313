#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;
  struct proc *p = myproc();

  if(argint(0, &n) < 0)
    return -1;

  addr = p->sz;

  if(n < 0){
    if(growproc(n) < 0)
      return -1;
  } else if(n > 0){
    if(p->energy_budget != -1 && p->sz + n > (uint)p->energy_budget){
      p->denied_allocs++;
      return -1;
    }
    p->sz += n;
  }

  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

extern int eco_mode;

int
sys_seteco(void)
{
  int mode;
  
  if(argint(0, &mode) < 0)
    return -1;
    
  eco_mode = mode;
  return 0;
}

int
sys_setbudget(void)
{
  int bytes;

  if(argint(0, &bytes) < 0)
    return -1;

  return setbudget(bytes);
}

// Create or attach to a shared page by key
int
sys_shmget(void)
{
  int key;

  if(argint(0, &key) < 0)
    return -1;
  return shmget(key);
}

// Detach the caller from a shared page
int
sys_shmclose(void)
{
  int key;

  if(argint(0, &key) < 0)
    return -1;
  return shmclose(key);
}
