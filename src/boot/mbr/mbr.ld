OUTPUT_FORMAT(binary)
SECTIONS {
    . = 0x0600;
    .text : { *(.text) }
}
