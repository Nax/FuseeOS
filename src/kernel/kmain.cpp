#include <kernel/kernel.h>

Kernel gKernel;

_EXTERNC _NORETURN void kmain(KernelBootParams* params)
{
    bzero(&gKernel, sizeof(gKernel));

    /* Copy the boot parameters */
    memcpy(&gKernel.boot_params, params, sizeof(KernelBootParams));

    init_screen();
    puts("FuseeOS Kernel");

    init_mem();

    for (;;)
    {
        //puthex64(alloc_phys_pages(1));
        //putchar('\n');
    }
}
