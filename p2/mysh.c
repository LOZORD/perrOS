//mysh, by Leo Rudberg and Nicholas Hyatt

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "mysh.h"

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


ArgList * newArgList();
void appendToArgList (ArgList * l, char * a);
void destroyArgList (ArgList * l);

CommandList * newCommandList();
void appendToCommandList (CommandList * l, char * s, commandType i, commandType o);
void destroyCommandList (CommandList * l);

void printCommandList (CommandList * l);
void execCommands (CommandList * l);

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

  while ((buffSize = getcmd(stdin, buff)) >= 0) {
    CommandList * commandList = newCommandList();
    //printf("mysh got: %s", buff);
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
          //printf("saw pipe, got token: %s\n", token);
          foo = buffItr + 1;
          appendToCommandList(commandList, token, inType, PIPE_CMD);
          inType = PIPE_CMD;
          break;
        case '%':
          //TODO
          token = strtok(foo, "%");
          //printf("saw tee, got token: %s\n", token);
          foo = buffItr + 1;
          appendToCommandList(commandList, token, inType, TEE_CMD);
          inType = TEE_CMD;
          break;
        case '>':
          if (buffItr[1] == '>') { //APPEND REDIRECTION
            token = strtok(foo, ">>");
            //printf("saw app, got token: %s\n", token);
            buffItr++;
            foo = buffItr + 1;
            appendToCommandList(commandList, token, inType, O_REDIR_CMD);
            inType = O_REDIR_CMD;
            //TODO append
          }
          else { //OVERWRITE REDIRECTION
            token = strtok(foo, ">");
            //printf("saw ovr, got token: %s\n", token);
            foo = buffItr + 1;
            appendToCommandList(commandList, token, inType, A_REDIR_CMD);
            inType = A_REDIR_CMD;
            //TODO ovr
          }
          break;
        default:
          //printf("just a regular char '%c'\n", *buffItr);
          continue;
      }
    }

    appendToCommandList(commandList, foo, inType, REGULAR_CMD);
    //printCommandList(commandList);
    execCommands(commandList);
    destroyCommandList(commandList);
  } //end while

  perror("ERROR: Problem with mysh input!\n");

  exit(EXIT_SUCCESS);
}

void execCommands (CommandList * list) {
  CommandNode * cNItr = list->head;
  char * leadExecArg = cNItr->command->argList->head->argVal;
  if(!strcmp(leadExecArg, "exit") && list->size == 1){
      exit(EXIT_SUCCESS);
  }else if(!strcmp(leadExecArg, "cd") && list->size == 1){
    int error;
    if(cNItr->command->argList->size > 1){
      error = chdir(cNItr->command->argList->head->next->argVal); 
    }else{
      error = chdir(getenv("HOME"));
    }
    if(error){
      fprintf(stderr,"Error: %s\n", strerror(errno));
    }
    return;
  }
  while(cNItr != NULL) {
    ArgNode * aItr = cNItr->command->argList->head;
    int i = 0;
    char * execArg = aItr->argVal;
    char * argv [cNItr->command->argList->size + 1];
    while(aItr != NULL) {
      argv[i] = (aItr->argVal);
      aItr = aItr->next;
      //printf("argv[%d] == %s\n", i, *argv[i]);
      i++;
    }
    argv[i] = NULL;
    int status;
    int pid = fork();
    if(pid == -1){
      fprintf(stderr,"Error: %s\n", strerror(errno));
    }else if (pid == 0){
      //CHILD
      execvp(execArg, argv);
      fprintf(stderr,"Error: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }else{
      //PARENT
      wait(&status);
     // printf("Child completed with status: %d\n", status);
    }
    cNItr = cNItr->next;
  }
}

void printCommandList (CommandList * list) {
  CommandNode * cNItr = list->head;
  printf("List size: %d\n", list->size);
  while(cNItr != NULL) {
    printf("This command:\n");
    printf("\tIN TYPE: %d\n", cNItr->command->inputType);
    printf("\tOUT TYPE: %d\n", cNItr->command->outputType);
    printf("\tARG LIST:\n");
    ArgNode * aItr = cNItr->command->argList->head;
    printf("\t\tARG COUNT: %d\n", cNItr->command->argList->size);
    while(aItr != NULL) {
      printf("\t\t%s\n", aItr->argVal);
      aItr = aItr->next;
    }
    cNItr = cNItr->next;
  }
  printf("*** DONE ***\n");
}


void appendToArgList (ArgList * list, char * arg) {
  //printf("Entered appendToArgList\n");
  ArgNode * temp = malloc(sizeof(ArgNode));
  size_t strSize = strlen(arg) + 1;
  temp->argVal = malloc(strSize);
  strncpy(temp->argVal, arg, strSize);
  temp->next = NULL;
  list->size += 1;
  if (!list->head || !list->tail) {
    list->head = list->tail = temp;
  }
  else {
    list->tail->next = temp;
    list->tail = temp;
  }
  //printf("Exited appendToArgList\n");
}

ArgList * newArgList () {
  ArgList * list = malloc(sizeof(ArgList));
  list->size = 0;
  list->head = list->tail = NULL;

  return list;
}

CommandList * newCommandList () {
  CommandList * list = malloc(sizeof(CommandList));
  list->size = 0;
  list->head = list->tail = NULL;

  return list;
}

void destroyArgList (ArgList * list) {
  list->size = 0;
  ArgNode * itr = list->head;
  //char * argItr;
  while(itr != NULL) {
    free(itr->argVal);
    ArgNode * temp = itr;
    itr = itr->next;
    free(temp);
  }

  free(list);
}

void appendToCommandList (CommandList * list, char * chunkOfText, commandType iType, commandType oType) {
  //printf("in = %d; out = %d\n", iType, oType);
  CommandNode * temp = malloc(sizeof(CommandNode));
  list->size += 1;

  temp->command = malloc(sizeof(Command));

  temp->command->inputType = iType;
  temp->command->outputType = oType;

  temp->command->argList = newArgList();


  char * word;
  //ArgNode aNode;

  //for (word = strchr(chunkOfText, ' '); word[0] != NULL; word = strchr(word, ' ')){
  for (word = strtok(chunkOfText, " \t"); word != NULL; word = strtok(NULL, " \t")) {
    //printf("\ttoken got word %s\n", word);
    appendToArgList(temp->command->argList, word);
  }
  temp->next = NULL;

  if (!(list->head && list->tail)) {
    list->head = list->tail = temp;
  }
  else {
    list->tail->next = temp;
    list->tail = temp;
  }
}

void destroyCommandList (CommandList * list) {
  CommandNode * itr = list->head;

  while(itr != NULL) {
    CommandNode * temp = itr;
    itr = itr->next;

    destroyArgList(temp->command->argList);
    free(temp->command);
    free(temp);
  }

  free(list);
}
