#ifndef KERNEL_H
#define NERNEL_H 1

#include <kernel/mem.h>
#include <kernel/params.h>
#include <string.h>
#include <strings.h>

#define BREAKPOINT __asm__ __volatile__("xchg %bx, %bx\r\n")

/* The main kernel structure */
typedef struct
{
    KernelBootParams        boot_params;
    PhysicalMemoryAllocator pmem;
    VirtualMemoryAllocator  vmem;
    uint64_t*               cr3;
} Kernel;

extern Kernel gKernel;

/* print */
void init_screen(void);
void putchar(int c);
void print(const char* str);
void puts(const char* str);
void puthex8(uint8_t v);
void puthex16(uint16_t v);
void puthex32(uint32_t v);
void puthex64(uint64_t v);

#endif
