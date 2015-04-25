#include "types.h"
#include "user.h"
#include "fcntl.h"

#define BIG 0x1000000

void actor (void * arg);
static volatile int globalA, globalB;
lock_t lock;

void * DO_NOT_LOCK = (void *)0x0a0;
void * DO_LOCK     = (void *)0x0b0;

int main(int argc, char ** argv) {
  printf(1, "You've entered the lock test program!\n");

  int tidA1, tidA2, tidB1, tidB2;
  globalA = globalB = 0;
  lock_init(&lock);

  tidA1 = thread_create(actor, DO_NOT_LOCK);
  tidA2 = thread_create(actor, DO_NOT_LOCK);
  printf(1, "CREATE tidA1:\t%d\ttidA2:\t%d\n", tidA1, tidA2);
  tidA1 = thread_join(tidA1);
  tidA2 = thread_join(tidA2);
  printf(1, "JOIN   tidA1:\t%d\ttidA2:\t%d\n", tidA1, tidA2);
  printf(1, "\n***\n\nNON-LOCKED globalA:\t%x\n\n***\n", globalA);

  tidB1 = thread_create(actor, DO_LOCK);
  tidB2 = thread_create(actor, DO_LOCK);
  printf(1, "CREATE tidB1:\t%d\ttidB2:\t%d\n", tidB1, tidB2);
  tidB1 = thread_join(tidB1);
  tidB2 = thread_join(tidB2);
  printf(1, "JOIN   tidB1:\t%d\ttidB2:\t%d\n", tidB1, tidB2);
  printf(1, "\n***\n\nLOCKED     globalB:\t%x\n\n***\n", globalB);
  exit();
}

void actor (void * arg) {
  int i;
  if (arg == DO_LOCK) {
    lock_acquire(&lock);
  }

  //TODO make non-locking more of an issue!
  //we want to make sure that our locks are solving the critical region!
  for (i = 0; i < BIG; i++) {
    if (arg == DO_NOT_LOCK) {
      globalA++;
    }
    else {
      globalB++;
    }
  }

  if (arg == DO_LOCK) {
    lock_release(&lock);
  }

  exit();
}
