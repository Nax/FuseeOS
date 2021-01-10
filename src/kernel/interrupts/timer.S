.section .text
.global int_timer

.extern kern_schedule

#include <kernel/interrupts/common.S>

int_timer:
    /* Save the process context */
    SAVE_CONTEXT

    /* Ack the IRQ */
    mov  $0x20, %eax
    out  %al, $0x20

    /* Resume kernel operations */
    jmp kern_schedule
