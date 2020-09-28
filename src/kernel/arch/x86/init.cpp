#include <kernel/kernel.h>

void arch_init(void)
{
    init_gdt();
    puts("GDT initialized");
}
