#include <kernel/kernel.h>

void arch_init(void)
{
    init_gdt();
    puts("GDT initialized");
    init_mem();
    init_screen();
    emu8086_init();
    vbe_init();
}
