#include <kernel/arch/x86/defs.h>
#include <kernel/arch/x86/gdt.h>
#include <kernel/arch/x86/pic.h>
#include <kernel/arch/x86/interrupts.h>
#include <kernel/arch/x86/syscall.h>
#include <kernel/mem.h>
#include <kernel/print.h>

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

    sys_init();
}
