#include <memory.h>

struct Memory_block
{
    char occupied; // ou enum (pra ver se o bloco está vazio)
    char value; //tamanho de memória
};

int initialize_block_list(struct Memory_block *block_list, size_t space_to_allocate)
{
    block_list = malloc(sizeof(struct Memory_block) * space_to_allocate);

    if (block_list == NULL)
    {
        return 0;
    }
    
    return 1;
}