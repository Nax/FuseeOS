#ifndef _INCLUDED_INITRAM_H
#define _INCLUDED_INITRAM_H 1

#define INITRAM_MAGIC "INITRAM\x7f"

#include <stdint.h>

typedef struct
{
    uint8_t  magic[8];
    uint64_t fh_cnt;
    uint64_t fh_off;
    uint64_t fh_size;
    uint64_t sh_off;
    uint64_t sh_size;
    uint64_t data_off;
    uint64_t data_size;
} __attribute__((packed)) InitRamHeader;

typedef struct
{
    uint64_t name_off;
    uint64_t name_size;
    uint64_t data_off;
    uint64_t data_size;
} __attribute__((packed)) InitRamFileEntry;

#endif
