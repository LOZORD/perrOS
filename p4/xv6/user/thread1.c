/* Creating of a thread from a child thread */
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
void worker_brahma(void *arg_ptr);

int
main(int argc, char *argv[])
{
   int i;
   ppid = getpid();

   int arg = num_threads / 2;

   int thread_pid = thread_create(worker_brahma, (void *)arg);
   assert(thread_pid > 0);

   for(i = 0; i < arg; i++)
   {
   	int thread_pid = thread_create(worker, (void *)arg);
    if (thread_pid <= 0) {
      printf(1, "CREATER GOT BAD PID OF %d\n", thread_pid);
    }
	assert(thread_pid > 0);
   }

   sleep(100);

   for(i = 0; i < num_threads + 1; i++)
   {
   	int join_pid = thread_join(-1);
    if (thread_pid <= 0) {
      printf(1, "JOINER GOT BAD PID OF %d\n", thread_pid);
    }
   	assert(join_pid > 0);
   }

   printf(1, "TEST PASSED\n");
   exit();
}

void
worker_brahma(void *arg_ptr) {
   int arg = (int)arg_ptr;
   int i;

   for(i = 0; i < arg; i++)
   {
   	int thread_pid = thread_create(worker, &arg);
    if (thread_pid <= 0) {
      printf(1, "BRAHMA GOT BAD PID OF %d\n", thread_pid);
    }
   	assert(thread_pid > 0);
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
