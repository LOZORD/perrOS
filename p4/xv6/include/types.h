#ifndef _TYPES_H_
#define _TYPES_H_

// Type definitions

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef uint pde_t;
//LOCK LIBRARY
//Taken from OSTEP CH 28, PG 13
typedef struct _lock_ {
  int ticket;
  int turn;
} lock_t;
void lock_init(lock_t * lock);
void lock_acquire(lock_t * lock);
void lock_release(lock_t * lock);
inline int FetchAndAdd(int * varPtr, int incr);

//CV LIBRARY
//CV struct TODO
#define CVAR_QUEUE_SIZE 64
typedef struct _cvar_ {
  int head;                     //the head position of our circular array
  int tail;                     //the number of live threads in our queue
  int init;
  int queue [CVAR_QUEUE_SIZE];  //the circular array
  //lock_t * lock; //TODO: do we want this?
} cond_t;
void cv_init(cond_t * cvar);
void cv_wait(cond_t * cvar, lock_t * lock);
void cv_signal(cond_t * cvar);
#ifndef NULL
#define NULL (0)
#endif

#endif //_TYPES_H_
