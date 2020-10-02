#include <boot/bootloader/boot.h>
#include <mfs.h>

static MfsPage gMetadata;
static MfsPage gPages[4];

static void mfs_read_page(MfsPage* page, uint64_t inode)
{
    disk_read_raw((char*)page, (MFS_RESERVED_PAGES + inode) * 8, MFS_PAGE_SIZE / 512);
}

void mfs_init(void)
{
    disk_read_raw((char*)&gMetadata, MFS_METADATA_PAGE * 8, MFS_PAGE_SIZE / 512);
    boot_printf("Found root inode: 0x%lx\n", gMetadata.meta.root);
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
        if (dir->namelen == namelen && memcmp(name, dir->name, namelen) == 0) { return dir->inode; }
        dir = (MfsDirEntry*)(((char*)(dir + 1)) + dir->namelen);
    }
}

uint64_t mfs_lookup_root(const char* name) { return mfs_lookup_at(gMetadata.meta.root, name); }

uint64_t mfs_file_size(uint64_t inode)
{
    mfs_read_page(&gPages[0], inode);
    return gPages[0].file.size;
}

static char* mfs_read_idata(char* dst, uint32_t count)
{
    for (uint32_t i = 0; i < count; ++i)
    {
        mfs_read_page((MfsPage*)dst, gPages[1].inodes[i]);
        dst += MFS_PAGE_SIZE;
    }
    return dst;
}

static char* mfs_read_data(char* dst, uint32_t count)
{
    for (uint32_t i = 0; i < count; ++i)
    {
        mfs_read_page((MfsPage*)dst, gPages[0].file.data[i]);
        dst += MFS_PAGE_SIZE;
    }
    return dst;
}

void mfs_read(char* dst, uint64_t inode)
{
    uint32_t npages;
    uint32_t tmp;

    npages = (mfs_file_size(inode) + MFS_PAGE_SIZE - 1) / MFS_PAGE_SIZE;
    if (!npages) return;
    mfs_read_page(&gPages[0], inode);
    tmp = (npages > MFS_DATA_POINTERS) ? MFS_DATA_POINTERS : npages;
    dst = mfs_read_data(dst, tmp);
    npages -= tmp;
    if (!npages) return;
    mfs_read_page(&gPages[1], gPages[0].file.idata);
    tmp = (npages > MFS_IDATA_POINTERS) ? MFS_IDATA_POINTERS : npages;
    dst = mfs_read_idata(dst, tmp);
    npages -= tmp;
    if (!npages) return;
}
