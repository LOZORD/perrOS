/* child thread joining on another child thread */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
int num_threads = 16;

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void worker(void *arg_ptr);
void worker_reaper(void *arg_ptr);

int
main(int argc, char *argv[])
{
   int i;
   ppid = getpid();

   int arg = num_threads / 2;

   for(i = 0; i < num_threads; i++)
   {
   	int thread_pid = thread_create(worker, (void *)arg);
	assert(thread_pid > 0);
   }

   int thread_pid = thread_create(worker_reaper, (void *)arg);
   assert(thread_pid > 0);

   sleep(100);
   for(i = 0; i < arg + 1; i++)
   {
   	int join_pid = thread_join(-1);
   	assert(join_pid > 0);
   }

   printf(1, "TEST PASSED\n");
   exit();
}

void
worker_reaper(void *arg_ptr) {
   int arg = (int)arg_ptr;
   int i;

   for(i = 0; i < arg; i++)
   {
   	int join_pid = thread_join(-1);
   	assert(join_pid > 0);
   }

   exit();
}

void
worker(void *arg_ptr) {
   int i;
   int tmp = 0;

   for(i = 0; i < 10000; i++)
	tmp++;

   sleep(100);

   exit();
}
