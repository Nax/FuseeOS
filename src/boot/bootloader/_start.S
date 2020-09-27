.code16
.section .text.start, "ax"

.macro PUTS, str
push %si
mov $\str, %si
call print_bios
pop %si
.endm

start:
    /* Disable interrupts */
    cli

    /* Set up segments */
    xor %ax, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    /* Force CS = 0 */
    jmp $0x00, $start2

start2:
    /* Set up the stack */
    mov $0x7c00, %sp

    /* Flush the screen */
    pusha
    mov $0x0003, %ax
    int $0x10
    popa

    PUTS str_init

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

    /* Enter main */
.code32
    mov $saved_mbr_record, %eax
    push %eax

    mov (saved_disk), %eax
    push %eax

    xor %eax, %eax
    push %eax

    jmp bmain

.code16

/* SI: Str */
print_bios:
    pusha

print_bios_loop:
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
    jz print_bios_end

    push %si

    mov $1, %cx
    mov $0x000f, %bx
    mov $0x09, %ah
    int $0x10

    pop %si
    inc %si
    jmp print_bios_loop

print_bios_end:
    movb $0, (cursor_x)
    incb (cursor_y)
    popa
    ret

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
.space 4

str_init: .asciz "FuseeOS VBR"

.org 0x1fe
.byte 0x55, 0xaa
