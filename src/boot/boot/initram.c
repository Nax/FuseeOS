#include <boot/boot/boot.h>

void initram_init(void)
{
    uint64_t inode;

    inode = mfs_lookup_root("boot");
    print("Found boot inode: ");
    puthex64(inode);
    putchar('\n');
    inode = mfs_lookup_at(inode, "initram");
    print("Found initram inode: ");
    puthex64(inode);
    putchar('\n');
    putchar('\n');

    g_kernel_params.initram_size = mfs_file_size(inode);
    g_kernel_params.initram      = memory_alloc(g_kernel_params.initram_size);
    mfs_read(g_kernel_params.initram, inode);

    print("Initram loaded at ");
    puthex32((uint32_t)g_kernel_params.initram);
    print(", size: ");
    puthex32((uint32_t)g_kernel_params.initram_size);
    putchar('\n');
}
