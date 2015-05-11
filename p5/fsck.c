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

#define DEBUG 0

//Function prototypes
int checkSuperblock (struct superblock *);
void reportAndDie (int, char *);
int seekToBlock (int blockNum);
int getImageFd (char * filename);
//Global data
int imageFd;
struct superblock mySuperblock;

int main (int argc, char ** argv) {
  #if DEBUG
  printf("Welcome to fsck!\n");
  #endif

  if (argc < 2) {
    fprintf(stdout, "Need an image file!\n");
    fprintf(stdout, "Usage: fsck <imagefile>\n");
    exit(EXIT_FAILURE);
  }

  reportAndDie(getImageFd(argv[1]), "Could not load image");

  reportAndDie(checkSuperblock(&mySuperblock), "Superblock corrupted");
  #if DEBUG
  printf("Superblock is ok\n");
  #endif

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

  #if DEBUG
  printf("super\n\tsize\t%d\n\tnblocks\t%d\n\tninodes\t%d\n",
    super->size, super->nblocks, super->ninodes);
  #endif

  if (  super->size    == 0 ||
        super->nblocks == 0 ||
        super->ninodes == 0) {
    return INVALID;
  }

  //computation from xv6/tools/mkfs.c
  int bitblocks, usedblocks, freeblock;

  bitblocks = super->size/(512*8) + 1;
  usedblocks = super->ninodes / IPB + 3 + bitblocks;
  freeblock = usedblocks;

  #if DEBUG
  printf("used %d (bit %d ninode %zu) free %u total %d\n", usedblocks,
         bitblocks, super->ninodes/IPB + 1, freeblock, super->nblocks+usedblocks);
  #endif

  int expectedSize = 4 + (super->ninodes/IPB) + super->nblocks;
  if (super->size != expectedSize) {
    super->size = expectedSize;
    seekToBlock(1);
    write(imageFd, super, sizeof(struct superblock));
  }
  #if DEBUG
  printf("super\n\tsize\t%d\n\tnblocks\t%d\n\tninodes\t%d\n",
    super->size, super->nblocks, super->ninodes);
  #endif
  if (super->size % BSIZE != 0) {
    return INVALID;
  }

  if (super->size < super->ninodes || super->size < super->nblocks) {
    return INVALID;
  }


  /*
  uint totalINodeSize = super->ninodes * sizeof(struct dinode);
  uint totalBlockSize = super->nblocks * BSIZE;

  if (  super->size < totalINodeSize ||
        super->size < totalBlockSize ||
        super->size < totalINodeSize + totalBlockSize) {
    return INVALID;
  }

  if (super->size < totalINodeSize + totalBlockSize) {
    return INVALID;
  }
  */

  /*
  if (super->size < super->nblocks + super->ninodes) {
    return INVALID;
  }
  */

  return VALID;
}

void reportAndDie (int isValid, char * report) {
  if (!isValid) {
    printf("Error!\n");
    #if DEBUG
    fprintf(stderr, "ERROR: %s\n", report);
    #endif
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
