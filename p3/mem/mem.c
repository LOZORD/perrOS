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

struct nextFitAllocator
{
  struct FreeHeader * freeHead;
  struct FreeHeader * nextPtr;
};

struct memAllocators
{
  struct nextFitAllocator nextFitAllocator;
  void * regionStartPtr;
  struct freeSlabNode * topOfSlabStack;
  void * nextFitRegionStartPtr;
  int slabUnitSize;
  int sizeOfRegion;
};

struct memAllocators myAllocators;
pthread_mutex_t mainLock = PTHREAD_MUTEX_INITIALIZER;

static volatile int initializedOnce = 0;

int fdin;

void * Mem_Init(int sizeOfRegion, int slabSize) //TODO greater than 8?
{
  pthread_mutex_lock(&mainLock);
  //sanity check
  if (sizeOfRegion < 4 || slabSize < 1 || sizeOfRegion < slabSize || (sizeOfRegion / 4) < slabSize)
  {
    pthread_mutex_unlock(&mainLock);
    return NULL;
  }

  //first check if we have allocated yet
  if (initializedOnce)
  {
    pthread_mutex_unlock(&mainLock);
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
    //fprintf(stderr, "Couldn't get memory!\n");
    pthread_mutex_unlock(&mainLock);
    exit(-1);
  }

  myAllocators.topOfSlabStack = NULL;

  myAllocators.nextFitRegionStartPtr = myAllocators.regionStartPtr + slabRegionSize;

  void * itr = myAllocators.nextFitRegionStartPtr-slabSize;

  while (itr >= myAllocators.regionStartPtr)
  {
    slabPush((struct freeSlabNode *)(itr));
    itr -= slabSize;
  }

  //init the nextFitAllocator
  myAllocators.nextFitAllocator.freeHead  = (struct FreeHeader *) (myAllocators.nextFitRegionStartPtr);
  myAllocators.nextFitAllocator.freeHead->length  = nextFitRegionSize - (sizeof(struct FreeHeader));
  myAllocators.nextFitAllocator.freeHead->next    = NULL;
  myAllocators.nextFitAllocator.nextPtr = myAllocators.nextFitAllocator.freeHead;


  pthread_mutex_unlock(&mainLock);
  return myAllocators.regionStartPtr;
}

//TODO: init memory to zero
void * Mem_Alloc (int size)
{
  pthread_mutex_lock(&mainLock);
  if (!initializedOnce)
  {
    pthread_mutex_unlock(&mainLock);
    return NULL;
  }

  void * ret = NULL;
  //attempt to slab allocate
  if (size == myAllocators.slabUnitSize)
  {
    ret = slabPop();
    if (ret != NULL)
    {
      pthread_mutex_unlock(&mainLock);
      return ret;
    }
  }

  //we do a next fit allocation, or slab couldn't allocate
  ret = nextFitAlloc(size);

  pthread_mutex_unlock(&mainLock);
  return ret;
}

int Mem_Free (void * ptr)
{
  pthread_mutex_lock(&mainLock);
  if (!initializedOnce)
  {
    pthread_mutex_unlock(&mainLock);
    return -1;
  }

  unsigned long ptrVal = (unsigned long) ptr;
  if (ptr == NULL || ptrVal % ALIGN_SIZE != 0)
  {
    fprintf(stderr, "SEGFAULT\n");
    #if DEBUG
    fprintf(stderr, "null pointer or improperly aligned pointer\n");
    #endif
    pthread_mutex_unlock(&mainLock);
    return (-1);
  }

  if (ptr < myAllocators.regionStartPtr ||
    ptr > (myAllocators.regionStartPtr + myAllocators.sizeOfRegion))
  {
    fprintf(stderr, "SEGFAULT\n");
    #if DEBUG
    fprintf(stderr, "pointer is out of bounds\n");
    #endif
    pthread_mutex_unlock(&mainLock);
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
      pthread_mutex_unlock(&mainLock);
      return -1;
    }

    slabPush(ptr); //ignore return val???
  }
  else
  {
    //TODO: next fit free
    retVal = nextFitFree(ptr);
  }
  pthread_mutex_unlock(&mainLock);
  return retVal;
}

void Mem_Dump()
{
  //lock the entire segment
  pthread_mutex_lock(&mainLock);

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
  pthread_mutex_unlock(&mainLock);
}


void * slabPush (struct freeSlabNode * nodeToAdd)
{
  assert(nodeToAdd != NULL);
  nodeToAdd->next = myAllocators.topOfSlabStack;
  myAllocators.topOfSlabStack = nodeToAdd;
  return myAllocators.topOfSlabStack;
}

void * slabPop ()
{
  if (myAllocators.topOfSlabStack == NULL)
  {
    return NULL; //empty stack
  }
  void * temp = myAllocators.topOfSlabStack;
  myAllocators.topOfSlabStack = myAllocators.topOfSlabStack->next;
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

  //now we attempt to allocated alignedSize bytes

  if (myAllocators.nextFitAllocator.freeHead == NULL)
  {
    return NULL;
  }

  //if we've reached the end of our list, start searching over at the beginning
  if (myAllocators.nextFitAllocator.nextPtr == NULL)
  {
    myAllocators.nextFitAllocator.nextPtr = myAllocators.nextFitAllocator.freeHead;
  }

  struct FreeHeader * itr = myAllocators.nextFitAllocator.nextPtr;
  struct AllocatedHeader * ret = NULL;
  //struct FreeHeader * temp = myAll

  do
  {
    //we can use this chunk
    if (itr->length + sizeof(struct FreeHeader) >= alignedSize + sizeof(struct AllocatedHeader))
    {
      if (itr->length + sizeof(struct FreeHeader) == alignedSize + sizeof(struct AllocatedHeader))
      {
        myAllocators.nextFitAllocator.nextPtr = itr->next;
      }
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
        myAllocators.nextFitAllocator.nextPtr = leftOver;
      }
      break;
    }
    else
    {
      if (itr->next == NULL)
      {
        itr = myAllocators.nextFitAllocator.freeHead;
      }
      else
      {
        itr = itr->next;
      }
    }
    #if DEBUG
    fprintf(stderr, "here 336\n"); //XXX
    #endif
  } while (itr != myAllocators.nextFitAllocator.nextPtr);


  return !ret ? NULL : ((void *)(ret) + sizeof(struct AllocatedHeader));
}

int nextFitFree (void * ptr)
{
  //ptr is guaranteed within the bounds of the nextFit region

  struct AllocatedHeader * allocatedPtr
    = (struct AllocatedHeader *)(ptr - sizeof(struct AllocatedHeader));

  if (allocatedPtr->magic != (void *)MAGIC)
  {
    #if DEBUG
    fprintf(stderr, "SEGFAULT\n");
    fprintf(stderr, "(bad magic number)\n");
    #endif
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
  nextFitCoalesce(freePtr);

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
    #if DEBUG
    fprintf(stderr, "here 422\n");//XXX
    #endif
  }


  //coalesce with next neighbor
  if (nextNeighborIsFree)
  {
    removeFreeNode(nextNeighbor);
    freeHeadPtr->length += nextNeighbor->length + sizeof(struct FreeHeader);
  }

  //coalesce with prev neighbor
  if (prevNeighbor != NULL)
  {
    prevNeighbor->next = freeHeadPtr->next;
    prevNeighbor->length += freeHeadPtr->length + sizeof(struct FreeHeader);
    removeFreeNode(freeHeadPtr);
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
    #if DEBUG
    fprintf(stderr, "here 464\n"); //XXX
    #endif
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
    if(itr == freePtr){
      //nothing already in list
      return;
    }
    else if (itr->next == NULL)
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
    #if DEBUG
    fprintf(stderr, "here 506\n"); //XXX
    #endif
  }
}
