.code16
.text

bootstrap:
    /* Disable interrupts */
    cli

    /* Relocate the code */
    mov $0x0200, %cx
    mov $0x7c00, %si
    mov $0x0600, %di
    rep movsb

    /* Jump to the relocated code */
    jmp $0x00, $start

start:
    /* Reset the segments */
    xor %ax, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    /* Load the stack */
    mov $0x600, %sp

    /* Save the original disk number */
    and $0xff, %dx
    push %dx

    /* Seek a bootable partition */
    mov $mbr_partitions, %si
seek_partiton_loop:
    mov (%si), %al
    test %al, %al
    js found
    add $0x10, %si
    cmp $mbr_end, %si
    jz halt
    jmp seek_partiton_loop

found:
    /* Save a ptr to the MBR entry */
    push %si

    /* Set up the DAP, copy the offset */
    add $8, %si
    mov $DAP_Offset, %di
    movsl

    /* Read from the disk */
    mov $0x4200, %ax
    mov $DAP, %si

    xchg %bx, %bx

    int $0x13

    /* Restore dl and si */
    pop %si
    pop %dx

    /* Jump into the VBR */
    jmp $0x00,$0x7c00

halt:
    /* Infinite loop */
    hlt
    jmp halt

.align 0x10
DAP:
DAP_Size:
    .byte 0x10
DAP_Unused:
    .byte 0x00
DAP_ReadSize:
    .short 0x10
DAP_DstOffset:
    .short 0x7c00
DAP_DstSegment:
    .short 0x0000
DAP_Offset:
    .skip 8

.org 0x1b8
mbr:
.space 6
mbr_partitions:
.space 16 * 4
mbr_end:
.byte 0x55, 0xaa
