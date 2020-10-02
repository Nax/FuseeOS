#include <kernel/kernel.h>

Kernel gKernel;

_EXTERNC _NORETURN void kmain(BootParams* params)
{
    bzero(&gKernel, sizeof(gKernel));

    /* Copy the boot parameters */
    memcpy(&gBootParams, params, sizeof(*params));

    kprintf("FuseeOS kernel loaded\n");
    init_physical_mapping();
    kprintf("Physical memory loaded\n");

    arch_init();

    kprintf("Memory initialized\n");
    kprintf("  Pages: 0x%lx\n", gKernel.pmem.pages_total);
    kprintf("  Free:  0x%lx\n", gKernel.pmem.pages_free);

    for (;;) {}
}
