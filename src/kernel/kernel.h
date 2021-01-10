#ifndef KERNEL_H
#define NERNEL_H 1

#include <libboot/libboot.h>

#include <kernel/proc/ProcessTree.h>
#include <kernel/arch.h>
#include <kernel/mem.h>
#include <kernel/sections.h>
#include <string.h>
#include <strings.h>

#define BREAKPOINT   __asm__ __volatile__("xchg %bx, %bx\r\n")
#define kprintf(...) boot_printf(__VA_ARGS__)

struct Kernel;

/* Each thread gets one of these */
struct KernelThread
{
    Process*            proc;
    uint64_t            thread_id;
    ArchKernelThread    arch;
    char                stack[8192 - 8 * 2 - sizeof(ArchKernelThread)];
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
