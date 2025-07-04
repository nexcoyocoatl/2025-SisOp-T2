#include <stddef.h>

#define INST_STRING_BUFFER 32

enum Operation
{
    OP_ERROR = -1,
    DISALLOC = 0,
    ALLOC = 1,
    UNUSED = 2
};

struct Instruction
{
    size_t pid;
    enum Operation operation;
};

struct Process
{
    size_t pid;
    size_t size;
    char name[INST_STRING_BUFFER];
};