#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{
  int i;

  i = getprocs();

  //TODO what to do with this file???
  //printf(1,"got getprocs return val: %d\n", i);

  exit();
}
