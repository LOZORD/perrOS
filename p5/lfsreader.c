#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "structdefs.h"
#include "assert.h" //TODO: replace with die function

#define BLOCK_SIZE 4096
#define ROOT_DIR_INODE_NUM 0
#define INVALID_DIR_ENTRY_INODE -1
#define CMD_SIZE  4
#define PATH_SIZE 64
#define INODE_NUM_DENOM 16
#define DIRECTORY_SIZE BLOCK_SIZE/sizeof(dirEnt)

//Function prototypes
int isCat (char * c);
void runCat ();
int isLs  (char * c);
void runLs ();
void getINode (char *, int);
int getIMapPtr (int);
int getINodePtr (inodeMap *, int);
//Global variables
int imageFd;
char imageName [PATH_SIZE + 1];
int inspecteeINodeNum;
char inspecteeName [PATH_SIZE + 1];
checkpoint myCheckpointRegion;
inodeMap rootIMap;
inode rootINode;

int main (int argc, char ** argv) {
  if (argc != 4) {
    fprintf(stderr, "Usage:\nlfsreader [cat | ls] <[file | dir]pathname> <lfs image>\n");
    exit(EXIT_FAILURE);
  }

  //Get the command
  char * cmd = argv[1];

  //Get the name of the thing we are inspecting (either a file or directory)
  strncpy(inspecteeName, argv[2], PATH_SIZE);
  //inspecteeFd = open(inspecteeName, O_RDONLY);

  //Open up the image file
  strncpy(imageName, argv[3], PATH_SIZE);
  imageFd = open(imageName, O_RDONLY);

  //Use the superblock/checkpoint region to get the inode number
  read(imageFd, &myCheckpointRegion, sizeof(checkpoint));

  printf("CR: size=%d\n", myCheckpointRegion.size);

  int i;

  for (i = 0; i < INODEPIECES; i++) {
    printf("\tiMapPtr[%d]\t%x\n", i, myCheckpointRegion.iMapPtr[i]);
  }

  getINode(inspecteeName[0] == '/' ? inspecteeName + 1 : inspecteeName,
    ROOT_DIR_INODE_NUM);

  if (isCat(cmd)) {
    runCat();
  }
  else if (isLs(cmd)) {
    runLs();
  }
  else {
    fprintf(stderr, "Unknown command\n");
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}

void getInspecteeINode (inode * inspecteeINode) {
  int inspecteeIMapPtr = getIMapPtr(inspecteeINodeNum);
  inodeMap inspecteeIMap;

  lseek(imageFd, inspecteeIMapPtr, SEEK_SET);
  read(imageFd, &inspecteeIMap, sizeof(inodeMap));

  int inspecteeINodePtr = getINodePtr(&inspecteeIMap, inspecteeINodeNum);

  lseek(imageFd, inspecteeINodePtr, SEEK_SET);
  read(imageFd, inspecteeINode, sizeof(inode));
}

int isCat (char * c) {
  return (
    c[0] == 'c' &&
    c[1] == 'a' &&
    c[2] == 't' &&
    c[3] == '\0'
  );
}

void runCat () {
  printf("You've entered runCat!\n");

  printf("We have the file %s @ inode %d\n", inspecteeName, inspecteeINodeNum);

  inode inspecteeINode;

  getInspecteeINode(&inspecteeINode);

  //we should only be cat-ing a file!
  assert(inspecteeINode.type == MFS_REGULAR_FILE);

  int i;
  char buff [BLOCK_SIZE];
  //printf("FOO: %d\n", inspecteeINode.size/BLOCK_SIZE);
  //TODO how do we know size of file???
  for (i = 0; i < inspecteeINode.size; i++) {
    //XXX indirect pointers?
    //printf("hello\n");
    lseek(imageFd, inspecteeINode.ptr[i], SEEK_SET);
    read(imageFd, buff, BLOCK_SIZE);

    printf("%s", buff); //FIXME: how to print only good data?

    /*
    int j;
    while(buff[j] != EOF && j < BLOCK_SIZE) {
      putchar(buff[j]);
      j++;
    }
    */
  }
}

int isLs (char * c) {
  return (
    c[0] == 'l' &&
    c[1] == 's' &&
    c[2] == '\0'
  );
}

void runLs () {
  printf("You've entered runLs!\n");

  printf("We have the directory %s @ inode %d\n", inspecteeName, inspecteeINodeNum);
}

int calcIMapPtr (int inodeNum) {
  return inodeNum / 16;
}

int calcINodePtr (int inodeNum) {
  return inodeNum % 16;
}

int getIMapPtr (int inodeNum) {
  int index = calcIMapPtr(inodeNum);
  return myCheckpointRegion.iMapPtr[index];
}

int getINodePtr (inodeMap * i, int inodeNum) {
  int index = calcINodePtr(inodeNum);
  return i->inodePtr[index];
}

void getINode (char * name, int inodeNum) {
  printf("Searching for %s using inode offset %x\n", name, inodeNum);

  inodeMap currINodeMap;
  inode currINode;

  int currIMapPtr = getIMapPtr(inodeNum);

  lseek(imageFd, currIMapPtr, SEEK_SET);
  read(imageFd, &currINodeMap, sizeof(inodeMap));

  int currINodePtr = getINodePtr(&currINodeMap, inodeNum);

  printf("Got currIMapPtr as %d\n", currINodePtr);

  lseek(imageFd, currINodePtr, SEEK_SET);
  read(imageFd, &currINode, sizeof(inode));

  printf("Got currINode!\n\tcurrINode.size=\t%d\n\tcurrINode.type=\t%d\n", currINode.size, currINode.type);

  int i, isParentDirectory = 0;
  char newDirPath [PATH_SIZE];

  i = 0;
  //printf("name\t%s'/':%d\t'0':%d\n", name, name[0] == '/', name[0] == '\0');
  while (name[i] != '/' && name[i] != '\0') {
    newDirPath[i] = name[i];
    i++;
  }
  if (name[i] == '/') {
    isParentDirectory = 1; //it's a directory
  }
  else {
    isParentDirectory = 0; //it's a file
  }
  newDirPath[i] = '\0';
  int newSearchStrIndex = i + 1;

  printf("\t\tGOT NEW DIR PATH AS %s\n", newDirPath);

  //just assume we are going through directories
  dirEnt currDirectory [DIRECTORY_SIZE];
  int newINodeNum = -1234; //TODO this is getting set incorrectly when `ls` is ran...
  //THIS MEANS THAT WE ARE NEVER FINDING A SUB-DIR...

  //currDirectory = (dirEnt)(malloc(currINode.size / sizeof(dirEnt)));
  //int blockIndex = 0;
  for (i = 0; i < NUM_PTRS_IN_INODE; i++) {
    if(currINode.ptr[i] > 0) {
      int blockPtr = currINode.ptr[i];
      lseek(imageFd, blockPtr, SEEK_SET);
      read(imageFd, currDirectory, BLOCK_SIZE);
      int j;
      for (j = 0; j < DIRECTORY_SIZE; j++) {
        if (currDirectory[j].name && currDirectory[j].inum >= 0) {
          printf("\t\t%s\n", currDirectory[j].name);
          if (strcmp(currDirectory[j].name, newDirPath) == 0) {
            printf("Got match! INode num is %d\n", currDirectory[j].inum);
            newINodeNum = currDirectory[j].inum;
            break; //any reason to continue on?
          }
        }
      }
    }
  }

  if (isParentDirectory) {
    printf("recursing...\n");
    getINode(name + newSearchStrIndex, newINodeNum);
  }
  else {
    inspecteeINodeNum = newINodeNum;
  }

  return;
}
