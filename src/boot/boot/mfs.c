#include <boot/boot/boot.h>
#include <mfs.h>

static MfsPage gMetadata;
static MfsPage gPages[2];

static void mfs_read_page(MfsPage* page, uint64_t inode)
{
    disk_read_raw((char*)page, (MFS_RESERVED_PAGES + inode) * 8, MFS_PAGE_SIZE / 512);
}

void mfs_init(void)
{
    disk_read_raw((char*)&gMetadata, MFS_METADATA_PAGE * 8, MFS_PAGE_SIZE / 512);
    print("Found root inode: ");
    puthex64(gMetadata.meta.root);
    putchar('\n');
}

uint64_t mfs_lookup_at(uint64_t inode, const char* name)
{
    MfsDirEntry* dir;
    uint32_t     namelen;

    mfs_read_page(&gPages[0], inode);
    mfs_read_page(&gPages[0], gPages[0].file.data[0]);
    dir     = &gPages[0].dir;
    namelen = strlen(name);

    for (;;)
    {
        if (dir->namelen == namelen && memcmp(name, dir->name, namelen) == 0)
        {
            return dir->inode;
        }
        dir = (MfsDirEntry*)(((char*)(dir + 1)) + dir->namelen);
    }
}

uint64_t mfs_lookup_root(const char* name)
{
    return mfs_lookup_at(gMetadata.meta.root, name);
}
