#include <kernel/kernel.h>

Kernel gKernel;

_NORETURN void kmain(KernelBootParams* params)
{
    bzero(&gKernel, sizeof(gKernel));

    /* Copy the boot parameters */
    memcpy(&gKernel.boot_params, params, sizeof(KernelBootParams));

    init_physical_mapping();
    init_physical_memory();

    for (;;)
    {
    }
}
