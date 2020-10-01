#include <kernel/kernel.h>

void arch_init(void)
{
    init_gdt();
    kprintf("GDT initialized\n");
    init_mem();
    emu8086_init();
    vbe_init();
}
