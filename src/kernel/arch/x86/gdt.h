#ifndef ARCH_X86_GDT_H
#define ARCH_X86_GDT_H

#include <stdint.h>
#include <sys/_cext.h>

_EXTERNC void gdt_add_segment(int slot, int code, int priv);
_EXTERNC void gdt_add_tss(int slot, uint64_t tss_addr);
_EXTERNC void gdt_reload(void);
_EXTERNC void gdt_init(void);

#endif
