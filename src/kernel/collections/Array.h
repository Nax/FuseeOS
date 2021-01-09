#ifndef ARRAY_H
#define ARRAY_H

#include <cstddef>

void* kmalloc(std::size_t);
void  kfree(void*);

template <typename T>
class Array
{
public:
    Array() : _data{}, _size{}, _capacity{} {}

    ~Array()
    {
        for (std::size_t i = 0; i < _size; ++i)
            (_data + i)->~T();
        kfree(_data);
    }

    std::size_t size() const { return _size; }
    std::size_t capacity() const { return _capacity; }

    T& operator[](std::size_t i)
    {
        return _data[i];
    }

    const T& operator[](std::size_t i) const
    {
        return _data[i];
    }

private:
    T*              _data;
    std::size_t     _size;
    std::size_t     _capacity;
}

#endif
