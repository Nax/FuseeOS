#include <kernel/arch.h>
#include <kernel/kernel.h>

static void init_nx(void)
{
    uint64_t info[4];
    uint64_t tmp;

    cpuid(info, 0x80000000);
    if (info[0] < 0x80000001) return;
    cpuid(info, 0x80000001);
    if (info[3] & (1 << 20))
    {
        /* Set the NX flag */
        gKernel.nx_mask = 0x8000000000000000;

        /* Enable the feature in EFER */
        tmp = rdmsr(X86_MSR_EFER);
        tmp |= (1 << 11);
        wrmsr(X86_MSR_EFER, tmp);
    }
}

void init_mem(void)
{
    /* Initialize the memory subsystems */
    init_physical_memory();
    init_virtual_memory();

    /* Detect availability of the NX flag */
    init_nx();

    /* Remap the kernel */
    kmprotect_kernel();

    /* Copy the initram */
    gKernel.initram_size = gBootParams.initram_size;
    gKernel.initram      = (char*)kmmap(nullptr, 0, gKernel.initram_size, KPROT_READ, KMAP_ANONYMOUS);
    memcpy(gKernel.initram, gBootParams.initram, gKernel.initram_size);

    /* Free the original initram */
    free_phys((uint64_t)gBootParams.initram, gKernel.initram_size);

    /* Remap the video buffer */
    gBootParams.video.framebuffer = physical_to_virtual((uint64_t)gBootParams.video.framebuffer);

    /* Free all lomem paging info */
    kmunmap_tree(nullptr, 0x100000000);
}
