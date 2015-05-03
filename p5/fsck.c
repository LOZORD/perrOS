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
int checkSuperblock (struct superblock *);
void reportAndDie (int, char *);
int seekToBlock (int blockNum);
int getImageFd (char * filename);
//Global data
int imageFd;
struct superblock mySuperblock;

int main (int argc, char ** argv) {
  printf("Welcome to fsck!\n");

  if (argc < 2) {
    fprintf(stderr, "Need an image file!\n");
    fprintf(stderr, "Usage: fsck <imagefile>\n");
    exit(EXIT_FAILURE);
  }

  reportAndDie(getImageFd(argv[1]), "Could not load image");

  reportAndDie(checkSuperblock(&mySuperblock), "Superblock corrupted");

  exit(EXIT_SUCCESS);
}

int getImageFd (char * filename) {
  imageFd = open(filename, O_RDWR);

  if (imageFd == -1) {
    return INVALID;
  }
  else {
    return VALID;
  }
}

int checkSuperblock (struct superblock * super) {
  //the second block is the superblock
  reportAndDie(seekToBlock(1), "Couldn't seek file\n");
  read(imageFd, super, sizeof(struct superblock));

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

int seekToBlock (int blockNum) {
  if (!imageFd) {
    return INVALID;
  }

  if(lseek(imageFd, blockNum * BSIZE, SEEK_SET) < 0) {
    return INVALID;
  }
  else {
    return VALID;
  }
}
