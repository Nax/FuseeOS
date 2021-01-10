#include <kernel/kernel.h>

void idt_init();
void gdt_init();

void arch_init(void)
{
    gdt_init();
    kprintf("GDT initialized\n");
    idt_init();
    kprintf("IDT initialized\n");
    pic_init();
    kprintf("PIC initialized\n");
    init_mem();

    idt_set_gate_interrupt(X86_INT_TIMER, 0, (void*)&int_timer);
}
