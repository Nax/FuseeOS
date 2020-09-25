ENTRY(_start)
SECTIONS
{
    . =  0xffffffff80000000;

    __KERNEL_IMAGE_START = .;
    __KERNEL_SECTION_EXEC_START = .;
    .text : { *(.text*) }
    __KERNEL_SECTION_EXEC_END = .;
    __KERNEL_SECTION_RODATA_START = .;
    .rodata : { *(.rodata*) }
    __KERNEL_SECTION_RODATA_END = .;
    __KERNEL_SECTION_DATA_START = .;
    .data : { *(.data*) }
    .bss : { *(.bss*) }
    __KERNEL_SECTION_DATA_END = .;
    __KERNEL_IMAGE_END = .;

    /DISCARD/ : { *(*) }
}
