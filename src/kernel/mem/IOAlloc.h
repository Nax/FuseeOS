#ifndef IO_ALLOC_H
#define IO_ALLOC_H

#include <cstddef>
#include <cstdint>

class IOAlloc
{
public:
    void* alloc(std::size_t size);
    void  free(void* addr);

private:
    std::size_t _size;
};

#endif
