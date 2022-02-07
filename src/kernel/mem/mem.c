#include <kernel/kernel.h>
#include <kernel/arch.h>

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

static void unmap_lomem(void)
{
    for (int i = 0; i < 128; ++i)
        gKernel.cr3[i] = 0;
}

void init_mem(void)
{
    /* Initialize the memory subsystems */
    init_physical_memory();

    /* Detect availability of the NX flag */
    init_nx();

    /* Remap the kernel */
    init_kernel_mem_prot();

    /* Copy the initram */
    char* tmp = (char*)kmalloc(gBootParams.initram_size, 0);
    memcpy(tmp, gBootParams.initram, gBootParams.initram_size);
    free_phys((uint64_t)gBootParams.initram, gBootParams.initram_size);
    gBootParams.initram = tmp;

    /* Remap the video buffer */
    gBootParams.video.framebuffer = kmap(NULL, (uint64_t)gBootParams.video.framebuffer, gBootParams.video.pitch * gBootParams.video.height, KPROT_READ | KPROT_WRITE);
    kprintf("Video Buffer: 0x%lx\n", (uint64_t)gBootParams.video.framebuffer);

    /* Free lomem */
    unmap_lomem();

    /* Free all lomem paging info */
    // kmunmap_tree(NULL, 0x100000000);
}
