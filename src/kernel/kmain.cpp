#include <kernel/kernel.h>

Kernel gKernel;

_EXTERNC _NORETURN void kmain(BootParams* params)
{
    bzero(&gKernel, sizeof(gKernel));

    /* Copy the boot parameters */
    memcpy(&gKernel.boot_params, params, sizeof(*params));

    init_physical_mapping();

    video_init();
    kprintf("FuseeOS Kernel\n");

    arch_init();

    kprintf("Memory initialized\n");
    kprintf("  Pages: 0x%lx\n", gKernel.pmem.pages_total);
    kprintf("  Free:  0x%lx\n", gKernel.pmem.pages_free);

    for (;;) {}
}
