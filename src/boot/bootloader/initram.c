#include <boot/bootloader/boot.h>
#include <initram.h>

void initram_init(void)
{
    uint64_t inode;

    inode = mfs_lookup_root("boot");
    boot_printf("Found boot inode: 0x%lx\n", inode);
    inode = mfs_lookup_at(inode, "initram");
    boot_printf("Found initram inode: 0x%lx\n\n", inode);

    gBootParams.initram = (void*)(-0x100000000);
    gBootParams.initram_size = mfs_file_size(inode);
    valloc(gBootParams.initram, gBootParams.initram_size);

    mfs_read(gBootParams.initram, inode);
    boot_printf(
        "Initram loaded at 0x%lx, size: 0x%lx\n", (uint64_t)gBootParams.initram, (uint64_t)gBootParams.initram_size);
}
