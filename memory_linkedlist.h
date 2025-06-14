#ifndef _MEMORY_LINKEDLIST_H_
#define _MEMORY_LINKEDLIST_H_

#include <stdint.h>
#include <stdlib.h>
// #include <memory.h> // TODO: necessário?

struct Node
{
    uint8_t b_allocated;
    size_t pid;
    size_t start_address;
    size_t size;

    struct Node* next;
};

struct Memory_list
{
    // TODO: criar header? (verificar se header seria um nodo diferente)
    struct Node* head;
    struct Node* tail;
    size_t size;
};

struct Memory_list* memlist_create();
size_t memlist_len(struct Memory_list* lst);
void memlist_add_circular(struct Memory_list* lst, size_t pid, size_t process_size);
void memlist_add_worst(struct Memory_list* lst, size_t pid, size_t process_size);
int memlist_remove_node(struct Memory_list* lst, size_t program_id);
int memlist_remove_node_index(struct Memory_list* lst, size_t index);
struct Node *memlist_get_node(struct Memory_list* lst, size_t index);
void memlist_dump(struct Memory_list* lst);
void memlist_clear(struct Memory_list* lst);

#endif