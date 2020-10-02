OUTPUT_FORMAT(binary)
SECTIONS {
    . = 0x7c00;

    .text ALIGN(0x0) : {
        *(.text.start*)
        *(.text.lo*)
        *(.data.lo*)
        *(.rodata.lo*)
        *(.bss.lo*)
        *(.text*)
        *(.data*)
        *(.rodata*)
        *(.bss*)
    }

    /DISCARD/ : { *(.*) }
}
