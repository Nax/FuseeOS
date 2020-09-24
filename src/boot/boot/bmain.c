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
    uint64_t inode;

    kernel_params_init(drive, mbr);
    screen_init();
    puts("FuseeOS Bootloader\n");
    memory_detect();
    mfs_init();
    inode = mfs_lookup_root("boot");
    print("Found boot inode: ");
    puthex64(inode);
    putchar('\n');
    inode = mfs_lookup_at(inode, "initram");
    print("Found initram inode: ");
    puthex64(inode);
    putchar('\n');
    for (;;)
    {
    }
}
