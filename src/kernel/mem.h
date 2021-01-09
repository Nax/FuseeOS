#ifndef KERNEL_MEM_H
#define KERNEL_MEM_H

#include <stddef.h>
#include <stdint.h>

#define PAGESIZE 4096

#define PMB_COUNT     16
#define PMB_MAX_ORDER 8

#define PML4_KERNEL_IMAGE 0x1ff
#define PML4_KERNEL_HEAP  0x1fe
#define PML4_PHYSICAL     0x100

#define BADPAGE 0xffffffffffffffffLL

#define KPROT_READ    0x00000001
#define KPROT_WRITE   0x00000002
#define KPROT_EXECUTE 0x00000004
#define KPROT_USER    0x00000008

#define KMAP_ANONYMOUS 0x00000001
#define KMAP_FIXED     0x00000002

#define MMASK_PHYS    0x7ffffffffffff000
#define MMASK_PROTECT 0x800000000000000e

inline static size_t page_count(size_t size) { return (size + PAGESIZE - 1) / PAGESIZE; }
inline static size_t page_round(size_t size) { return page_count(size) * PAGESIZE; }

#include <kernel/mem/HeapAlloc.h>

typedef struct
{
    uint64_t base;
    uint64_t npages;
    uint8_t* bitmap[PMB_MAX_ORDER];
} PhysicalMemoryBlock;

typedef struct
{
    PhysicalMemoryBlock blocks[PMB_COUNT];
    uint64_t            pages_total;
    uint64_t            pages_free;
} PhysicalMemoryAllocator;

typedef struct
{
    uint64_t base;
    uint64_t size;
} VirtualMemoryBlock;

typedef struct
{
    uint64_t            base;
    uint64_t            free_list_size;
    uint64_t            free_list_capacity;
    VirtualMemoryBlock* free_list;
} VirtualMemoryAllocator;

void init_mem(void);
void init_physical_mapping(void);
void init_physical_memory(void);
void init_virtual_memory(void);

uint64_t alloc_phys_pages(int npages);
uint64_t alloc_phys(uint64_t size);
uint64_t alloc_phys_early(int npages);

void free_phys_pages(uint64_t page, size_t npages);
void free_phys(uint64_t page, size_t size);

void* alloc_virtual(uint64_t size);

void* physical_to_virtual(uint64_t physical);

void  kmprotect(void* ptr, size_t size, int prot);
void* kmmap(void* ptr, uint64_t phys, size_t size, int prot, int flags);
void  kmunmap(void* ptr, size_t size);
void  kmunmap_tree(void* ptr, size_t size);
void  kmprotect_kernel(void);

void*   kmalloc(size_t size);
void    kfree(void* addr);

#endif
