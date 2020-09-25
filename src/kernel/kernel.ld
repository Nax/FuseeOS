ENTRY(_start)
SECTIONS
{
    . =  0xfffffff800000000;

    .text : { *(.text) }
    .data : { *(.data) }
    .rodata : { *(.rodata) }
    .bss : { *(.bss) }
}
