#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{
  int i;

  i = getprocs();

  printf("got getprocs return val: %d\n", i);

  exit();
}
