#ifndef KERNEL_MEM_H
#define KERNEL_MEM_H

#include <stdint.h>

#define PAGESIZE 4096

#define PMB_COUNT     16
#define PMB_MAX_ORDER 8

#define PML4_KERNEL_IMAGE 0x1ff
#define PML4_KERNEL_HEAP  0x1fe
#define PML4_PHYSICAL     0x100

#define BADPAGE 0xffffffffffffffffLL

typedef struct
{
    uint64_t base;
    uint64_t npages;
    uint8_t* bitmap[PMB_MAX_ORDER];
} PhysicalMemoryBlock;

void init_physical_mapping(void);
void init_physical_memory(void);

uint64_t alloc_phys_pages(int npages);
uint64_t alloc_phys(uint64_t size);
uint64_t alloc_phys_early(int npages);

void* physical_to_virtual(uint64_t physical);

#endif
