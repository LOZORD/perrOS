#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{
  int i;

  i = getprocs();

  //TODO probably have to use trap()

  printf("got getprocs return val: %d\n", i);

  exit();
}
