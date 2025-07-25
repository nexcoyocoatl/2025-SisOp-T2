#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "instruction.h"
#include "memory_linkedlist.h"

struct List_node *last; // Para o circular-fit

// Cria nova linked-list de espaços de memória
// com um nodo contendo toda memória que depois será divido
struct Memory_list *memlist_create(size_t memory_size)
{
    struct Memory_list *lst = malloc(sizeof(struct Memory_list));

    struct List_node *node = malloc(sizeof(struct List_node));

    node->b_allocated = false;
    node->pid = 0;
    node->start_address = 0;
    node->size = memory_size;

    node->next = node;
    lst->head = node;
    lst->tail = node;
    lst->size = 1;

    last = NULL;
    
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
    uint8_t b_found = false;
    struct List_node *current = NULL;
    
    // Se ainda não fez search, começa pelo primeiro nodo
    if (last == NULL) { last = lst->head; }

    current = last;

    // Procura espaço para alocar
    do
    {
        if (!(current->b_allocated) && process_size <= current->size)
        {
            b_found = true;
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
        struct List_node *free_space = NULL;
        free_space = malloc(sizeof(struct List_node));
        free_space->b_allocated = false;
        free_space->pid = 0;
        free_space->start_address = current->start_address + process_size;
        free_space->size = current->size - process_size;
        free_space->next = current->next;

        current->size = process_size;

        last = free_space;
        current->next = free_space;

        lst->size++;
    }
    current->b_allocated = true;
    current->pid = pid;

    last = current;

    return (long long)current->start_address;
}

// Adiciona nodo por worst-fit
long long memlist_add_worst(struct Memory_list *lst, size_t pid, size_t process_size)
{
    struct List_node *current = lst->head;
    struct List_node *worst = NULL;

    // Procura nodo desde o início da lista até voltar para o começo,
    // mesmo quando encontra um antes do final
    do
    {
        // Se existe um espaço livre maior, salva como worst-fit
        if (!(current->b_allocated) && (worst == NULL || current->size > worst->size))
        {
            worst = current;
        }

        current = current->next;
    }
    while (current != lst->head);

    // Se o worst está nulo ou é menor que o espaço necessário, retorna -1
    if (worst == NULL || (process_size > worst->size)) { return -1; }

    // Se não preencheu exatamente, então divide em dois
    if (process_size < worst->size)
    {
        struct List_node *free_space = NULL;
        free_space = malloc(sizeof(struct List_node));
        free_space->b_allocated = false;
        free_space->start_address = worst->start_address + process_size;
        free_space->size = worst->size - process_size;
        free_space->next = worst->next;

        worst->size = process_size;

        last = free_space;
        worst->next = free_space;

        lst->size++;
    }
    worst->b_allocated = true;
    worst->pid = pid;

    return (long long)(worst->start_address);
}

// Remove nodo independente da estratégia
long long memlist_remove_node(struct Memory_list *lst, size_t pid)
{
    uint8_t b_found = false;
    uint8_t b_circular = false;
    size_t address = 0;

    struct List_node *current = lst->head;
    struct List_node *prev_node = current;

    do
    {
        // Remove apenas se está alocado (pode ser garbage)
        if (current->b_allocated && current->pid == pid)
        {
            b_found = true;
            break;
        }

        prev_node = current;
        current = current->next;
    }
    while (current != lst->head);

    // Se não encontrou, é porque não foi alocado
    if (!b_found) { return -1; }

    // Endereço de início do processo que será retornado no fim
    address = current->start_address;

    if (last != NULL)
    {
        b_circular = true;
    }

    // Se o anterior é livre, junta
    if (prev_node != current && prev_node->b_allocated == false)
    {
        prev_node->size += current->size;
        prev_node->next = current->next;

        // Verifica se last será perdido
        if (current == last)
        {
            last = NULL;
        }

        free(current);
        current = prev_node;
        prev_node = NULL;
        lst->size--;

        // Caso o last tenha sido perdido com esta junção
        if (b_circular && last == NULL)
        {
            last = current;
        }
    }

    current->b_allocated = false;

    // Se o próximo também é livre, junta
    if (current->next != lst->head && current->next->b_allocated == false)
    {
        struct List_node *next_node = NULL;
        next_node = current->next;
        current->size += next_node->size;
        current->next = next_node->next;
        free(next_node);
        lst->size--;
    }

    return (long long)address;
}

// Imprime todos nodos
void memlist_dump(struct Memory_list *lst)
{
    if (lst->size == 0)
    {
        printf("Empty\n");
        return;
    }

    printf("\n[HEAD ");
    struct List_node *current = lst->head;
    do
    {
        char s[21];
        snprintf(s, 21, "%lu", current->pid);
        printf("%lu-%lu: %s%s] -> [",
            current->start_address, (current->start_address + current->size-1),
            current->b_allocated?"P-":"H", current->b_allocated?s:"");

        current = current->next;
    }
    while (current != lst->head);
    printf("HEAD]\n");
}

// Função para imprimir fragmentos de memória livre
void memlist_print(struct Memory_list *lst)
{
    struct List_node *current = lst->head;
    size_t contiguous_free_blocks = 0;

    printf("Frag. Ext.: |");
    uint8_t b_free_blocks = false;
    do
    {
        if (!current->b_allocated)
        {
            b_free_blocks = true;
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
    
    if (!b_free_blocks) { printf("0|"); }
    printf("\n");
}

// Limpa toda a linked list, deixando apenas um nodo do tamanho máximo de memória
void memlist_flush(struct Memory_list *lst, size_t memory_size)
{
    struct List_node *current = NULL;

    while (lst->size > 0)
    {
        current = lst->head;
        lst->head = lst->head->next;
        free(current);
        current = NULL;
        lst->size--;
    }

    struct List_node *node = malloc(sizeof(struct List_node));

    node->b_allocated = false;
    node->start_address = 0;
    node->size = memory_size;

    node->next = node;
    lst->head = node;
    lst->tail = node;
    lst->size = 1;
}

// Deleta todos nodos
void memlist_clear(struct Memory_list *lst)
{
    struct List_node *current = NULL;

    last = NULL;

    while (lst->size > 0)
    {
        current = lst->head;
        lst->head = lst->head->next;
        free(current);
        current = NULL;
        lst->size--;
    }
}