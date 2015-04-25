#ifndef LOCK_LIB
#define LOCK_LIB
#endif

#include "types.h"
#include "user.h"
#include "fcntl.h"

inline int FetchAndAdd(int * varPtr);

void lock_init(lock_t * lock) {
  lock->ticket = 0;
  lock->turn   = 0;
}

void lock_acquire(lock_t * lock) {
  int myTurn;

  myTurn = FetchAndAdd(&lock->ticket);

  while (lock->turn != myTurn) {
    //spin
  }
}

void lock_release(lock_t * lock) {
  FetchAndAdd(&lock->turn);
}

inline int FetchAndAdd(int * varPtr) {
  int val;

  asm volatile (  "lock; xaddl %%eax, %2;"
                 :"=a" (val)              //output
                 :"a"  (1), "m" (*varPtr) //input
                 :"memory"  );
  return val;
}
