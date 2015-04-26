/* ticket lock order */
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

void worker1(void *arg_ptr);
void worker2(void *arg_ptr);
void worker3(void *arg_ptr);
void worker4(void *arg_ptr);

lock_t lock;

int
main(int argc, char *argv[])
{
   ppid = getpid();
   lock_init(&lock);
   
   lock_acquire(&lock);

   
   thread_create(worker1, 0);
   thread_create(worker2, 0);
   thread_create(worker3, 0);
   thread_create(worker4, 0);
   sleep(1200);
   lock_release(&lock);
   sleep(500);
   assert(global == 4);

   printf(1, "TEST PASSED\n");
   exit();
}

void
worker1(void *arg_ptr) {
   sleep(200);
   lock_acquire(&lock);
   assert(global == 0);
   global++;
   lock_release(&lock);
   exit();
}

void
worker2(void *arg_ptr) {
   sleep(400);
   lock_acquire(&lock);
   assert(global == 1);
   global++;
   lock_release(&lock);
   exit();
}

void
worker3(void *arg_ptr) {
   sleep(600);
   lock_acquire(&lock);
   assert(global == 2);
   global++;
   lock_release(&lock);
   exit();
}

void
worker4(void *arg_ptr) {
   sleep(800);
   lock_acquire(&lock);
   assert(global == 3);
   global++;
   lock_release(&lock);
   exit();
}


