#include <kernel/kernel.h>
#include <stdint.h>

struct IDTDesc
{
    uint16_t offset_lo;
    uint16_t selector;
} _PACKED;
