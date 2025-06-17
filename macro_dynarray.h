#ifndef _MACRO_DYNARRAY_H_
#define _MACRO_DYNARRAY_H_

// baseado em https://crocidb.com/post/simple-vector-implementation-in-c/
//    e https://www.youtube.com/watch?v=HvG03MY2H04

#include <stdlib.h>

struct dynarray_header
{
    size_t m_capacity;
    size_t m_size;
};

#include <string.h>
#include "macro_dynarray.h"

// Criação de uma array dinâmica (ex: dynarray(int a) -> int a*)
#define dynarray(T) T*

// Inicializa array dinâmica
#define dynarray_init(DA) \
    { \
        size_t initial_capacity = 8; \
        struct dynarray_header *header = malloc((sizeof(*header)) + (size_t)(initial_capacity*sizeof(DA))); \
        header->m_capacity = initial_capacity; \
        header->m_size = 0; \
        DA = (void*)(header + 1); \
    }

// Para chegar no header, faz cast da array para o tipo do header e subtrai 1
#define dynarray_get_header(DA) \
    (((struct dynarray_header*)(DA)) - 1)

// Retorna tamanho através do header
#define dynarray_size(DA) \
    ((DA)? dynarray_get_header(DA)->m_size : 0)

// Retorna capacidade através do header
#define dynarray_capacity(DA) \
    ((DA)? dynarray_get_header(DA)->m_capacity : 0)

#define dynarray_resize(DA, required_size) \
    { \
        struct dynarray_header *header = dynarray_get_header(DA); \
        header->m_capacity = required_size; \
        header = (struct dynarray_header*)realloc(header, (sizeof *header) + (size_t)(header->m_capacity * sizeof(DA))); \
        header->m_capacity = required_size; \
        DA = (void*)(header + 1); \
    }

#define dynarray_shift_left(DA) \
    { \
        for (size_t i = 0; i < dynarray_size(DA) - 1; i++) { DA[i] = DA[i+1]; } \
        dynarray_get_header(DA)->m_size--; \
    }

// Põe um novo elemento no final da lista, mudando capacidade com realloc se necessário
#define dynarray_push(DA, E) \
    { \
        if ( (dynarray_get_header(DA)->m_size << 1) > dynarray_get_header(DA)->m_capacity)\
            { \
                size_t new_capacity = dynarray_get_header(DA)->m_capacity << 1; \
                dynarray_resize(DA, new_capacity); \
            } \
        DA[dynarray_size(DA)] = E; \
        dynarray_get_header(DA)->m_size++; \
    }

// Remove elemento final da lista
#define dynarray_remove_last(DA) \
    { \
        if (dynarray_get_header(DA)->m_size) \
        { \
            if ( (dynarray_get_header(DA)->m_size) < ((dynarray_get_header(DA)->m_capacity) >> 1) ) \
            { \
                size_t new_capacity = dynarray_get_header(DA)->m_capacity >> 1; \
                dynarray_resize(DA, new_capacity); \
            } \
            dynarray_get_header(DA)->m_size--;\
        } \
    }

// Retira elemento do final da lista
#define dynarray_pop(DA, out) \
    { out = DA[dynarray_size(DA)-1]; dynarray_remove_last(DA); }

#define dynarray_enqueue(DA, E) \
    dynarray_push(DA, E)
    
#define dynarray_dequeue(DA, out) \
    { out = DA[0]; dynarray_shift_left(DA); }

// out recebe elemento inicial da lista
#define dynarray_get_first(DA, out) \
    out = DA[0]

// out recebe elemento final da lista
#define dynarray_get_last(DA, out) \
    out = DA[dynarray_size(DA)-1]

// Limpa lista
#define dynarray_free(DA) \
    { \
        if (DA) { \
            memset(DA, 0, (size_t)dynarray_size(DA) * (sizeof *(DA))); \
            free(dynarray_get_header(DA));\
            (DA) = NULL;\
        } \
    }

#endif