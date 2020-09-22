OUTPUT_FORMAT(binary)
SECTIONS {
    . = 0x0600 ;

    .text : {
        *(.text16.boot)
        *(.text16)
        *(.data16)
        *(.text)
        *(.data)
        *(.rodata)
    }
}
