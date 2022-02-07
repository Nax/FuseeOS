#ifndef ARCH_X86_ARCH_H
#define ARCH_X86_ARCH_H 1

#include <kernel/arch/x86/tss.h>
#include <kernel/arch/x86/gdt.h>

/* Arch */
#define ARCH_PROC_REGS      18
#define ARCH_PROC_EXTRA     512

typedef struct ArchKernelThread
{
    TSS tss;
} ArchKernelThread;

typedef struct Process Process;
_EXTERNC void proc_run(Process*);
_EXTERNC void proc_run_sysret(Process*);

#endif
