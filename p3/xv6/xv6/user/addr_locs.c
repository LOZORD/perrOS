#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
//#include "assert.h"
#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   exit(); \
}
int main (int argc, char ** argv) {
  int * stackPtr, * heapPtr;
  int i = 1234;
  stackPtr = &i;
  heapPtr = malloc(sizeof(int));
  assert(heapPtr < stackPtr);
  printf(1, "TESTS PASS\n");
  exit();
}
