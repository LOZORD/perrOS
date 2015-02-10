#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "user.h"

int main (int argc, char ** argv)
{
  printf(1,"hello world!\n");
  printf(1,"\t-->getprocs: %d\n", getprocs());
  return 0;
}
