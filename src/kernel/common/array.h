#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>

typedef struct
{
    void*       data;
    size_t      size;
    size_t      capacity;
} Array;

void  array_init(Array* array, size_t elementSize, size_t capacity);
void* array_emplace(Array* array, size_t elementSize);

#define ARRAY_INIT(a, type, cap)    array_init(a, sizeof(type), cap)
#define ARRAY_AT(a, type, index)    (((type*)((a)->data)) + index)
#define ARRAY_BACK(a, type)         ARRAY_AT(a, type, (a)->size)
#define ARRAY_EMPLACE(a, type)      ((type*)array_emplace((a), sizeof(type)))
#define ARRAY_POP(a)                ((void)((a)->size--))
#define ARRAY_EMPTY(a)              (!(a)->size)

#endif
