//Header file for mysh.c
typedef enum { REGULAR_CMD, PIPE_CMD, TEE_CMD, O_REDIR_CMD, A_REDIR_CMD } commandType;


typedef struct arg_node {
  char * argVal;
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

ArgList * newArgList();
void appendToArgList (ArgList * l, char * a);
void destroyArgList (ArgList * l);

CommandList * newCommandList();
void appendToCommandList (CommandList * l, char * s, commandType i, commandType o);
void destroyCommandList (CommandList * l);
void execSingleCommand(CommandList * list, char **argv);
char ** buildArgv(ArgList * list);
int badCharNext(char * c);
void execSinglePipe(char **argv, char **argv2);
void execDoublePipe(char **argv, char **argv2,char **argv3);
void execSinglePipeRedir(char **argv, char **argv2, char * file, commandType mode);
void execDoublePipeRedir(char **argv, char **argv2,char **argv3, char * file, commandType mode);

void printCommandList (CommandList * l);
void execCommands (CommandList * l);
int checkOutputType( CommandList * list);

int streq (char * a, char * b, int n);
