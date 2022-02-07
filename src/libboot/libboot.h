#ifndef LIBBOOT_H
#define LIBBOOT_H 1

#include <libboot/video/video.h>
#include <stdint.h>
#include <sys/_cext.h>

#define MAX_MEM_REGIONS 64
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
}
BootMemRegion;

typedef struct
{
    uint32_t            boot_drive;
    BootPartitionRecord mbr_partition;
    BootVideo           video;
    BootMemRegion       mem_map[MAX_MEM_REGIONS];
    BootMemRegion       mem_free[MAX_MEM_REGIONS];
    char*               initram;
    uint64_t            initram_size;
    char*               cr3;
} BootParams;

extern BootParams gBootParams;

_EXTERNC void boot_printf(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
_EXTERNC char* initram_lookup(const char* name);

#endif
