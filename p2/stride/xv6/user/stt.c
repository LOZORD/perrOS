#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#define NPROC 64

void printPStat (struct pstat * p);

int main (int argc, char ** argv)
{
  int rc;
  struct pstat pinfoArr [NPROC];

  rc = settickets(50);
  int i;

  getpinfo(pinfoArr);

  for (i = 0; i < NPROC; i++)
  {
    if(pinfoArr[i].inuse)
      printPStat(pinfoArr + i);
  }

  printf(1,"\t\tFORKING AND SLEEPING\t\t\n");

  int pid1, pid2;
  pid2 = -1;

  if ((pid1 = fork()) < 0)
  {
    printf(1,"FORK ERROR\n");
    exit();
  }

  //parent also forks into child2
  if (pid1 != 0)
  {
    if ((pid2 = fork()) < 0)
    {
      printf(1,"FORK ERROR\n");
      exit();
    }
  }

  //child1 should run 3 times as often as parent
  if (pid1 == 0 && pid2 != 0) //we are in child1
  {
    settickets(150);
  }
  //child2 should run the slowest
  if (pid2 == 0) //we are in child2
  {
    settickets(10);
  }

  sleep(1000); //sleep for 10 seconds


  //parent waits for child to complete
  if (pid1 != 0 && pid2 != 0)
  {
    wait();
    wait();
  }
  else
  {
    getpinfo(pinfoArr);
    for (i = 0; i < NPROC; i++)
    {
      if(pinfoArr[i].inuse)
        printPStat(pinfoArr + i);
    }
  }


  exit();
}

void printPStat (struct pstat * p)
{
  printf(1, "\n\tNAME: %s\n", p->name);
  printf(1, "\n\tPID: %d\n", p->pid);
  printf(1, "\n\tIN USE: %s\n", p->inuse ? "YES" : "NO");
  printf(1, "\n\tNUM TICKETS: %d\n", p->tickets);
  printf(1, "\n\tPASS VAL: %d\n", p->pass);
  printf(1, "\n\tSTRIDE VAL: %d\n", p->stride);
  printf(1, "\n\tNUM TIMES SCHEDULED: %d\n", p->n_schedule);
  printf(1, "\n\t*****---*****\n");
  /*
  if (p->numTickets != 0)
  {
    printf(1, "\n\tPID: %d\n", p->pid);
    printf(1, "\n\tIN USE: %s\n", p->inuse ? "YES" : "NO");
    printf(1, "\n\tNUM TICKETS: %d\n", p->numTickets);
    printf(1, "\n\tPASS VAL: %d\n", p->passVal);
    printf(1, "\n\tSTRIDE VAL: %d\n", p->strideVal);
    printf(1, "\n\t*****---*****\n");
  }
  else
  {
    //it's unused or something
  }
  */
}
