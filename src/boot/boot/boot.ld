OUTPUT_FORMAT(binary)
SECTIONS {
    . = 0x7c00;
    .text : {
        *(.text16.start)
        *(.text16)
    }
}
