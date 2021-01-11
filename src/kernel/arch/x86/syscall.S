.section .text

#include <sys/_errno.h>
#include <kernel/defs.h>
#include <kernel/arch/x86/defs.h>

.extern proc_run_sysret
.extern gSysHandlers

.global x86_sys_handler
x86_sys_handler:
    /* Boundary check */
    cmp $KERNEL_MAX_SYSCALL, %rax
    jge bad_sys

    /* Handler check */
    mov     gSysHandlers(,%rax,8), %rax
    test    %rax, %rax
    jz      bad_sys

    /* Load the kernel GS */
    swapgs

    /* Store rsp in scratch space */
    mov %rsp, %gs:(24)

    /* Load the proc ptr */
    mov %gs:(8), %rsp

    /* Store some proc state */
    mov %rbx, (X86_REG_BX * 8)(%rsp)
    mov %rdx, (X86_REG_DX * 8)(%rsp)
    mov %rdi, (X86_REG_DI * 8)(%rsp)
    mov %rsi, (X86_REG_SI * 8)(%rsp)
    mov %rbp, (X86_REG_BP * 8)(%rsp)
    mov %r8,  (X86_REG_R8 * 8)(%rsp)
    mov %r9,  (X86_REG_R9 * 8)(%rsp)
    mov %r10, (X86_REG_R10 * 8)(%rsp)
    mov %r12, (X86_REG_R12 * 8)(%rsp)
    mov %r13, (X86_REG_R13 * 8)(%rsp)
    mov %r14, (X86_REG_R14 * 8)(%rsp)
    mov %r15, (X86_REG_R15 * 8)(%rsp)
    mov %rcx, (X86_REG_IP * 8)(%rsp)
    mov %r11, (X86_REG_FLAGS * 8)(%rsp)

    /* Restore the user SP */
    mov %gs:(24), %rbx
    mov %rbx, (X86_REG_SP * 8)(%rsp)

    /* Mark the proc as being in a syscall */
    movq $proc_run_sysret, (18 * 8 + 512)(%rsp)

    /* Load the kernel stack */
    mov %gs:(0), %rsp
    lea (32 + KERNEL_STACK_SIZE)(%rsp), %rsp

    /* Alloc the args */
    /* RDI RSI RDX R10 R8 R9 */
    sub     $(6 * 8), %rsp
    mov     %rdi, (0 * 8)(%rsp)
    mov     %rsi, (1 * 8)(%rsp)
    mov     %rdx, (2 * 8)(%rsp)
    mov     %r10, (3 * 8)(%rsp)
    mov     %r8,  (4 * 8)(%rsp)
    mov     %r9,  (5 * 8)(%rsp)

    mov     %gs:(8),    %rdi
    mov     %rsp,       %rsi

    /* Call the handler */
    call    *%rax

    /*
     * If the handler returns, then the syscall was
     * handled synchronously. We must return to userland.
     */

    mov     %gs:(8), %rdi
    mov     %rax, (8 * X86_REG_AX)(%rdi)
    jmp     proc_run_sysret

bad_sys:
    mov $(-_ENOSYS), %rax
    sysretq
