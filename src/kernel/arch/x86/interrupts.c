#include <kernel/arch/x86/asm.h>
#include <kernel/arch/x86/defs.h>
#include <kernel/arch/x86/interrupts.h>

typedef struct _PACKED
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
} IDTDesc;

static IDTDesc gIDT[256];

typedef struct _PACKED
{
    uint16_t    size;
    IDTDesc*    idt;
} IDTDescPtr;

_ALIGN(16) static IDTDescPtr gIDTDescPtr = { sizeof(gIDT) - 1, gIDT };

static void idt_reload()
{
    ASM("lidt (%0)\r\n" :: "a"(&gIDTDescPtr));
}

void idt_init()
{
    idt_reload();
}

void idt_set_gate_interrupt(int interrupt, int dpl, void* handler)
{
    IDTDesc* desc;

    desc = &gIDT[interrupt];
    desc->offset_lo = (uint64_t)handler & 0xffff;
    desc->selector  = X86_SEL_CODE0;
    desc->ist       = 0;
    desc->type      = 0xe;
    desc->z         = 0;
    desc->dpl       = dpl;
    desc->p         = 1;
    desc->offset_md = ((uint64_t)handler >> 16) & 0xffff;
    desc->offset_hi = ((uint64_t)handler >> 32);
    desc->zero      = 0;
    idt_reload();
}
