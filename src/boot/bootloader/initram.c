#include <boot/bootloader/boot.h>
#include <initram.h>

void initram_init(void)
{
    uint64_t inode;

    inode = mfs_lookup_root("boot");
    boot_printf("Found boot inode: 0x%llx\n", inode);
    inode = mfs_lookup_at(inode, "initram");
    boot_printf("Found initram inode: 0x%llx\n\n", inode);

    gBootParams.initram_size = mfs_file_size(inode);
    gBootParams.initram      = memory_alloc(gBootParams.initram_size);
    mfs_read(gBootParams.initram, inode);
    boot_printf(
        "Initram loaded at 0x%lx, size: 0x%lx\n", (uint32_t)gBootParams.initram, (uint32_t)gBootParams.initram_size);
}

char* initram_lookup(const char* name)
{
    char*             base;
    InitRamHeader*    h;
    InitRamFileEntry* fe;

    base = gBootParams.initram;
    h    = (InitRamHeader*)base;
    fe   = (InitRamFileEntry*)(base + h->fh_off);

    for (int i = 0; i < h->fh_cnt; ++i)
    {
        if (memcmp(name, base + h->sh_off + fe[i].name_off, fe[i].name_size) == 0)
            return base + h->data_off + fe[i].data_off;
    }

    return NULL;
}
