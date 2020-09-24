#ifndef _KERNEL_PARAMS_H
#define _KERNEL_PARAMS_H 1

#include <stdint.h>
#include <sys/_cext.h>

#define KERNEL_MAX_MEM_REGIONS 64

typedef struct
{
    uint8_t  unused[8];
    uint32_t lba_start;
    uint32_t lba_size;
} _PACKED PartitionRecord;

typedef struct
{
    uint64_t base;
    uint64_t size;
} KernelBootMemRegion;

typedef struct
{
    uint32_t        boot_drive;
    PartitionRecord mbr_partition;

    KernelBootMemRegion mem_map[KERNEL_MAX_MEM_REGIONS];
    KernelBootMemRegion mem_free[KERNEL_MAX_MEM_REGIONS];
} KernelBootParams;

#endif
