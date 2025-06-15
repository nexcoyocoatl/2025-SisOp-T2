#ifndef _MEMORY_LINKEDLIST_H_
#define _MEMORY_LINKEDLIST_H_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

struct List_node
{
    uint8_t b_allocated;
    size_t pid;
    size_t start_address;
    size_t size;

    struct List_node *next;
};

struct Memory_list
{
    struct List_node *head;
    struct List_node *tail;
    size_t size;
};

struct Memory_list *memlist_create(size_t memory_size);
size_t memlist_len(struct Memory_list *lst);
long long memlist_add_circular(struct Memory_list *lst, size_t pid, size_t process_size);
long long memlist_add_worst(struct Memory_list *lst, size_t pid, size_t process_size);
long long memlist_remove_node(struct Memory_list *lst, size_t pid);
void memlist_dump(struct Memory_list *lst);
void memlist_flush(struct Memory_list *lst, size_t memory_size);
void memlist_print(struct Memory_list *lst);
void memlist_clear(struct Memory_list *lst);

#endif