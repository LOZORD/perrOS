//mysh, by Leo Rudberg and Nicholas Hyatt

#include <stdlib.h>
#include <stdio.h>
#include "mysh.h"

size_t MAX_INPUT_LENGTH = 1024;

extern FILE * stdin;
extern FILE * stdout;
extern FILE * stderr;

int getcmd (FILE * file, char * buff) {
  if (file == NULL || buff == NULL) {
    return -1;
  }

  int result;

  printf("mysh> ");
  result = getline(&buff, &MAX_INPUT_LENGTH, file);

  return result;
}


int main (int argc, char ** argv) {
  printf("Hello and welcome to mysh!\n");
  //TODO: open tee.txt or whatever
  //

  char buff [MAX_INPUT_LENGTH + 1];

  while (getcmd(stdin, buff) >= 0) {
    printf("mysh got: %s\n", buff);
  }


  exit(EXIT_SUCCESS);
}
