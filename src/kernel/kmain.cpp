#include <kernel/kernel.h>

Kernel gKernel;

_EXTERNC _NORETURN void kmain(KernelBootParams* params)
{
    bzero(&gKernel, sizeof(gKernel));

    /* Copy the boot parameters */
    memcpy(&gKernel.boot_params, params, sizeof(KernelBootParams));

    init_physical_mapping();

    video_init();
    puts("FuseeOS Kernel");
    putchar('\n');

    arch_init();

    puts("Memory initialized");
    print("  Pages: ");
    puthex64(gKernel.pmem.pages_total);
    putchar('\n');

    print("  Free:  ");
    puthex64(gKernel.pmem.pages_free);
    putchar('\n');
    putchar('\n');
    print("Screen buffer remapped to ");
    puthex64((uint64_t)gKernel.screenbuf);
    putchar('\n');

    for (;;)
    {
        //puthex64(alloc_phys_pages(1));
        //putchar('\n');
    }
}
