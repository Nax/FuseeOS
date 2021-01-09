#ifndef ARRAY_H
#define ARRAY_H

#include <cstddef>

void* kmalloc(std::size_t);
void  kfree(void*);

template <typename T>
class Array
{
    Array() : _data{}, _size{}, _capacity{} {}

    ~Array()
    {
        for (std::size_t i = 0; i < _size; ++i)
            (_data + i)->~T();
    }

private:
    T*              _data;
    std::size_t     _size;
    std::size_t     _capacity;
}

#endif
