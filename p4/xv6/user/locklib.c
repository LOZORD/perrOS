#ifndef LOCK_LIB
#define LOCK_LIB
#endif

#include "types.h"
#include "user.h"
#include "fcntl.h"

void lock_init(lock_t * lock) {
  lock->ticket = 0;
  lock->turn   = 0;
}

void lock_acquire(lock_t * lock) {
  int myTurn;

  myTurn = FetchAndAdd(&lock->ticket, 1);
  //TODO do we need to put anything to sleep here?
  while (lock->turn != myTurn) {
    //spin
  }
}

void lock_release(lock_t * lock) {
  FetchAndAdd(&lock->turn, 1);
}

//using Wikipedia's example of fetch_and_add
inline int FetchAndAdd(int * varPtr, int incr) {

  asm volatile (  "lock; xaddl %%eax, %2;"
                 :"=a" (incr)              //output
                 :"a"  (incr), "m" (*varPtr) //input
                 :"memory"  );
  return incr;
}
