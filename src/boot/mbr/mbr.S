.code16
.text

bootstrap:
    /* Disable interrupts */
    cli

    /* Relocate the code */
    mov $0x0200, %cx
    mov $0x7c00, %si
    mov $0x0600, %di
    rep movsb

    /* Jump to the relocated code */
    jmp $0x00, $start

start:
    /* Reset the segments */
    xor %ax, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    /* Load the stack */
    mov $0x600, %sp

halt:
    /* Infinite loop */
    hlt
    jmp halt


.org 510
.byte 0x55, 0xaa
