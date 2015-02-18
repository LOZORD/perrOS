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


typedef struct arg_node {
  char * arg;
  struct arg_node * next;
} ArgNode;

typedef struct arg_list {
  int size;
  ArgNode * head;
  ArgNode * tail;
} ArgList;

typedef struct my_cmd {
  ArgList * argList;
  commandType inputType;
  commandType outputType;
} Command;

typedef struct cmd_nod {
  Command * command;
  struct cmd_nod * next;
} CommandNode;

typedef struct cmd_list {
  int size;
  CommandNode * head;
  CommandNode * tail;
} CommandList;

void destroyCommandList (CommandList * l);
void appendToCommandList (CommandList * l, char * s, commandType i, commandType o);

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
  //
  CommandList commandList;

  while ((buffSize = getcmd(stdin, buff)) >= 0) {
    printf("mysh got: %s", buff);
    buff[buffSize - 1] = '\0'; //change the term char from \n to \0

    char * foo = buff;
    commandType inType, outType;

    inType = outType = REGULAR_CMD;

    for (buffItr = buff; (*buffItr) != '\0'; buffItr++) {
      token = buffItr;
      if (token == NULL) {
        break;
      }
      switch(*buffItr) {
        case '|':
          //TODO
          token = strtok(foo, "|");
          printf("saw pipe, got token: %s\n", token);
          foo = buffItr + 1;
          appendToCommandList(&commandList, token, inType, PIPE_CMD);
          inType = PIPE_CMD;
          break;
        case '%':
          //TODO
          token = strtok(foo, "%");
          printf("saw tee, got token: %s\n", token);
          foo = buffItr + 1;
          appendToCommandList(&commandList, token, inType, TEE_CMD);
          inType = TEE_CMD;
          break;
        case '>':
          if (buffItr[1] == '>') { //APPEND REDIRECTION
            token = strtok(foo, ">>");
            printf("saw app, got token: %s\n", token);
            buffItr++;
            foo = buffItr + 1;
            appendToCommandList(&commandList, token, inType, O_REDIR_CMD);
            inType = O_REDIR_CMD;
            //TODO append
          }
          else { //OVERWRITE REDIRECTION
            token = strtok(foo, ">");
            printf("saw ovr, got token: %s\n", token);
            foo = buffItr + 1;
            appendToCommandList(&commandList, token, inType, A_REDIR_CMD);
            inType = A_REDIR_CMD;
            //TODO ovr
          }
          break;
        default:
          //printf("just a regular char '%c'\n", *buffItr);
          continue;
      }
    }

    appendToCommandList(&commandList, foo, inType, REGULAR_CMD);

    destroyCommandList(&commandList);
  }

  perror("ERROR: Problem with mysh input!\n");

  exit(EXIT_SUCCESS);
}

/*
void appendToArgList (ArgList * list, char * arg) {
  ArgNode temp = malloc(sizeof(ArgNode));

  temp->arg = arg;
  temp->next = NULL;

  list->tail = temp;
*/

void appendToCommandList (CommandList * list, char * chunkOfText, commandType iType, commandType oType) {
  printf("in = %d; out = %d\n", iType, oType);
  CommandNode * temp = malloc(sizeof(CommandNode));

  temp->command = malloc(sizeof(Command));

  temp->command->inputType = iType;
  temp->command->outputType = oType;

  temp->command->argList = malloc(sizeof(ArgList));


  char * word;
  //ArgNode aNode;

  //for (word = strchr(chunkOfText, ' '); word[0] != NULL; word = strchr(word, ' ')){
  for (word = strtok(chunkOfText, " "); word != NULL; word = strtok(NULL, " ")) {
    printf("token got word %s\n", word);
    //temp->command-argList->
  }

  list->tail = temp;
  temp->next = NULL;
}

void destroyCommandList (CommandList * list) {
  printf("TODO\n");
}
