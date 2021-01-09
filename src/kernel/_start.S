.global _start
.extern kmain

.section .text

/* rdi: Kernel args */
_start:
    /* Set the kernel stack */
    mov $kernel_stack, %rsp

    /* Enable SSE (need to do this before executing any C code) */
    mov %cr0, %rax
    and $~(1 << 2), %rax
    or $(1 << 1), %rax
    mov %rax, %cr0

    mov %cr4, %rax
    or $(3 << 9), %rax
    mov %rax, %cr4

    /* Enter kmain */
    jmp kmain

.section .bss
.align 0x1000
.space 0x10000
kernel_stack:
