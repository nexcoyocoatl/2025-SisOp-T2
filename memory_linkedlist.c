#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "memory_linkedlist.h"

struct Node *last; // Para o circular

struct Memory_list* memlist_create(size_t memory_size)
{
    struct Memory_list *lst = malloc(sizeof(struct Memory_list *));

    struct Node* node = malloc(sizeof(struct Node*));

    node->b_allocated = 0;
    node->start_address = 0;
    node->size = memory_size;

    node->next = node;
    lst->head = node;
    lst->tail = node;
    lst->size = 1;
    
    return lst;
}

size_t memlist_len(struct Memory_list* lst)
{
    return lst->size;
}

int memlist_add_circular(struct Memory_list* lst, size_t pid, size_t process_size)
{
    uint8_t b_found = 0;

    struct Node* current = lst->head;
    
    if (last == NULL) { last = lst->head; }

    do
    {
        if (!(current->next->b_allocated) && process_size <= current->size)
        {
            b_found = 1;
            last = current->next;
            break;
        }

        current = current->next;
    }
    while (current != last);

    if (!b_found) { return -1; }

    if (process_size < current->size)
    {
        struct Node *free_space = malloc(sizeof(struct Node));
        free_space->b_allocated = 0;
        free_space->start_address = current->start_address + process_size;
        free_space->size = current->size - process_size;
        free_space->next = current->next;

        current->size = process_size;

        last = free_space;
        current->next = free_space;

        lst->size++;
    }
    current->b_allocated = 1;
    current->pid = pid;

    return (int)current->start_address;
}

int memlist_add_worst(struct Memory_list* lst, size_t pid, size_t process_size)
{
    uint8_t b_found = 0;

    struct Node* current = lst->head;
    struct Node* worst = current;

    do
    {
        if (current->size > worst->size)
        {
            worst = current;
        }

        if (!b_found && (process_size <= worst->size))
        {
            b_found = 1;
        }

        current = current->next;
    }
    while (current != lst->head);

    if (!b_found) { return -1; }

    if (process_size < worst->size)
    {
        struct Node *free_space = malloc(sizeof(struct Node));
        free_space->b_allocated = 0;
        free_space->start_address = worst->start_address + process_size;
        free_space->size = worst->size - process_size;
        free_space->next = worst->next;

        worst->size = process_size;

        last = free_space;
        worst->next = free_space;

        lst->size++;
    }
    worst->b_allocated = 1;
    worst->pid = pid;

    return (int)worst->start_address;
}

// TODO: Sistema buddy

int memlist_remove_node(struct Memory_list* lst, size_t pid)
{
    uint8_t b_found = 0;

    if (lst->size == 1)
    {
        lst->head->b_allocated = 0;
        return 0;
    }

    struct Node* current = lst->head;
    struct Node* prev_node = current;

    while (current != lst->head)
    {
        if (current->pid == pid)
        {
            b_found = 1;
            break;
        }

        prev_node = current;
        current = current->next;
    }

    if (!b_found) { return -1; }

    // Se o anterior é livre, junta
    if (prev_node != current && prev_node->b_allocated == 0)
    {
        prev_node->size += current->size;
        prev_node->next = current->next;
        free(current);
        current = prev_node;
        lst->size--;
    }

    current->b_allocated = 0;

    // Se o próximo também é livre, junta
    if (current->next != lst->head && current->next->b_allocated == 0)
    {
        struct Node *next_node = current->next;
        current->size += next_node->size;
        current->next = next_node->next;
        free(next_node);
        lst->size--;
    }

    return (int)current->start_address;

    // if (current->pid == pid)
    // {
    //     lst->head = lst->head->next;
    //     lst->size--;

    //     if (lst->size == 0)
    //     {
    //         lst->tail = NULL;
    //     }

    //     free(current);
    //     return 1;
    // }

    // while (current->next != NULL)
    // {
    //     if (current->next->pid == pid)
    //     {
    //         struct Node* temp = current->next;

    //         // testar se funciona
    //         if (temp == lst->tail)
    //         {
    //             lst->tail = current;
    //         }

    //         current->next = current->next->next;
    //         lst->size--;

    //         free(temp);

    //         if (lst->size == 0)
    //         {
    //             lst->tail = NULL;
    //         }

    //         return 1;
    //     }
    //     current = current->next;
    // }

    // return 0;
}

int memlist_remove_node_index(struct Memory_list* lst, size_t index)
{
    size_t counter = 0;

    if (lst->size == 0)
    {
        return 0;
    }

    struct Node* current = lst->head;

    if (index == 0)
    {
        lst->head = lst->head->next;
        lst->size--;

        if (lst->size == 0)
        {
            lst->tail = NULL;
        }

        free(current);
        return 1;
    }

    while (current->next != NULL)
    {
        counter++;
        if (counter == index)
        {
            struct Node* temp = current->next;

            if (temp == lst->tail)
            {
                lst->tail = current;
            }

            current->next = current->next->next;
            lst->size--;

            free(temp);

            if (lst->size == 0)
            {
                lst->tail = NULL;
            }

            return 2;
        }
        current = current->next;
    }

    return 0;
}

struct Node *memlist_get_node(struct Memory_list* lst, size_t index)
{
    if (lst->size == 0) return NULL;
        
    struct Node* current = lst->head;
    size_t counter = 0;

    while(current != NULL)
    {
        if (counter == index)
        {
            return current;
        }
        counter++;
        current = current->next;
    }

    return NULL;
}

void memlist_dump(struct Memory_list* lst)
{
    if (lst->size == 0)
    {
        printf("Empty\n");
        return;
    }

    struct Node* current = lst->head;
    do
    {
        // printf("[\n  allocated: %d\n pid: %lu\n  start address: %lu\n  size: %lu\n]\n",
        //     current->b_allocated, current->pid, current->start_address, current->size);

        char s[10];
        snprintf(s, 10, "%lu", current->pid);
        printf("[\n  allocated:%s%s\n  start address: %lu\n  size: %lu\n]\n",
            current->b_allocated?"P\n pid: ":"H", current->b_allocated?s:"", current->start_address, current->size);

        current = current->next;
    }
    while (current != lst->head);
}

void memlist_clear(struct Memory_list* lst, size_t memory_size)
{
    struct Node* current;

    do
    {
        current = lst->head;
        lst->head = lst->head->next;
        free(current);
        lst->size--;
    }
    while (current != lst->head);

    struct Node* node = malloc(sizeof(struct Node*));

    node->b_allocated = 0;
    node->start_address = 0;
    node->size = memory_size;

    node->next = node;
    lst->head = node;
    lst->tail = node;
    lst->size = 1;
}