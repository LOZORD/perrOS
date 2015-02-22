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

