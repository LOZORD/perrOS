#ifndef CV_LIB
#define CV_LIB
#endif

#include "types.h"
#include "user.h"
#include "fcntl.h"

void cv_init(cond_t * cvar) {
  cvar->headPosition  = 0;
  cvar->size          = 0;
  cvar->lock          = NULL;
}

void cv_wait(cond_t * cvar, lock_t * lock) {
  if (cvar->size >= CVAR_QUEUE_SIZE) {
    printf(1, "ERROR: condition variable queue full!\n");
    return;
  }

  int position = (cvar->headPosition + cvar->size) % CVAR_QUEUE_SIZE;

  //if (cvar->size != 0) {
  cvar->queue[position] = getpid();
  //}

  cvar->size++;

  cvar->lock = lock;

  lock_acquire(lock); //get the lock, or wait on it
}

void cv_signal(cond_t * cvar) {
  if (cvar->size <= 0) {
    printf(1, "ERROR: condition variable queue empty!\n");
    return;
  }

  cvar->queue[cvar->headPosition] = -1; //clear this current pid

  cvar->headPosition = (cvar->headPosition + 1) % CVAR_QUEUE_SIZE;

  cvar->size--;
  //TODO call wake thread on the new pid (do it in the right order!)
  //wakeup(cvar->queue[cvar->headPosition]);
  lock_release(cvar->lock);
}
