/*call sbrk from clone and see changes in parent */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

int ppid;
#define PGSIZE (4096)

volatile int global = 1;

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void worker(void *arg_ptr);

int
main(int argc, char *argv[])
{
   ppid = getpid();
   void *stack = malloc(PGSIZE*2);
   assert(stack != NULL);
   if((uint)stack % PGSIZE)
     stack = stack + (4096 - (uint)stack % PGSIZE);

   void *before = sbrk(0);
   int clone_pid = clone(worker, 0, stack);
   assert(clone_pid > 0);
   while(global != 5);
   void *after = sbrk(0);
   assert (after > before);
   printf(1, "TEST PASSED\n");
   exit();
}

void
worker(void *arg_ptr) {
   assert(global == 1);
   sbrk(1024);
   sleep(2);
   global = 5;
   sleep(5);
   exit();
}



