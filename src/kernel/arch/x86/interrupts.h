#ifndef ARCH_X86_INTERRUPTS_H
#define ARCH_X86_INTERRUPTS_H

#include <sys/_cext.h>

_EXTERNC void idt_init(void);
_EXTERNC void idt_set_gate_interrupt(int interrupt, int dpl, void* handler);
_EXTERNC void int_timer(void);

#endif
