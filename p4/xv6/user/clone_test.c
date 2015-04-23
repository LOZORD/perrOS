#include "types.h"
#include "stat.h"
#include "user.h"

void foo (void *);

int main(int argc, char *argv[])
{
  int myNum = 10;
  void * stackPtr = malloc(0x1000); //allocate a page...
  
  if (stackPtr == NULL) {
    printf(1, "ERROR!\n");
    exit();
  }

  printf(1, "BEGINNING CLONE TEST!\n");
  int rc = clone(NULL, NULL, NULL);
  printf(1, "Should fail (be -1):\t%d\n", rc);
  printf(1, "foo ptr\t%p\nmyNum ptr\t%p\nstackPtr\t%p\n", foo, &myNum, stackPtr);
  rc = clone(foo, (void *) (&myNum), stackPtr);
  sleep(500);
  printf(1, "Should succeed (be >0):\t%d\n", rc);
  printf(1, "ENDING CLONE TEST WITH rc=%d\n", rc);
  free(stackPtr);
  exit();
}

void foo (void * arg) {
  printf(1, "ENTERED FOO!\n");
  int * ptr = arg;
  int n = *ptr;
  while (n > 0) {
    printf(1, "Count is\t%d\n", n--);
  }
  exit();
}
