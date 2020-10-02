#include <boot/bootloader/boot.h>

typedef void (*KernelEntry)(BootParams*);

static void kernel_params_init(int drive, const BootPartitionRecord* mbr)
{
    gBootParams.boot_drive = (uint32_t)drive;
    memcpy(&gBootParams.mbr_partition, mbr, sizeof(gBootParams.mbr_partition));
}

_NORETURN void bmain(int drive, const BootPartitionRecord* mbr)
{
    KernelEntry entry;

    kernel_params_init(drive, mbr);
    screen_init();
    boot_printf("FuseeOS Bootloader\n");
    memory_detect();
    /* Identity map the first 1GiB */
    mmap64((void*)0, 0, 0x40000000);
    mfs_init();
    initram_init();

    entry = (KernelEntry)elf_load("/boot/kernel");

    /* Jump! */
    entry(&gBootParams);

    for (;;) {}
}
