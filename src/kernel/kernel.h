#ifndef KERNEL_H
#define NERNEL_H 1

#include <kernel/mem.h>
#include <kernel/params.h>
#include <kernel/sections.h>
#include <string.h>
#include <strings.h>

#define BREAKPOINT __asm__ __volatile__("xchg %bx, %bx\r\n")

/* The main kernel structure */
typedef struct
{
    KernelBootParams        boot_params;
    PhysicalMemoryAllocator pmem;
    VirtualMemoryAllocator  vmem;
    uint16_t*               screenbuf;
    uint64_t                nx_mask;
    uint64_t*               cr3;
    char*                   initram;
    size_t                  initram_size;
} Kernel;

extern Kernel gKernel;

/* print */
void init_screen(void);
void init_screen_early(void);
void putchar(int c);
void print(const char* str);
void puts(const char* str);
void puthex8(uint8_t v);
void puthex16(uint16_t v);
void puthex32(uint32_t v);
void puthex64(uint64_t v);

/* gdt */
void init_gdt(void);

void arch_init(void);

#endif
