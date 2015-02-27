# P3A: Memory management -- this is done in Linux (not xv6)
 _Sometimes project details change_
 __THIS WILL BE A HARD PROJECT__

 We will be implementing _TWO_ allocators for heap memory: slab and next fit

 Through a command line arg, we will be given a `size of heap` value

 Our functions will be called `void * Mem_Alloc(size)` and `int Mem_Free(ptr)`

 We have two allocators running at the same time: _how do we tell the
 difference?_

 If we are given 1MB for our heap size, 1/4 of 1MB will go to slab, 3/4 will go
 to next fit

`Mem_Init(size, slab_size)`

If we get an alloc request of slab_size, then use the slab allocator.

If slab is full and we get a slab_size alloc request, use the next fit
allocator.

In order to make testing easier, we are going to have a shared object `so`, but
we need to write a `main` for a tester. They will be releasing _some_ test
scripts.

We need to make our allocators __THREAD SAFE!__

We can't hang or crash. Think one level below "naive" for locking, etc.

You can tell if something is coming from slab or next_fit depending on the
pointer returned from allocation.

Use `mmap` to generate the heap area for our allocator.

Have some globals that store the mutexes, futexes, and some pointers.

We store the free list INSIDE the heap data.

Since we are embedding the free list in the, the largest we can allocate with
next fit is `sizeof(next_fit) - sizeof(alloc_header)`.

```c
  //this header will be provided to us
  struct FreeHeader
  {
    int length;
    struct FreeHeader * next;
    //for slab, next will always be (this_header_addr + header_size +
    //special_slab_size)
  };

  struct AllocatedHeader
  {
    int length;
    void * magic; //used for testing for buffer attacks
  };
```

We don't need `AllocatedHeaders` for our slab allocator. We know the size inherently.

We just need `FreeHeaders` to know if a slab is available or not. For slab, we
do not need to worry about magic number validation, unlike with next fit.

For `free(ptr)`:

* Check if (ptr-h_size)->length == slab size`
* If it is, then free in slab, we don't need to coalesce
* Else, free in next_fit, then coalesce

Memory returns should always be in __16 byte__ offsets for slab.


Global variables to have
* Pointer to the first free block, we need 2
* We need mutexes for thread safety (use 2)
* Bookkeeping values (init args)
* Next fit's moving pointer
