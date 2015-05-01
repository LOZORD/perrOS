//Leo's implementation of xv6 fsck
//Here we go...

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <assert.h>
#include "fs.h"
#define VALID 1
#define INVALID 0

//Function prototypes
int loadImage (char *);
int checkSuperblock (struct superblock *);
void * Mmap (char *);
void reportAndDie (int, char *);
//Global data
void * imagePointer;
struct superblock mySuperblock;

int main (int argc, char ** argv) {
  printf("Welcome to fsck!\n");

  if (argc < 2) {
    fprintf(stderr, "Need an image file!\n");
    fprintf(stderr, "Usage: fsck <imagefile>\n");
    exit(EXIT_FAILURE);
  }

  reportAndDie(loadImage(argv[1]), "Could not load image");

  reportAndDie(checkSuperblock(&mySuperblock), "Superblock corrupted");

  exit(EXIT_SUCCESS);
}

int loadImage (char * filename) {
  void * ptr = Mmap(filename);
  if (ptr == NULL) {
    return 0;
  }
  else {
    imagePointer = ptr;
  }
  return 1;
}

void * Mmap (char * filename) {
  void * ptr = NULL;
  int fd;
  struct stat filestatus;

  fd = open(filename, O_RDONLY);

  if (fd == -1) {
    fprintf(stderr, "Couldn't read file descriptor\n");
    exit(EXIT_FAILURE);
  }

  if (fstat(fd, &filestatus) == -1) {
    fprintf(stderr, "Couldn't read file status\n");
    exit(EXIT_FAILURE);
  }

  ptr = mmap(NULL, filestatus.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

  if (ptr == MAP_FAILED) {
    fprintf(stderr, "Couldn't mmap file image\n");
    exit(EXIT_FAILURE);
  }

  return ptr;
}

int checkSuperblock (struct superblock * super) {
  //the first thing in the data should be the superblock
  super = (struct superblock *) imagePointer;

  if (  super->size    == 0 ||
        super->nblocks == 0 ||
        super->ninodes == 0) {
    return INVALID;
  }

  uint totalINodeSize = super->ninodes * sizeof(struct dinode);
  uint totalBlockSize = super->nblocks * BSIZE;

  /*
  if (  super->size < totalINodeSize ||
        super->size < totalBlockSize ||
        super->size < totalINodeSize + totalBlockSize) {
    return INVALID;
  }
  */

  if (super->size < totalINodeSize + totalBlockSize) {
    return INVALID;
  }


  return VALID;
}

void reportAndDie (int isValid, char * report) {
  if (!isValid) {
    fprintf(stderr, "ERROR: %s\n", report);
    exit(EXIT_FAILURE);
  }
}
