#ifndef IO_ALLOC_H
#define IO_ALLOC_H

#ifdef __cplusplus
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
#else
typedef struct IOAlloc
{
    size_t size;
} IOAlloc;
#endif
#endif
