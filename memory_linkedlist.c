#include <stdio.h>
#include <stdlib.h>

#include "memory_linkedlist.h"

struct Memory_list* memlist_create()
{
    struct Memory_list* lst = malloc(sizeof(*lst));
    lst->head = NULL;
    lst->tail = NULL;
    lst->size = 0;
    
    return lst;
}

size_t memlist_len(struct Memory_list* lst)
{
    return lst->size;
}

// TODO: Variável para lembrar da posição de onde parou no circular fit
void memlist_add_circular(struct Memory_list* lst, size_t pid, size_t process_size)
{
    // TODO:
}

void memlist_add_worst(struct Memory_list* lst, size_t pid, size_t process_size)
{
    // TODO:
}

// void memlist_add_node(struct Memory_list* lst, struct Program *program)
// {
//     struct Node* node = malloc(sizeof(*node));
//     node->program = program;
//     node->next = NULL;

//     if (lst->size == 0)
//     {
//         lst->head = node;
//     }
//     else
//     {
//         struct Node* current = lst->head;
//         while(current->next != NULL)
//         {
//             current = current->next;
//         }

//         current->next = node;
//     }

//     lst->tail = node;

//     lst->size++;
// }

// TODO: Remove para os dois?
int memlist_remove_node(struct Memory_list* lst, size_t pid)
{
    if (lst->size == 0)
    {
        return 0;
    }

    struct Node* current = lst->head;

    if (current->pid == pid)
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
        if (current->next->pid == pid)
        {
            struct Node* temp = current->next;

            // testar se funciona
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

            return 1;
        }
        current = current->next;
    }

    return 0;
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
    while (current != NULL)
    {
        printf("[\n  allocated: %d\n pid: %lu\n  start address: %lu\n  size: %lu\n]\n",
            current->allocated, current->pid, current->start_address, current->size);
        current = current->next;
    }
}

void memlist_clear(struct Memory_list* lst)
{
    struct Node* current;

    while (lst->head != NULL)
    {
        current = lst->head;
        lst->head = lst->head->next;
        free(current);
        lst->size--;
    }
}