.code16

.section .bss
.align 0x10000
bootstrap_pages_pml4:
    .skip 0x1000
bootstrap_pages_pdp:
    .skip 0x1000
bootstrap_pages_pd:
    .skip 0x1000

.section .text.lo, "ax"
/* Sets up a temporary paging structure, just enough to bootstrap the system */
.global bootstrap_paging
bootstrap_paging:
    pusha
    push %es

    /* Zero out PML4 */
    mov $bootstrap_pages_pml4, %eax
    shr $4, %eax
    mov %ax, %es
    xor %di, %di
    xor %al, %al
    mov $0x1000, %cx
    rep stosb
    movl $bootstrap_pages_pdp + 1, %es:(0x0000)

    /* Zero out PDP */
    mov $bootstrap_pages_pdp, %eax
    shr $4, %eax
    mov %ax, %es
    xor %di, %di
    xor %al, %al
    mov $0x1000, %cx
    rep stosb
    movl $bootstrap_pages_pd + 1, %es:(0x0000)

    /* Fill the PT */
    mov $bootstrap_pages_pd, %eax
    shr $4, %eax
    mov %ax, %es
    mov $0x200, %ecx
    xor %eax, %eax
    xor %ebx, %ebx
    or $0x81, %eax
    xor %edi, %edi

fill_pd_loop:
    mov %eax, %es:0(%edi)
    mov %ebx, %es:4(%edi)
    add $0x200000, %eax
    add $8, %edi
    loop fill_pd_loop

    mov $bootstrap_pages_pml4, %eax
    mov %eax, %cr3

    pop %es
    popa
    ret
