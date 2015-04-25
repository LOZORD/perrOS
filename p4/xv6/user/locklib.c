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
  int myturn = -1;

  //myTurn = FetchAndAdd(&lock->ticket); //TODO look up howto/asm code online

  while (lock->turn != myturn) {
    //spin
  }
}

void lock_release(lock_t * lock) {
  //FetchAndAdd(&lock->turn); TODO
}
