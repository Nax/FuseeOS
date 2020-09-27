#include <kernel/kernel.h>

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

/*
 * Various paging actions are available:
 *
 * Touch   - Set flags on the whole hierarchy
 * Protect - Set flags on pages only
 * Map     - Map contiguous physical pages
 * MapAnon - Map new pages
 * Unmap   - Unmap pages
 */

enum class AlterMode
{
    Touch   = 0,
    Protect = 1,
    Map     = 2,
    MapAnon = 3,
    Unmap   = 4,
};

template <AlterMode mode, int order>
struct PageAlterator
{
    static void run(uint64_t* dir, uint64_t dir_base, uint64_t virt_start, uint64_t phys_start, uint64_t npages, uint64_t flags)
    {
        uint64_t tmp;
        uint64_t virt_end;
        uint64_t dir_end;
        uint64_t order_size;
        int      first;
        int      last;

        virt_end   = virt_start + npages * PAGESIZE - 1;
        order_size = ((uint64_t)PAGESIZE << (9 * order));
        dir_end    = dir_base + 512 * order_size - 1;
        first      = (dir_base >= virt_start) ? 0 : ((((virt_start - dir_base) / PAGESIZE) >> (9 * order)) & 0x1ff);
        last       = 512 - ((virt_end >= dir_end) ? 0 : (((dir_end - virt_end) / PAGESIZE) >> (9 * order)) & 0x1ff);

        for (int i = first; i < last; ++i)
        {
            tmp = dir[i];
            if ((order == 0 && mode == AlterMode::Protect) || mode == AlterMode::Touch)
            {
                tmp &= ~MMASK_PROTECT;
                tmp |= flags;
                dir[i] = tmp;
            }

            if (order)
            {
                PageAlterator<mode, order - 1>::run((uint64_t*)physical_to_virtual(tmp & MMASK_PHYS), sext48(dir_base + i * order_size), virt_start, phys_start, npages, flags);
            }
        }
    }
};

template <AlterMode mode>
struct PageAlterator<mode, -1>
{
    static void run(uint64_t* dir, uint64_t dir_base, uint64_t virt_start, uint64_t phys_start, uint64_t npages, uint64_t flags)
    {
    }
};

template <AlterMode mode>
void alter_pages(void* ptr, uint64_t phys, size_t size, int prot)
{
    uint64_t flags;

    if (!size)
        return;

    flags = 0;

    if (prot & MM_WRITE)
        flags |= 0x2;
    if (prot & MM_USER)
        flags |= 0x4;
    if (!(prot & MM_EXECUTE))
        flags |= gKernel.nx_mask;

    PageAlterator<mode, 3>::run(gKernel.cr3, 0, (uint64_t)ptr, phys, (size + PAGESIZE - 1) / PAGESIZE, flags);
}

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

void kmtouch(void* ptr, size_t size, int prot)
{
    alter_pages<AlterMode::Touch>(ptr, 0, size, prot);
}

void kmprotect(void* ptr, size_t size, int prot)
{
    alter_pages<AlterMode::Protect>(ptr, 0, size, prot);
}

void kmprotect_kernel(void)
{
    kmtouch(&__KERNEL_IMAGE_START, &__KERNEL_IMAGE_END - &__KERNEL_IMAGE_START, MM_READ | MM_WRITE | MM_EXECUTE);
    kmprotect(&__KERNEL_SECTION_EXEC_START, &__KERNEL_SECTION_EXEC_END - &__KERNEL_SECTION_EXEC_START, MM_READ | MM_EXECUTE);
    kmprotect(&__KERNEL_SECTION_RODATA_START, &__KERNEL_SECTION_RODATA_END - &__KERNEL_SECTION_RODATA_START, MM_READ);
    kmprotect(&__KERNEL_SECTION_DATA_START, &__KERNEL_SECTION_DATA_END - &__KERNEL_SECTION_DATA_START, MM_READ | MM_WRITE);
    kmprotect(&__KERNEL_SECTION_BSS_START, &__KERNEL_SECTION_BSS_END - &__KERNEL_SECTION_BSS_START, MM_READ | MM_WRITE);
}
