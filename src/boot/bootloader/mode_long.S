.code32
.section .rodata, "a"
.align 0x10
gdt64:
gdt64_null:
    .short 0x0
    .short 0x0
    .byte 0x0
    .byte 0x0
    .byte 0x0
    .byte 0x0
gdt64_code:
    .short 0xffff
    .short 0x0
    .byte 0x0
    .byte 0x9a
    .byte 0xaf
    .byte 0x0
gdt64_data:
    .short 0x0
    .short 0x0
    .byte 0x0
    .byte 0x92
    .byte 0xa0
    .byte 0x0
gdt64_desc:
    .short gdt64_desc - gdt64 - 1
    .int gdt64
    .int 0

.section .text.lo, "ax"
.code16
/* Switch to long mode */
.global switch_mode_long
switch_mode_long:
    /* Fix the return address */
    pop %ax
    pushl $0
    pushl $0
    mov %ax, (%esp)

    /* Disable interrupts */
    cli

    /* Enable long mode */
    mov %cr0, %eax
    or $0x80000001, %eax
    mov %eax, %cr0

    /* Load a long mode GDT */
    lgdt (gdt64_desc)

    /* Jump into lmode */
    jmp $0x08,$lmode

.section .text, "ax"
.code64

lmode:
    /* Load correct segments */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    ret
