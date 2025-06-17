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

    tree->root = malloc(sizeof(struct Tree_node));
    tree->root->parent = NULL;
    tree->root->b_allocated = DISALLOC;
    tree->root->start_address = 0;
    tree->root->size = memory_size;
    tree->root->occupied_size = 0;
    tree->root->b_is_leaf = 1;

    return tree;
}

int memtree_subdivide(struct Tree_node *node)
{
    if (!(node->b_is_leaf)) { return 0; }

    size_t new_size = node->size/2;
    if (new_size == 0) { return 0; }

    node->child_left = malloc(sizeof(struct Tree_node));
    node->child_right = malloc(sizeof(struct Tree_node));

    if (node->child_left == NULL || node->child_right == NULL) { return 0; }

    node->b_is_leaf = 0;
    
    node->child_left->parent = node;
    node->child_left->b_allocated = 0;
    node->child_left->size = new_size;
    node->child_left->occupied_size = 0;
    node->child_left->start_address = node->start_address;
    node->child_left->b_is_leaf = 1;
    node->child_left->b_allocated = DISALLOC;
    node->child_left->child_left = NULL;
    node->child_left->child_right = NULL;

    node->child_right->parent = node;
    node->child_right->b_allocated = 0;
    node->child_right->size = new_size;
    node->child_right->occupied_size = 0;
    node->child_right->b_is_leaf = 1;
    node->child_right->start_address = node->start_address + new_size;
    node->child_right->b_allocated = DISALLOC;
    node->child_right->child_left = NULL;
    node->child_right->child_right = NULL;

    // REMOVER
    // printf("alloc: %d, leaf: %d, address: %lu, size: %lu, occupied: %lu\n",
    //     node->b_allocated, node->b_is_leaf, node->start_address, node->size, node->occupied_size);
    // printf("    child left - alloc: %d, leaf: %d, address: %lu, size: %lu, occupied: %lu\n",
    //     node->child_left->b_allocated, node->child_left->b_is_leaf, node->child_left->start_address, node->child_left->size, node->child_left->occupied_size);
    // printf("    child right - alloc: %d, leaf: %d, address: %lu, size: %lu, occupied: %lu\n",
    //     node->child_right->b_allocated, node->child_right->b_is_leaf, node->child_right->start_address, node->child_right->size, node->child_right->occupied_size);

    return 1;
}

int memtree_coalesce_node(struct Tree_node *node)
{
    if ( node->b_is_leaf || !(node->child_left->b_is_leaf) || !(node->child_right->b_is_leaf) )
    {
        return 0;
    }

    free(node->child_left);
    free(node->child_right);
    
    node->child_left = NULL;
    node->child_right = NULL;

    node->b_is_leaf = 1;

    return 1;
}

// Busca por DFS, da esquerda para a direita
struct Tree_node *memtree_find_node_by_pid(struct Memory_tree *tree, size_t pid, size_t proc_size)
{
    struct Tree_node *current = NULL;

    // Funções para criação de uma array dinâmica por macro
    dynarray(struct Tree_node *) queue;
    dynarray_init(queue);

    dynarray_push(queue, tree->root);
    
    while (dynarray_size(queue) > 0)
    {
        dynarray_get_last(queue, current);
        dynarray_pop(queue);

        // Só entra nos filhos se o processo procurado cabe no tamanho de um deles
        if (current->size/2 >= proc_size)
        {
            if (current->child_right != NULL) { dynarray_push(queue, current->child_right); }
            if (current->child_left != NULL) { dynarray_push(queue, current->child_left); }
        }

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
    struct Tree_node *current = NULL;
    struct Tree_node *best = NULL;

    dynarray(struct Tree_node *) queue;
    dynarray_init(queue);

    dynarray_push(queue, tree->root);

    while (dynarray_size(queue) > 0)
    {        
        dynarray_get_last(queue, current);
        dynarray_pop(queue);

        // Só entra no nodo se tem espaço livre
        if ( !(current->b_allocated) )
        {
            // Só entra nos filhos se cabe neles
            if ( (current->size)/2 >= process_size)
            {
                if (current->b_is_leaf) { memtree_subdivide(current); }

                if (current->child_right != NULL) { dynarray_push(queue, current->child_right); }
                if (current->child_left != NULL) { dynarray_push(queue, current->child_left); }
            }         
            
            // Se achar um nodo livre de tamanho >= processo e que seus filhos < processo
            if (!(current->b_allocated) && current->b_is_leaf && process_size <= current->size && process_size > current->size/2)
            {

                // REMOVER
                // printf("alloc: %d, leaf: %d, address: %lu, size: %lu, occupied: \
                //  %lu\n", current->b_allocated, current->b_is_leaf, current->start_address, current->size, current->occupied_size);
                dynarray_free(queue);
                current->pid = pid;
                current->b_allocated = ALLOC;
                current->occupied_size = process_size;
                return current->start_address;
            }
        }
    }

    dynarray_free(queue);

    return -1;
}

long long memtree_remove_node(struct Memory_tree *tree, size_t pid, size_t process_size)
{
    struct Tree_node *node = memtree_find_node_by_pid(tree, pid, process_size);

    size_t address = node->start_address;
    node->b_allocated = DISALLOC;
    node->occupied_size = 0;

    // NÃO SEI SE ESTÁ FUNCIONANDO DIREITO
    while (node != tree->root)
    {
        node = node->parent;

        if ((node->child_left->b_allocated == 0
            && node->child_right->b_allocated == 0)
            && node->child_left->b_is_leaf
            && node->child_left->b_is_leaf)
        {
            // REMOVER
            printf("coalesce alloc: %d, leaf: %d, address: %lu, size: %lu, occupied: %lu\n",
                node->b_allocated, node->b_is_leaf, node->start_address, node->size, node->occupied_size);
                
            memtree_coalesce_node(node);
        }
    }
    
    return (long long)address;
}

void memtree_dump(struct Memory_tree *tree)
{

}

void memtree_print(struct Memory_tree *tree)
{

}

void memtree_clear(struct Memory_tree *tree)
{
    struct Tree_node *current = NULL;

    dynarray(struct Tree_node *) queue;
    dynarray_init(queue);

    dynarray_push(queue, tree->root);
    
    while (dynarray_size(queue) > 0)
    {
        dynarray_get_last(queue, current);
        dynarray_pop(queue);

        if ( !(current->b_is_leaf) )
        {
            if (current->child_right != NULL) { dynarray_push(queue, current->child_right); }
            if (current->child_left != NULL) { dynarray_push(queue, current->child_left); }
        }

        free(current);
    }

    dynarray_free(queue);
}