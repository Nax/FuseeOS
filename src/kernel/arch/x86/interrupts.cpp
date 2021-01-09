#include <kernel/kernel.h>
#include <stdint.h>

struct IDTEntry
{
    uint16_t    offset_lo;
    uint16_t    selector;
    uint8_t     ist;
    uint8_t     type:4;
    uint8_t     z:1;
    uint8_t     dpl:2;
    uint8_t     p:1;
    uint16_t    offset_md;
    uint32_t    offset_hi;
    uint32_t    zero;
} _PACKED;

static IDTEntry gIDT[256];

struct alignas(16) IDTDescriptor
{
    uint16_t    size;
    IDTEntry*   idt;
} _PACKED;

static IDTDescriptor gIDTDescriptor = { sizeof(gIDT) - 1, gIDT };

void init_idt()
{
    ASM("lidt (%0)\r\n" :: "a"(&gIDTDescriptor));
}
