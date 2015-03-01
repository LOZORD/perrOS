
#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#define NPROC 64

void printPStat (struct pstat * p);

int main (int argc, char ** argv)
{
  int pid [6];
  int i;
  struct pstat pinfoArr [NPROC];

  for(i = 0; i < 6; i++){
      pid[i] = fork();
      if(pid[i] == 0){
        switch (i) {  
          case 0:
            settickets(10);
            break;
          case 1:
            settickets(30);
            break;
          case 2:
            settickets(50);
            break;
          case 3:
            settickets(80);
            break;
          case 4:
            settickets(110);
            break;
          case 5:
            settickets(150);
            break;
          default:
            break;
        }
        while(1){};
      }
  }

  int j;
  for(i = 0; i < 6; i++){
    getpinfo(pinfoArr);
    for (j = 0; j < NPROC; j++)
    {
      if(pinfoArr[j].inuse)
        printPStat(pinfoArr + j);
    }
    sleep(6000); //sleep for 1 minute
  }

  for(i = 0; i < 6; i++){
    kill(pid[i]);
  }
  exit();
}

void printPStat (struct pstat * p)
{
  //printf(1, "\n\tNAME: %s\n", p->name);
  printf(1, "\n\tPID: %d\n", p->pid);
  //printf(1, "\n\tIN USE: %s\n", p->inuse ? "YES" : "NO");
  printf(1, "\n\tNUM TICKETS: %d\n", p->tickets);
  //printf(1, "\n\tPASS VAL: %d\n", p->pass);
  //printf(1, "\n\tSTRIDE VAL: %d\n", p->stride);
  printf(1, "\n\tNUM TIMES SCHEDULED: %d\n", p->n_schedule);
  printf(1, "\n\t*****---*****\n\n\n");
}
