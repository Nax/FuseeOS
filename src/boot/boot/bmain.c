#include <boot/boot/boot.h>

KernelBootParams g_kernel_params;

static void
kernel_params_init(int drive, const PartitionRecord* mbr)
{
    bzero(&g_kernel_params, sizeof(g_kernel_params));
    g_kernel_params.boot_drive = (uint32_t)drive;
    memcpy(&g_kernel_params.mbr_partition, mbr, sizeof(g_kernel_params.mbr_partition));
}

_NORETURN void bmain(int drive, const PartitionRecord* mbr)
{
    uint64_t entry;

    kernel_params_init(drive, mbr);
    screen_init();
    puts("FuseeOS Bootloader\n");
    memory_detect();
    mfs_init();
    initram_init();

    entry = elf_load("/boot/kernel");

    /* Identity map the first gigabyte */
    mmap64((void*)0, 0, 0x20000000);

    /* Jump! */
    jump_mode_long(entry, (uint32_t)g_kernel_params.cr3, &g_kernel_params);

    for (;;)
    {
    }
}
