ENTRY(_start)
SECTIONS
{
    . =  0xffffffff80000000;

    __KERNEL_IMAGE_START = .;
    .text ALIGN(4K) :
    {
        __KERNEL_SECTION_EXEC_START = .;
        *(.text*)
        __KERNEL_SECTION_EXEC_END = .;
    }
    .rodata ALIGN(4K) :
    {
        __KERNEL_SECTION_RODATA_START = .;
        *(.rodata*)
        __KERNEL_SECTION_RODATA_END = .;
    }
    .data ALIGN(4K) :
    {
        __KERNEL_SECTION_DATA_START = .;
        *(.data*)
        __KERNEL_SECTION_DATA_END = .;
    }
    .bss ALIGN(4K) :
    {
        __KERNEL_SECTION_BSS_START = .;
        *(.bss*)
        __KERNEL_SECTION_BSS_END = .;
    }
    __KERNEL_IMAGE_END = .;

    /DISCARD/ : { *(*) }
}
