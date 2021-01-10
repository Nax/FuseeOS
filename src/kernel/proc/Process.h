#ifndef KERNEL_PROC_PROCESS_H
#define KERNEL_PROC_PROCESS_H 1

#include <cstdint>
#include <kernel/arch.h>

struct Process
{
    Process() : regs{}, extra{} {};

    uint64_t    regs[ARCH_PROC_REGS];
#if (ARCH_PROC_EXTRA > 0)
    char        extra[ARCH_PROC_EXTRA];
#endif
};

void load_proc_initram(const char* path);

#endif
