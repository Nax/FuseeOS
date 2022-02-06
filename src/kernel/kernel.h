#ifndef KERNEL_H
#define NERNEL_H 1

#include <libboot/libboot.h>

#include <kernel/arch.h>
#include <kernel/defs.h>
#include <kernel/mem.h>
#include <kernel/print.h>
#include <kernel/sections.h>
#include <kernel/proc.h>
#include <string.h>
#include <strings.h>

#define BREAKPOINT   __asm__ __volatile__("xchg %bx, %bx\r\n")

typedef union
{
    uint8_t     u8;
    uint16_t    u16;
    uint32_t    u32;
    uint64_t    u64;
    int8_t      i8;
    int16_t     i16;
    int32_t     i32;
    int64_t     i64;
    void*       ptr;
} SysArg;

struct Process;

typedef int64_t (*SysHandler)(Process* proc, SysArg* args);

extern SysHandler gSysHandlers[KERNEL_MAX_SYSCALL];

struct Kernel;

/* Each thread gets one of these */
typedef struct _ALIGN(4096) KernelThread
{
    KernelThread*       self;
    Process*            proc;
    uint64_t            thread_id;
    uint64_t            scratch;
    char                stack[KERNEL_STACK_SIZE];
    ArchKernelThread    arch;
} KernelThread;

/* The main kernel structure */
typedef struct Kernel
{
    PhysicalMemoryAllocator pmem;
    VirtualMemoryAllocator  vmem;
    size_t                  heap_size;
    size_t                  io_size;
    KernelThread*           threads;
    uint64_t                nx_mask;
    uint64_t*               cr3;
} Kernel;

extern Kernel gKernel;

_EXTERNC void arch_init(void);
_EXTERNC void thread_init(void);
_EXTERNC void timer_ns(uint64_t ns);

#endif
