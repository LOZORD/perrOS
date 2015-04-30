//Leo's implementation of xv6 fsck
//Here we go...

#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include "fs.h"

int loadImage (int, char **);
int checkSuperblock (struct superblock *);

void * imagePointers [16];

int main (int argc, char ** argv) {
  printf("Welcome to fsck!\n");
  int i = 0;
  for (;i < argc; i++) {
    printf("Got argv[%d] as \"%s\"\n", i, argv[i]);
  }

  //first let's put the file into memory using mmap
  int fsIsValid = 1;

  if (argc > 16 - 1) {
    fprintf(stderr, "Too many image files!\n");
    exit(EXIT_FAILURE);
  }
  fsIsValid = loadImage(argc, argv);

  struct superblock mySuperblock;
  fsIsValid = checkSuperblock(&mySuperblock);

  exit(EXIT_SUCCESS);
}

int loadImage (int numFiles, char ** filenames) {
  int i;
  for (i = 1; i < numFiles; i++) {
    //mmap filenames and place in imagePointers array
  }
  return -1;
}

int checkSuperblock (struct superblock * super) {

  return -1;
}
