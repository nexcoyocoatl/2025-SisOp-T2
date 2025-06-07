#include <stdlib.h>
#include <stdio.h>
#include "memory.h"

int initialize_block_list(struct Memory_block **block_list, size_t space_to_allocate)
{
    *block_list = malloc(sizeof(struct Memory_block) * space_to_allocate);

    if (*block_list == NULL) { return 0; }

    for (size_t i = 0; i < space_to_allocate; i++)
    {
        (*block_list)[i].occupied = 0;
    }
    printf("\n");
    
    return 1;
}