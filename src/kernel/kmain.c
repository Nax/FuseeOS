#include <kernel/kernel.h>

Kernel gKernel;

_EXTERNC _NORETURN void kmain(BootParams* params)
{
    /* Copy the boot parameters */
    memcpy(&gBootParams, params, sizeof(*params));

    kprintf("FuseeOS kernel loaded\n");
    arch_init_physical_mapping();
    kprintf("Physical memory loaded\n");

    arch_init();

    kprintf("Memory initialized\n");
    kprintf("  Pages: 0x%lx\n", gKernel.pmem.pages_total);
    kprintf("  Free:  0x%lx\n", gKernel.pmem.pages_free);

    thread_init();
    kprintf("Thread-local kernel structures initialized\n");

    Process* p;
    proc_init();
    p = proc_create_initram("/sbin/init");
    proc_schedule(p);
    kern_schedule();

    _UNREACHABLE();
}
