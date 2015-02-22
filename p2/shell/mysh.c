//mysh, by Leo Rudberg and Nicholas Hyatt

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "mysh.h"

#define DEBUG 0

size_t MAX_INPUT_LENGTH = 1024;

extern FILE * stdin;
extern FILE * stdout;
extern FILE * stderr;
int oldOut, new, oldIn, newIn;
int p[2];
int p2[2];

char * buffItr;

//Our generic error printing
void alertError() {
  fprintf(stderr, "Error!\n");
}

void switchStdout(const char *newStream, commandType write_mode) {
  int rc;
  if (write_mode == O_REDIR_CMD || write_mode == A_REDIR_CMD) {
    rc = fflush(stdout);
    if (rc == EOF) {
      alertError();
      return;
    }

    oldOut = dup(1);

    if (write_mode == O_REDIR_CMD) {
      new = open(newStream, O_WRONLY|O_TRUNC |O_CREAT, 0777);
    }
    else {
      new = open(newStream, O_WRONLY|O_APPEND|O_CREAT, 0777);
    }

    if (new == -1) {
      alertError();
      return;
    }

    dup2(new, 1);
    close(new);
  }
  //bad commandType
  else {
    alertError();
  }
}

void revertStdout()
{
  fflush(stdout);
  dup2(oldOut, 1);
  close(oldOut);
}

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

  char buff [MAX_INPUT_LENGTH + 1];
  char * token = NULL;
  int buffSize;

  while ((buffSize = getcmd(stdin, buff)) >= 0) {
    CommandList * commandList = newCommandList();
    buff[buffSize - 1] = '\0'; //change the term char from \n to \0

    char * currInput = buff;
    commandType inType = REGULAR_CMD;
    int errorSeen = 0;

    //iterate through all of the characters in the buffer
    for (buffItr = buff; (*buffItr) != '\0'; buffItr++) {
      token = buffItr;
      if (token == NULL) {
        break;
      }
      switch(*buffItr) {
        case '|':
          if(badCharNext(buffItr)) {
            alertError();
            errorSeen = 1;
            break;
          }
          token = strtok(currInput, "|");
          #if DEBUG
          printf("saw pipe, got token: %s\n", token);
          #endif
          currInput = buffItr + 1;
          appendToCommandList(commandList, token, inType, PIPE_CMD);
          inType = PIPE_CMD;
          break;
        case '%':
          if(badCharNext(buffItr)) {
            alertError();
            errorSeen = 1;
            break;
          }
          token = strtok(currInput, "%");
          #if DEBUG
          printf("saw tee, got token: %s\n", token);
          #endif
          currInput = buffItr + 1;
          appendToCommandList(commandList, token, inType, TEE_CMD);
          inType = TEE_CMD;
          break;
        case '>':
          if(badCharNext(buffItr)) {
            alertError();
            errorSeen = 1;
            break;
          }
          //APPEND REDIRECTION
          if (buffItr[1] == '>') {
            token = strtok(currInput, ">>");
            #if DEBUG
            printf("saw app, got token: %s\n", token);
            #endif
            buffItr++;
            currInput = buffItr + 1;
            appendToCommandList(commandList, token, inType, A_REDIR_CMD);
            inType = A_REDIR_CMD;
          }
          //OVERWRITE REDIRECTION
          else {
            if(badCharNext(buffItr)){
              alertError();
              errorSeen = 1;
              break;
            }
            token = strtok(currInput, ">");
            #if DEBUG
            printf("saw ovr, got token: %s\n", token);
            #endif
            currInput = buffItr + 1;
            appendToCommandList(commandList, token, inType, O_REDIR_CMD);
            inType = O_REDIR_CMD;
          }
          break;
        default:
          #if DEBUG
          printf("just a regular char '%c'\n", *buffItr);
          #endif
          continue;
      }
      if(errorSeen){
        break;
      }
    }

    appendToCommandList(commandList, currInput, inType, REGULAR_CMD);
    #if DEBUG
    printCommandList(commandList);
    #endif

    //check for blank entry or erroneous input
    if ((commandList->size == 1 && commandList->head->command->argList->size == 0) || errorSeen) {
      //do nothing
    }
    else {
      execCommands(commandList);
    }

    destroyCommandList(commandList);
  } //end while

  fputc('\n', stdout);
  exit(EXIT_SUCCESS);
}

int badCharNext(char * c) {
  if (c[0] == '|' || c[0] == '%') {
    if (c[1] != '\0' && (c[1] == '|' || c[1] == '>' || c[1] == '%')) {
      return 1;
    }
  }
  else if( c[0] == '>') {
    if(c[1] != '\0' && c[2] != '\0' && (c[1] == '|' || c[2] == '>' || c[1] == '%')) {
      return 1;
    }
  }
  //no bad character input next
  return 0;
}

char ** buildArgv(ArgList * list) {
  ArgNode * aItr = list->head;
  int i = 0;
  char ** argv = malloc(sizeof(char *) * (list->size + 1));

  while(aItr != NULL) {
    argv[i] = (aItr->argVal);
    aItr = aItr->next;
    #if DEBUG
    printf("argv[%d] == %s\n", i, (argv[i]));
    #endif
    i++;
  }
  //NULL terminate the argv array
  argv[i] = NULL;
  return argv;
}

void execCommands (CommandList * list) {

  CommandNode * itr = list->head;
  while(itr != NULL){
    if(itr->command->argList->size == 0){
      alertError();
      return;
    }
    itr = itr->next;
  }
  char ** argv = buildArgv(list->head->command->argList);

  //SOME BUILT-IN COMMANDS
  //`exit` command
  if (
    streq(argv[0], "exit", 4) &&
    list->size == 1 &&
    list->head->command->argList->size == 1
  )
  {
    free(argv);
    destroyCommandList(list);
    exit(EXIT_SUCCESS);
  }
  //`cd` commands
  else if (streq(argv[0], "cd", 2) && list->size == 1) {
    int error;
    if (list->head->command->argList->size > 1) {
      error = chdir(list->head->command->argList->head->next->argVal);
    }
    else {
      error = chdir(getenv("HOME"));
    }

    //error changing directory?
    if (error) {
      #if DEBUG
      fprintf(stderr,"Error: %s\n", strerror(errno));
      #endif
      alertError();
    }

    free(argv);
    return;
  }

  //single commands and single redirects
  if(list->size == 1
      || (list->size == 2
        && (
          list->head->command->outputType == O_REDIR_CMD
          || list->head->command->outputType == A_REDIR_CMD))
    ) {
    execSingleCommand(list, argv);
    free(argv);
    return;
  }

  //pipe && tee chains
  if(list->size == 2) {
    //We know we have either a valid pipe or tee command
    if(list->head->command->outputType == PIPE_CMD) {
      //we have a pipe
      char ** argv2 = buildArgv(list->tail->command->argList);
      execSinglePipe(argv,argv2);
      free(argv2);
    }
    else {
      //we have a tee
      #if DEBUG
      printf("\tAT TEE!\n");
      #endif
      char *argv2[2] = { "./mytee", NULL };
      char ** argv3 = buildArgv(list->tail->command->argList);
      execDoublePipe(argv,argv2,argv3);
      free(argv3);
    }
  }
  else if(list->size == 3) {
    //We know either 2 pipes or pipe redirect
    char ** argv2 = buildArgv(list->head->next->command->argList);
    char ** argv3 = buildArgv(list->tail->command->argList);
    if(list->tail->command->inputType == PIPE_CMD) {
      execDoublePipe(argv,argv2,argv3);
    }
    else {
      if(list->head->command->outputType == PIPE_CMD) {
      //Pipe then redirect
      execSinglePipeRedir(argv, argv2, argv3[0], list->tail->command->inputType);
      }
      else {
        //Tee redirect
        char *argvT[2] = { "./mytee", NULL }; //the Tee payload
        execDoublePipeRedir(argv, argvT, argv2, argv3[0], list->tail->command->inputType);
      }
    }
    free(argv2);
    free(argv3);
  }
  else if(list->size == 4) {
    //We know 2 pipes followed by redirect
    char ** argv2 = buildArgv(list->head->next->command->argList);
    char ** argv3 = buildArgv(list->head->next->next->command->argList);
    execDoublePipeRedir(argv, argv2, argv3,list->tail->command->argList->head->argVal, list->tail->command->inputType);
    free(argv2);
    free(argv3);
  }
  else {
    alertError();
  }

  free(argv);
}


/* What follows are our executor functions. They are built off of the executor,
 * piping, and redirection functionality found in xv6/user/sh.c
 */

void execDoublePipeRedir(char **argv, char **argv2,char **argv3, char * file, commandType mode){
  int childpid;
  int status;
  if(pipe(p) < 0){
    alertError();
    return;
  }
  if(pipe(p2) < 0){
    alertError();
    return;
  }
  //LEFT HAND SIDE CMD
  if((childpid = fork()) == -1){
    alertError();
    return;
  }else if(childpid == 0){
    close(1);
    dup(p[1]);
    close(p[0]);
    close(p[1]);
    close(p2[0]);
    close(p2[1]);
    execvp(argv[0], argv);
    alertError(); //execvp call should never return
  }
  //MIDDLE CMD
  if((childpid = fork()) == -1){
    alertError();
    return;
  }else if(childpid == 0){
    close(0);
    dup(p[0]);
    close(1);
    dup(p2[1]);
    close(p[0]);
    close(p[1]);
    close(p2[0]);
    close(p2[1]);
    execvp(argv2[0], argv2);
    alertError(); //execvp call should never return
  }
  //FAR RIGHT CMD
  if((childpid = fork()) == -1){
    alertError();
    return;
  }else if(childpid == 0){
    close(0);
    dup(p2[0]);
    switchStdout(file, mode);
    close(p[0]);
    close(p[1]);
    close(p2[0]);
    close(p2[1]);
    execvp(argv3[0], argv3);
    alertError(); //execvp call should never return
  }
  close(p[0]);
  close(p[1]);
  close(p2[0]);
  close(p2[1]);
  wait(&status);
  wait(&status);
  wait(&status);
}

void execSinglePipeRedir(char **argv, char **argv2, char * file, commandType mode){
  int childpid;
  int status;
  if(pipe(p) < 0){
    alertError();
    return;
  }
  //LEFT HAND SIDE CMD
  if((childpid = fork()) == -1){
    alertError();
    return;
  }else if(childpid == 0){
    close(1);
    dup(p[1]);
    close(p[0]);
    close(p[1]);
    execvp(argv[0], argv);
    alertError(); //execvp call should never return
  }
  //RIGHT HAND SIDE CMD
  if((childpid = fork()) == -1){
    alertError();
    return;
  }else if(childpid == 0){
    close(0);
    dup(p[0]);
    close(p[0]);
    close(p[1]);
    switchStdout(file, mode);
    execvp(argv2[0], argv2);
    alertError(); //execvp call should never return
  }
  close(p[0]);
  close(p[1]);
  wait(&status);
  wait(&status);
}

void execSinglePipe(char **argv, char **argv2){
  int pid1;
  int pid2;
  int status1;
  int status2;
  if(pipe(p) < 0){
    alertError();
    return;
  }
  //LEFT HAND SIDE CMD
  if((pid1 = fork()) == -1){
    alertError();
    return;
  }else if(pid1 == 0){
    close(1);
    dup(p[1]);
    close(p[0]);
    close(p[1]);
    execvp(argv[0], argv);
    alertError(); //execvp call should never return
  }
  //RIGHT HAND SIDE CMD
  if((pid2 = fork()) == -1){
    alertError();
    return;
  }else if(pid2 == 0){
    close(0);
    dup(p[0]);
    close(p[0]);
    close(p[1]);
    execvp(argv2[0], argv2);
    alertError(); //execvp call should never return
  }
  close(p[0]);
  close(p[1]);
  waitpid(pid1, &status1,0);
  waitpid(pid2, &status2,0);
}

void execDoublePipe(char **argv, char **argv2,char **argv3){
  int childpid;
  int status;
  if(pipe(p) < 0){
    alertError();
    return;
  }
  if(pipe(p2) < 0){
    alertError();
    return;
  }
  //LEFT HAND SIDE CMD
  if((childpid = fork()) == -1){
    alertError();
    return;
  }else if(childpid == 0){
    #if DEBUG
      int w; for(w = 0; argv[w] != NULL; w++) { printf("\tLEFT ARGS: %s\n", argv[w]); }
    #endif
    close(1);
    dup(p[1]);
    close(p[0]);
    close(p[1]);
    close(p2[0]);
    close(p2[1]);
    execvp(argv[0], argv);
    #if DEBUG
    fprintf(stderr,"Error: %s\n", strerror(errno));
    #endif
    alertError();
    exit(EXIT_FAILURE);
  }
  //MIDDLE CMD
  if((childpid = fork()) == -1){
    alertError();
    return;
  }else if(childpid == 0){
    #if DEBUG
      int w; for(w = 0; argv2[w] != NULL; w++) { printf("\tMID ARGS: %s\n", argv2[w]); }
    #endif
    close(0);
    dup(p[0]);
    close(1);
    dup(p2[1]);
    close(p[0]);
    close(p[1]);
    close(p2[0]);
    close(p2[1]);
    execvp(argv2[0], argv2);
    #if DEBUG
    fprintf(stderr,"Error: %s\n", strerror(errno));
    #endif
    alertError();
    exit(EXIT_FAILURE);
  }
  //FAR RIGHT CMD
  if((childpid = fork()) == -1){
    alertError();
    return;
  }else if(childpid == 0){
    #if DEBUG
      int w; for(w = 0; argv3[w] != NULL; w++) { printf("\tRIGHT ARGS: %s\n", argv3[w]); }
    #endif
    close(0);
    dup(p2[0]);
    close(p[0]);
    close(p[1]);
    close(p2[0]);
    close(p2[1]);
    execvp(argv3[0], argv3);
    #if DEBUG
    fprintf(stderr,"Error: %s\n", strerror(errno));
    #endif
    alertError();
    exit(EXIT_FAILURE);
  }
  close(p[0]);
  close(p[1]);
  close(p2[0]);
  close(p2[1]);
  wait(&status);
  wait(&status);
  wait(&status);
}

void execSingleCommand(CommandList * list, char **argv){
    int status;
    int error = checkOutputType(list);
    if(error){
      return;
    }
    int pid = fork();

    //ERROR
    if(pid == -1) {
      #if DEBUG
      fprintf(stderr,"Error: %s\n", strerror(errno));
      #endif
      alertError();
    }
    //CHILD
    else if (pid == 0) {
      execvp(argv[0], argv);
      #if DEBUG
      fprintf(stderr,"Error: %s\n", strerror(errno));
      #endif
      alertError();
      exit(EXIT_FAILURE);
    }
    //PARENT
    else {
      wait(&status);
      if(list->head->command->outputType == O_REDIR_CMD){
        revertStdout();
      }
      if(list->head->command->outputType == A_REDIR_CMD){
        revertStdout();
      }

      #if DEBUG
      printf("Child completed with status: %d\n", status);
      #endif
    }
}

int checkOutputType( CommandList * list){
  if(list->head->command->outputType == O_REDIR_CMD){
    if(list->size == 2 && list->head->next->command->argList->size == 0){
      alertError();
      return 1;
    }
    switchStdout(list->head->next->command->argList->head->argVal, O_REDIR_CMD);
    return 0;
  }else if(list->head->command->outputType == A_REDIR_CMD){
    if(list->size == 2 && list->head->next->command->argList->size == 0){
      alertError();
      return 1;
    }
    switchStdout(list->head->next->command->argList->head->argVal, A_REDIR_CMD);
    return 0;
  }

  return 0;
}

//for debugging
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
  while(itr != NULL) {
    free(itr->argVal);
    ArgNode * temp = itr;
    itr = itr->next;
    free(temp);
  }

  free(list);
}

void appendToCommandList (CommandList * list, char * chunkOfText, commandType iType, commandType oType) {
  CommandNode * temp = malloc(sizeof(CommandNode));
  list->size += 1;

  temp->command = malloc(sizeof(Command));

  temp->command->inputType = iType;
  temp->command->outputType = oType;

  temp->command->argList = newArgList();


  char * word;

  for (word = strtok(chunkOfText, " \t"); word != NULL; word = strtok(NULL, " \t")) {
    #if DEBUG
    printf("\ttoken got word %s\n", word);
    #endif
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

int streq(char * a, char * b, int n) {
  return 0 == strncmp(a,b,n);
}
