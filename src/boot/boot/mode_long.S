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


.section .text, "ax"
/* Arg1: Entry Point */
/* Arg2: cr3 */
/* Arg3: Kernel Boot Params */
.global jump_mode_long
jump_mode_long:
    /* Load cr3 */
    mov 12(%esp), %eax
    mov %eax, %cr3

    /* Set up PAE */
    mov %cr4, %eax
    or $0x20, %eax
    mov %eax, %cr4

    /* Enable Long Mode */
    mov $0xc0000080, %ecx
    rdmsr
    or $0x100, %eax
    wrmsr

    /* Enable paging */
    mov %cr0, %eax
    or $0x80000000, %eax
    mov %eax, %cr0

    /* Reload the code seg */
    lgdt (gdt64_desc)
    jmp $0x08,$lmode

.code64
lmode:
    /* Reload the other segments */
    /* For most of them, this is the last time they are changed */
    mov $0x10, %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    /* Fetch the argument - the kernel ptr */
    mov 16(%esp), %edi

    /* Fetch the entry point */
    mov 4(%esp), %rax

    /* Enter the kernel */
    jmp %rax
