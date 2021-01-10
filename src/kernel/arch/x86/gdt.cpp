#include <kernel/kernel.h>

struct GdtEntrySystem
{
    uint16_t limit_lo;
    uint16_t base_lo;
    uint8_t  base_md;
    uint8_t  ac : 1;
    uint8_t  rw : 1;
    uint8_t  dc : 1;
    uint8_t  ex : 1;
    uint8_t  system : 1;
    uint8_t  priv : 2;
    uint8_t  present : 1;
    uint8_t  limit_hi : 4;
    uint8_t  zero : 1;
    uint8_t  l : 1;
    uint8_t  sz : 1;
    uint8_t  gr : 1;
    uint8_t  base_hi;
} _PACKED;

struct GdtDescriptor
{
    uint16_t size;
    void*    ptr;
} _PACKED;

static _ALIGN(0x10) uint8_t gGdtBuffer[0x1000];
static _ALIGN(0x10) GdtDescriptor gGdtDescriptor = {0x7, &gGdtBuffer};

_EXTERNC void enable_gdt(const GdtDescriptor* desc, uint64_t code_seg, uint64_t data_seg);

static void make_entry_system(int slot, int code, int priv)
{
    GdtEntrySystem* sys;

    sys           = (GdtEntrySystem*)(gGdtBuffer + slot);
    sys->limit_lo = 0xffff;
    sys->base_lo  = 0x0000;
    sys->base_md  = 0x00;
    sys->ac       = 1;
    sys->rw       = code ? 0 : 1;
    sys->dc       = 0;
    sys->ex       = code ? 1 : 0;
    sys->system   = 1;
    sys->priv     = priv & 0x3;
    sys->present  = 1;
    sys->limit_hi = 0xf;
    sys->zero     = 0;
    sys->l        = 1;
    sys->sz       = 0;
    sys->gr       = 1;
    sys->base_hi  = 0x00;

    if (gGdtDescriptor.size < (slot + 7))
    {
        gGdtDescriptor.size = slot + 7;
    }
}

void init_gdt()
{
    make_entry_system(X86_SEL_CODE0, 1, 0);
    make_entry_system(X86_SEL_DATA0, 0, 0);
    make_entry_system(X86_SEL_CODE3, 1, 3);
    make_entry_system(X86_SEL_DATA3, 0, 3);

    enable_gdt(&gGdtDescriptor, 0x8, 0x10);
}
