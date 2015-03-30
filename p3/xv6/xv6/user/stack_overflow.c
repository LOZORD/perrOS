#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   exit(); \
}
int main (int argc, char ** argv) {
  printf(1, "TESTING STACK OVERFLOW\n");
  char buff [0x1001]; //4k+1 buffer, should alloc new page
  buff[4] = 'a';
  printf(1, "No overflow yet...\n");
  char over [0x2001]; //8k+1 buffer, should fail
  //char * over = malloc(sizeof(char) * 0x2001);
  over[4] = 'b';
  over[0x2000 - 1] = 'c';
  over[0x2001] = 'd';
  printf(1, "SHOULD NEVER PRINT!\n");
  exit();
}
