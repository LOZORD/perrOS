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

  assert(pArr[0] && pArr[1] && pArr[2]);

  Mem_Dump();

  fprintf(stderr, "FREEING SLABS OUT OF ORDER\n");

  Mem_Free(pArr[1]);
  Mem_Free(pArr[0]);
  Mem_Free(pArr[2]);

  Mem_Dump();

  fprintf(stderr, "ALLOCATING ALL SLABS\n");

  int i;

  for (i = 0; i < 16; i++)
  {
    pArr[i] = Mem_Alloc(16);
    assert(pArr[i] != NULL);
  }

  Mem_Dump();

  //test next fit

  fprintf(stderr, "*** NOW TESTING NEXT FIT ***\n\n");

  fprintf(stderr, "\ttesting alloc of 14 bytes\n");
  pArr[16] = Mem_Alloc(14);
  assert(pArr[16] != NULL);

  fprintf(stderr, "\ttesting alloc of 17 bytes\n");
  pArr[17] = Mem_Alloc(17);
  assert(pArr[17] != NULL);

  fprintf(stderr, "\ttesting alloc of 512 bytes\n");
  pArr[18] = Mem_Alloc(512);
  assert(pArr[18] != NULL);

  fprintf(stderr, "\ttesting alloc 16 bytes\n");
  pArr[19] = Mem_Alloc(16); //attempts to be a slab, but is put in nextFit
  assert(pArr[19] != NULL);

  fprintf(stderr, "\ttesting alloc 404 bytes\n");
  pArr[20] = Mem_Alloc(404);
  assert(pArr[20] == NULL);

  pArr[20] = Mem_Alloc(113);
  assert(pArr[20] == NULL);

  assert(Mem_Free(pArr[19]) == 0);

  Mem_Dump();

  pArr[20] = Mem_Alloc(113);
  assert(pArr[20] != NULL);

  Mem_Dump();

  assert(Mem_Free(pArr[0]) == 0);
  assert(Mem_Alloc(16) != NULL);

  int j;
  int k;
  for(j = 0; j < 16; j++){
    for(k = 0; k < 16; k++){ 
      assert(((char *)pArr[k])[j] == 0);
    }
    assert(((char *)pArr[19])[j] == 0);
  }

  assert(Mem_Free(pArr[19]) == 0);
  assert(Mem_Free(pArr[18]) == 0);

  pArr[19] = Mem_Alloc(256);

  assert(pArr[19] != NULL);
  fprintf(stderr, "\t\t\t\t\t\t\tpArr[19] is %p\n", pArr[19]);

  fprintf(stderr, "*** should be segfault ***\n");
  assert(Mem_Free(NULL) == -1);

  struct AllocatedHeader * aHead = pArr[19] - sizeof(struct AllocatedHeader);
  void * maybeMagic = aHead->magic;
  fprintf(stderr, "\t\t\t\tMaybe Magic? %p\n", maybeMagic);
  assert(maybeMagic == (void *)MAGIC);

  assert(Mem_Free(pArr[19]) == 0);
  assert(aHead->magic != (void *)MAGIC);

  assert(Mem_Free(pArr[19]) == -1);

  printf("TESTS PASS!\n");
  exit(EXIT_SUCCESS);
}
