#include "types.h"
#include "user.h"
#include "fcntl.h"

void blarg (void * arg);

int main(int argc, char ** argv) {
  int tid1, tid2;
  char myChar;
  printf(1, "Welcome to the thread library test!\n");

  myChar = 'A';

  tid1 = thread_create(blarg, &myChar);
  tid2 = thread_create(blarg, &myChar);
  printf(1, "CREATE tid1:\t%d\ttid2:\t%d\n--NOW JOINING--\n", tid1, tid2);
  tid1 = thread_join(tid1);
  tid2 = thread_join(tid2);
  printf(1, "JOIN  tid1:\t%d\ttid2:\t%d\n", tid1, tid2);

  exit();
}

void blarg (void * arg) {
  char * cPtr = arg;
  char c = *cPtr;

  if (c > 'z') {
    return;
  }

  while(c <= 'z') {
    printf(1, "\t%c\n", c++);
  }

  exit();
}
