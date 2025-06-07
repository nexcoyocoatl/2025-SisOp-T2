#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdlib.h>

struct Memory_block
{
    char occupied; // ou enum (pra ver se o bloco está vazio)
    char value; //tamanho de memória
};

int initialize_block_list(struct Memory_block **block_list, size_t space_to_allocate);

#endif