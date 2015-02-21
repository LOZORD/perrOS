#include <stdio.h>
#include <string.h>
#define MAX_LENGTH 1025

extern FILE * stdin;
extern FILE * stdout;
extern FILE * stderr;
char buff [MAX_LENGTH];

int main( int argc, char ** argv){
  FILE * teeFile = fopen("tee.txt", "w");

  while(fgets(buff, MAX_LENGTH, stdin)) {
    fwrite(buff, sizeof(char), MAX_LENGTH, teeFile);
    fwrite(buff, sizeof(char), MAX_LENGTH, stdout);
    memset(buff, '\0', MAX_LENGTH);
  }

  return 0; //Success
}
