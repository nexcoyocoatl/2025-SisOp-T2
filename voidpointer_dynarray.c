#include <stdlib.h>
#include <stdio.h>

#define DYNARRAY_INIT_SIZE 4

#define DYNARRAY_INIT(da) dynarray da;\
                          init(&da)

typedef struct
{
    void **m_items;
    size_t m_size;
    size_t m_capacity;
} s_dynarraylist;

typedef struct s_dynarray dynarray;
struct s_dynarray
{
    s_dynarraylist dynarray_list;
    size_t (*size) (dynarray *);
    void (*resize) (dynarray *, size_t);
    void (*add) (dynarray *, void *);
    void (*set) (dynarray *, size_t, void *);
    void *(*get) (dynarray *, size_t);
    void (*del) (dynarray *, size_t);
    void (*free) (dynarray *);
};

size_t dynarray_size(dynarray *da)
{
    if (da)
    {
        return da->dynarray_list.m_size;
    }

    return 0;
}

void dynarray_resize(dynarray *da, size_t capacity)
{
    if (da)
    {
        void **items = realloc(da->dynarray_list.m_items, sizeof(void *) * capacity);
        if (items)
        {
            da->dynarray_list.m_items = items;
            da->dynarray_list.m_capacity = capacity;
        }
    }
}

void dynarray_add(dynarray *da, void *item)
{
    if (da)
    {
        if (da->dynarray_list.m_capacity == da->dynarray_list.m_size)
        {
            dynarray_resize( da, da->dynarray_list.m_capacity <<= 1 );
        }

        da->dynarray_list.m_items[da->dynarray_list.m_size++] = item;
    }
}

void dynarray_set(dynarray *da, size_t index, void *item)
{
    if (da)
    {
        if ( (index >= 0) && (index < da->dynarray_list.m_size) )
        {
            da->dynarray_list.m_items[index] = item;
        }
    }
}

void *dynarray_get(dynarray *da, size_t index)
{
    void *data = NULL;

    if (da)
    {
        if ( (index>=0) && (index < da->dynarray_list.m_size) )
        {
            data = da->dynarray_list.m_items[index];
        }
    }

    return data;
}

void dynarray_del(dynarray *da, size_t index)
{
    size_t i = 0;
    if(da)
    {
        if ( (index < 0) || (index >= da->dynarray_list.m_size) )
        {
            return;
        }

        da->dynarray_list.m_items[index] = NULL;

        for (i = index; (i < da->dynarray_list.m_size - 1); i++)
        {
            da->dynarray_list.m_items[i] = da->dynarray_list.m_items[i+1];
        }
        da->dynarray_list.m_items[da->dynarray_list.m_size] = NULL;

        da->dynarray_list.m_size--;

        if ( (da->dynarray_list.m_size > 0) && ((da->dynarray_list.m_size) == ((da->dynarray_list.m_capacity) >>= 2 )) )
        {
            dynarray_resize(da, da->dynarray_list.m_capacity >> 1);
        }
    }
}

void dynarray_free(dynarray *da)
{
    if (da)
    {
        free(da->dynarray_list.m_items);
        da->dynarray_list.m_items = NULL;
    }
}

void init(dynarray *da)
{
    da->size = dynarray_size;
    da->resize = dynarray_resize;
    da->add = dynarray_add;
    da->set = dynarray_set;
    da->get = dynarray_get;
    da->del = dynarray_del;
    da->free = dynarray_free;

    da->dynarray_list.m_capacity = DYNARRAY_INIT_SIZE;
    da->dynarray_list.m_size = 0;
    da->dynarray_list.m_items = malloc( sizeof(void *) * da->dynarray_list.m_capacity );

    if (!da->dynarray_list.m_items)
    {
        exit(EXIT_FAILURE);
    }
}

void test()
{
    DYNARRAY_INIT(da);

    da.add(&da,"test");
    int value = 1;
    da.add(&da, &value);
    da.add(&da, "a");
    // da.add(&da, (void*) 3); // breaks, needs to copy the int value to void* inside the add function, or else it points to a null value

    // Print elements through casting
    for (size_t i = 0; i < da.size(&da); i++)
    {
        if (i == 1 || i == 3)
        {
            printf("%d\n", *((int*)da.get(&da, i)));

            continue;
        }
        printf("%s\n", (char*)da.get(&da, i));
    }
}

int main(int argc, char *argv[])
{
    test();

    return 0;
}