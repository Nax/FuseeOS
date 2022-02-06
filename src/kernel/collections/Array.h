#ifndef ARRAY_H
#define ARRAY_H

#ifdef __cplusplus
#include <new>
#include <cstddef>

extern "C" void* kmalloc(std::size_t);
extern "C" void  kfree(void*);

template <typename T>
class Array
{
public:
    Array() : _data{}, _size{}, _capacity{} {}

    /*
    ~Array()
    {
        for (std::size_t i = 0; i < _size; ++i)
            (_data + i)->~T();
        kfree(_data);
    }
    */

    bool        empty() const { return _size == 0; }
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

    T& front()
    {
        return _data[0];
    }

    const T& front() const
    {
        return _data[0];
    }

    T& back()
    {
        return _data[_size - 1];
    }

    const T& back() const
    {
        return _data[_size - 1];
    }

    void push_back(const T& elem)
    {
        if (_size == _capacity)
        {
            realloc(_capacity ? _capacity + _capacity / 2 : 8);
        }
        new (_data + _size) T(elem);
        _size++;
    }

    void pop_back()
    {
        back().~T();
        _size--;
    }

    void push_front(const T& elem)
    {
        if (_size == _capacity)
        {
            realloc(_capacity ? _capacity + _capacity / 2 : 8);
        }
        new (_data + _size) T;
        for (std::size_t i = 0; i < _size; ++i)
        {
            _data[_size - i] = _data[_size - i - 1];
        }
        _data[0] = elem;
        _size++;
    }

    void pop_front()
    {
        for (std::size_t i = 0; i < _size - 1; ++i)
        {
            _data[i] = _data[i + 1];
        }
        back().~T();
        _size--;
    }

    void realloc(std::size_t capacity)
    {
        T* data;

        data = (T*)kmalloc(sizeof(T) * capacity);
        for (std::size_t i = 0; i < _size; ++i)
        {
            data[i] = _data[i];
            (_data + i)->~T();
        }

        kfree(_data);
        _data = data;
        _capacity = capacity;
    }

private:
    T*              _data;
    std::size_t     _size;
    std::size_t     _capacity;
};

#endif

#endif
