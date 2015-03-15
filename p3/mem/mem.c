#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <pthread.h>
#include "mymem.h"

#define ALIGN_SIZE 16
#define DEBUG 0

//declarations
struct freeSlabNode
{
  struct freeSlabNode * next;
};

void *  slabPush (struct freeSlabNode * nodeToAdd);
void *  slabPop ();
void *  nextFitAlloc(int size);
int     nextFitFree(void * ptr);
void    nextFitCoalesce (struct FreeHeader * freeHeadPtr);
void    addFreeNode (struct FreeHeader * freePtr);
void    removeFreeNode (struct FreeHeader * freePtr);

struct slabAllocator
{
  pthread_mutex_t slabLock;
};

struct nextFitAllocator
{
  struct FreeHeader * freeHead;
  struct FreeHeader * nextPtr;
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

  assert(sizeOfRegion % 4 == 0);

  myAllocators.slabUnitSize = slabSize;
  myAllocators.sizeOfRegion = sizeOfRegion;

  int slabRegionSize = sizeOfRegion / 4;

  int nextFitRegionSize = sizeOfRegion - slabRegionSize;

  //TODO: check that we use all of slab region (ie no remainder?)

  //we will actually have less memory because of embedded nodes
  myAllocators.regionStartPtr =
    mmap(NULL, (size_t) sizeOfRegion, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  if (myAllocators.regionStartPtr == MAP_FAILED)
  {
    fprintf(stderr, "Couldn't get memory!\n");
    exit(-1);
  }

  //init the slabAllocator
  myAllocators.slabAllocator.slabLock = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
  myAllocators.topOfSlabStack = NULL;

  myAllocators.nextFitRegionStartPtr = myAllocators.regionStartPtr + slabRegionSize;

  void * itr = myAllocators.nextFitRegionStartPtr-slabSize;

  while (itr >= myAllocators.regionStartPtr)
  {
    slabPush((struct freeSlabNode *)(itr));
    itr -= slabSize;
  }

  //init the nextFitAllocator
  myAllocators.nextFitAllocator.nextFitLock = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
  myAllocators.nextFitAllocator.freeHead  = (struct FreeHeader *) (myAllocators.nextFitRegionStartPtr);
  myAllocators.nextFitAllocator.freeHead->length  = nextFitRegionSize - (sizeof(struct FreeHeader));
  myAllocators.nextFitAllocator.freeHead->next    = NULL;

  pthread_mutex_unlock(&mainAllocatorLock);

  return myAllocators.regionStartPtr;
}

//TODO: init memory to zero
void * Mem_Alloc (int size)
{
  //pthread_mutex_lock();
  if (!initializedOnce)
  {
    return NULL;
  }

  void * ret = NULL;
  //attempt to slab allocate
  if (size == myAllocators.slabUnitSize)
  {
    ret = slabPop();
    if (ret != NULL)
    {
      return ret;
    }
  }

  //we do a next fit allocation, or slab couldn't allocate
  ret = nextFitAlloc(size);

  return ret;
}

int Mem_Free (void * ptr)
{
  //TODO return null if not initializedOnce
  //TODO locking
  unsigned long ptrVal = (unsigned long) ptr;
  if (ptr == NULL || ptrVal % ALIGN_SIZE != 0)
  {
    fprintf(stderr, "SEGFAULT\n");
    #if DEBUG
    fprintf(stderr, "null pointer or improperly aligned pointer\n");
    #endif
    return (-1);
  }

  if (ptr < myAllocators.regionStartPtr ||
    ptr > (myAllocators.regionStartPtr + myAllocators.sizeOfRegion))
  {
    fprintf(stderr, "SEGFAULT\n");
    #if DEBUG
    fprintf(stderr, "pointer is out of bounds\n");
    #endif
    return (-1);
  }

  int isSlabAllocated = ptr < myAllocators.nextFitRegionStartPtr ? 1 : 0;
  int retVal = 0;

  if (isSlabAllocated)
  {
    //if the pointer does not correspond to a slab of slabUnitSize
    if((int)(myAllocators.nextFitRegionStartPtr - ptr) % myAllocators.slabUnitSize != 0)
    {
      fprintf(stderr, "SEGFAULT\n");
      return -1;
    }

    slabPush(ptr); //ignore return val???
  }
  else
  {
    //TODO: next fit free
    retVal = nextFitFree(ptr);
  }

  return retVal;
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

  struct FreeHeader * nextFitItr = myAllocators.nextFitAllocator.freeHead;
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
  //TODO check that not out of bounds
  assert(nodeToAdd != NULL);
  pthread_mutex_lock(&(myAllocators.slabAllocator.slabLock));
  nodeToAdd->next = myAllocators.topOfSlabStack;
  myAllocators.topOfSlabStack = nodeToAdd;
  pthread_mutex_unlock(&(myAllocators.slabAllocator.slabLock));
  return myAllocators.topOfSlabStack;
}

void * slabPop ()
{
  pthread_mutex_lock(&(myAllocators.slabAllocator.slabLock));
  if (myAllocators.topOfSlabStack == NULL)
  {
    return NULL; //empty stack
  }
  void * temp = myAllocators.topOfSlabStack;
  myAllocators.topOfSlabStack = myAllocators.topOfSlabStack->next;
  pthread_mutex_unlock(&(myAllocators.slabAllocator.slabLock));
  return temp;
}

void * nextFitAlloc (int size)
{
  //first make allocation 16 btye-aligned

  int alignedSize = 0;

  if (size % ALIGN_SIZE != 0)
  {
    alignedSize = size + (ALIGN_SIZE -(size % ALIGN_SIZE));
  }
  else
  {
    alignedSize = size;
  }

  pthread_mutex_lock(&myAllocators.nextFitAllocator.nextFitLock);
  //now we attempt to allocated alignedSize bytes
  //

  struct FreeHeader * itr = myAllocators.nextFitAllocator.freeHead;
  struct AllocatedHeader * ret = NULL;

  while (itr != NULL)
  {
    //we can use this chunk
    if (itr->length + sizeof(struct FreeHeader) >= alignedSize + sizeof(struct AllocatedHeader))
    {
      int currLen = itr->length;
      removeFreeNode(itr);
      ret = (struct AllocatedHeader *) itr;
      ret->magic = (void *)MAGIC;
      ret->length = alignedSize;
      //TODO: remove itr from the free list
      if (currLen + sizeof(struct FreeHeader) > alignedSize + sizeof(struct AllocatedHeader))
      {
        //make new free node
        struct  FreeHeader * leftOver = (void *)(itr) + alignedSize + sizeof(struct AllocatedHeader);
        leftOver->length = currLen - alignedSize - sizeof(struct FreeHeader);
        addFreeNode(leftOver);
      }
      break;
    }
    else
    {
      itr = itr->next;
    }
  }

  pthread_mutex_unlock(&myAllocators.nextFitAllocator.nextFitLock);

  return !ret ? NULL : ((void *)(ret) + sizeof(struct AllocatedHeader));
}

int nextFitFree (void * ptr)
{
  //ptr is guaranteed within the bounds of the nextFit region
  pthread_mutex_lock(&myAllocators.nextFitAllocator.nextFitLock);

  struct AllocatedHeader * allocatedPtr
    = (struct AllocatedHeader *)(ptr - sizeof(struct AllocatedHeader));

  if (allocatedPtr->magic != (void *)MAGIC)
  {
    #if DEBUG
    fprintf(stderr, "SEGFAULT\n");
    fprintf(stderr, "(bad magic number)\n");
    #endif
    pthread_mutex_unlock(&myAllocators.nextFitAllocator.nextFitLock);
    return (-1);
  }

  int length = allocatedPtr->length;

  struct FreeHeader * freePtr = (struct FreeHeader *) allocatedPtr;

  freePtr->length = length;

  /*
  freePtr->next = myAllocators.nextFitAllocator.freeHead;

  myAllocators.nextFitAllocator.freeHead = freePtr;
  */

  addFreeNode(freePtr);

  //now, we attempt to coalesce
  nextFitCoalesce(myAllocators.nextFitAllocator.freeHead);

  pthread_mutex_unlock(&myAllocators.nextFitAllocator.nextFitLock);
  return 0;
}

void nextFitCoalesce (struct FreeHeader * freeHeadPtr)
{
  #if DEBUG
  fprintf(stderr, "\t\t\tCOALESCING WITH FREE HEAD PTR: %p\n", freeHeadPtr);
  #endif
  struct FreeHeader * nextNeighbor = NULL;
  struct FreeHeader * prevNeighbor = NULL;
  struct FreeHeader * itr = myAllocators.nextFitAllocator.freeHead;
  int nextNeighborIsFree = 0;

  nextNeighbor = (void *)(freeHeadPtr) + freeHeadPtr->length + sizeof(struct FreeHeader);

  #if DEBUG
  fprintf(stderr, "\t\t\tNEXT NEIGHBOR: %p\n", nextNeighbor);
  #endif

  struct FreeHeader * nextNode;


  while (itr != NULL)
  {
    nextNode = (void *)(itr) + itr->length + sizeof(struct FreeHeader);

    //coalesce with prevNeighbor
    if (nextNode == freeHeadPtr)
    {
      prevNeighbor = itr;
    }
    //coalesce with nextNeighbor
    else if (itr == nextNeighbor)
    {
      nextNeighborIsFree = 1;
    }
    else
    {
      //do nothing
    }

    itr = itr->next;
  }


  //coalesce with next neighbor
  if (nextNeighborIsFree)
  {
    //freeHeadPtr->next = nextNeighbor->next;
    removeFreeNode(nextNeighbor);
    freeHeadPtr->length += nextNeighbor->length + sizeof(struct FreeHeader);
  }

  //coalesce with prev neighbor
  if (prevNeighbor != NULL)
  {
    removeFreeNode(prevNeighbor);
    prevNeighbor->next = freeHeadPtr->next;
    prevNeighbor->length += freeHeadPtr->length + sizeof(struct FreeHeader);
    myAllocators.nextFitAllocator.freeHead = prevNeighbor;
  }

}

void removeFreeNode (struct FreeHeader * freePtr)
{
  struct FreeHeader * itr = myAllocators.nextFitAllocator.freeHead;

  if (itr == freePtr) //we haven't allocated yet
  {
    myAllocators.nextFitAllocator.freeHead = itr->next;
    return;
  }

  while (itr != NULL)
  {
    if (itr->next == freePtr)
    {
      itr->next = freePtr->next;
      return;
    }

    itr = itr->next;
  }
}

void addFreeNode (struct FreeHeader * freePtr)
{
  //freePtr->next = myAllocators.nextFitAllocator.freeHead;
  //myAllocators.nextFitAllocator.freeHead = freePtr;
  assert(freePtr != NULL);

  struct FreeHeader * itr = myAllocators.nextFitAllocator.freeHead;
  if (itr == NULL)
  {
    freePtr->next = NULL;
    myAllocators.nextFitAllocator.freeHead = freePtr;
    return;
  }
  else if (itr > freePtr)
  {
    freePtr->next = myAllocators.nextFitAllocator.freeHead;
    myAllocators.nextFitAllocator.freeHead = freePtr;
    return;
  }

  while (itr != NULL)
  {
    if (itr->next == NULL)
    {
      itr->next = freePtr;
      freePtr->next = NULL;
      return;
    }
    else if (itr->next > freePtr)
    {
      freePtr->next = itr->next;
      itr->next = freePtr;
      return;
    }
    else
    {
      itr = itr->next;
    }
  }
}

void sortList (struct FreeHeader * freePtr)
{
}
