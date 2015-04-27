#ifndef CV_LIB
#define CV_LIB
#endif

#include "types.h"
#include "user.h"
#include "x86.h"
#include "fcntl.h"

void cv_init(cond_t * cvar) {
  cvar->head = 0;
  cvar->tail = 0;
  cvar->lock = NULL;
}

void cv_wait(cond_t * cvar, lock_t * lock) {
  if (cvar->tail >= CVAR_QUEUE_SIZE || (cvar->tail + 1 == cvar->head)) {
    printf(1, "ERROR: condition variable queue full!\n");
    return;
  }

  int currPid = getpid();
  cvar->queue[cvar->tail] = currPid;

  /*
  if (cvar->head == cvar->tail) {
    //no one else in the queue
  }
  else {
    //you have to wait
  }
  */

  cvar->tail = (cvar->tail + 1) % CVAR_QUEUE_SIZE;

  if (cvar->lock != NULL) {
    cvar->lock = lock;
  }

  ticket_sleep(currPid, (char *) lock);
  lock_acquire(lock);
}

void cv_signal(cond_t * cvar) {
  if (cvar->head <= 0) {
    printf(1, "ERROR: condition variable queue empty!\n");
    return;
  }

  int currPid = cvar->queue[cvar->head];
  cvar->queue[cvar->head] = -1; //clear this current pid

  cvar->head = (cvar->head + 1) % CVAR_QUEUE_SIZE;
  wake(currPid); //wakeup the next pid
  lock_release(cvar->lock);
}
