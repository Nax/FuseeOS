#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <fusee/mfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUFFERSIZE 4096

typedef struct
{
    uint64_t size;
    union
    {
        char*    data;
        MfsPage* pages;
    };
    uint64_t now;
    uint64_t root;
    uint64_t next_inode;
} MfsVolume;

typedef struct
{
    MfsVolume*   volume;
    MfsPageFile* file;
    uint64_t     pos;
} MfsFile;

static MfsPage*
inode_page(MfsVolume* volume, uint64_t inode)
{
    return &volume->pages[MFS_RESERVED_PAGES + inode];
}

static uint64_t alloc_page(MfsVolume* volume)
{
    MfsPage* page_bitmap;
    uint64_t bit;
    uint64_t bitmap;

    volume->next_inode++;
    if ((volume->next_inode % MFS_BITMAP_SIZE) == 0)
        volume->next_inode++;
    bit         = volume->next_inode % MFS_BITMAP_SIZE;
    bitmap      = (volume->next_inode / MFS_BITMAP_SIZE) * MFS_BITMAP_SIZE;
    page_bitmap = inode_page(volume, bitmap);
    page_bitmap->data[bit / 8] |= (1 << (bit % 8));

    return volume->next_inode;
}

static void inode_open(MfsFile* f, MfsVolume* volume, uint64_t inode)
{
    f->volume = volume;
    f->file   = &inode_page(volume, inode)->file;
    f->pos    = 0;
}

static char* ptr_from_data_index(MfsVolume* volume, MfsPageFile* file, uint64_t index)
{
    MfsPage* page;
    uint64_t inode;

    if (index < 16)
    {
        inode = file->data[index];
        if (!inode)
        {
            inode             = alloc_page(volume);
            file->data[index] = inode;
        }
    }
    else if (index < MFS_IDATA_POINTERS + 16)
    {
        inode = file->idata;
        if (!inode)
        {
            inode       = alloc_page(volume);
            file->idata = inode;
        }
        page  = inode_page(volume, inode);
        inode = page->inodes[index - 16];
        if (!inode)
        {
            inode                    = alloc_page(volume);
            page->inodes[index - 16] = inode;
        }
    }
    else
    {
        /* ??? */
    }
    return inode_page(volume, inode)->data;
}

static uint64_t file_read(char* buf, MfsFile* f, uint64_t len)
{
    uint64_t    res;
    uint64_t    page_index;
    uint64_t    page_offset;
    uint64_t    tmp;
    const char* src;

    res = 0;

    for (;;)
    {
        if (!len || f->pos == f->file->size)
            return res;

        page_index  = f->pos / MFS_PAGE_SIZE;
        page_offset = f->pos % MFS_PAGE_SIZE;
        src         = ptr_from_data_index(f->volume, f->file, page_index);
        tmp         = MFS_PAGE_SIZE - page_offset;
        if (tmp > len)
            tmp = len;
        if (tmp + f->pos > f->file->size)
            tmp = f->file->size - f->pos;
        memcpy(buf + res, src + page_offset, tmp);
        res += tmp;
        f->pos += tmp;
        len -= tmp;
    }
}

static uint64_t file_write(const char* buf, MfsFile* f, uint64_t len)
{
    uint64_t res;
    uint64_t page_index;
    uint64_t page_offset;
    uint64_t tmp;
    char*    dst;

    res = 0;

    for (;;)
    {
        if (!len)
            return res;

        page_index  = f->pos / MFS_PAGE_SIZE;
        page_offset = f->pos % MFS_PAGE_SIZE;
        dst         = ptr_from_data_index(f->volume, f->file, page_index);
        tmp         = MFS_PAGE_SIZE - page_offset;
        if (tmp > len)
            tmp = len;
        memcpy(dst + page_offset, buf + res, tmp);
        res += tmp;
        f->pos += tmp;
        f->file->size += tmp;
        len -= tmp;
    }
}

static uint64_t create_file_raw(MfsVolume* volume, uint32_t mode)
{
    uint64_t     inode;
    MfsPageFile* file;

    inode       = alloc_page(volume);
    file        = &inode_page(volume, inode)->file;
    file->mode  = mode;
    file->btime = volume->now;
    file->atime = volume->now;
    file->ctime = volume->now;
    file->mtime = volume->now;

    return inode;
}

static uint64_t create_directory(MfsVolume* volume)
{
    return create_file_raw(volume, MFS_TYPE_DIR | 0755);
}

static uint64_t create_file(MfsVolume* volume)
{
    return create_file_raw(volume, MFS_TYPE_FILE | 0644);
}

static uint64_t find_file_dir(MfsVolume* volume, uint64_t dir_inode, const char* name)
{
    MfsFile     f;
    MfsDirEntry entry;
    char        namebuf[1024];

    inode_open(&f, volume, dir_inode);
    for (;;)
    {
        if (f.pos == f.file->size)
            return 0;
        file_read((char*)&entry, &f, sizeof(MfsDirEntry));
        file_read(namebuf, &f, entry.namelen);
        if (strncmp(namebuf, name, entry.namelen) == 0)
            return entry.inode;
    }
}

static void append_dir(MfsVolume* volume, uint64_t dir_inode, const char* name, uint64_t file_inode)
{
    MfsFile     f;
    MfsDirEntry entry;

    inode_open(&f, volume, dir_inode);
    f.pos         = f.file->size;
    entry.inode   = file_inode;
    entry.namelen = strlen(name);
    file_write((const char*)&entry, &f, sizeof(entry));
    file_write(name, &f, strlen(name));
}

static void make_volume(MfsVolume* volume, int size)
{
    MfsPageMetadata* meta;
    struct timespec  spec;

    /* Get current time */
    timespec_get(&spec, TIME_UTC);
    volume->now = spec.tv_sec * 1000 + spec.tv_nsec / 1000000;

    /* Adjust the size so it's a multiple of the page size */
    size         = ((size + MFS_PAGE_SIZE - 1) / MFS_PAGE_SIZE) * MFS_PAGE_SIZE;
    volume->size = (uint64_t)size;
    volume->data = calloc(1, size);

    meta = &volume->pages[MFS_METADATA_PAGE].meta;
    memcpy(meta->magic, MFS_MAGIC, 4);
    meta->version    = 0x01000000;
    meta->page_count = volume->size / MFS_PAGE_SIZE;
    meta->root       = create_directory(volume);
    volume->root     = meta->root;
}

static void make_vbr(MfsVolume* volume, const char* path)
{
    FILE*  src;
    size_t size;
    char*  dst;

    src = fopen(path, "rb");
    dst = volume->data;
    for (;;)
    {
        size = fread(dst, 1, BUFFERSIZE, src);
        if (!size)
            break;
        dst += size;
    }
    fclose(src);
}

static uint64_t copy_file(MfsVolume* volume, const char* path, const char* src)
{
    uint64_t dir_inode;
    uint64_t file_inode;
    uint64_t tmp;
    char     namebuf[1024];
    char     buf[BUFFERSIZE];
    FILE*    f;
    char*    e;
    int      len;
    MfsFile  mfs_file;

    len = strlen(path);
    memcpy(namebuf, path, len + 1);
    e         = namebuf;
    dir_inode = volume->root;
    for (int i = 0; i < len; ++i)
    {
        if (namebuf[i] == '/')
        {
            namebuf[i] = 0;
            tmp        = find_file_dir(volume, dir_inode, e);
            if (!tmp)
            {
                tmp = create_directory(volume);
                append_dir(volume, dir_inode, e, tmp);
            }
            dir_inode = tmp;
            e         = namebuf + i + 1;
        }
    }
    file_inode = create_file(volume);
    inode_open(&mfs_file, volume, file_inode);

    f = fopen(src, "rb");
    for (;;)
    {
        tmp = fread(buf, 1, BUFFERSIZE, f);
        if (!tmp)
            break;
        file_write(buf, &mfs_file, tmp);
    }
    fclose(f);
    append_dir(volume, dir_inode, e, file_inode);
}

static void parse_append_str(char* buf, FILE* f)
{
    int c;
    int cursor = 0;

    /* Skip leading whitespace */
    for (;;)
    {
        c = fgetc(f);
        if (!isspace(c))
            break;
    }

    buf[cursor++] = c;

    for (;;)
    {
        c = fgetc(f);
        if (isspace(c))
            break;
        buf[cursor++] = c;
    }

    buf[cursor] = 0;
}

static void apply_buildspec_file(MfsVolume* volume, FILE* f)
{
    char dst[1024];
    char src[1024];

    parse_append_str(dst, f);
    parse_append_str(src, f);
    copy_file(volume, dst, src);
}

static void apply_buildspec(MfsVolume* volume, const char* path)
{
    FILE* f;
    int   c;

    f = fopen(path, "r");
    for (;;)
    {
        if (feof(f))
            break;
        c = fgetc(f);
        switch (c)
        {
        case '#':
            while (c != '\n' && !feof(f))
            {
                c = fgetc(f);
            }
            break;
        case 'F':
            apply_buildspec_file(volume, f);
            break;
        default:
            break;
        }
    }
    fclose(f);
}

int main(int argc, char** argv)
{
    FILE*     f;
    MfsVolume volume;

    memset(&volume, 0, sizeof(volume));
    make_volume(&volume, atoi(argv[2]));
    make_vbr(&volume, argv[3]);
    apply_buildspec(&volume, argv[4]);
    f = fopen(argv[1], "wb");
    fwrite(volume.data, volume.size, 1, f);
    fclose(f);
    return 0;
}
