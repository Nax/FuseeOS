#ifndef KERNEL_H
#define NERNEL_H 1

#include <libboot/libboot.h>

#include <kernel/proc/ProcessTree.h>
#include <kernel/arch.h>
#include <kernel/defs.h>
#include <kernel/mem.h>
#include <kernel/sections.h>
#include <string.h>
#include <strings.h>

#define BREAKPOINT   __asm__ __volatile__("xchg %bx, %bx\r\n")
#define kprintf(...) boot_printf(__VA_ARGS__)

union SysArg
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
};

struct Process;

using SysHandler = int64_t (*)(Process* proc, SysArg* args);

_EXTERNC SysHandler gSysHandlers[KERNEL_MAX_SYSCALL];

struct Kernel;

/* Each thread gets one of these */
struct alignas(4096) KernelThread
{
    KernelThread*       self;
    Process*            proc;
    uint64_t            thread_id;
    uint64_t            scratch;
    char                stack[KERNEL_STACK_SIZE];
    ArchKernelThread    arch;
};

/* The main kernel structure */
struct Kernel
{
    PhysicalMemoryAllocator pmem;
    VirtualMemoryAllocator  vmem;
    HeapAlloc               heap;
    IOAlloc                 io;
    ProcessTree             procs;
    KernelThread*           threads;
    uint64_t                nx_mask;
    uint64_t*               cr3;
};

extern Kernel gKernel;

/* gdt */
void gdt_init(void);

void arch_init(void);

void thread_init();
void timer_ns(std::uint64_t ns);

_EXTERNC void int_timer(void);
_EXTERNC void kern_schedule(void);

#endif
