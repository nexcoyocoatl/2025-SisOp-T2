#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "instruction.h"
#include "memory_linkedlist.h"

struct List_node *last; // Para o circular-fit

// Cria nova linked-list de espaços de memória
// com um nodo contendo toda memória que depois será divido
struct Memory_list *memlist_create(size_t memory_size)
{
    struct Memory_list *lst = malloc(sizeof(struct Memory_list *));

    struct List_node *List_node = malloc(sizeof(struct List_node *));

    List_node->b_allocated = DISALLOC;
    List_node->start_address = 0;
    List_node->size = memory_size;

    List_node->next = List_node;
    lst->head = List_node;
    lst->tail = List_node;
    lst->size = 1;
    
    return lst;
}

// Retorna o tamanho de divisões (não é necessário para este programa)
size_t memlist_len(struct Memory_list *lst)
{
    return lst->size;
}

// Adiciona nodo por circular-fit
long long memlist_add_circular(struct Memory_list *lst, size_t pid, size_t process_size)
{
    uint8_t b_found = 0;
    struct List_node *current;
    
    // Se ainda não fez search, começa pelo primeiro nodo
    if (last == NULL) { last = lst->head; }

    current = last;

    // Procura espaço para alocar
    do
    {
        if (!(current->b_allocated) && process_size <= current->size)
        {
            b_found = 1;
            break;
        }

        current = current->next;
    }
    while (current != last);

    // Se não encontrou espaço, é porque não tem nenhum fragmento de tamanho
    // livre suficiente, então retorna
    if (!b_found) { return -1; }

    // Se não preencheu exatamente, então divide em dois
    if (process_size < current->size)
    {
        struct List_node *free_space = malloc(sizeof(struct List_node));
        free_space->b_allocated = DISALLOC;
        free_space->start_address = current->start_address + process_size;
        free_space->size = current->size - process_size;
        free_space->next = current->next;

        current->size = process_size;

        last = free_space;
        current->next = free_space;

        lst->size++;
    }
    current->b_allocated = ALLOC;
    current->pid = pid;

    last = current;

    return (long long)current->start_address;
}

// Adiciona nodo por worst-fit
long long memlist_add_worst(struct Memory_list *lst, size_t pid, size_t process_size)
{
    uint8_t b_found = 0;

    struct List_node *current = lst->head;
    struct List_node *worst = current;

    // Procura nodo desde o início da lista até voltar para o começo,
    // mesmo quando encontra um antes do final
    do
    {
        // Se existe um espaço livre maior, salva como worst-fit
        if (!(current->b_allocated) && current->size > worst->size)
        {
            worst = current;
        }

        // Se encontrou pelo menos um, marca flag como true
        if (!b_found && (process_size <= worst->size))
        {
            b_found = 1;
        }

        current = current->next;
    }
    while (current != lst->head);

    // Se não encontrou espaço, é porque não tem nenhum fragmento de tamanho
    // livre suficiente, então retorna
    if (!b_found) { return -1; }

    // Se não preencheu exatamente, então divide em dois
    if (process_size < worst->size)
    {
        struct List_node *free_space = malloc(sizeof(struct List_node));
        free_space->b_allocated = DISALLOC;
        free_space->start_address = worst->start_address + process_size;
        free_space->size = worst->size - process_size;
        free_space->next = worst->next;

        worst->size = process_size;

        last = free_space;
        worst->next = free_space;

        lst->size++;
    }
    worst->b_allocated = ALLOC;
    worst->pid = pid;

    return (long long)(worst->start_address);
}

// TODO: Adicionar sistema buddy com inserção de List_nodes

// Remove nodo independente da estratégia
long long memlist_remove_node(struct Memory_list *lst, size_t pid)
{
    uint8_t b_found = 0;
    uint8_t b_circular = 0;
    size_t address;

    struct List_node *current = lst->head;
    struct List_node *prev_List_node = current;

    do
    {
        // Remove apenas se está alocado (pode ser garbage)
        if (current->b_allocated && current->pid == pid)
        {
            b_found = 1;
            break;
        }

        prev_List_node = current;
        current = current->next;
    }
    while (current != lst->head);

    // Se não encontrou, é porque não foi alocado
    if (!b_found) { return -1; }

    // Endereço de início do processo que será retornado no fim
    address = current->start_address;

    if (last != NULL)
    {
        b_circular = 1;
    }

    // Se o anterior é livre, junta
    if (prev_List_node != current && prev_List_node->b_allocated == DISALLOC)
    {
        prev_List_node->size += current->size;
        prev_List_node->next = current->next;
        free(current);
        current = prev_List_node;
        lst->size--;

        // Caso o last seja perdido com esta junção
        if (b_circular && last == NULL)
        {
            last = current;
        }
    }

    current->b_allocated = DISALLOC;

    // Se o próximo também é livre, junta
    if (current->next != lst->head && current->next->b_allocated == DISALLOC)
    {
        struct List_node *next_List_node = current->next;
        current->size += next_List_node->size;
        current->next = next_List_node->next;
        free(next_List_node);
        lst->size--;
    }

    return (long long)address;
}

// Imprime todos nodos (talvez melhorar?)
void memlist_dump(struct Memory_list *lst)
{
    if (lst->size == 0)
    {
        printf("Empty\n");
        return;
    }

    struct List_node *current = lst->head;
    do
    {
        char s[10];
        snprintf(s, 10, "%lu", current->pid);
        printf("[\n  allocated:%s%s\n  start address: %lu\n  size: %lu\n]\n",
            current->b_allocated?"P\n  pid: ":"H", current->b_allocated?s:"", current->start_address, current->size);

        current = current->next;
    }
    while (current != lst->head);
}

void memlist_print(struct Memory_list *lst)
{
    struct List_node *current = lst->head;
    size_t contiguous_free_blocks = 0;

    printf("|");
    do
    {
        if (!current->b_allocated)
        {
            contiguous_free_blocks += current->size;
        }
        else
        {
            if (contiguous_free_blocks > 0)
                { printf("%lu|", contiguous_free_blocks); }
                contiguous_free_blocks = 0;
        }

        current = current->next;
    }
    while (current != lst->head);
    if (contiguous_free_blocks > 0)
        { printf("%lu|", contiguous_free_blocks); }
    printf("\n");
}

// Limpa toda a linked list, deixando apenas um nodo do tamanho máximo de memória
void memlist_flush(struct Memory_list *lst, size_t memory_size)
{
    struct List_node *current;

    // não sei se está deletando todos
    while (lst->size > 0)
    {
        current = lst->head;
        lst->head = lst->head->next;
        free(current);
        lst->size--;
    }

    struct List_node *List_node = malloc(sizeof(struct List_node *));

    List_node->b_allocated = DISALLOC;
    List_node->start_address = 0;
    List_node->size = memory_size;

    List_node->next = List_node;
    lst->head = List_node;
    lst->tail = List_node;
    lst->size = 1;
}

// Deleta todos nodos
void memlist_clear(struct Memory_list *lst)
{
    struct List_node *current;

    // não sei se está deletando todos
    while (lst->size > 0)
    {
        current = lst->head;
        lst->head = lst->head->next;
        free(current);
        lst->size--;
    }
}