OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)
SECTIONS {
    . = 0x7c00;
    .text : {
        *(.text.start)
        *(.text.lo)
        *(.data.lo)
        *(.rodata.lo)
        *(.text)
        *(.data)
        *(.rodata)
    }
}
