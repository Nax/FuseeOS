.code32
.section .text.lo, "ax"

/* BIOS args at esp+28, int number at esp+24*/
.global bios_call
bios_call:
    /* We must restore ebx, esi, edi and ebp */
    sub  $20, %esp
    movl $0,   0(%esp)
    mov  %ebx, 4(%esp)
    mov  %esi, 8(%esp)
    mov  %edi, 12(%esp)
    mov  %ebp, 16(%esp)

    /* Patch the future BIOS call */
    mov 24(%esp), %eax
    mov %al, (int_call+1)

    /* Load up the BIOS args */
    mov 28(%esp), %ebp
    mov 0(%ebp), %eax
    mov 4(%ebp), %ebx
    mov 8(%ebp), %ecx
    mov 12(%ebp), %edx
    mov 16(%ebp), %esi
    mov 20(%ebp), %edi

    /* Go back to real mode */
    call switch_mode_real

.code16
int_call:
    /* Perform the BIOS call */
    int $0x00

    /* Check for error */
    jc after_call

    /* No error, save 1 */
    movl $1, 0(%esp)

after_call:
    /* Go back to protected mode */
    call switch_mode_protected

.code32
    /* Move the results back into the structure */
    mov 28(%esp), %ebp
    mov %eax, 0(%ebp)
    mov %ebx, 4(%ebp)
    mov %ecx, 8(%ebp)
    mov %edx, 12(%ebp)
    mov %esi, 16(%ebp)
    mov %edi, 20(%ebp)

    /* Restore the registers */
    mov 0(%esp), %eax
    mov 4(%esp), %ebx
    mov 8(%esp), %esi
    mov 12(%esp), %edi
    mov 16(%esp), %ebp

    add  $20, %esp
    ret
