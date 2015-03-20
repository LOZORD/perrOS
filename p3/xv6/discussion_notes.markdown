# P3B (Moving segments)

__Plan, read, and study first__

Videos will be posted. __Know what you're doing ahead of time__

## Segments in xv6

- We usually have a `[code/static | heap -> | <- stack]` (stack at high addrs,
  code at low addrs)
- xv6 does it like `[code/static | stack | heap ->]`
- xv6 has a var called `USRTOP`, which is 640K
- Pages are 4K in size
- xv6 stack is fixed to be one page in size
- xv6 code and heap: size depends (heap grows, stack can't)
- xv6 puts code/static segment at addr 0


## The three steps

* __Add guard page__ (`NULL` region)
  * This moves the program chunk one page down in memory
  * There might be some issues with init though
  * This will look like `[guard | code/static | stack | heap ->]`
  * xv6 has a `pgdir` address for the 2-level page table system
  * There are already kernel function calls in the kernel that help with this
  * This will help us create segfaults (yay!)
  * The guard page will have to offset the code/static region
  * Non-guard pages are offset by 1 page, use syscalls for this
  * Look for `USER_LDFLAGS += --section` in the Makefile
* __Move the stack__
  * We've implemented and tested our guard page
  * We want to have a one page stack at the end of the chunk of memory
  * We also need to work with exec here
  * We need to move the stack from the middle to the end
  * `sz` or its assignee need to be changed
  * _A big change_: now we have a growning stack __and__ heap
  * We have non-contiguous regions
  * We need to have two `sz`'s for the stack and heap in order to determine what segment things fall in
  * _See test program ideas below_
* __Make the stack and heap growable__
  * By now, we should have `[code/static | heap -> | <- stack]`
  * The heap growing should already work
  * We just need to focus on the stack
  * What do we do when a request falls in the no-man's land? => `PAGE FAULT`
  * We need to add the page fault handling
  * It's a "handleable" page fault if the addr is right above the stack's page
  * We should handle and die on stack overflows
  * __We have to arrange to have a 1 page buffer between the stack and the heap__
  * That buffer _is_ in the address space, but it is __not__ a valid page!

## `exec`

In a file, we have an ELF header and a set of program headers.
This is related to the `USER_LDFLAGS` and the `.text` label in xv6.

In `kernel/exec` (`exec` replaces a program), kernel does read based on inode.

`sz` is a _very_ important variable. `phnum` = amount of program headers.
It is the number of bytes in the address space.

Look around line 58 in the `exec` code. The first program header _should_ be 4k
instead of zero.

### Testing
We can test by:
- Writing a program that stack allocates buffers of <=> page size
- Allocate a huge stack
- Allocate a huge heap (should get near the stack)
- Stack addrs should all be > heap addrs
- Test for heap/stack overflow
- Recursive function calls (deterministic and non-deterministic) use the Ackermann function 8|

### Misc
Look at the hints in the spec.
Look at `exec`.
Look at `trap`.
Perry: "Suppose you made a system call that passed an address"
Perry: "Pay attention to the `sz` variable"
__Watch the variable__
