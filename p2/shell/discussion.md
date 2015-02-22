# Discussion of Project 2A - Feb 13, 2015

- User mode program in regular Linux
- __MAKING A SHELL__
- A shell is a CLI (command line interpreter), a _user_ process
- A shell allows access to programs, arranges for input and output
- example: `ls`
  * shell allows for certain input and certain output
  * when `ls` terminates, you get another command prompt
- shells have shell scripts
  * leverage basic philosophy behind unix
  * small pipelines that you string together

## Pipes

- Think of a pipe as a tube (with input and output)
- One proc is sending data to another proc (InterProcComm)
- One proc to talk to itself or its own threads
- Data is passed on a _character by character_ basis
- Think of _everything as a file_
- Writing to a pipe is like writing to file
- Character devices
  * Std out/console
  * Very limited random access (due to serial ports)
  * You _can_ peek one character back into the input queue
- Block device
  * Disc drive
- You can also think of a pipe as a buffer
  * It is the size of a page (4KB by default)
- Writing end & reading end
  * Both have no knowledge of buffer size
  * Buffer appears as a continuous stream
  * W & R are async
  * W cannot be faster than R, otherwise pipe becomes full
  * ___What do you do when you get a full pipe?___
    - Deschedule the writing proc of the pipe by making it BLOCKED
    - Same thing if reader is too fast
    - The process will become unblocked when `request write n bytes to pipe` has
    been completed by the kernel (since it is just file I/O)
- __Implementation__
  * We can think of a pipe as a queue, but it is not implemented like this
  * It is actually implemented as a circular array or _ring buffer_
  * Have two pointers, one chasing the other
  * When the pipes are at the same location:
    - Pipe may either be empty or full depending on which one has caught up

## Fork

  - A system call that creates new procs
  - Parent always survives forking fork(Pn) -> Pn & Pm
  - The Parent and Child are identical except for the return value of the fork()
    call
  - A greedy/agressive copy is when a fork automatically creates Child and
    Parent proc without "thinking"
  - Lazy copying is when you copy pages only when Child and Parent become
    different
    * This is kind of like the dirty bit concept in caching
    * Write only when you must
  - `int pid = fork();`
    * `if pid == 0`
      - This process is the Child
      - Do something different than parent
      - In the Child if stmt, there is usually an `exec` call

## Exec

  - How do we move beyond cloning processes?
  - We do `exec("newProgramName");`
  - In the child, once exec happens, the child is obliterated and replaced by
    the new program
  - Once a program calls exec, it never returns
    * Unless the program cannot be found
  - So when we run `ls`
    * sh forks to create child shell
    * Child shell does `exec("ls")`
    * When the child completes, the only thing left is the parent shell


## Implementation stuff

```c
pipe(inf fd[2]);

int fd[2];

fd[0] =  readpipe(fd);
fd[1] = writepipe(fd);

//We know we have an open file table

//The one and zero here are indices to the open file table
//The length of the file table is NOFILE
//Any process can have NOFILE (usually 20) things that LOOK like files

//GOAL: given 'a|b' we want -> a -> b ->

//In shell...

pipe(int fd[2]);

int pid = fork();

//if curr proc is a child __OF SHELL__ (not yet proc a)
if (pid == 0)
{
  close(readpipe);
  //There are always 3 open files
  /*
    OpenFileTable[0] -> stdin
    OpenFileTable[1] -> stdout
    OpenFileTable[2] -> stderr

    We must change a's out to writepipe
  */
  close(stdout); //allow stdout to be "overloaded"
  dup(fd[1]); //change a's stdout to writepipe
  //dup2 also exists: dup2(1, fd[1]) == (close(1 == stdout); dup(fd[1]);)

  //now that I/O is set up, we can exec
  exec(a);
}
else //we're the parent
{
  //We still have both ends of the pipe

  close(writepipe);

  int pid2 = fork();

  if (pid2 == 0) //we're in proc b
  {
    //pipe and file setup stuff

    exec(b);
  }
  else
  {
    //re-assemble std pipes
  }
}
```

## Misc

`foo>file` -- write the output of foo to file
`foo>>file` -- append the output of food to file

When working with files:
`open(..., O_CREAT | O_TRUNC | O_APPEND)`

### The "tee" operator (%)

We are responsible for a maximum of two pipes: `a|b|c`

`a % b` == `a | mytee | b`

Already a Linux thing, just not in the shell

* tee takes stdin and writes to stdout
* It _also_ makes a copy to a file
* Like echo, but it also writes to a file (called tee.txt)

## Builtins (things we do not fork, implemented directly in shell)

`cd`   -> aided by chdir system call
`pwd`  -> current working directory contains relative path
`exit` -> kills current shell

Relative path names do not begin with a slash ('/')

Our shell program will have to configure argc/argv stuff

Let's do this!
