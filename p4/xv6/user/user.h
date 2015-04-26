#ifndef _USER_H_
#define _USER_H_

struct stat;

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(char*, int);
int mknod(char*, short, short);
int unlink(char*);
int fstat(int fd, struct stat*);
int link(char*, char*);
int mkdir(char*);
int chdir(char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int clone(void (* fnc) (void *), void * arg, void * stack);
int join(int pid);
int getThreadStack (int pid);

// user library functions (ulib.c)
int stat(char*, struct stat*);
char* strcpy(char*, char*);
void *memmove(void*, void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, char*, ...);
char* gets(char*, int max);
uint strlen(char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);
//THREAD LIBRARY
int thread_create(void (*start_routine)(void *), void * arg);
int thread_join(int pid);
//LOCK LIBRARY
//Taken from OSTEP CH 28, PG 13
typedef struct _lock_ {
  int ticket;
  int turn;
} lock_t;
void lock_init(lock_t * lock);
void lock_acquire(lock_t * lock);
void lock_release(lock_t * lock);
inline int FetchAndAdd(int * varPtr);
//CV LIBRARY
//CV struct TODO
#define CVAR_QUEUE_SIZE 64
typedef struct _cvar_ {
  int headPosition;             //the head position of our circular array
  int queue [CVAR_QUEUE_SIZE];  //the circular array
  int size;                     //the number of live threads in our queue
  lock_t * lock; //TODO: do we want this?
} cond_t;
void cv_init(cond_t * cvar);
void cv_wait(cond_t * cvar, lock_t * lock);
void cv_signal(cond_t * cvar);

#endif // _USER_H_

