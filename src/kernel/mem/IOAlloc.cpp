#include <kernel/mem.h>
#include <kernel/mem/IOAlloc.h>

#define IO_BASE ((void*)0xfffffff800000000)

void* IOAlloc::alloc(std::size_t size)
{
    void* tmp;

    size = page_round(size);
    tmp = (char*)IO_BASE + _size;
    _size += size;

    return tmp;
}

void IOAlloc::free(void* addr)
{

}

