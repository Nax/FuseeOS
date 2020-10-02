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
    vbe_init();
    memory_detect();
    mmap64_4GiB();
    boot_printf("FuseeOS Bootloader\n");
    boot_printf("Detected memory:\n");
    for (int i = 0; gBootParams.mem_map[i].size; ++i)
    {
        boot_printf("  0x%lx - 0x%lx\n",
                    gBootParams.mem_map[i].base,
                    gBootParams.mem_map[i].base + gBootParams.mem_map[i].size - 1);
    }
    boot_printf("\n");
    mfs_init();
    initram_init();

    entry = (KernelEntry)elf_load("/boot/kernel");

    /* Jump! */
    entry(&gBootParams);

    for (;;) {}
}
