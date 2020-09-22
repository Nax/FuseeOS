.code16
.section .text.start, "ax"

start:
    /* Disable interrupts */
    cli

    /* Force CS = 0 */
    jmp $0x00, $start2

start2:
    /* Set up segments and stack */
    xor %ax, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    mov $0x7c00, %sp

    /* Save disk number */
    and $0xff, %dx
    mov %dx, (saved_disk)

    /* Save MBR record we are booting from */
    mov $0x10, %cx
    mov $saved_mbr_record, %di
    rep movsb

    /* We need to load the rest of the code, as this won't fit into 512 bytes */
    mov (saved_mbr_record + 8), %eax
    inc %eax
    mov %eax, (DAP_Offset)

    mov $0x4200, %ax
    mov (saved_disk), %dx
    mov $DAP, %si
    int $0x13

    /* Now we have more leeway */
    /* Enable the A20 gate */
    call enable_a20

    /* We will switch to protected mode soon */
    /* Make sure to zero extend relevant registers first */
    xor %eax, %eax
    xor %ebx, %ebx
    xor %ecx, %ecx
    xor %edx, %edx
    xor %esi, %esi
    xor %edi, %edi
    xor %ebp, %ebp

    /* We will change that stack once we are in pmode */
    mov $0x7c00, %esp

    /* enter pmode */
    call switch_mode_protected
.code32
    xchg %bx, %bx
.code16


.align 0x10
DAP:
DAP_Size:
    .byte 0x10
DAP_Unused:
    .byte 0x00
DAP_ReadSize:
    .short 63
DAP_DstOffset:
    .short (0x7c00 + 512)
DAP_DstSegment:
    .short 0x0000
DAP_Offset:
    .skip 8

saved_mbr_record:
.space 16

saved_disk:
.space 2

.org 0x1fe
.byte 0x55, 0xaa
