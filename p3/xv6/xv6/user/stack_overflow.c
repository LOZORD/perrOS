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
  char buff [0x1000 - 0x20];
  buff[4] = 'a';
  printf(1, "No overflow yet...\n");
  char over [0x2000];
  over[4] = 'b';
  //over[0x2000 - 1] = 'c';
  //over[0x2001] = 'd';
  printf(1, "SHOULD NEVER PRINT!\n");
  exit();
}
