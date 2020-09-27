#include <kernel/kernel.h>

void init_virtual_memory(void)
{
    VirtualMemoryAllocator* alloc = &gKernel.vmem;

    alloc->base = 0xfffffff000000000;
}

void* alloc_virtual(uint64_t size)
{
    VirtualMemoryAllocator* alloc = &gKernel.vmem;
    uint64_t                tmp;

    tmp = alloc->base;
    alloc->base += size;
    return (void*)tmp;
}
