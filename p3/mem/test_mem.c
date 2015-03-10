#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//void * Mem_Init(int a, int b);

#include "mymem.h"

int main (int argc, char ** argv)
{
  printf("Hello world!\n");

  void * heapPtr = Mem_Init(-1,-1);

  assert(heapPtr == NULL); //bad values

  heapPtr = Mem_Init(3, 4); //region < slab

  assert(heapPtr == NULL);

  heapPtr = Mem_Init(1024, 300); // region / 4 < slab

  assert(heapPtr == NULL);

  heapPtr = Mem_Init(1024, 16); // good init call

  assert(heapPtr != NULL);

  heapPtr = Mem_Init(1024, 16);

  assert(heapPtr == NULL); // should fail due to called twice

  Mem_Dump();

  //TODO: use threading to test mutex
  //
  //



  void * pArr [128];

  pArr[0] = Mem_Alloc(16);
  fprintf(stderr, "\tALLOC'D SLAB: %p\n", pArr[0]);
  pArr[1] = Mem_Alloc(16);
  fprintf(stderr, "\tALLOC'D SLAB: %p\n", pArr[1]);
  pArr[2] = Mem_Alloc(16);
  fprintf(stderr, "\tALLOC'D SLAB: %p\n", pArr[2]);
  pArr[3] = Mem_Alloc(15); //this should be a next fit alloc, ie unimplemented

  assert(pArr[0] && pArr[1] && pArr[2]);
  assert(pArr[3] == NULL);

  Mem_Dump();

  fprintf(stderr, "FREEING SLABS OUT OF ORDER\n");

  Mem_Free(pArr[1]);
  Mem_Free(pArr[0]);
  Mem_Free(pArr[2]);

  Mem_Dump();

  printf("TESTS PASS!\n");
  exit(EXIT_SUCCESS);
}
