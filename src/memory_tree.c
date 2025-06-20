#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "instruction.h"
#include "memory_tree.h"
#include "macro_dynarray.h"

// Cria nova árvore de espaços de memória
// com um nodo contendo toda memória que depois será subdivido
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

// Função de subdivisão
// Se um nodo não está sendo utilizado e não tem filhos, poderá ser subdividido
int memtree_subdivide(struct Tree_node *node)
{
    if (!(node->b_is_leaf) || node->b_allocated) { return 0; }

    // Tamanho dos filhos
    size_t new_size = node->size/2;

    // Se a unidade não pode ser subdividida (eg. 1/2), retorna sem sucesso
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

    return 1;
}

// Função de coalescer nodos
int memtree_coalesce_node(struct Tree_node *node)
{
    // Retorna sem sucesso se pai ou algum dos filhos não é folha ou se estão alocados
    if ( node->b_is_leaf || !(node->child_left->b_is_leaf) || !(node->child_right->b_is_leaf) 
    || node->child_left->b_allocated || node->child_right->b_allocated) { return 0; }

    free(node->child_left);
    free(node->child_right);
    
    node->child_left = NULL;
    node->child_right = NULL;

    node->b_is_leaf = 1;

    return 1;
}

// Busca id por DFS, da esquerda para a direita
struct Tree_node *memtree_find_node_by_pid(struct Memory_tree *tree, size_t pid, size_t proc_size)
{
    struct Tree_node *current = NULL;

    // Funções para criação de uma array dinâmica por macro
    dynarray(struct Tree_node *) stack;
    dynarray_init(stack);

    dynarray_push(stack, tree->root);
    
    while (dynarray_size(stack) > 0)
    {
        dynarray_pop(stack, current);

        // Só entra nos filhos se o processo procurado cabe no tamanho de um deles
        if (current->size/2 >= proc_size)
        {
            if (current->child_right != NULL) { dynarray_push(stack, current->child_right); }
            if (current->child_left != NULL) { dynarray_push(stack, current->child_left); }
        }

        if (current->b_allocated && current->pid == pid)
        {
            dynarray_free(stack);
            return current;
        }
    }

    dynarray_free(stack);

    return NULL;
}

// Procura nodo livre existente por BFS, mas se necessita subdividir, usa DFS
long long memtree_add_buddy(struct Memory_tree *tree, size_t pid, size_t process_size)
{
    struct Tree_node *current = NULL;

    dynarray(struct Tree_node *) queue;                 // Queue para BFS
    dynarray(struct Tree_node *) subdivide_stack;       // Stack para DFS
    dynarray_init(queue);
    dynarray_init(subdivide_stack);

    dynarray_enqueue(queue, tree->root);

    // Enquanto as duas arrays não forem esgotadas
    while (dynarray_size(queue) > 0 || dynarray_size(subdivide_stack) > 0)
    {
        // Se ainda tem nodos existentes livres
        if (dynarray_size(queue) > 0)
        {
            dynarray_dequeue(queue, current);
        }
        // Se não, começa a retirar do stack
        else
        {
            dynarray_pop(subdivide_stack, current);
        }        

        // Só entra no nodo se tem espaço livre
        if ( !(current->b_allocated) )
        {
            // Só entra nos filhos se cabe neles
            if ( (current->size)/2 >= process_size )
            {
                // Se pode ser subdividido, guarda para depois
                if (current->b_is_leaf) { dynarray_push(subdivide_stack, current); }
                // Se não, tem filhos, então adiciona os dois
                else
                {
                    if (current->child_left != NULL) { dynarray_enqueue(queue, current->child_left); }
                    if (current->child_right != NULL) { dynarray_enqueue(queue, current->child_right); }
                }
            }

            // Se achar um nodo livre de tamanho >= processo e que seus filhos < processo, insere
            if (!(current->b_allocated) && current->b_is_leaf && process_size <= current->size && process_size > current->size/2)
            {
                dynarray_free(subdivide_stack);
                dynarray_free(queue);

                current->pid = pid;
                current->b_allocated = ALLOC;
                current->occupied_size = process_size;
                return current->start_address;
            }
        }

        // Se não encontrou espaço ainda, mas tem nodos que podem ser subdivididos,
        // subdivide e adiciona filhos ao stack
        if ( (dynarray_size(queue) == 0) && (dynarray_size(subdivide_stack) > 0) )
        {
            dynarray_pop(subdivide_stack, current);
                
            memtree_subdivide(current);
            if (current->child_right != NULL) { dynarray_push(subdivide_stack, current->child_right); }
            if (current->child_left != NULL) { dynarray_push(subdivide_stack, current->child_left); }
        }
    }    

    dynarray_free(subdivide_stack);
    dynarray_free(queue);

    return -1;
}

// Remove nodo e tenta coalescer a cada passo, subindo dele até o pai
long long memtree_remove_node(struct Memory_tree *tree, size_t pid, size_t process_size)
{
    struct Tree_node *node = memtree_find_node_by_pid(tree, pid, process_size);

    if (node == NULL) { return -1; }

    size_t address = node->start_address;

    if (node->b_allocated = DISALLOC) { return -1; }

    node->b_allocated = DISALLOC;
    node->occupied_size = 0;

    while (node != tree->root)
    {
        node = node->parent;

        memtree_coalesce_node(node);
    }
    
    return (long long)address;
}

// Função de procura BFS para imprimir nodos da árvore
void memtree_dump(struct Memory_tree *tree)
{
    struct Tree_node *current = NULL;

    dynarray(struct Tree_node *) queue;
    dynarray_init(queue);

    dynarray_enqueue(queue, tree->root);

    size_t temp_size = tree->root->size;

    printf("\n");
    while (dynarray_size(queue) > 0)
    {
        dynarray_dequeue(queue, current);

        if (temp_size != current->size)
        {
            temp_size = current->size;
            printf("\n");
        }

        if ( !(current->b_is_leaf) )
        {
            dynarray_enqueue(queue, current->child_left);
            dynarray_enqueue(queue, current->child_right);
        }

        char s[21];
        char s2[30];
        snprintf(s, 21, "-%lld", current->pid);
        snprintf(s2, 30, "%s%s%s", current->b_allocated?": P":": H", current->b_allocated?s:"", current->b_is_leaf?" LEAF":"");
        printf("(%lu-%lu%s) ",
            current->start_address, (current->start_address + current->size-1),
            current->b_is_leaf?s2:"");
    }
    printf("\n");

    dynarray_free(queue);
}

// Função para imprimir fragmentos de memória livre
void memtree_print(struct Memory_tree *tree)
{
    struct Tree_node *current = NULL;
    size_t contiguous_free_blocks = 0;

    dynarray(struct Tree_node *) stack;
    dynarray(struct Tree_node *) external_frag_print_queue;
    dynarray(size_t) internal_frag_print_queue;
    dynarray_init(stack);
    dynarray_init(external_frag_print_queue);
    dynarray_init(internal_frag_print_queue);

    dynarray_push(stack, tree->root);

    size_t temp_size = tree->root->size;

    // Insere folhas em ordem por DFS
    while (dynarray_size(stack) > 0)
    {
        dynarray_pop(stack, current);

        if ( !(current->b_is_leaf) )
        {
            dynarray_push(stack, current->child_right);
            dynarray_push(stack, current->child_left);
        }
        else
        {
            dynarray_enqueue(external_frag_print_queue, current);
        }
    }

    dynarray_free(stack);

    // Imprime fragmentação externa
    printf("Frag. Ext.: |");
    uint8_t b_free_blocks = 0;
    while (dynarray_size(external_frag_print_queue) > 0)
    {
        dynarray_dequeue(external_frag_print_queue, current);

        if (!(current->b_allocated))
        {
            b_free_blocks = 1;
            contiguous_free_blocks += current->size;
        }
        else
        {
            if (current->size > current->occupied_size)
            {
                dynarray_enqueue(internal_frag_print_queue, (current->size - current->occupied_size));
            }

            if (contiguous_free_blocks > 0)
                { printf("%lu|", contiguous_free_blocks); }

            contiguous_free_blocks = 0;
        }        
    }
    if (contiguous_free_blocks > 0)
        { printf("%lu|", contiguous_free_blocks); }
    
    if (!b_free_blocks) { printf("0|"); }

    printf("\n");

    printf("Frag. Int.: |");
    if (dynarray_size(internal_frag_print_queue) == 0) { printf("0|"); }
    for (size_t i = 0; i < dynarray_size(internal_frag_print_queue); i++)
    {
        printf("%lu|", internal_frag_print_queue[i]);
    }
    printf("\n");

    dynarray_free(external_frag_print_queue);
    dynarray_free(internal_frag_print_queue);
}

// Limpa todos nodos por DFS
void memtree_clear(struct Memory_tree *tree)
{
    struct Tree_node *current = NULL;

    dynarray(struct Tree_node *) stack;
    dynarray_init(stack);

    dynarray_push(stack, tree->root);
    
    while (dynarray_size(stack) > 0)
    {
        dynarray_pop(stack, current);

        if ( !(current->b_is_leaf) )
        {
            if (current->child_right != NULL) { dynarray_push(stack, current->child_right); }
            if (current->child_left != NULL) { dynarray_push(stack, current->child_left); }
        }

        free(current);
    }

    dynarray_free(stack);
}