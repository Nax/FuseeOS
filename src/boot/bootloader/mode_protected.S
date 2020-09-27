.section .data.lo, "a"
.align 0x10
gdt_protected_mode:
gdt_protected_mode_null:
    .short  0
    .short  0
    .byte   0
    .byte   0
    .byte   0
    .byte   0
gdt_protected_mode_code:
    .short  0xffff
    .short  0x0000
    .byte   0x00
    .byte   0x98
    .byte   0xcf
    .byte   0x00
gdt_protected_mode_data:
    .short  0xffff
    .short  0x0000
    .byte   0x00
    .byte   0x92
    .byte   0xcf
    .byte   0x00
gdt_protected_mode_descriptor:
    .short  gdt_protected_mode_descriptor - gdt_protected_mode - 1
    .short  gdt_protected_mode
    .short  0

.code16
.section .text.lo, "ax"

/* Switch to protected mode */
.global switch_mode_protected
switch_mode_protected:
    /* The return address is 2 bytes */
    /* We need to pad it */
    sub     $30, %sp
    mov     %eax, 0(%esp)
    mov     %ebx, 4(%esp)
    mov     %ecx, 8(%esp)
    mov     %edx, 12(%esp)
    mov     %esi, 16(%esp)
    mov     %edi, 20(%esp)
    mov     %ebp, 24(%esp)
    mov     30(%esp), %ax
    mov     %ax, 28(%esp)
    movw    $0, 30(%esp)

    /* Disable interrupts */
    cli

    /* Load a protected mode GDT */
    lgdt (gdt_protected_mode_descriptor)

    /* Enable protected mode */
    mov %cr0, %eax
    or  $1, %ax
    mov %eax, %cr0

    /* Jump into pmode */
    jmp $0x08,$pmode

.section .text, "ax"
.code32

pmode:
    /* Load correct segments */
    mov $0x10, %ax
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
