#ifndef THREAD_LIB
#define THREAD_LIB
#endif

#include "types.h"
#include "user.h"
#include "fcntl.h"

struct pidMemPair {
  short isFree;
  int pid;
  void * memRegion;
};

#define PID_MEM_TABLE_SIZE 64
#define PGSIZE 4096
struct pidMemPair pidMemTable [PID_MEM_TABLE_SIZE] = { { 1, -1, 0 } };

int thread_create(void (*start_routine) (void*), void * arg) {

  void * stackPage = malloc(PGSIZE * 2);//(PGSIZE);
  void * alignedStack = (void *)((uint)stackPage + (PGSIZE - (uint)stackPage % PGSIZE));

  if (!stackPage) {
    return -1;
  }

  int pid = clone(start_routine, arg, alignedStack);
  if(pid != -1){
    int i;
    for(i = 0; i < PID_MEM_TABLE_SIZE; i++){
      if(pidMemTable[i].isFree){
        pidMemTable[i].isFree = 0;
        pidMemTable[i].pid = pid;
        pidMemTable[i].memRegion = stackPage;
        break;
      }
    }
  }
  return pid;
}

int thread_join(int pid) {
  void * stackPtr; //= (void *) getThreadStack(pid);
  int joinRet = join(pid);
  if(joinRet != -1){
    int i;
    for(i = 0; i < PID_MEM_TABLE_SIZE; i++){
      if(pidMemTable[i].isFree)
        continue;
      if(pidMemTable[i].pid == joinRet){
        stackPtr = pidMemTable[i].memRegion;
        free(stackPtr);
        pidMemTable[i].isFree = 1;
        pidMemTable[i].memRegion = 0;
        pidMemTable[i].pid = -1;
        break;
      }
    }
  }

  return joinRet > 0 ? joinRet : -1;
}
