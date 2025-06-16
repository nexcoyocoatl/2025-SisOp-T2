#ifndef _MACRO_DYNARRAY_H_
#define _MACRO_DYNARRAY_H_

// based on https://crocidb.com/post/simple-vector-implementation-in-c/
//    and https://www.youtube.com/watch?v=HvG03MY2H04

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
    do\
    { \
        size_t initial_capacity = 8; \
        struct dynarray_header *header = malloc((sizeof(*header)) + (size_t)(initial_capacity*sizeof(DA))); \
        header->m_capacity = initial_capacity; \
        header->m_size = 0; \
        DA = (void*)(header + 1); \
    } while(0)

// Para chegar no header, faz cast da array para o tipo do header e subtrai 1
#define dynarray_get_header(DA) \
    (((struct dynarray_header*)(DA)) - 1)

// Retorna tamanho através do header
#define dynarray_size(DA) \
    ((DA)? dynarray_get_header(DA)->m_size : 0)

// Retorna capacidade através do header
#define dynarray_capacity(DA) \
    ((DA)? dynarray_get_header(DA)->m_capacity : 0)

// Põe um novo elemento no final da lista, mudando capacidade com realloc se necessário
#define dynarray_push(DA, E) \
    do \
    { \
        if ( (dynarray_get_header(DA)->m_size << 1) > dynarray_get_header(DA)->m_capacity)\
            { \
                size_t new_capacity = dynarray_get_header(DA)->m_capacity << 1; \
                dynarray_resize(DA, new_capacity); \
            } \
        DA[dynarray_size(DA)] = E; \
        dynarray_get_header(DA)->m_size++; \
    } \
    while (0)

// Retira elemento do final da lista
#define dynarray_pop(DA) \
    do { \
        if (dynarray_get_header(DA)->m_size) \
        { \
            if ( (dynarray_get_header(DA)->m_size) < ((dynarray_get_header(DA)->m_capacity) >> 1) ) \
            { \
                size_t new_capacity = dynarray_get_header(DA)->m_capacity >> 1; \
                dynarray_resize(DA, new_capacity); \
            } \
            dynarray_get_header(DA)->m_size--;\
        } \
    } \
    while (0)

// out recebe elemento no final da lista
#define dynarray_get_last(DA, out) { out = DA[dynarray_size(DA)-1]; }

#define dynarray_resize(DA, required_size) \
    do   { \
        struct dynarray_header *header = dynarray_get_header(DA); \
        header->m_capacity = required_size; \
        header = (struct dynarray_header*)realloc(header, (sizeof *header) + (size_t)(header->m_capacity * sizeof(DA))); \
        header->m_capacity = required_size; \
        DA = (void*)(header + 1); \
    } \
    while(0)

// Limpa lista
#define dynarray_free(DA) \
    do { \
        if (DA) { \
            memset(DA, 0, (size_t)dynarray_size(DA) * (sizeof *(DA))); \
            free(dynarray_get_header(DA));\
            (DA) = NULL;\
        } \
    } \
    while (0)

#endif