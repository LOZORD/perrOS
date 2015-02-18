//mysh, by Leo Rudberg and Nicholas Hyatt

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

size_t MAX_INPUT_LENGTH = 1024;
char * TOKEN_DELIMS = " |>%\t";

#define REGULAR 0
#define PIPE 1
#define TEE 2
#define O_REDIR 3
#define A_REDIR 4

//char * SPECIAL_DELIMS = "";

extern FILE * stdin;
extern FILE * stdout;
extern FILE * stderr;

char * buffItr;

//what should be done after this command
//so for a | b, a would be PIPE, b would be normal
//


typedef enum { REGULAR_CMD, PIPE_CMD, TEE_CMD, O_REDIR_CMD, A_REDIR_CMD } commandType;

//THE ABSTRACT PARENT -- structs below from xv6/user/sh.c
/*
struct cmd {
  commandType type;
};

struct execcmd {
  commandType type;
  char *argv[MAXARGS];
  char *eargv[MAXARGS];
};

struct redircmd {
  commandType type;
  struct cmd *cmd;
  char *file;
  char *efile;
  int mode;
  int fd;
};

struct pipecmd {
  commandType type;
  struct cmd *left;
  struct cmd *right;
};

struct listcmd {
  commandType type;
  struct cmd *left;
  struct cmd *right;
};

struct backcmd {
  commandType type;
  struct cmd *cmd;
};
*/

typedef struct my_cmd {
  char ** args;
  commandType inputType;
  commandType outputType;
} Command;

Command * commandList;

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
  char * token = NULL;
  int buffSize;
  //char ** tokenList;

  while ((buffSize = getcmd(stdin, buff)) >= 0) {
    printf("mysh got: %s", buff);
    buff[buffSize - 1] = '\0'; //change the term char from \n to \0

    char * foo = buff;

    for (buffItr = buff; (*buffItr) != '\0'; buffItr++) {
      token = buffItr;
      if (token == NULL) {
        break;
      }
      switch(*buffItr) {
        case '|':
          //TODO
          token = strtok(foo, "|");
          foo = NULL;
          printf("saw pipe, got token: %s\n", token);
          break;
        case '%':
          //TODO
          token = strtok(foo, "%");
          printf("saw tee, got token: %s\n", token);
          foo = NULL;
          break;
        case '>':
          if (buffItr[1] == '>') { //APPEND REDIRECTION
            buffItr++;
            token = strtok(foo, ">>");
            printf("saw app, got token: %s\n", token);
            foo = NULL;
            //TODO append
          }
          else { //OVERWRITE REDIRECTION
            token = strtok(foo, ">");
            printf("saw ovr, got token: %s\n", token);
            foo = NULL;
            //TODO ovr
          }
          break;
        default:
          printf("just a regular char '%c'\n", *buffItr);
          continue;
      }
    }
    printf("Last token is %s\n", token);
    //first find all of the pipes
  }

  perror("ERROR: Problem with mysh input!\n");

  exit(EXIT_SUCCESS);
}


/*
void * getNextCommand (char * buff) {
  char * spacePtr, * pipePtr, * ovrRedirPtr, * appRedirPtr, * teePtr;

  char * smallestPtr;

  char ** ptrArray = {
    [REGULAR] = NULL,
    [PIPE]    = strchr(buff, '|'),
    [A_REDIR] = strstr(buff, ">>"),
    [O_REDIR] = strchr(buff, '>'),
    [TEE]     = strchr(buff, '%'),
  };

  int i, type;

  smallestPtr = ptrArray[1];
  type = PIPE;

  for (i = 2; i < 5; i++) {
    if (ptrArray[i] != NULL && smallestPtr > ptrArray[i]) {
      smallestPtr = ptrArray[i];
      type = i;
    }
  }

  type = smallestPtr == NULL ? REGULAR : type;

  void * someCommand = malloc(sizeof(Command));

  switch (type) {
    case REGULAR:
      someCommand->inputType = REGULAR;
      someCommand->argv
}
*/
