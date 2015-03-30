#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
int * ptr;

void foo() {
  int local[100];
  if((uint)&local >= 120*4096) foo();
}
int pid;
int main (int argc, char ** argv) {
  /* grow the stack a bit */
  foo();
  //uint STACK = 120*4096;
  //uint USERTOP = 160*4096;
  char str [] = "hello world!";
  if ((pid = fork()) < 0) {
    printf(1, "fork failed\n");
    exit();
  }
  //child proc
  else if (pid == 0) {
    str[1]= 'E';
    printf(1, "child says: %s\n", str);
  }
  //parent
  else {
    wait();
    str[4] = '0';
    printf(1, "parent says: %s\n", str);
  }
  exit();
}
