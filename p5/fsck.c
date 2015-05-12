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
#define SUPERBLOCK_NUMBER 1
#define INODES_BLOCK_NUMBER 2

#define DEBUG 1
// File system implementation.  Four layers:
//   + Blocks: allocator for raw disk blocks.
//   + Files: inode allocator, reading, writing, metadata.
//   + Directories: inode with special contents (list of other inodes!)
//   + Names: paths like /usr/rtm/xv6/fs.c for convenient naming.
//
// Disk layout is: superblock, inodes, block in-use bitmap, data blocks.

//Function prototypes
int checkSuperblock (struct superblock *);
void reportAndDie (int, char *);
int seekToBlock (int blockNum);
int getImageFd (char * filename);
int checkFreeMap ();
int getBitIndexFromAddr (int);
int getCharIndexFromAddr (int);
void printBitMap (unsigned char *);
void destroyInodeFromBlockAddr(int);
//Global data
int imageFd;
struct superblock mySuperblock;
unsigned char * theirFreeMap;
unsigned char * myFreeMap;
struct dinode * myInodes;
int numBlockChars; //the number of blocks (in units of bytes)

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

  //read in inodes and bitmap
  int inodeRegionStart  = 1 + 1; //garbage, superblock, inodes
  seekToBlock(inodeRegionStart);
  myInodes = malloc(mySuperblock.ninodes * sizeof(struct dinode)); //TODO free
  read(imageFd, myInodes, mySuperblock.ninodes * sizeof(struct dinode));
  //garbage, super, inodes, garbage, bitmap
  int bitMapRegionStart = 1 + 1 + (mySuperblock.ninodes / IPB) + 1;

  #if DEBUG
  printf("bit map starts at:%x\t%d\n", bitMapRegionStart, bitMapRegionStart);
  #endif

  seekToBlock(bitMapRegionStart); //seek to bitmap region
  //sizeof(char); //size of the freemap in char sizes (8 bits = 1 byte = 1 char)
  numBlockChars = (mySuperblock.nblocks/8) + (mySuperblock.nblocks % 8 != 0 ? 1 : 0);

  #if DEBUG
  printf("numBlockChars: %d\tBlock size: %d\n", numBlockChars, BSIZE);
  #endif
  theirFreeMap = calloc(numBlockChars, sizeof(unsigned char));
  myFreeMap = calloc(numBlockChars, sizeof(unsigned char)); //malloc and init to zero
  //TODO free
  read(imageFd, theirFreeMap, numBlockChars);

  #if DEBUG
  printf("their map\n");
  printBitMap(theirFreeMap);
  #endif

  reportAndDie(checkFreeMap(), "Freemap is not good");
  #if DEBUG
  printf("after ANDing\n");
  printBitMap(myFreeMap);
  #endif
  //write the corrected bitmap and corrected inodes
  //seekToBlock(INODES_BLOCK_NUMBER);
  //write(imageFd, myInodes, mySuperblock.ninodes * sizeof(struct dinode));

  //seekToBlock(bitMapRegionStart);
  //write(imageFd, theirFreeMap, numBlockChars);

  #if DEBUG
  printf("their size:%lu\tour size:%lu\n", sizeof(theirFreeMap[0]), sizeof(myFreeMap[0]));
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
  //XXX should these checks come beforehand?
  if (super->size % BSIZE != 0) {
    return INVALID;
  }

  if (super->size < super->ninodes || super->size < super->nblocks) {
    return INVALID;
  }

  return VALID;
}

void markAsBusyInMyFreeMap (int addr) {
  //SHIFTING
  int charInd = getCharIndexFromAddr(addr);
  char c = myFreeMap[charInd];
  int bitInd = getBitIndexFromAddr(addr);
  char bit = 0x80 >> bitInd;
  bit |= c;
  myFreeMap[charInd] = bit;
}

int getCharIndexFromAddr (int addr) {
  return addr/8;
}
int getBitIndexFromAddr (int addr) {
  return addr % 8;
}

void printInode (struct dinode * i) {
  printf("inode.size:%d\n\t.type:%d\n\t.nlinks:%d\n", i->size, i->type, i->nlink);
}

int checkFreeMap () {
  //we need to check that each inode correlates to a block

  int i;

  for (i = 1; i <= mySuperblock.ninodes; i++) {
    //printf("myInodes[%d]\n", i);
    //printInode(&myInodes[i]);
    if(myInodes[i].type > 0) {
      int j;
      for (j = 0; j < NDIRECT; j++) {
        int addr = myInodes[i].addrs[j];
        markAsBusyInMyFreeMap(addr);
      }
      //TODO then do indirect
      if (myInodes[i].addrs[NDIRECT] > 0) {
        int indirectBlock = myInodes[i].addrs[NDIRECT];
        int indirectBlockEntries [BSIZE / sizeof(int)];
        seekToBlock(indirectBlock);
        read(imageFd, indirectBlockEntries, BSIZE);
        for (j = 0; j < BSIZE / sizeof(int); j++) {
          if(indirectBlockEntries[j] > 0) {
            int indirectEntryAddr = indirectBlockEntries[j];
            markAsBusyInMyFreeMap(indirectEntryAddr);
          }
        }
      }
    }
    else {
      //TODO
    }
  }

  #if DEBUG
  printf("before ANDing\n");
  printBitMap(myFreeMap);
  #endif
  //now AND the bitmaps together
  for (i = 0; i < numBlockChars; i++) {
    //myFreeMap[i] ^= theirFreeMap[i];
    theirFreeMap[i] &= myFreeMap[i];
    myFreeMap[i] ^= theirFreeMap[i];
    if (myFreeMap[i]) {
      //search for corresponding inode and remove it
      char byte = myFreeMap[i];

      int n;
      for (n = 0; n < 8; n++) {
        //if a certain bit is set...
        if (byte & (0x80 >> n)) {
          destroyInodeFromBlockAddr(i*8 + n);
        }
      }
    }
  }

  #if DEBUG
  printf("finally...\n");
  printBitMap(theirFreeMap);
  #endif

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

void printBitMap (unsigned char * map) {
  #if DEBUG
  int i;
  for (i = 0; i < numBlockChars; i++) {
    printf("%x ", map[i]);
  }
  printf("\n");
  #endif
}

void destroyInodeFromBlockAddr(int addr) {
  int i,j;

  for (i = 0; i < mySuperblock.ninodes; i++) {
    for (j = 0; j < NDIRECT; j++) {
      if (myInodes[i].addrs[j] == addr) {
        myInodes[i].type = -1;
        return;
      }
    }
    //also indirect block TODO
    if (myInodes[i].addrs[NDIRECT] == addr) {
      myInodes[i].type = -1;
      return;
    }
    if (myInodes[i].addrs[NDIRECT] > 0) {
      int indirectBlock = myInodes[i].addrs[NDIRECT];
      int indirectBlockEntries [BSIZE / sizeof(int)];
      seekToBlock(indirectBlock);
      read(imageFd, indirectBlockEntries, BSIZE);
      for (j = 0; j < BSIZE / sizeof(int); j++) {
        if(indirectBlockEntries[j] == addr) {
          myInodes[i].type = -1;
          return;
        }
      }
    }
  }
}
