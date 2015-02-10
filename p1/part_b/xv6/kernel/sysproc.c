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

  //done by the kernel, gets stuff from the user stack
  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  //the current process
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

int sys_getprocs(void)
{
  //TODO count the number of non-unused procs in the proc table
  //TODO lock the table
  //extern struct ptable;
  //cprintf("\t-->%p\n", ptable);
  /*
   * struct proc * itr = ptable->proc;
   * int count = 0;
   *
   * while(itr && *itr)
   * {
   *   if (itr->state != UNUSED)
   *   {
   *     count++;
   *   }
   *
   *   itr++;
   * }
   *
   * return count;
   *
   *
   *
   *
    acquire(&ptable.lock); and then unlock the table
   */
  return -1;
}
