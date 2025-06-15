#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "instruction.h"
#include "memory_tree.h"

struct Memory_tree *memtree_create(size_t memory_size)
{
    struct Memory_tree *tree = malloc(sizeof(struct Memory_tree));
    tree->head = malloc(sizeof(struct Tree_node));
    tree->head->parent = NULL;
    tree->head->b_allocated = DISALLOC;
    tree->head->start_address = 0;
    tree->head->size = memory_size;

    return tree;
}

int memtree_subdivide(struct Memory_tree *tree, size_t pid)
{
    
}

long long memtree_add_buddy(struct Memory_tree *tree, size_t pid, size_t process_size)
{

}

long long memtree_remove_node(struct Memory_tree *tree, size_t pid)
{

}

void memtree_dump(struct Memory_tree *tree)
{

}

void memtree_print(struct Memory_tree *tree)
{

}

void memtree_clear(struct Memory_tree *tree)
{

}