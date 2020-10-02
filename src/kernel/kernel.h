#ifndef KERNEL_H
#define NERNEL_H 1

#include <libboot/libboot.h>

#include <kernel/arch.h>
#include <kernel/mem.h>
#include <kernel/sections.h>
#include <string.h>
#include <strings.h>

#define BREAKPOINT   __asm__ __volatile__("xchg %bx, %bx\r\n")
#define kprintf(...) boot_printf(__VA_ARGS__)

/* The main kernel structure */
typedef struct
{
    PhysicalMemoryAllocator pmem;
    VirtualMemoryAllocator  vmem;
    uint64_t                nx_mask;
    uint64_t*               cr3;
    char*                   initram;
    size_t                  initram_size;
} Kernel;

extern Kernel gKernel;

/* gdt */
void init_gdt(void);

void arch_init(void);

#endif
