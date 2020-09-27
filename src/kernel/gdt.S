.code64
.text

/*
 * RDI: desc
 * RSI: code seg
 * RDX: data seg
 */
.global enable_gdt
enable_gdt:
    /* Load the new GDT */
    lgdt (%rdi)

    /* Load the new stack & code seg */
    mov %rsp, %rax
    pushq %rdx
    pushq %rax
    pushfq
    pushq %rsi
    pushq $enable_gdt_codeseg

    iretq

enable_gdt_codeseg:
    /* Load the data seg */
    mov %dx, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    ret

