// mytee - Rudberg and Hyatt
#include <stdio.h>
#include <string.h>
#define MAX_LENGTH 1025

extern FILE * stdin;
extern FILE * stdout;
extern FILE * stderr;

//what we will use to store incoming data
char buff [MAX_LENGTH];

int main( int argc, char ** argv) {

  int len;

  //attempt the tee.txt file for writing
  FILE * teeFile = fopen("tee.txt", "w");

  if (teeFile == NULL) {
    fprintf(stderr, "Error!\n");
    return 1; //Failure
  }

  //read from standard in and write to standard out and the teeFile
  while(fgets(buff, MAX_LENGTH, stdin)) {

    len = strlen(buff);
    len = len < (MAX_LENGTH - 1) ? len : MAX_LENGTH;

    fwrite(buff, sizeof(char), len, teeFile);
    fwrite(buff, sizeof(char), len, stdout);
    //empty the buffer
    memset(buff, '\0', MAX_LENGTH);
  }

  fclose(teeFile);

  return 0; //Success
}
