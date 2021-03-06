#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"

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
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
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
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since boot.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int sys_clone (void) {
  void (* entryPoint) (void *) = NULL;
  void * entryPointArgs = NULL, * stack = NULL;
  int rc = 0, voidSize = sizeof(void *);

  if (argptr(0, (char **) &entryPoint, voidSize) < 0) {
    return -1;
  }
  if (argptr(1, (char **) &entryPointArgs, voidSize) < 0) {
    return -1;
  }
  if (argptr(2, (char **) &stack, voidSize) < 0) {
    return -1;
  }

  //return pid on success
  rc = proc_clone(entryPoint, entryPointArgs, stack);

  return rc;
}

int sys_join (void) {
  //cprintf("entered sys_join!\n");
  int pid;
  if(argint(0, &pid) < 0)
    return -1;

  int joinRet = proc_join(pid);
  //cprintf("Got pid as %d\n", pid);
  //cprintf("Got jR as %d\n", joinRet);
  return joinRet;
}

int sys_wake (void) {
  int pid;

  if (argint(0, &pid) < 0)
    return -1;

  proc_wakeup(pid);

  return 0;
}

int sys_ticket_sleep (void) {
  int pid;
  void * lock;

  if(argint(0, &pid) < 0) {
    return -1;
  }
  if(argptr(1, (char **) &lock, sizeof(void *)) < 0) {
    return -1;
  }

  ticket_sleep(pid, lock);
  return 0;
}
