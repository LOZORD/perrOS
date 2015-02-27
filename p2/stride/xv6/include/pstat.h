#ifndef _PSTAT_H_
#define _PSTAT_H_

struct pstat {
        int inuse; // whether this slot of the process process table is in use (1 or 0)
        int pid;   // the PID of each process
        char name[16];  // name of the process
        int tickets;    // number of tickets assigned to this process
        int pass;       // current pass value for the process
        int stride;     // stride value
        int n_schedule; // number of times chosen for scheduling
};

/*
 * Since xv6 can hold upto NPROC (64) processes, be sure to allocate via 
 * memory object sized (64 * sizeof(struct pstat)).
 */ 


#endif // _PSTAT_H_
