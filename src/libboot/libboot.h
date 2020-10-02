#ifndef LIBBOOT_H
#define LIBBOOT_H 1

#include <stdint.h>
#include <sys/_cext.h>

#define MAX_MEM_REGIONS 64

#define CONCAT(a, b) a##b

#if defined(__x86_64__)
#define PTR64(type, name, ap) type* name
#else
#define PTR64(type, name, ap)                                                                                          \
    type*    name;                                                                                                     \
    uint32_t CONCAT(zero_, ap)
#endif

typedef struct
{
    uint8_t  unused[8];
    uint32_t lba_start;
    uint32_t lba_size;
} _PACKED BootPartitionRecord;

typedef struct
{
    uint64_t base;
    uint64_t size;
} BootMemRegion;

typedef struct
{
    uint32_t            boot_drive;
    BootPartitionRecord mbr_partition;
    BootMemRegion       mem_map[MAX_MEM_REGIONS];
    BootMemRegion       mem_free[MAX_MEM_REGIONS];
    PTR64(char, initram, 0);
    uint64_t initram_size;
    PTR64(char, cr3, 1);
} _PACKED BootParams;

#endif
