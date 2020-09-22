ENTRY(_start)
SECTIONS
{
    . =  0xffffffff80000000;

    .text : { *(.text) }
    .data : { *(.data) }
    .rodata : { *(.rodata) }
    .bss : { *(.bss) }
}
