#include "types.h"
#include "stat.h"
#include "user.h"
#define NPROC 64

struct pstat {
  int strideVal;
  int passVal;
  int numTickets;
  int pid;
};

void printPStat (struct pstat * p);

int main (int argc, char ** argv)
{
  int rc;
  struct pstat pinfoArr [NPROC];

  printf(1,"Hello world!\n\n");
  procdump();

  rc = settickets(150);
  procdump();
  printf(1, "FOR SETTICKETS 150, GOT RC OF %d\n", rc);

  getpinfo(pinfoArr);

  int i;

  for (i = 0; i < NPROC; i++)
  {
    printPStat(pinfoArr + i);
  }

  rc = settickets(10);
  procdump();
  printf(1, "FOR SETTICKETS 10, GOT RC OF %d\n", rc);
  
  /*
  rc = settickets(160);
  procdump();
  printf(1, "FOR SETTICKETS 160, GOT RC OF %d\n", rc);
  
  rc = settickets(0);
  procdump();
  printf(1, "FOR SETTICKETS 0, GOT RC OF %d\n", rc);

  rc = settickets(11);
  procdump();
  printf(1, "FOR SETTICKETS 11, GOT RC OF %d\n", rc);
  */

  getpinfo(pinfoArr);

  for (i = 0; i < NPROC; i++)
  {
    printPStat(pinfoArr + i);
  }
  exit();
}

void printPStat (struct pstat * p)
{
  if (p->numTickets != 0)
  {
    printf(1, "\n\tPID: %d\n", p->pid);
    printf(1, "\n\tNUM TICKETS: %d\n", p->numTickets);
    printf(1, "\n\tPASS VAL: %d\n", p->passVal);
    printf(1, "\n\tSTRIDE VAL: %d\n", p->strideVal);
    printf(1, "\n\t*****\t*****\n");
  }
  else
  {
    //it's unused or something
  }
}
