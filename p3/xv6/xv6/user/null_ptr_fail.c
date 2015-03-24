#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
int * ptr;
int main (int argc, char ** argv) {
  int i = 0xBABECAFE;
  ptr = &i;
  printf(1, "GOOD ptr val: %x\n", *ptr);
  ptr = 0;
  printf(1, "BAD  ptr val: %x\n", *ptr);
  exit();
}
