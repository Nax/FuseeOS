.section .rodata, "a"
.align 0x10
gdt_real_mode:
gdt_real_mode_null:
    .short  0
    .short  0
    .byte   0
    .byte   0
    .byte   0
    .byte   0
gdt_real_mode_code:
    .short  0xffff
    .short  0x0000
    .byte   0x00
    .byte   0x98
    .byte   0x8f
    .byte   0x00
gdt_real_mode_data:
    .short  0xffff
    .short  0x0000
    .byte   0x00
    .byte   0x92
    .byte   0x8f
    .byte   0x00
gdt_real_mode_descriptor:
    .short  gdt_real_mode_descriptor - gdt_real_mode - 1
    .short  gdt_real_mode
    .short  0
    .short  0
    .short  0

.code64
.section .text, "ax"

/* Switch to real mode */
.global switch_mode_real
switch_mode_real:
    /* We need to fix the return address */
    pop %rax
    sub $2, %rsp
    mov %ax, (%rsp)

    /* Load a real mode GDT */
    lgdt (gdt_real_mode_descriptor)

    /* Enter 16 bit protected mode */
    pushq $0x08
    pushq $pmode16
    retfq

.code16
pmode16:
    /* Reload the other segments */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    /* Disable long mode */
    mov %cr0, %eax
    and $0x7ffffffe, %eax
    mov %eax, %cr0

    /* Jump into real mode */
    jmp $0x00,$rmode

.section .text.lo, "ax"
.code16

rmode:
    /* Reload the other segments */
    xor %ax, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    /* Leave */
    ret
