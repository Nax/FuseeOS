.section .text
.global int_timer

.extern proc_reschedule
.extern kern_schedule

#include <kernel/interrupts/common.S>

int_timer:
    /* Save the process context */
    SAVE_CONTEXT

    /* Ack the IRQ */
    mov  $0x20, %eax
    out  %al, $0x20

    /* Re-schedule the process */
    call proc_reschedule

    /* Go back to the scheduler */
    jmp kern_schedule
