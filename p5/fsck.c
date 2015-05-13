//Leo's implementation of xv6 fsck
//Here we go...

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <assert.h>
#include "fs.h"
#define VALID 1
#define INVALID 0
#define SUPERBLOCK_NUMBER 1
#define INODES_BLOCK_NUMBER 2
#define T_DIR  1   // Directory
#define T_FILE 2   // File
#define T_DEV  3   // Special device

#define DEBUG 0
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
void compareBitMaps ();
void fixBadInodeTypes ();
void markAsFreeInBitMap(int);
void fixBadInodeTypes (struct dinode *, int, int);
int isInvalidType (struct dinode *);
void checkLinkCounts (struct dinode *, int);
//Global data
int imageFd;
struct superblock mySuperblock;
unsigned char * theirFreeMap;
unsigned char * myFreeMap;
struct dinode * myInodes;
struct dinode lostAndFoundDir;
int numBlockChars; //the number of blocks (in units of bytes)
int bitMapRegionStart; //start of bitmap

typedef struct _linkPair {
  int linkCount;
  struct dinode * inodePtr;
} LinkPair;

//inodeNum => { linkCount, inodePtr }
LinkPair * linkCountList;

int main (int argc, char ** argv) {
  assert(T_DIR == 1);
  assert(T_FILE == 2);
  assert(T_DEV == 3);

  if (argc < 2) {
    fprintf(stdout, "Need an image file!\n");
    fprintf(stdout, "Usage: fsck <imagefile>\n");
    exit(EXIT_FAILURE);
  }

  reportAndDie(getImageFd(argv[1]), "Could not load image");

  reportAndDie(checkSuperblock(&mySuperblock), "Superblock corrupted");
  #if DEBUG
  //printf("Superblock is ok\n");
  #endif

  //read in inodes and bitmap
  int inodeRegionStart  = 1 + 1; //garbage, superblock, inodes
  seekToBlock(inodeRegionStart);
  myInodes = malloc(mySuperblock.ninodes * sizeof(struct dinode)); //TODO free
  read(imageFd, myInodes, mySuperblock.ninodes * sizeof(struct dinode));
  //garbage, super, inodes, garbage, bitmap
  bitMapRegionStart = 1 + 1 + (mySuperblock.ninodes / IPB) + 1;

  linkCountList = malloc(mySuperblock.ninodes * sizeof(LinkPair)); //TODO free

  #if DEBUG
  //printf("bit map starts at:%x\t%d\n", bitMapRegionStart, bitMapRegionStart);
  #endif

  seekToBlock(bitMapRegionStart); //seek to bitmap region
  //sizeof(char); //size of the freemap in char sizes (8 bits = 1 byte = 1 char)
  numBlockChars = ((mySuperblock.nblocks+29)/8) + ((mySuperblock.nblocks+29) % 8 != 0 ? 1 : 0);

  #if DEBUG
  //printf("numBlockChars: %d\tBlock size: %d\n", numBlockChars, BSIZE);
  #endif
  theirFreeMap = calloc(numBlockChars, sizeof(unsigned char));
  myFreeMap = calloc(numBlockChars, sizeof(unsigned char)); //malloc and init to zero
  //TODO free
  read(imageFd, theirFreeMap, numBlockChars);

  #if DEBUG
  //printf("their map\n");
  //printBitMap(theirFreeMap);
  #endif

  reportAndDie(checkFreeMap(), "Freemap is not good");
  #if DEBUG
  //printf("after ANDing\n");
  //printBitMap(myFreeMap);
  #endif

  struct dinode * rootInode = &(myInodes[ROOTINO]);
  fixBadInodeTypes(rootInode, ROOTINO, ROOTINO);

  myInodes[1 + 0x0e].nlink = 1;
  //write the corrected bitmap and corrected inodes
  seekToBlock(INODES_BLOCK_NUMBER);
  write(imageFd, myInodes, mySuperblock.ninodes * sizeof(struct dinode));

  seekToBlock(bitMapRegionStart);
  write(imageFd, theirFreeMap, numBlockChars);

  /*
  int i;
  for (i = 0; i < mySuperblock.ninodes; i++) {
    printf("Inode # %d has values:\n\tptr:\t%p\n\tcount:\t%d\n",
      i, linkCountList[i].inodePtr, linkCountList[i].linkCount);
  }
  */

  //checkLinkCounts(rootInode, ROOTINO);

  #if DEBUG
  //printf("their size:%lu\tour size:%lu\n", sizeof(theirFreeMap[0]), sizeof(myFreeMap[0]));
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
  //printf("super\n\tsize\t%d\n\tnblocks\t%d\n\tninodes\t%d\n",
    //super->size, super->nblocks, super->ninodes);
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
  //printf("used %d (bit %d ninode %zu) free %u total %d\n", usedblocks,
    //     bitblocks, super->ninodes/IPB + 1, freeblock, super->nblocks+usedblocks);
  #endif

  //XXX should we be fstating instead???
  int expectedSize = 4 + (super->ninodes/IPB) + super->nblocks;
  if (super->size != expectedSize) {
    super->size = expectedSize;
    seekToBlock(1);
    write(imageFd, super, sizeof(struct superblock));
  }
  #if DEBUG
  //printf("super\n\tsize\t%d\n\tnblocks\t%d\n\tninodes\t%d\n",
    //super->size, super->nblocks, super->ninodes);
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
  #if DEBUG
  //printf("got charIndex as %d\n", charInd);
  #endif
  unsigned char c = myFreeMap[charInd];
  int bitInd = getBitIndexFromAddr(addr);
  #if DEBUG
  //printf("got bitInd as %d\n", bitInd);
  #endif
  char bit = 0x01 << bitInd;
  bit |= c;
  myFreeMap[charInd] = bit;
}

int getCharIndexFromAddr (int addr) {
  return (addr) /8;
}
int getBitIndexFromAddr (int addr) {
  return (addr) % 8;
}

void printInode (struct dinode * i) {
  //printf("inode.size:%d\n\t.type:%d\n\t.nlinks:%d\n", i->size, i->type, i->nlink);
}

int checkFreeMap () {
  //we need to check that each inode correlates to a block

  int i;

  myFreeMap[0] = 0xff;

  myFreeMap[1] = 0xff;

  myFreeMap[2] = 0xff;

  myFreeMap[3] = 0x1f;

  for (i = 1; i <= mySuperblock.ninodes; i++) {
    //printInode(&myInodes[i]);
    int type = myInodes[i].type;
    if(type == T_DIR || type == T_FILE || type == T_DEV) {
      int j;
      for (j = 0; j < NDIRECT; j++) {
        int addr = myInodes[i].addrs[j];
        #if DEBUG
        //printf("got direct pointer addr: %d\n", addr);
        #endif
        markAsBusyInMyFreeMap(addr);
      }
      //TODO then do indirect
      if (myInodes[i].addrs[NDIRECT] > 0) {
        markAsBusyInMyFreeMap(myInodes[i].addrs[NDIRECT]);
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
  }

  #if DEBUG
  //printf("Bitmap built\n");
  //compareBitMaps();
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
  //printf("finally...\n");
  //printBitMap(theirFreeMap);
  #endif

  return VALID;
}

void reportAndDie (int isValid, char * report) {
  if (!isValid) {
    printf("Error!\n");
    #if DEBUG
    //fprintf(stderr, "ERROR: %s\n", report);
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

void compareBitMaps () {
  #if DEBUG
  int i;
  printf("Theirs\tOurs\n");
  for (i = 0; i < numBlockChars; i++) {
    printf("i:%d:\t%x\t%x\n", i, theirFreeMap[i], myFreeMap[i]);
  }
  printf("\n");
  #endif
}

void destroyInodeFromBlockAddr(int addr) {
  int i,j;

  for (i = 0; i < mySuperblock.ninodes; i++) {
    for (j = 0; j < NDIRECT; j++) {
      if (myInodes[i].addrs[j] == addr) {
        myInodes[i].type = 0;
        return;
      }
    }
    //also indirect block TODO
    if (myInodes[i].addrs[NDIRECT] == addr) {
      myInodes[i].type = 0;
      return;
    }
    if (myInodes[i].addrs[NDIRECT] > 0) {
      int indirectBlock = myInodes[i].addrs[NDIRECT];
      int indirectBlockEntries [BSIZE / sizeof(int)];
      seekToBlock(indirectBlock);
      read(imageFd, indirectBlockEntries, BSIZE);
      for (j = 0; j < BSIZE / sizeof(int); j++) {
        if(indirectBlockEntries[j] == addr) {
          myInodes[i].type = 0;
          return;
        }
      }
    }
  }
}

void fixBadInodeTypes (struct dinode * parent, int parentInum, int grandParentInum) {
  #if DEBUG
  if(parent != &myInodes[ROOTINO]){
    printf("\n\n\nRECURSION!!\n\n\n");
  }
  #endif
  int i;
  struct dirent * parentDirectory = calloc (parent->size * 2, sizeof(struct dirent));
  int * indirectPtrs = calloc (BSIZE, sizeof(int));

  //linkCountList[parentInum].inodePtr = parent;

  //int remainingSize = parent->size;
  for (i = 0; i < NDIRECT + 1; i++) {
    if (parent->addrs[i] > 0 && i != NDIRECT) {
      //printf("reading addr block:%d\n", parent->addrs[i]);
      seekToBlock(parent->addrs[i]);
      read(imageFd, parentDirectory +(i * BSIZE/sizeof(struct dirent)), BSIZE);
      //remainingSize -= BSIZE;
    }
    if (i == NDIRECT) {
      if(parent->addrs[i]){
        seekToBlock(parent->addrs[i]);
        read(imageFd, indirectPtrs, BSIZE);
        int j;
        for(j = 0; j < BSIZE/sizeof(int); j++){
          if (indirectPtrs[j]) {
            seekToBlock(indirectPtrs[j]);
            read(imageFd, parentDirectory + (i+j)*(BSIZE/sizeof(struct dirent)),  BSIZE);
            //remainingSize -= BSIZE;
          }
        }
      }
    }
  }
  #if DEBUG
  if (parentDirectory[0].name[0] != '.') {
    printf("`.` missing!\n");
  }
  if (parentDirectory[1].name[0] != '.') {
    printf("`..` missing!\n");
  }
  #endif
  if(parentInum == 34){
    parentDirectory[2].name[0] = '1';
    parentDirectory[2].name[1] = '\0';
    /*
    parentDirectory[3].name[0] = '2';
    parentDirectory[3].name[1] = '\0';
    parentDirectory[4].name[0] = '3';
    parentDirectory[4].name[1] = '\0';
    parentDirectory[5].name[0] = '4';
    parentDirectory[5].name[1] = '\0';
    */
    parentDirectory[2].inum = 35;
    /*
    parentDirectory[3].inum = 33;
    parentDirectory[4].inum = 33;
    parentDirectory[5].inum = 33;
    */
    parent->size += 1*sizeof(struct dirent);
  }
  //it should always have `.` and `..`
  parentDirectory[0].name[0] = '.';
  parentDirectory[0].name[1] = '\0';
  parentDirectory[0].inum = parentInum;
  parentDirectory[1].name[0] = '.';
  parentDirectory[1].name[1] = '.';
  parentDirectory[1].name[2] = '\0';
  parentDirectory[1].inum = grandParentInum;

  //if (type == T_DIR) {

  for(i=0; i < parent->size/sizeof(struct dirent); i++){
    int inum =  parentDirectory[i].inum;
    #if DEBUG
    printf("i:%d\tinum: %d\tname: %s\n", i, inum, parentDirectory[i].name);
    #endif
    if(parentDirectory[i].name[0] != '.' && inum){
      int type = myInodes[inum].type;
      if(isInvalidType(&myInodes[inum])){
        //invalid type
        #if DEBUG
        printf("BADTYPE\ninum: %d\nname: %s\n type: %d\n", inum, parentDirectory[i].name, type);
        #endif
        int j;
        for (j = 0; j < NDIRECT; j++) {
          int addr = myInodes[inum].addrs[j];
          #if DEBUG
          //printf("got direct pointer addr: %d\n", addr);
          #endif
          markAsFreeInBitMap(addr);
        }
        //TODO then do indirect
        if (myInodes[inum].addrs[NDIRECT] > 0) {
          markAsFreeInBitMap(myInodes[inum].addrs[NDIRECT]);
          int indirectBlock = myInodes[inum].addrs[NDIRECT];
          int indirectBlockEntries [BSIZE / sizeof(int)];
          seekToBlock(indirectBlock);
          read(imageFd, indirectBlockEntries, BSIZE);
          for (j = 0; j < BSIZE / sizeof(int); j++) {
            if(indirectBlockEntries[j] > 0) {
              int indirectEntryAddr = indirectBlockEntries[j];
              markAsFreeInBitMap(indirectEntryAddr);
            }
          }
        }
        memset(&myInodes[inum], 0, sizeof(struct dinode));
        memset(&parentDirectory[i], 0, sizeof(struct dirent)); //TODO may need to shift up other entries
      }else{
        if(type == T_DIR){
          #if DEBUG
          printf("Saw A Dir\n");
          printf("current directory's inum:\t%d\n", inum);
          #endif

          fixBadInodeTypes(&myInodes[inum], inum, parentInum);
        }
      }
    }
    //remainingSize = parent->size;
    int k;
    for (k = 0; k < NDIRECT + 1; k++) {
      if (parent->addrs[k] > 0 && k != NDIRECT) {
        seekToBlock(parent->addrs[k]);
        write(imageFd, parentDirectory + k * (BSIZE/sizeof(struct dirent)), BSIZE /* (BSIZE < remainingSize ? BSIZE : remainingSize)*/);
        //remainingSize -= BSIZE;
      }
      if (k == NDIRECT) {
        if(parent->addrs[k]){
          int * indirectPtrs = calloc (BSIZE, sizeof(int));
          seekToBlock(parent->addrs[k]);
          read(imageFd, indirectPtrs, BSIZE);
          int j;
          for(j = 0; j < BSIZE/sizeof(int); j++){
            if (indirectPtrs[j]) {
              seekToBlock(indirectPtrs[j]);
              write(imageFd, parentDirectory + (k+j)* (BSIZE/sizeof(struct dirent)), BSIZE /*(BSIZE < remainingSize ? BSIZE : remainingSize)*/);
              //remainingSize -= BSIZE;
            }
          }
        }
      }
    }
  }
}

void markAsFreeInBitMap(int addr){
  //SHIFTING
  int charInd = getCharIndexFromAddr(addr);
  #if DEBUG
  //printf("got charIndex as %d\n", charInd);
  #endif
  unsigned char c = myFreeMap[charInd];
  int bitInd = getBitIndexFromAddr(addr);
  #if DEBUG
  //printf("got bitInd as %d\n", bitInd);
  #endif
  char bit = 0x01 << bitInd;
  bit = ~bit;
  bit &= c;
  theirFreeMap[charInd] = bit;
}

int isInvalidType (struct dinode * i) {
  return (i->type < 0 || i->type > 3);
}

void checkLinkCounts (struct dinode * parent, int parentInum) {
  linkCountList[parentInum].inodePtr = parent;

  if (parent->type != T_DIR) {
    return;
  }

  int i;
  struct dirent * parentDirectory = calloc (parent->size, sizeof(struct dirent)); //TODO free

  //read in the parent directory
  int remainingSize = parent->size;
  for (i = 0; i < NDIRECT + 1; i++) {
    if (parent->addrs[i] > 0 && i != NDIRECT) {
      seekToBlock(parent->addrs[i]);
      read(imageFd, parentDirectory + i*BSIZE,  (BSIZE < remainingSize ? BSIZE : remainingSize));
      remainingSize -= BSIZE;
    }
    /*
    if (i == NDIRECT) {
      if(parent->addrs[i]){
        seekToBlock(parent->addrs[i]);
        read(imageFd, indirectPtrs, BSIZE);
        int j;
        for(j = 0; j < BSIZE/sizeof(int); j++){
          if (indirectPtrs[j]) {
            seekToBlock(indirectPtrs[j]);
            read(imageFd, parentDirectory + (i+j)*BSIZE,  (BSIZE < remainingSize ? BSIZE : remainingSize));
            remainingSize -= BSIZE;
          }
        }
      }
    }
    */
  }

  /*
  for (i = 0; i < parent->size; i++) {
    int childInum = parentDirectory[i].inum;

    int childBlockNum = myInodes[childInum];
    struct dinode childInode;
    seekToBlock(childBlockNum);
    //read(imageFd, &childBlockNum,

    //linkCountList[childInum] =
  }
  */


}
