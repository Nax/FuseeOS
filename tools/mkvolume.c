#include <fusee/mfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERSIZE 4096

typedef struct
{
    uint64_t size;
    union
    {
        char*    data;
        MfsPage* pages;
    };
    uint64_t root;
} MfsVolume;

/*
static uint64_t alloc_page(FILE* dst)
{
    static uint64_t inode;
    uint64_t        bit;
    uint64_t        bitmap;
    uint8_t         tmp;

    inode++;
    if ((inode % MFS_BITMAP_SIZE) == 0)
        inode++;
    bit    = inode % MFS_BITMAP_SIZE;
    bitmap = inode / MFS_BITMAP_SIZE;
    fseek(dst, (MFS_RESERVED_PAGES + bitmap) * MFS_PAGE_SIZE + bit / 8, SEEK_SET);
    fread(&tmp, 1, 1, dst);
    fseek(dst, (MFS_RESERVED_PAGES + bitmap) * MFS_PAGE_SIZE + bit / 8, SEEK_SET);
    tmp |= (1 << (bit % 8));
    fwrite(&tmp, 1, 1, dst);
    return inode;
}
*/

static void make_volume(MfsVolume* volume, int size)
{
    MfsPageMetadata* meta;

    /* Adjust the size so it's a multiple of the page size */
    size         = ((size + MFS_PAGE_SIZE - 1) / MFS_PAGE_SIZE) * MFS_PAGE_SIZE;
    volume->size = (uint64_t)size;
    volume->data = calloc(1, size);

    meta = &volume->pages[MSF_METADATA_PAGE].meta;
    memcpy(meta->magic, MFS_MAGIC, 4);
    meta->version    = 0x01000000;
    meta->page_count = volume->size / MFS_PAGE_SIZE;
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

int main(int argc, char** argv)
{
    MfsVolume volume;

    memset(&volume, 0, sizeof(volume));
    make_volume(&volume, atoi(argv[2]));
}

/*
int main(int argc, char** argv)
{
    FILE* f;

    f = fopen(argv[1], "wb");
    resize(f, atoi(argv[2]));
    add_vbr(f, argv[3]);
    fclose(f);
    return 0;
}
*/
