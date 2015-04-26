/* call sbrk from multiple threads */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
int global;

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
   int i, thread_pid, join_pid;

   for (i = 0; i < 20; ++i)
   {
      thread_pid = thread_create(worker, 0);
      assert(thread_pid > 0);
   }

   for (i = 0; i < 20; ++i) 
   {
      join_pid = thread_join(-1);
      assert(join_pid > 0);
   }   
   assert(global == 20);

   printf(1, "TEST PASSED\n");
   exit();
}

void
worker(void *arg_ptr) {
   sbrk(1024);
   global++;
   exit();
}

