#include <kernel/arch.h>
#include <kernel/kernel.h>

static void init_nx(void)
{
    uint64_t info[4];
    uint64_t tmp;

    cpuid(info, 0x80000000);
    if (info[0] < 0x80000001)
        return;
    cpuid(info, 0x80000001);
    if (info[3] & (1 << 20))
    {
        /* Set the NX flag */
        gKernel.nx_mask = 0x8000000000000000;

        /* Enable the feature in EFER */
        tmp = rdmsr(MSR_IA32_EFER);
        tmp |= (1 << 11);
        wrmsr(MSR_IA32_EFER, tmp);
    }
}

static void unmap_lomem(void)
{
    /* Free all lomem paging info */
    kmunmap_tree(nullptr, 0x100000000);

    /* Reclaim the 0x80 pages of low-memory */
    for (unsigned i = 0; i < 0x80; ++i)
        free_phys_pages(i * PAGESIZE, 1);
}

void init_mem(void)
{
    /* Initialize the memory subsystems */
    init_physical_mapping();
    init_physical_memory();
    init_virtual_memory();

    /* Detect availability of the NX flag */
    init_nx();

    /* Remap the kernel */
    kmprotect_kernel();

    /* Copy the initram */
    gKernel.initram_size = gKernel.boot_params.initram_size;
    gKernel.initram      = (char*)kmmap(nullptr, 0, gKernel.initram_size, KPROT_READ, KMAP_ANONYMOUS);
    memcpy(gKernel.initram, gKernel.boot_params.initram, gKernel.initram_size);

    /* Free the original initram */
    free_phys((uint64_t)gKernel.boot_params.initram, gKernel.initram_size);

    /* Reclaim low-memory */
    unmap_lomem();
}
