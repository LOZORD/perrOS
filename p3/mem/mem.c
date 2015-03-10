#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <pthread.h>
#include "mymem.h"

//declarations
struct freeSlabNode
{
  struct freeSlabNode * next;
};

void * slabPush (struct freeSlabNode * nodeToAdd);
void * slabPop ();

struct slabAllocator
{
  //struct FreeHeader * slabHead; //XXX: perhaps AllocatedHeader?
  //struct freeSlabNode * slabHead;
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
  void * regionStartPtr;
  struct freeSlabNode * topOfSlabStack;
  void * nextFitRegionStartPtr;
  int slabUnitSize;
  int sizeOfRegion;
};

struct memAllocators myAllocators;
pthread_mutex_t mainAllocatorLock = PTHREAD_MUTEX_INITIALIZER;

static volatile int initializedOnce = 0;

int fdin;

void * Mem_Init(int sizeOfRegion, int slabSize) //TODO greater than 8?
{
  //printf("\tGOT region: %d, slab: %d\n", sizeOfRegion, slabSize);
  //sanity check
  if (sizeOfRegion < 4 || slabSize < 1 || sizeOfRegion < slabSize || (sizeOfRegion / 4) < slabSize)
  {
    return NULL;
  }

  //first check if we have allocated yet
  pthread_mutex_lock(&mainAllocatorLock);
  if (initializedOnce)
  {
    return NULL;
  }
  else
  {
    initializedOnce = 1;
  }
  pthread_mutex_unlock(&mainAllocatorLock);

  //assert(sizeOfRegion % 4 == 0);
  //

  myAllocators.slabUnitSize = slabSize;
  myAllocators.sizeOfRegion = sizeOfRegion;

  int slabRegionSize = sizeOfRegion / 4;

  //TODO: check that we use all of slab region (ie no remainder?)

  //int nextFitRegionSize = sizeOfRegion - slabRegionSize;

  //we will actually have less memory because of embedded nodes
  myAllocators.regionStartPtr =
    mmap(NULL, (size_t) sizeOfRegion, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);

  if (myAllocators.regionStartPtr == MAP_FAILED)
  {
    fprintf(stderr, "Couldn't get memory!\n");
    exit(EXIT_FAILURE);
  }

  //init the slabAllocator
  myAllocators.slabAllocator.slabLock = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
  //myAllocators.slabAllocator.slabHead = (struct freeSlabNode *) myAllocators.regionStartPtr;
  myAllocators.topOfSlabStack = (struct freeSlabNode *) myAllocators.regionStartPtr;
  myAllocators.topOfSlabStack->next = NULL;

  myAllocators.nextFitRegionStartPtr = myAllocators.regionStartPtr + slabRegionSize;

  void * itr = myAllocators.topOfSlabStack;
  fprintf(stderr,"itr init val:\t\t%p\n", itr);
  fprintf(stderr,"nf ptr:\t\t%p\n", myAllocators.nextFitRegionStartPtr);

  while (itr < myAllocators.nextFitRegionStartPtr) //myAllocators.nextFitRegionStartPtr)
  {
    slabPush((struct freeSlabNode *)(itr));
    itr += slabSize;
  }

  //init the nextFitAllocator
  myAllocators.nextFitAllocator.nextFitLock = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
  myAllocators.nextFitAllocator.freeHead = (struct FreeHeader *) (myAllocators.nextFitRegionStartPtr);
  myAllocators.nextFitAllocator.allocatedHead = (struct AllocatedHeader *) (myAllocators.nextFitRegionStartPtr);
  myAllocators.nextFitAllocator.nextPtr = (struct AllocatedHeader *) (myAllocators.nextFitRegionStartPtr); //XXX correct type?

  return myAllocators.regionStartPtr;
}

void * Mem_Alloc (int size)
{
  return NULL;
}

int Mem_Free (void * ptr)
{
  if (ptr == NULL)
  {
    fprintf(stderr, "SEGFAULT\n");
    return -1;
  }

  if (ptr < myAllocators.regionStartPtr ||
    ptr > (myAllocators.regionStartPtr + myAllocators.sizeOfRegion))
  {
    fprintf(stderr, "SEGFAULT\n");
    return -1;
  }

  void * headerPtr = ptr - sizeof(struct AllocatedHeader);

  if (((struct AllocatedHeader *)(headerPtr))->magic != (void *)MAGIC)
  {
    return -1;
  }

  int isSlabAllocated = headerPtr < myAllocators.nextFitRegionStartPtr ? 1 : 0;

  if (isSlabAllocated)
  {
    //TODO: slab free
  }
  else
  {
    //TODO: next fit free
  }

  return 0;
}

void Mem_Dump()
{
  //lock the entire segment
  pthread_mutex_lock(&mainAllocatorLock);

  struct freeSlabNode * slabItr = myAllocators.topOfSlabStack;
  int slabCount = 0;

  fprintf(stderr, "REGION START IS %p\n\n", myAllocators.regionStartPtr);

  while (slabItr != NULL)
  {
    fprintf(stderr, "\tFREE SLAB at %p\n", slabItr);
    slabItr = slabItr->next;
    slabCount++;
  }

  fprintf(stderr, "\n\nFOUND %d FREE SLABS\nENTERING NEXT FIT REGION\n", slabCount);

  struct FreeHeader * nextFitItr = myAllocators.nextFitRegionStartPtr;
  int nFCount = 0;

  while (nextFitItr != NULL)
  {
    fprintf(stderr, "\tFREEHEADER at %p:\tLENGTH = %d\n", nextFitItr, nextFitItr->length);
    nextFitItr = nextFitItr->next;
    nFCount++;
  }
  fprintf(stderr, "\n\nFOUND %d NEXT FIT BLOCKS\n--DONE--\n", nFCount);
  pthread_mutex_unlock(&mainAllocatorLock);
}


void * slabPush (struct freeSlabNode * nodeToAdd)
{
  assert(nodeToAdd != NULL);
  pthread_mutex_lock(&(myAllocators.slabAllocator.slabLock));
  /*
  if (myAllocators.topOfSlabStack <= myAllocators.regionStartPtr) //stack overflow
  {
    return NULL; //we can't push anything more!
  }
  */

  if (myAllocators.topOfSlabStack == nodeToAdd)
  {
    nodeToAdd->next = NULL;
  }
  else
  {
    nodeToAdd->next = myAllocators.topOfSlabStack;
  }

  myAllocators.topOfSlabStack = nodeToAdd;
  pthread_mutex_unlock(&(myAllocators.slabAllocator.slabLock));
  return myAllocators.topOfSlabStack;
}

void * slabPop ()
{
  pthread_mutex_lock(&(myAllocators.slabAllocator.slabLock));
  struct freeSlabNode * topOfStack = myAllocators.topOfSlabStack;
  if (topOfStack == NULL)
  {
    return NULL;
  }
  struct freeSlabNode * temp = topOfStack;
  topOfStack = topOfStack->next;
  pthread_mutex_unlock(&(myAllocators.slabAllocator.slabLock));
  return temp;
}
