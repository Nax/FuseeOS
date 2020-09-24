#ifndef _INCLUDED_MSF_H
#define _INCLUDED_MSF_H 1

#include <stdint.h>

#define MFS_PAGE_SIZE      4096
#define MFS_RESERVED_PAGES 64
#define MSF_METADATA_PAGE  63
#define MFS_BITMAP_SIZE    MFS_PAGE_SIZE * 8
#define MFS_DATA_POINTERS  16

#define MFS_TYPE(x)   (x & (0xffff0000))
#define MFS_TYPE_FILE 0x00000000
#define MFS_TYPE_DIR  0x00010000
#define MFS_MAGIC     "MFS"

typedef struct
{
    uint8_t  magic[4];
    uint32_t version;
    uint64_t page_count;
    uint64_t root;
} __attribute__((packed)) MfsPageMetadata;

typedef struct
{
    uint32_t mode;
    uint32_t zero;
    uint32_t uid;
    uint32_t gid;
    uint64_t size;
    uint64_t btime;
    uint64_t atime;
    uint64_t ctime;
    uint64_t mtime;
    uint64_t data[MFS_DATA_POINTERS];
    uint64_t idata;
    uint64_t idata2;
    uint64_t idata3;
} __attribute__((packed)) MfsPageFile;

typedef union
{
    MfsPageMetadata meta;
    MfsPageFile     file;
    uint8_t         data;
} MfsPage;

#endif
