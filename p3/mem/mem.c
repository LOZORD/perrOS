#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <pthread.h>
#include "mymem.h"

struct slabAllocator
{
  struct FreeHeader * freeHead;
  pthread_mutex_t slabLock;
};

struct nextFitAllocator
{
  struct FreeHeader * freeHead;
  struct AllocatedHeader * allocatedHead;
  struct AllocatedHeader * nextPtr;
  pthread_mutex_t nextFitLock;
};

struct memAllocators
{
  struct slabAllocator slabAllocator;
  struct nextFitAllocator nextFitAllocator;
};

struct memAllocators myAllocators;
pthread_mutex_t mainAllocatorLock = PTHREAD_MUTEX_INITIALIZER;

static volatile int allocatedOnce = 0;

int fdin;

void * Mem_Init(int sizeOfRegion, int slabSize)
{

  //sanity check
  if (sizeOfRegion < 4 || slabSize < 1 || sizeOfRegion < slabSize)
  {
    return NULL;
  }

  //first check if we have allocated yet
  pthread_mutex_lock(&mainAllocatorLock);
  if (allocatedOnce)
  {
    return NULL;
  }
  else
  {
    allocatedOnce = 1;
  }
  pthread_mutex_unlock(&mainAllocatorLock);

  //assert(sizeOfRegion % 4 == 0);

  int slabRegionSize = sizeOfRegion / 4;

  //TODO: check that we use all of slab region (ie no remainder?)

  //int nextFitRegionSize = sizeOfRegion - slabRegionSize;

  //we will actually have less memory because of embedded nodes
  void * regionStartPtr = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);//mmap(sizeOfRegion);

  if (regionStartPtr == MAP_FAILED)
  {
    fprintf(stderr, "Couldn't get memory!\n");
    exit(EXIT_FAILURE);
  }

  //init the slabAllocator
  myAllocators.slabAllocator.slabLock = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
  myAllocators.slabAllocator.freeHead = (struct FreeHeader *) regionStartPtr;

  //init the nextFitAllocator
  myAllocators.nextFitAllocator.nextFitLock = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
  void * nextFitRegionStartPtr = regionStartPtr + slabRegionSize;
  myAllocators.nextFitAllocator.freeHead = (struct FreeHeader *) (nextFitRegionStartPtr);
  myAllocators.nextFitAllocator.allocatedHead = (struct AllocatedHeader *) (nextFitRegionStartPtr);

  return regionStartPtr;
}
