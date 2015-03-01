#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#define NPROC 64
#define BIG_NUM 1E8

void printPStat (struct pstat * p);
int longOp();

int main (int argc, char ** argv)
{
  int rc;
  struct pstat pinfoArr [NPROC];

  rc = settickets(10);
  int i;

  /*
  getpinfo(pinfoArr);

  for (i = 0; i < NPROC; i++)
  {
    if(pinfoArr[i].inuse)
      printPStat(pinfoArr + i);
  }
  */

  printf(1,"\t\tFORKING AND SLEEPING\t\t\n");

  int pid1;

  if ((pid1 = fork()) < 0)
  {
    printf(1,"FORK ERROR\n");
    exit();
  }

  int pid2, pid3, pid4;
  pid2 = pid3 = pid4 = 10;
  //child1 should run 3 times as often as parent
  if (pid1 == 0) //we are in child1
  {
    pid2 = fork();
    settickets(20);
    if (pid2 == 0)
    {
      pid3 = fork();
      settickets(60);
      if (pid3 == 0)
      {
        pid4 = fork();
        settickets(100);
        if (pid4 == 0)
        {
          settickets(150);
        }
      }
    }
  }

  sleep(100); //sleep for 1 second

  printf(1,"\tstarted longOp\n");
  int foo = longOp();
  printf(1,"\tgot foo: %d\n", foo);

  //parent waits for child to complete
  if ((pid1 && pid2 && pid3 && pid4))
  {
    wait();
    wait();
    wait();
    wait();
  }
  else if(pid4 == 0) //child4 does second print stmt
  {
    int a = getpinfo(pinfoArr);
    if (a < 0)
    {
      printf(1, "ERROR WITH getpinfo!\n");
    }
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
}

int longOp (void)
{
  int a, b;

  b = 5;

  for (a = 0; a <= BIG_NUM; a++)
  {
    b |= (b * a - 206);
  }

  return b;
}
