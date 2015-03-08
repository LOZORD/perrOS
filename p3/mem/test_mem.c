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

  printf("TESTS PASS!\n");
  exit(EXIT_SUCCESS);
}
