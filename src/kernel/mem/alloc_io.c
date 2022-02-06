#include <kernel/kernel.h>
#include <kernel/mem.h>

#define IO_BASE ((void*)0xfffffff800000000)

void* io_alloc(size_t size)
{
    void* tmp;

    size = page_round(size);
    tmp = (char*)IO_BASE + gKernel.io_size;
    gKernel.io_size += size;

    return tmp;
}

void io_free(void* addr)
{

}

