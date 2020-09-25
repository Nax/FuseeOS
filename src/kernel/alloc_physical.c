#include <kernel/kernel.h>

_ALIGN(0x1000)
static uint8_t bootstrap_pages[0x1000 * 16];

void init_physical_memory(void)
{
    /* Allocate the buddy for every memory zone */
    for (int i = 0; i < PMB_COUNT; ++i)
    {
    }
}

uint64_t alloc_phys(int npages)
{
}

uint64_t alloc_phys_early(int npages)
{
    uint64_t size;
    uint64_t base;

    size = npages * PAGESIZE;
    base = 0;
    for (int i = 0; i < 32; ++i)
    {
        if (gKernel.boot_params.mem_free[i].size >= size)
        {
            base = gKernel.boot_params.mem_free[i].base;
            gKernel.boot_params.mem_free[i].base += size;
            gKernel.boot_params.mem_free[i].size -= size;
            break;
        }
    }

    return base;
}
