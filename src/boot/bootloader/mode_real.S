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

.code32
.section .text, "ax"

/* Switch to real mode */
.global switch_mode_real
switch_mode_real:
    /* We need to fix the return address */
    sub     $26, %esp
    mov     %eax, 0(%esp)
    mov     26(%esp), %ax
    mov     %ax, 28(%esp)
    mov     %ebx, 4(%esp)
    mov     %ecx, 8(%esp)
    mov     %edx, 12(%esp)
    mov     %esi, 16(%esp)
    mov     %edi, 20(%esp)
    mov     %ebp, 24(%esp)

    /* Load a real mode GDT */
    lgdt (gdt_real_mode_descriptor)

    /* Enter 16 bit protected mode */
    jmp $0x08,$pmode16

.code16
pmode16:
    /* Reload the other segments */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    /* Disable protected mode */
    mov %cr0, %eax
    and $0xfffe, %ax
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

    /* Restore registers */
    mov 0(%esp), %eax
    mov 4(%esp), %ebx
    mov 8(%esp), %ecx
    mov 12(%esp), %edx
    mov 16(%esp), %esi
    mov 20(%esp), %edi
    mov 24(%esp), %ebp

    /* Leave */
    add $28, %esp
    ret
