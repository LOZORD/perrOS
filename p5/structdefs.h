#define INODEPIECES 256
#define MFS_DIRECTORY 0
#define MFS_REGULAR_FILE 1

typedef struct __attribute__((__packed__)) __checkpoint__       {
        int size;
        int iMapPtr[INODEPIECES];
} checkpoint;

typedef struct __attribute__((__packed__)) __dirEnt__   {
        char name[60];
        int inum;
} dirEnt;

typedef struct __attribute__((__packed__)) __inode__    {
        int size;
        int type;
        int ptr[14];
} inode;

typedef struct __attribute__((__packed__)) __inodeMap__ {
        int inodePtr[16];
} inodeMap;


