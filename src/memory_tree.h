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
    size_t occupied_size;
    uint8_t b_is_leaf;

    struct Tree_node *child_left;
    struct Tree_node *child_right;
};

struct Memory_tree
{
    struct Tree_node *root;
};

struct Memory_tree *memtree_create(size_t memory_size);
int memtree_subdivide(struct Tree_node *node);
int memtree_coalesce_node(struct Tree_node *node);
struct Tree_node *memtree_find_node_by_pid(struct Memory_tree *tree, size_t pid, size_t proc_size);
long long memtree_add_buddy(struct Memory_tree *tree, size_t pid, size_t process_size);
long long memtree_remove_node(struct Memory_tree *tree, size_t pid, size_t process_size);
void memtree_dump(struct Memory_tree *tree);
void memtree_print(struct Memory_tree *tree);
void memtree_clear(struct Memory_tree *tree);

#endif