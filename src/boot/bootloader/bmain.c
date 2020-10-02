#include <boot/bootloader/boot.h>

KernelBootParams g_kernel_params;

typedef void (*KernelEntry)(KernelBootParams*);

static void kernel_params_init(int drive, const PartitionRecord* mbr)
{
    bzero(&g_kernel_params, sizeof(g_kernel_params));
    g_kernel_params.boot_drive = (uint32_t)drive;
    memcpy(&g_kernel_params.mbr_partition, mbr, sizeof(g_kernel_params.mbr_partition));
}

_NORETURN void bmain(int drive, const PartitionRecord* mbr)
{
    KernelEntry entry;

    kernel_params_init(drive, mbr);
    screen_init();
    puts("FuseeOS Bootloader\n");
    memory_detect();
    /* Identity map the first 1GiB */
    mmap64((void*)0, 0, 0x40000000);
    mfs_init();
    initram_init();

    entry = (KernelEntry)elf_load("/boot/kernel");

    /* Jump! */
    entry(&g_kernel_params);

    for (;;) {}
}
