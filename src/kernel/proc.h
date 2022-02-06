#ifndef KERNEL_PROC_PROCESS_H
#define KERNEL_PROC_PROCESS_H 1

#include <stdint.h>
#include <kernel/arch.h>

typedef struct ProcessAddrSpaceRange ProcessAddrSpaceRange;
typedef struct ProcessAddrSpace ProcessAddrSpace;
typedef struct Process Process;

struct ProcessAddrSpaceRange
{
    uint64_t   base;
    uint64_t   size;
};

struct ProcessAddrSpace
{
    uint64_t                        cr3;
};

typedef void (*ProcessRunFunc)(Process*);

struct Process
{
    uint64_t    regs[ARCH_PROC_REGS];
#if (ARCH_PROC_EXTRA > 0)
    char        extra[ARCH_PROC_EXTRA];
#endif
    ProcessRunFunc      run;
    ProcessAddrSpace    addr_space;
};

_EXTERNC void       proc_init(void);
_EXTERNC Process*   proc_create(void);
_EXTERNC Process*   proc_create_initram(const char* path);
_EXTERNC void       proc_schedule(Process* proc);
_EXTERNC void       proc_reschedule(void);
_EXTERNC void       kern_schedule(void);

#endif
