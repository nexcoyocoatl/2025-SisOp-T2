#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "instruction.h"
#include "memory_tree.h"
#include "macro_dynarray.h"

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

int memtree_subdivide(struct Tree_node *node)
{
    size_t new_size = node->size/2;
    node->child_left = malloc(sizeof(struct Tree_node));
    node->child_right = malloc(sizeof(struct Tree_node));

    if (node->child_left == NULL || node->child_right == NULL) { return 0; }
    
    node->child_left->parent = node;
    node->child_left->b_allocated = 0;
    node->child_left->size = new_size;
    node->child_left->start_address = node->start_address;
    node->child_left->child_left = NULL;
    node->child_left->child_right = NULL;

    node->child_right->parent = node;
    node->child_right->b_allocated = 0;
    node->child_right->size = new_size;
    node->child_right->start_address = node->start_address+new_size;
    node->child_right->child_left = NULL;
    node->child_right->child_right = NULL;

    return 1;
}

int memtree_remove_children(struct Tree_node *node)
{
    free(node->child_left);
    free(node->child_right);

    node->child_left = NULL;
    node->child_right = NULL;

    return 1;
}

// Busca por DFS, da esquerda para a direita
struct Tree_node *memtree_find_node_by_pid(struct Memory_tree *tree, size_t pid)
{
    struct Tree_node *current = NULL;

    // Funções para criação de uma array dinâmica por macro
    dynarray(struct Tree_node *) queue;
    dynarray_init(queue);

    dynarray_push(queue, tree->head);
    
    while (dynarray_size(queue) > 0)
    {
        dynarray_get_last(queue, current);
        dynarray_pop(queue);

        if (current->child_right != NULL) { dynarray_push(queue, current->child_right); }
        if (current->child_left != NULL) { dynarray_push(queue, current->child_left); }

        if (current->b_allocated && current->pid == pid)
        {
            return current;
        }
    }

    dynarray_free(queue);

    return current;
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