#ifndef THREAD_LIB
#define THREAD_LIB
#endif

#include "types.h"
#include "user.h"
#include "fcntl.h"

int thread_create(void (*start_routine) (void*), void * arg) {
  //sanity check
  if (!arg) {
    return -1;
  }

  void * stackPage = malloc(0x1000);//(PGSIZE);

  return clone(start_routine, arg, stackPage);
}

int thread_join(int pid) {
  int joinRet = join(pid);

  return joinRet > 0 ? joinRet : -1;
}