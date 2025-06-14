#include <stddef.h>

#define STRING_BUFFER 32

enum Operation
{
    OP_ERROR = -1,
    DISALLOC = 0,
    ALLOC
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
    char name[STRING_BUFFER];
};