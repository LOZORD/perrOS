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

#define DEBUG 0

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
char * imageName;
int inspecteeINodeNum;
char * inspecteeName;
checkpoint myCheckpointRegion;
inodeMap rootIMap;
inode rootINode;

int main (int argc, char ** argv) {
  if (argc != 4) {
    fprintf(stdout, "Usage:\nlfsreader [cat | ls] <[file | dir]pathname> <lfs image>\n");
    exit(EXIT_FAILURE);
  }

  //Get the command
  char * cmd = argv[1];

  int inspecteeNameSize, imageNameSize;

  inspecteeNameSize = strlen(argv[2]);
  imageNameSize = strlen(argv[3]);

  inspecteeName = malloc(inspecteeNameSize + 1);
  imageName = malloc(imageNameSize + 1);
  //Get the name of the thing we are inspecting (either a file or directory)
  strncpy(inspecteeName, argv[2], inspecteeNameSize);

  //Open up the image file
  strncpy(imageName, argv[3], imageNameSize);
  imageFd = open(imageName, O_RDONLY);

  //Use the superblock/checkpoint region to get the inode number
  read(imageFd, &myCheckpointRegion, sizeof(checkpoint));

  #if DEBUG
  printf("CR: size=%d\n", myCheckpointRegion.size);
  #endif

  #if DEBUG
  int i;
  for (i = 0; i < INODEPIECES; i++) {
    printf("\tiMapPtr[%d]\t%x\n", i, myCheckpointRegion.iMapPtr[i]);
  }
  #endif

  getINode(inspecteeName[0] == '/' ? inspecteeName + 1 : inspecteeName,
      ROOT_DIR_INODE_NUM);

  if (isCat(cmd)) {
    runCat();
  }
  else if (isLs(cmd)) {
    runLs();
  }
  else {
    fprintf(stdout, "Unknown command\n");
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
  #if DEBUG
  printf("You've entered runCat!\n");
  printf("We have the file %s @ inode %d\n", inspecteeName, inspecteeINodeNum);
  #endif

  //we got a directory or a nonexistant file
  if (inspecteeINodeNum < 0) {
    printf("Error!\n");
    exit(EXIT_FAILURE);
  }

  inode inspecteeINode;

  getInspecteeINode(&inspecteeINode);

  //we should only be cat-ing a file!
  assert(inspecteeINode.type == MFS_REGULAR_FILE);
  assert(inspecteeINode.size >= 0);

  #if DEBUG
  printf("inode size:\t%d\n", inspecteeINode.size);
  #endif

  int i = 0;
  char * buff = malloc(inspecteeINode.size + 1);

  for (i = 0; i < NUM_PTRS_IN_INODE; i++) {
    if (inspecteeINode.ptr[i]) {
      lseek(imageFd, inspecteeINode.ptr[i], SEEK_SET);
      read(imageFd, buff + i * BLOCK_SIZE, BLOCK_SIZE);
    }
  }

  buff[inspecteeINode.size] = EOF;
  i = 0;
  while(buff[i] != EOF) {
    putchar(buff[i++]);
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
  #if DEBUG
  printf("You've entered runLs!\n");
  printf("We have the directory %s @ inode %d\n", inspecteeName, inspecteeINodeNum);
  #endif
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
  #if DEBUG
  printf("Searching for %s using inode offset %x\n", name, inodeNum);
  #endif

  inodeMap currINodeMap;
  inode currINode;

  int currIMapPtr = getIMapPtr(inodeNum);

  lseek(imageFd, currIMapPtr, SEEK_SET);
  read(imageFd, &currINodeMap, sizeof(inodeMap));

  int currINodePtr = getINodePtr(&currINodeMap, inodeNum);

  #if DEBUG
  printf("Got currIMapPtr as %d\n", currINodePtr);
  #endif

  lseek(imageFd, currINodePtr, SEEK_SET);
  read(imageFd, &currINode, sizeof(inode));

  #if DEBUG
  printf("Got currINode!\n\tcurrINode.size=\t%d\n\tcurrINode.type=\t%d\n",
    currINode.size, currINode.type);
  #endif

  int i, isParentDirectory = 0;
  char newDirPath [PATH_SIZE];

  i = 0;
  #if DEBUG
  printf("name\t%s'/':%d\t'0':%d\n", name, name[0] == '/', name[0] == '\0');
  #endif
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

  #if DEBUG
  printf("\t\tGOT NEW DIR PATH AS %s\n", newDirPath);
  #endif

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
          #if DEBUG
          printf("\t\t%s\n", currDirectory[j].name);
          #endif
          if (strcmp(currDirectory[j].name, newDirPath) == 0) {
            #if DEBUG
            printf("Got match! INode num is %d\n", currDirectory[j].inum);
            #endif
            newINodeNum = currDirectory[j].inum;
            break; //any reason to continue on?
          }
        }
      }
    }
  }

  if (isParentDirectory) {
    #if DEBUG
    printf("recursing...\n");
    #endif
    getINode(name + newSearchStrIndex, newINodeNum);
  }
  else {
    inspecteeINodeNum = newINodeNum;
  }

  return;
}
