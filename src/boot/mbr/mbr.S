.code16
.text

.macro PUTS, str
push %si
mov $\str, %si
call print
pop %si
.endm

bootstrap:
    /* Disable interrupts */
    cli

    /* Reset the segments */
    xor %ax, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    /* Relocate the code */
    mov $0x0200, %cx
    mov $0x7c00, %si
    mov $0x0600, %di
    rep movsb

    /* Jump to the relocated code */
    jmp $0x00, $start

start:
    /* Load the stack */
    mov $0x600, %sp

    /* Save the original disk number */
    and $0xff, %dx
    push %dx

    /* Flush the screen */
    pusha
    mov $0x0003, %ax
    int $0x10
    popa

    /* Print a nice entry message */
    PUTS str_init

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

    PUTS str_found

    /* Set up the DAP, copy the offset */
    add $8, %si
    mov $DAP_Offset, %di
    movsl

    /* Read from the disk */
    mov $0x4200, %ax
    mov $DAP, %si

    int $0x13

    /* Restore dl and si */
    pop %si
    pop %dx

    /* Jump into the VBR */
    jmp $0x00,$0x7c00

halt:
    PUTS str_notfound

    /* Infinite loop */
    hlt
    jmp halt


/* SI: Str */
print:
    pusha

print_loop:
    /* Move the cursor */
    push %si
    xor %bh, %bh
    mov (cursor_x), %dl
    mov (cursor_y), %dh
    incb (cursor_x)
    mov $0x02, %ah
    int $0x10
    pop %si

    /* Print a character */
    mov  (%si), %al
    test %al, %al
    jz print_end

    push %si

    mov $1, %cx
    mov $0x000f, %bx
    mov $0x09, %ah
    int $0x10

    pop %si
    inc %si
    jmp print_loop

print_end:
    movb $0, (cursor_x)
    incb (cursor_y)
    popa
    ret

str_init:
    .asciz "FuseeOS MBR"
str_found:
    .asciz "Partition Found"
str_notfound:
    .asciz "Partition Not Found"

cursor_x:
    .byte 0x00
cursor_y:
    .byte 0x00

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
