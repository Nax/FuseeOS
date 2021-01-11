.section .text

#if (!(defined(__LIBC_MINIMAL__)))

/* Syscall: RDI RSI RDX R10 R8 R9 */
/* SysV   : RDI RSI RDX RCX R8 R9 */
.global __syscall6
__syscall6:
    mov     %edi, %eax
    mov     %rsi, %rdi
    mov     %rdx, %rsi
    mov     %rcx, %rdx
    mov     %r8,  %r10
    mov     %r9,  %r8
    movq    8(%rsp), %r9

    syscall

    ret

#endif
