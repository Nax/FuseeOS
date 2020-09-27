#include <kernel/kernel.h>

Kernel gKernel;

_EXTERNC _NORETURN void kmain(KernelBootParams* params)
{
    bzero(&gKernel, sizeof(gKernel));

    /* Copy the boot parameters */
    memcpy(&gKernel.boot_params, params, sizeof(KernelBootParams));

    init_screen_early();
    puts("FuseeOS Kernel");
    putchar('\n');

    init_mem();
    init_screen();
    puts("Memory initialized");
    print("Screen buffer remapped to ");
    puthex64((uint64_t)gKernel.screenbuf);
    putchar('\n');

    for (;;)
    {
        //puthex64(alloc_phys_pages(1));
        //putchar('\n');
    }
}
