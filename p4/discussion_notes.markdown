# P4: Threads, Locks, and CVs in xv6

Rewarding but terrifying...

Threads are like one house with multiple roommates. Threads share the same
virtual address space. This is a major difference between `fork`, `exec`, and
what we're adding: `clone`.

## `clone(entrypoint, args, stackPtr)`
* Different from `fork`:
  * `fork` creates a new home
  * `clone` adds a roommate
* The new thread starts running at `entrypoint`
* We have to allocate memory to add the thread's stack
* Starting with a fresh xv6, we have `[ code | stack | heap -> ]`
* Have the concept of an original thread
* We have to allocate a page-aligned stack (4K region) __in the heap__ when we
  create an additional thread
* It __must__ start on a page boundary
* Each time clone executes, we have to have to allocate a stack
* `fork` has different return values for parent and child, then execution
  continues in the next instruction (look at `fork` and `exec` in `proc.c`)
* `clone` does _not_ set up a new address, and execution is different in parent
  and child
* `exec` sets up the stack for a child process, and sets up a new entry point
* We are combining `fork` and `exec` into `clone`
* `clone` returns pid, the thread is just another entry in the proc table
* `clone` assumes heap region (via `stackPtr`) is already allocated

## `join(pid)`
* Takes a `pid`, which is either `-1` or a valid process id
* We have to keep track of all the pids that our parent process creates
* `join` doesn't return until a thread exits
* `-1` arg makes join return when any thread exits
* Will be similar to `wait`
* `exit` stops threads. We have to modify `exit` so that it knows if a "runnable" is a proc or a thread
* The kernel should just clean up the process table entry

## User mode functions: `thread_create` and `thread_join`
* `thread_create(entryPoint, args)`
  * Allocates stack memory for `clone`
* `thread_join(pid)`

## Locks
We're going to need to use `inline`/`asm` calls
We're going to need to modify the xv6 x86 assembly -- see Wikipedia for F&A lock
We are going to create several functions:
### `lock_init(lock_t * l)`
* See implementation in the book (ch 28, page 13)
### `lock_acquire(lock_t * l)`
* Has to lock atomically
* We just spin instead of yielding
* Fetch and add-based locks
* Ticket/deli lock
### `lock_release(lock_t * l)`

## Condition variables
* Made of queues and mutexes (locks, the ones that we built!)
* CV implementation can be found in the book
* See page 3 in chapter 30
* `wait`, `signal`, `broadcast`
* `cv_init`
* `cv_wait`
  * If the lock is free -> grab it and continue
  * Otherwise, go to sleep (`RUNNING -> BLOCKED`, yield?) and wait for signal
* `cv_signal`
  * Wakes up another thread by consulting the queue
  * This means marking it as `BLOCKED -> RUNNABLE`


## Misc
The stack of each thread will not go beyond 4K. We have to avoid concurrency issues in the heap!
Perhaps we should put a lock on it! Like somewhere around malloc.

Cloning leads to there only being one parent. If we have Parent, Child1, and Child1 `clone`s, Parent is still the parent!

What happens if Parent exits before children? We need to add something that adds an original thread or a child thread.
If the Parent exits, all threads should be killed. The Parent owns the address space.
The user is in charge of freeing the heap space used by the thread.

Modify `exit()`: It should use the proc table to check if something is a thread or not. If it is, do special things in `exit()`.
