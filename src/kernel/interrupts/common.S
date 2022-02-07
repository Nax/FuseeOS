#include <kernel/arch/x86/defs.h>

.extern proc_run
.extern proc_get

.macro SAVE_CONTEXT
    /* Restore kernel GS */
    swapgs

    push    %rdi
    mov     %gs:(8), %rdi

    /* Mark the process as being interrupted */
    movq    $proc_run, (18 * 8 + 512)(%rdi)

    mov     %rax,  (X86_REG_AX  * 8)(%rdi)
    mov     %rbx,  (X86_REG_BX  * 8)(%rdi)
    mov     %rcx,  (X86_REG_CX  * 8)(%rdi)
    mov     %rdx,  (X86_REG_DX  * 8)(%rdi)
    mov     %rsi,  (X86_REG_SI  * 8)(%rdi)
    pop     %rax
    mov     %rax,  (X86_REG_DI  * 8)(%rdi)
    mov     24(%rsp), %rax
    mov     %rax,  (X86_REG_SP  * 8)(%rdi)
    mov     %rbp,  (X86_REG_BP  * 8)(%rdi)
    mov     %r8,   (X86_REG_R8  * 8)(%rdi)
    mov     %r9,   (X86_REG_R9  * 8)(%rdi)
    mov     %r10,  (X86_REG_R10 * 8)(%rdi)
    mov     %r11,  (X86_REG_R11 * 8)(%rdi)
    mov     %r12,  (X86_REG_R12 * 8)(%rdi)
    mov     %r13,  (X86_REG_R13 * 8)(%rdi)
    mov     %r14,  (X86_REG_R14 * 8)(%rdi)
    mov     %r15,  (X86_REG_R15 * 8)(%rdi)
    mov     0(%rsp), %rax
    mov     %rax,  (X86_REG_IP  * 8)(%rdi)
    mov     16(%rsp), %rax
    mov     %rax,  (X86_REG_FLAGS  * 8)(%rdi)
    add     $(8 * 5), %rsp
.endm
