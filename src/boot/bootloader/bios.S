.code64
.section .data.lo, "aw"
.align 8
saved_bios_params:
    .quad 0x0
saved_bios_cf:
    .quad 0x0

.section .text.lo, "ax"

/* RDI: int num */
/* RSI: BIOS params */
.global bios_call
bios_call:
    /* We must preserve rbx and rbp */
    sub  $16, %rsp
    mov  %rbx, 0(%rsp)
    mov  %rbp, 8(%rsp)

    /* Patch the future BIOS call */
    mov %edi, %eax
    mov %al, (int_call+1)

    /* Save the bios args */
    mov %rsi, (saved_bios_params)

    /* Go back to real mode */
    call switch_mode_real

.code16
    /* Load the BIOS args */
    mov (saved_bios_params), %ebp
    mov 0(%ebp), %eax
    mov 4(%ebp), %ebx
    mov 8(%ebp), %ecx
    mov 12(%ebp), %edx
    mov 16(%ebp), %esi
    mov 20(%ebp), %edi

int_call:
    /* Perform the BIOS call */
    int $0x00

    /* Save results */
    mov (saved_bios_params), %ebp
    mov %eax, 0(%ebp)
    mov %ebx, 4(%ebp)
    mov %ecx, 8(%ebp)
    mov %edx, 12(%ebp)
    mov %esi, 16(%ebp)
    mov %edi, 20(%ebp)

    /* Save the carry status */
    setc (saved_bios_cf)

    /* Go back to long mode */
    call switch_mode_long

.code64
    /* Load the result */
    mov (saved_bios_cf), %eax

    /* Restore the registers */
    mov  0(%rsp), %rbx
    mov  8(%rsp), %rbp

    add $16, %rsp
    ret
