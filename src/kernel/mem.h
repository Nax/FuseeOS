#ifndef KERNEL_MEM_H
#define KERNEL_MEM_H

#include <sys/_cext.h>
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

_EXTERNC void init_mem(void);
_EXTERNC void init_physical_mapping(void);
_EXTERNC void init_physical_memory(void);
_EXTERNC void init_virtual_memory(void);

_EXTERNC uint64_t alloc_phys_pages(int npages);
_EXTERNC uint64_t alloc_phys(uint64_t size);
_EXTERNC uint64_t alloc_phys_early(int npages);

_EXTERNC void free_phys_pages(uint64_t page, size_t npages);
_EXTERNC void free_phys(uint64_t page, size_t size);

_EXTERNC void* alloc_virtual(uint64_t size);

_EXTERNC void* physical_to_virtual(uint64_t physical);

_EXTERNC void  kmprotect(void* ptr, size_t size, int prot);
_EXTERNC void* kmmap(void* ptr, uint64_t phys, size_t size, int prot, int flags);
_EXTERNC void  kmunmap(void* ptr, size_t size);
_EXTERNC void  kmunmap_tree(void* ptr, size_t size);
_EXTERNC void  kmprotect_kernel(void);

_EXTERNC void* io_alloc(size_t size);
_EXTERNC void  io_free(void* addr);

_EXTERNC void* kmalloc(size_t size, int flags);
_EXTERNC void  kfree(void* addr);

#endif
