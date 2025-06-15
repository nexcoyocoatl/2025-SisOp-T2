#ifndef _MEMORY_TREE_H_
#define _MEMORY_TREE_H_

#include <stddef.h>
#include <stdint.h>

struct Tree_node
{
    struct Tree_node *parent;

    uint8_t b_allocated;
    size_t pid;
    size_t start_address;
    size_t size;

    struct Tree_node *child_left;
    struct Tree_node *child_right;
};

struct Memory_tree
{
    struct Tree_node *head;
    size_t level;
};

struct Memory_tree *memtree_create(size_t memory_size);
int memtree_subdivide(struct Memory_tree *tree, size_t pid);
long long memtree_add_buddy(struct Memory_tree *tree, size_t pid, size_t process_size);
long long memtree_remove_node(struct Memory_tree *tree, size_t pid);
void memtree_dump(struct Memory_tree *tree);
void memtree_print(struct Memory_tree *tree);
void memtree_clear(struct Memory_tree *tree);

#endif