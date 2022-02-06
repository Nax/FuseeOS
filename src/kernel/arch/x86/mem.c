#include <kernel/mem.h>
#include <kernel/kernel.h>
#include <kernel/print.h>
#include <libboot/libboot.h>

#define PAGE_PRESENT    0x01
#define PAGE_WRITE      0x02
#define PAGE_USER       0x04
#define PAGE_LARGE      0x80

static int is_1gb_pages_supported(void)
{
    uint64_t v[4];

    cpuid(v, 0x80000001);
    return !!(v[3] & (1 << 26));
}

/*
 * x86_64 paging levels:
 *  - PML4
 *  - PDP
 *  - PD
 *  - PT
 */
void arch_init_physical_mapping(void)
{
    uint64_t  pdp;
    uint64_t* vpdp;
    uint64_t  pd;
    uint64_t* vpd;

    gKernel.cr3 = (uint64_t*)gBootParams.cr3;
    pdp         = alloc_phys_early(1);
    vpdp        = (uint64_t*)pdp;

    kprintf("Physical Mapping Page Size: ");
    if (is_1gb_pages_supported())
    {
        /* Map 512 x 1GB pages */
        kprintf("1GiB\n");
        for (int i = 0; i < 512; ++i)
        {
            vpdp[i] = (i * 0x40000000ULL) | PAGE_PRESENT | PAGE_WRITE | PAGE_LARGE;
        }
    }
    else
    {
        /* Map 512 * 512 * 2 MB pages */
        kprintf("2MiB\n");
        for (int i = 0; i < 512; ++i)
        {
            pd = alloc_phys_early(1);
            vpd = (uint64_t*)pd;
            for (int j = 0; j < 512; ++j)
            {
                vpd[j] = ((i * 512 + j) * 0x200000ULL) | PAGE_PRESENT | PAGE_WRITE | PAGE_LARGE;
            }
            vpdp[i] = pd | PAGE_PRESENT | PAGE_WRITE;
        }
    }
    gKernel.cr3[PML4_PHYSICAL] = pdp | PAGE_PRESENT | PAGE_WRITE;
    __asm__ __volatile__("mov %0, %%cr3\r\n" ::"a"(gKernel.cr3));
}
