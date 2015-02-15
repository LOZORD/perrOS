#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "user.h"

//int ctr = 2;

int main (int argc, char ** argv)
{
  /*
  while(ctr-- > 0)
  {
    fork();
  }
  */

  //printf(1,"hello world!\t%d\n", ctr);
  printf(1,"Hello world!\nTesting getprocs...\n");
  printf(1,"\t--> getprocs value: %d\n", getprocs());

  /* another crazy test -- we should only see NPROC=64 max
  while(1)
  {
    fork();
    printf(1,"**\t%d\n", getprocs());
  }
  */

  exit();
}
