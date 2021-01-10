#include <kernel/kernel.h>
#include <kernel/arch/x86/PIC.h>

void init_idt();
void gdt_init();

void arch_init(void)
{
    gdt_init();
    kprintf("GDT initialized\n");
    init_idt();
    kprintf("IDT initialized\n");
    pic_init();
    kprintf("PIC initialized\n");
    init_mem();
}
