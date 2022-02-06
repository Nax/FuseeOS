#ifndef KERNEL_PROC_PROCESS_H
#define KERNEL_PROC_PROCESS_H 1

#include <stdint.h>
#include <kernel/arch.h>
#include <kernel/collections/Array.h>

typedef struct ProcessAddrSpaceRange ProcessAddrSpaceRange;
typedef struct ProcessAddrSpace ProcessAddrSpace;
typedef struct Process Process;

#ifdef __cplusplus

struct ProcessAddrSpaceRange
{
    uint64_t   base;
    uint64_t   size;
};

struct ProcessAddrSpace
{
    uint64_t                        cr3;
    Array<ProcessAddrSpaceRange>    ranges;
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
#endif

_EXTERNC Process*    proc_create(void);
_EXTERNC Process*    proc_create_initram(const char* path);
_EXTERNC void*       proc_alloc(Process* proc, void* addr, uint64_t size, int prot);
_EXTERNC void        proc_exec(Process* proc);
_EXTERNC void        proc_schedule(Process* proc);
_EXTERNC void        proc_schedule_io(Process* proc);
_EXTERNC void        proc_run_next();

#endif
