#include <kernel/kernel.h>
#include <kernel/arch/x86/PIC.h>

void init_idt();

void arch_init(void)
{
    init_gdt();
    kprintf("GDT initialized\n");
    init_idt();
    kprintf("IDT initialized\n");
    pic_init();
    kprintf("PIC initialized\n");
    init_mem();
}
