#include <string.h>
#include <kernel/common/array.h>
#include <kernel/mem.h>

void array_init(Array* array, size_t elementSize, size_t capacity)
{
    array->size = 0;
    array->capacity = capacity;
    array->data = kmalloc(elementSize * capacity, 0);
}

void* array_emplace(Array* array, size_t elementSize)
{
    size_t index;
    size_t oldCapacity;
    void* oldData;

    if (!(array->size < array->capacity))
    {
        oldCapacity = array->capacity;
        oldData = array->data;
        array->capacity = array->capacity + array->capacity / 2;
        array->data = kmalloc(array->capacity * elementSize, 0);
        memcpy(array->data, oldData, oldCapacity * elementSize);
    }
    index = array->size++;
    return ((char*)array->data) + index * elementSize;
}
