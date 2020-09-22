.code16
.section .text16, "ax"

check_a20:
    pusha

    xor %ax, %ax
    mov %ax, %es

    not %ax
    mov %ax, %ds

    mov $0x0500, %di
    mov $0x0510, %si

    mov %es:(%di), %al
    push %ax

    mov %ds:(%si), %al
    push %ax

    movb $0x00, %es:(%di)
    movb $0xff, %ds:(%si)

    cmpb $0x00, %es:(%di)

    pop %ax
    mov %al, %ds:(%si)

    pop %ax
    mov %al, %es:(%di)

    popa
    ret

enable_a20_bios:
    pusha

    mov     $0x2401, %ax
    int     $0x15

    popa
    ret

enable_a20_fast:
    pusha

    in      $0x92, %al
    or      $2, %al
    out     %al, $0x92

    popa
    ret


.global enable_a20
enable_a20:
    /* Some BIOSes already enable A20 */
    call check_a20
    jz enabled

    /* Try the BIOS method */
    call enable_a20_bios
    call check_a20
    jz enabled

    /* Try the fast A20 */
    call enable_a20_fast
    call check_a20
    jz enabled

    /* Give up */

enabled:
    ret
