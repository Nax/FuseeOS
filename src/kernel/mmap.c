#include <kernel/kernel.h>

/*
 * x86_64 paging levels:
 *  - PML4
 *  - PDP
 *  - PD
 *  - PT
 */

void init_physical_mapping(void)
{
    uint64_t  pdp;
    uint64_t* vpdp;

    gKernel.cr3 = (uint64_t*)gKernel.boot_params.cr3;
    pdp         = alloc_phys_early(1);
    vpdp        = (uint64_t*)pdp;

    for (int i = 0; i < 512; ++i)
    {
        vpdp[i] = (i * 0x40000000LL) | 0x83;
    }
    gKernel.cr3[PML4_PHYSICAL] = pdp | 0x03;
    __asm__ __volatile__("mov %0, %%cr3\r\n" ::"a"(gKernel.cr3));
}

void* physical_to_virtual(uint64_t physical)
{
    uint64_t v;

    return (void*)(0xffff800000000000 | physical);
}
