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
void pic_init();
void pic_enable(int i);
void pic_disable(int i);
void idt_set_gate_interrupt(int interrupt, int dpl, void* handler);

struct Process;
_EXTERNC void proc_run(Process*);
_EXTERNC void proc_run_sysret(Process*);

void x86_sys_init();

#endif
