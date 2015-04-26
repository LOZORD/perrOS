/* parent exit must kill all threads */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
int global = 1;

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
   int i;
   ppid = getpid();
   int thread_pid;

   int spid = fork();

   if (spid > 0)
   {
      wait();

      /*
       * 50 threads are spawned again. This will
       * pass only if the child process (with 50
       * threads) exited properly.
       */ 
      for (i = 0; i < 50; ++i)
      {
         thread_pid = thread_create(worker, 0);
	 assert(thread_pid > 0);
      }

      printf(1, "TEST PASSED\n");
      exit();
   }
   else
   {
      /* 
       * Spawn 50 threads and all threads should be 
       * killed during exit 
       */
      for (i = 0; i < 50; ++i)
      {
         thread_pid = thread_create(worker, 0);
      }
      exit();
   }
}

void
worker(void *arg_ptr) {
   while(1);
   exit();
}

