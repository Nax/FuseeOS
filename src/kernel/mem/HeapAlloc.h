#ifndef HEAP_ALLOC_H
#define HEAP_ALLOC_H

#include <cstddef>
#include <cstdint>

class HeapAlloc
{
public:
    void* alloc(std::size_t size);
    void  free(void* addr);

private:
    void* make_new_block(std::size_t size);
    void  shrink();

    struct alignas(32) Block
    {
        bool        used : 1;
        std::size_t size : 63;
    };

    std::size_t _size;
};

#endif
