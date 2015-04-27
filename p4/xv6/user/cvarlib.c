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
  cvar->init = 1;
}

void cv_wait(cond_t * cvar, lock_t * lock) {
  if(!cvar->init){
    cv_init(cvar);
  }
  if (cvar->tail >= CVAR_QUEUE_SIZE || (cvar->tail + 1 == cvar->head)) {
    printf(1, "ERROR: condition variable queue full!\n");
    return;
  }

  int currPid = getpid();
  cvar->queue[cvar->tail] = currPid;

  cvar->tail = (cvar->tail + 1) % CVAR_QUEUE_SIZE;

  ticket_sleep(currPid, (char *) lock);
  lock_acquire(lock);
}

void cv_signal(cond_t * cvar) {
  if(!cvar->init){
    cv_init(cvar);
  }
  if (cvar->head == cvar->tail) {
    return;
  }

  int currPid = cvar->queue[cvar->head];
  cvar->queue[cvar->head] = -1; //clear this current pid

  cvar->head = (cvar->head + 1) % CVAR_QUEUE_SIZE;
  wake(currPid); //wakeup the next pid
}
