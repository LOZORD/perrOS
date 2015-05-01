#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "structdefs.h"

#define BLOCK_SIZE 4096
#define ROOT_DIR_INODE 0
#define INVALID_DIR_ENTRY_INODE -1
#define CMD_SIZE  4
#define PATH_SIZE 32

//Function prototypes
int isCat (char * c);
void runCat ();
int isLs  (char * c);
void runLs ();
//Global variables
int imageFd;
char imageName [PATH_SIZE + 1];
int inspecteeFd;
char inspecteeName [PATH_SIZE + 1];

int main (int argc, char ** argv) {
  if (argc != 4) {
    fprintf(stderr, "Usage:\nlfsreader [cat | ls] <[file | dir]pathname> <lfs image>\n");
    exit(EXIT_FAILURE);
  }

  //Get the command
  char * cmd = argv[1];

  //Get the name of the thing we are inspecting (either a file or directory)
  strncpy(inspecteeName, argv[2], PATH_SIZE);
  inspecteeFd = open(inspecteeName, O_RDONLY);

  //Open up the image file
  strncpy(imageName, argv[3], PATH_SIZE);
  imageFd = open(imageName, O_RDONLY);

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
