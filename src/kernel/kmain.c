#include <kernel/kernel.h>

Kernel gKernel;

_NORETURN void kmain(KernelBootParams* params)
{
    bzero(&gKernel, sizeof(gKernel));

    /* Copy the boot parameters */
    memcpy(&gKernel.boot_params, params, sizeof(KernelBootParams));

    init_screen();
    puts("FuseeOS Kernel");
    init_physical_mapping();
    puts("Physical memory mapped");
    init_physical_memory();
    puts("Physical memory allocator initialized");

    for (;;)
    {
        puthex64(alloc_phys_pages(1));
        putchar('\n');
    }
}
