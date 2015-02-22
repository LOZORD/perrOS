# P2B Discussion
## xv6 scheduler

__Pre-emptive__: scheduler doesn't necessarily let procs run to completion.

- We can use the proc state enum to decide what process to use next
- When a RR scheduler gets a proc with IO (blocked), the IO trap alerts the
  scheduler to the state change
- Then the scheduler would run the next RUNNABLE proc

## Stride scheduling
  * Like lottery (randomized), but deterministic
  * How would we _implement_ a stride scheduler?
    - We need to write a syscall that adds more "tickets" to a process
    - This will allow us to increase or decrease priority
    - Data structures:
      - We need to know a proc's stride value (`BIG_NUMBER/proc->numTickets`)
      - We also need to have an initial pass value ("times strided")
      - Everytime we need to reschedule: `last_proc->passVal += last_proc->strideVal`

## TODOs
  - Replace current xv6 RoundRobin scheduler with our new stride scheduler
  - Add `settickets` syscall (this is required for our stride implementation)
  - Add `getpinfo` syscall (this allows the testers to examine our scheduler)
  - Use `struct pstat`

## Talking about context switches
* To switch contexts, we need to change the PTBR to point to the new proc's pages/page table
* We also need to change the registers (`esp`, `eip`, etc.)
* We save the registers away, and then overwrite them
* We need to switch contexts when quantas end or a process becomes blocked
* The procs don't even know that they are being shared


We have P1 and P2

- First stop P1
- Then we go into kernel mode, using the scheduler to switch contexts
- The scheduler sets the PTBR and saves the registers belonging to the old process
- This needs to be done in `xv6/kernel/proc.c`, so we can access the process table
- See the asm code for `switch` to understand how it is changing (`kernel/switch.S`)

Where is the old proc's state getting set from RUNNING to RUNNABLE?
_Answer_: in the `yield` function in `proc.c`
