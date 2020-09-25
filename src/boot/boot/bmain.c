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
    kernel_params_init(drive, mbr);
    screen_init();
    puts("FuseeOS Bootloader\n");
    memory_detect();
    mfs_init();
    initram_init();

    print("Kernel found at ");
    puthex32((uint32_t)initram_lookup("/boot/kernel"));
    putchar('\n');

    for (;;)
    {
    }
}
