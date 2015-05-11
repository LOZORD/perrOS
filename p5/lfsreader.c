#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "structdefs.h"

#define BLOCK_SIZE 4096
#define ROOT_DIR_INODE 0
#define INVALID_DIR_ENTRY_INODE -1
#define CMD_SIZE  4
#define PATH_SIZE 32
#define DIRECTORY_SIZE BLOCK_SIZE/sizeof(dirEnt)

//Function prototypes
int isCat (char * c);
void runCat ();
int isLs  (char * c);
void runLs ();
void getINode (char *, int);
//Global variables
int imageFd;
char imageName [PATH_SIZE + 1];
int inspecteeInode;
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

  int rootINodeLoc = myCheckpointRegion.iMapPtr[0];
  lseek(imageFd, rootINodeLoc, SEEK_SET);
  read(imageFd, &rootINode, sizeof(inode));
  //TODO check type
  int rootDirLoc = rootINode.ptr[0];
  dirEnt * rootDir = malloc(rootINode.size);
  lseek(imageFd, rootDirLoc, SEEK_SET);
  read(imageFd, rootDir, rootINode.size);

  for(i = 0; i < rootINode.size/sizeof(dirEnt); i++) {
    //printf("rootInode[%d]\tname:%s\tinode:%d\n", i, rootDir[i].name, rootDir[i].inum);
    if (rootDir[i].name && rootDir[i].name[0]) {
      printf("\t\t\t%s\n", rootDir[i].name);
    }
  }

  /*
  //TODO find inode for inspecteeInode
  int rootIMapPtr = myCheckpointRegion.iMapPtr[ROOT_DIR_INODE];// + myCheckpointRegion.iMapPtr[1];

  //rootIMapPtr = myCheckpointRegion.iMapPtr[ROOT_DIR_INODE] + (void *)(&myCheckpointRegion);
  //

  lseek(imageFd, rootIMapPtr, SEEK_SET);
  read(imageFd, &rootIMap, sizeof(inodeMap));

  for(i = 0; i < 16; i++) {
    printf("root imap[%d]->%x\n", i, rootIMap.inodePtr[i]);
  }
  int rootINodeOffset = rootIMap.inodePtr[ROOT_DIR_INODE];
  lseek(imageFd, rootINodeOffset, SEEK_SET);
  read(imageFd, &rootINode, sizeof(inode));

  printf("\n\nROOT INODE\n\n");
  //XXX check that it is a directory, not a file!
  printf("\tsize:\t%d\n\ttype:\t%d\n", rootINode.size, rootINode.type);
  for(i = 0; i < 14; i++) {
    printf("\t\trootInode.ptr[%d]=%x\n", i, rootINode.ptr[i]);
  }

  int rootDirOffset = rootINode.ptr[0];
  lseek(imageFd, rootDirOffset, SEEK_SET);

  int rootDirSize = rootINode.size / sizeof(dirEnt);
  dirEnt * rootDirectory = malloc(rootINode.size/sizeof(dirEnt));

  for(i = 0; i < rootDirSize; i++) {
    printf("rootDirectory[%d]\t = %s\t%x\n", i, rootDirectory[i].name, rootDirectory[i].inum);
  }

  //TODO

  getINode(inspecteeName, rootIMap.inodePtr[0] ///rootIMapPtr/);
  //getINode(inspecteeName, rootIMap.inodePtr[1]);
  //getINode(inspecteeName, rootIMap.inodePtr[2]);
  */

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
}

void getINode (char * name, int inodeLoc) {
  printf("Searching for %s using inode offset %x\n", name, inodeLoc);
  //inode * myInode = NULL;
  inode myInode;
  dirEnt myDirectory [DIRECTORY_SIZE];
  lseek(imageFd, inodeLoc , SEEK_SET);
  read(imageFd, &myInode, sizeof(inode));
  int directoryLoc = myInode.ptr[0];
  lseek(imageFd, directoryLoc, SEEK_SET);
  read(imageFd, &myDirectory, DIRECTORY_SIZE);
  //TODO verify type
  //TODO load block into array of dirEnts
  //TODO check that inode is valid (not -1)
  int i;
  for (i = 0; i < DIRECTORY_SIZE; i++) {
    printf("directory[%d]=\t%s\n", i, myDirectory[i].name);
  }
  //TODO trim string
  //TODO recurse to get inode number using CR iMapPtr
}
