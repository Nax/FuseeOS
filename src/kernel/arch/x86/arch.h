#ifndef ARCH_X86_ARCH_H
#define ARCH_X86_ARCH_H 1

#include <kernel/arch/x86/TSS.h>

/* Arch */
#define ARCH_PROC_REGS      18
#define ARCH_PROC_EXTRA     512

struct ArchKernelThread
{
    TSS tss;
};

void gdt_add_segment(int slot, int code, int priv);
void gdt_add_tss(int slot, uint64_t tss_addr);
void gdt_reload();

#endif
