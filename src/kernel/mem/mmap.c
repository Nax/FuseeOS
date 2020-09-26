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
    return (void*)(0xffff800000000000 | physical);
}

static uint64_t sext48(uint64_t v)
{
    uint64_t mask;

    mask = v & 0x0000800000000000;
    mask |= (mask << 1);
    mask |= (mask << 2);
    mask |= (mask << 4);
    mask |= (mask << 8);
    mask |= (mask << 16);
    return v | mask;
}

static void _kmprotect(uint64_t* dir, uint64_t dir_base, uint64_t addr_start, uint64_t addr_end, int order, uint64_t flags)
{
    uint64_t tmp;
    uint64_t dir_end;
    uint64_t order_size;
    int      first;
    int      last;

    order_size = ((uint64_t)PAGESIZE << (9 * order));
    dir_end    = dir_base + 512 * order_size - 1;
    first      = (dir_base >= addr_start) ? 0 : ((((addr_start - dir_base) / PAGESIZE) >> (9 * order)) & 0x1ff);
    last       = 512 - ((addr_end >= dir_end) ? 0 : (((dir_end - addr_end) / PAGESIZE) >> (9 * order)) & 0x1ff);

    for (int i = first; i < last; ++i)
    {
        tmp = dir[i];

        if (order)
        {
            tmp |= 0x2;
            dir[i] = tmp;
            tmp &= MMASK_PHYS;
            _kmprotect(physical_to_virtual(tmp), sext48(dir_base + i * order_size), addr_start, addr_end, order - 1, flags);
        }
        else
        {
            tmp &= ~MMASK_PROTECT;
            tmp |= flags;
            dir[i] = tmp;
        }
    }
}

void kmprotect(void* ptr, size_t size, int prot)
{
    uint64_t flags;

    if (!size)
        return;

    flags = 0;

    if (prot & MM_WRITE)
        flags |= 0x2;
    if (prot & MM_USER)
        flags |= 0x4;

    _kmprotect(gKernel.cr3, 0, (uint64_t)ptr, (uint64_t)ptr + size - 1, 3, flags);
}

void kmprotect_kernel(void)
{
    kmprotect(&__KERNEL_SECTION_EXEC_START, &__KERNEL_SECTION_EXEC_END - &__KERNEL_SECTION_EXEC_START, MM_READ | MM_EXECUTE);
    kmprotect(&__KERNEL_SECTION_DATA_START, &__KERNEL_SECTION_DATA_END - &__KERNEL_SECTION_DATA_START, MM_READ | MM_WRITE);
    kmprotect(&__KERNEL_SECTION_RODATA_START, &__KERNEL_SECTION_RODATA_END - &__KERNEL_SECTION_RODATA_START, MM_READ);
}
