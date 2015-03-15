/* Fill half of next fit region. The next allocation after the fill should be the 
 * one that comes after the last allocation */
#include <assert.h>
#include <stdlib.h>
#include "mymem.h"
#include <stdio.h>

int main() {
   char *ptr = (char *)Mem_Init(4096, 64);
   assert(ptr != NULL);
   int i = 0;
   char *nfPtr,*nfPtr1,*nfPtr2,*nfPtr3;
   for(i=0; i<32; i++)
   {
   	nfPtr = (char *)Mem_Alloc(32);
    assert(nfPtr != NULL);
    assert(nfPtr-ptr-sizeof(struct AllocatedHeader) >= 1024);
    if (i == 13)
      nfPtr1 = nfPtr;
    else if (i == 23)
      nfPtr2 = nfPtr;
   }
   //Mem_Dump();
   assert(Mem_Free(nfPtr1) == 0);
   assert(Mem_Free(nfPtr2) == 0);
   Mem_Dump();
   nfPtr3 = (char *)Mem_Alloc(32);
   assert(nfPtr3 != NULL);
   assert(nfPtr3-ptr-sizeof(struct AllocatedHeader) >= 1024);
   Mem_Dump();
   assert((nfPtr + 32 + sizeof(struct AllocatedHeader)) == nfPtr3);
   exit(0);
}
