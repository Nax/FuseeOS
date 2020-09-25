#ifndef KERNEL_H
#define NERNEL_H 1

#include <kernel/params.h>
#include <string.h>
#include <strings.h>

#define BREAKPOINT __asm__ __volatile__("xchg %bx, %bx\r\n")
#define PAGESIZE   4096

#define PMB_COUNT     16
#define PMB_MAX_ORDER 16

#define PML4_KERNEL_IMAGE 0x1ff
#define PML4_KERNEL_HEAP  0x1fe
#define PML4_PHYSICAL     0x100

typedef struct
{
    uint64_t base;
    uint64_t npages;
    uint8_t* bitmap[PMB_MAX_ORDER];
} PhysicalMemoryBlock;
/* The main kernel structure */
typedef struct
{
    KernelBootParams    boot_params;
    PhysicalMemoryBlock pmem[PMB_COUNT];
    uint64_t*           cr3;
} Kernel;

extern Kernel gKernel;

void init_physical_mapping(void);
void init_physical_memory(void);

uint64_t alloc_phys(int npages);
uint64_t alloc_phys_early(int npages);

void* physical_to_virtual(uint64_t physical);

#endif
