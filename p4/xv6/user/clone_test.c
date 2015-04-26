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
  //myNum = 0;
  rc = clone(foo, (void *) (&myNum), stackPtr);
  int joinRet = join(rc);
  printf(1, "Should succeed (be >0):\t%d\n", rc);
  printf(1, "Should succeed (be >0):\t%d\n", joinRet);
  printf(1, "\n\nTESTING JOIN WITH -1\n");
  myNum /= 2; //change my num for lels
  //myNum = 200;
  rc = clone(foo, (void *) (&myNum), stackPtr);
  //joinRet = join(-1); //uncomment and fix this case (parent ends before child TODO)
  printf(1, "ENDING CLONE TEST WITH rc=%d\n", rc);
  free(stackPtr);
  sleep(200); //wait 2 seconds
  exit();
}

void foo (void * arg) {
  printf(1, "ENTERED FOO!\n");
  int * ptr = arg;
  int n = *ptr;
  while (n > 0) {
    printf(1, "Count is\t%d\n", n--);
  }
  printf(1, "child is going to sleep...\n");
  sleep(1000); //wait 2 seconds
  printf(1, "child foo exiting!\n");
  exit();
}
