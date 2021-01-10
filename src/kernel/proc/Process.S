#include <kernel/arch/x86/defs.h>

.section .text

/* rdi: process */
.global _exec_proc
_exec_proc:
    /* Prepare the stack for the iretq */
    sub $(5 * 8), %rsp

    /* Load the RIP */
    mov  (X86_REG_IP * 8)(%rdi), %rax
    mov  %rax, 0(%rsp)

    /* Load the CS */
    movq $(X86_SEL_CODE3 | X86_RING3), 8(%rsp)

    /* Load the flags */
    mov  (X86_REG_FLAGS * 8)(%rdi), %rax
    mov  %rax, 16(%rsp)

    /* Load the stack */
    mov  (X86_REG_SP * 8)(%rdi), %rax
    mov  %rax, 24(%rsp)

    /* Load the SS */
    movq $(X86_SEL_DATA3 | X86_RING3), 32(%rsp)

    /* Load the registers */
    mov (X86_REG_AX  * 8)(%rdi), %rax
    mov (X86_REG_BX  * 8)(%rdi), %rbx
    mov (X86_REG_CX  * 8)(%rdi), %rcx
    mov (X86_REG_DX  * 8)(%rdi), %rdx
    mov (X86_REG_SI  * 8)(%rdi), %rsi
    mov (X86_REG_BP  * 8)(%rdi), %rbp
    mov (X86_REG_R8  * 8)(%rdi), %r8
    mov (X86_REG_R9  * 8)(%rdi), %r9
    mov (X86_REG_R10 * 8)(%rdi), %r10
    mov (X86_REG_R11 * 8)(%rdi), %r11
    mov (X86_REG_R12 * 8)(%rdi), %r12
    mov (X86_REG_R13 * 8)(%rdi), %r13
    mov (X86_REG_R14 * 8)(%rdi), %r14
    mov (X86_REG_R15 * 8)(%rdi), %r15

    mov (X86_REG_DI  * 8)(%rdi), %rdi

    /* Load a ring3 data selector */
    push %rax
    mov $(X86_SEL_DATA3 | X86_RING3), %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    pop %rax

    /* Go to ring3 */
    iretq
