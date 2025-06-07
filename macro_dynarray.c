// based on https://crocidb.com/post/simple-vector-implementation-in-c/
//    and https://www.youtube.com/watch?v=HvG03MY2H04

#include <stdlib.h>
#include <string.h>

struct dynarray_header
{
    size_t m_capacity;
    size_t m_size;
};

#define dynarray(T) T*

#define dynarray_init(DA) do    { \
                                    size_t initial_capacity = 8; \
                                    struct dynarray_header *header = malloc((sizeof(*header)) + (size_t)(initial_capacity*sizeof(DA))); \
                                    header->m_capacity = initial_capacity; \
                                    header->m_size = 0; \
                                    DA = (void*)(header + 1); \
                                } while(0)

#define dynarray_get_header(DA) (((struct dynarray_header*)(DA)) - 1)

#define dynarray_size(DA) ((DA)? dynarray_get_header(DA)->m_size : 0)

#define dynarray_capacity(DA) ((DA)? dynarray_get_header(DA)->m_capacity : 0)

#define dynarray_push(DA, E) do { \
                                    if ( (dynarray_get_header(DA)->m_size << 1) > dynarray_get_header(DA)->m_capacity)\
                                        { \
                                            size_t new_capacity = dynarray_get_header(DA)->m_capacity << 1; \
                                            dynarray_resize(DA, new_capacity); \
                                        } \
                                    DA[dynarray_size(DA)] = E; \
                                    dynarray_get_header(DA)->m_size++; \
                                } \
                                while (0)

#define dynarray_pop(DA) do { \
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

#define dynarray_insert(DA, index) do   { \
                                            \
                                        } \
                                        while (0)

#define dynarray_shuffle_right_from(DA, index) do   { \
                                                        \
                                                    } \
                                                    while(0)

#define dynarray_del(DA, index) do { \
                                        \
                                    } \
                                    while (0)

#define dynarray_remove_head() do   { \
                                        \
                                    } \
                                    while (0)

#define dynarray_shuffle_left_from(DA, index) do    { \
                                                        \
                                                    } \
                                                    while(0)

#define dynarray_resize(DA, required_size) do   { \
                                                    struct dynarray_header *header = dynarray_get_header(DA); \
                                                    header->m_capacity = required_size; \
                                                    header = (struct dynarray_header*)realloc(header, (sizeof *header) + (size_t)(header->m_capacity * sizeof(DA))); \
                                                    header->m_capacity = required_size; \
                                                    DA = (void*)(header + 1); \
                                                } \
                                                while(0)

#define dynarray_free(DA) do { \
                                if (DA) { \
                                    memset(DA, 0, (size_t)dynarray_size(DA) * (sizeof *(DA))); \
                                    free(dynarray_get_header(DA));\
                                    (DA) = NULL;\
                                } \
                            } \
                            while (0)
                          
#define dynarray_free_all(DA, F) do { if (DA) { \
                                        for (size_t i = 0; i < dynarray_size(DA); i++) \
                                        { \
                                            F((DA)[i]); \
                                        } \
                                        memset(DA, 0, (size_t)dynarray_size(DA) * (sizeof *(DA))); \
                                        free(dynarray_get_header(DA)); (DA) = NULL; } \
                                } \
                                while (0)
