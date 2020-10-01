#ifndef KERNEL_H
#define NERNEL_H 1

#include <kernel/arch.h>
#include <kernel/mem.h>
#include <kernel/params.h>
#include <kernel/sections.h>
#include <kernel/video/video.h>
#include <string.h>
#include <strings.h>

#define BREAKPOINT __asm__ __volatile__("xchg %bx, %bx\r\n")

/* The main kernel structure */
typedef struct
{
    KernelBootParams        boot_params;
    PhysicalMemoryAllocator pmem;
    VirtualMemoryAllocator  vmem;
    Video                   video;
    uint64_t                nx_mask;
    uint64_t*               cr3;
    char*                   initram;
    size_t                  initram_size;
} Kernel;

extern Kernel gKernel;

/* print */
void kprintf(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

/* gdt */
void init_gdt(void);

void arch_init(void);

#endif
