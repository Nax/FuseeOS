#include <boot/bootloader/boot.h>

static uint64_t zero_page(uint64_t page)
{
    memset((void*)page, 0, PAGESIZE);
    return page;
}

uint64_t alloc_page_lo(void)
{
    static uint64_t page = 0x100000;

    uint64_t tmp = page;
    page += PAGESIZE;
    return zero_page(tmp);
}

uint64_t alloc_page_hi(void)
{
    uint64_t tmp;

    for (int i = 0; i < 32; ++i)
    {
        if (gBootParams.mem_free[i].size)
        {
            tmp = gBootParams.mem_free[i].base;
            gBootParams.mem_free[i].base += PAGESIZE;
            gBootParams.mem_free[i].size -= PAGESIZE;
            return zero_page(tmp);
        }
    }
    return 0;
}

static void decompose_addr(uint64_t virtual, int* a, int* b, int* c, int* d)
{
    *d = (virtual >> 12) & 0x1ff;
    *c = (virtual >> 21) & 0x1ff;
    *b = (virtual >> 30) & 0x1ff;
    *a = (virtual >> 39) & 0x1ff;
}

static void map_single(uint64_t virtual)
{
    uint64_t* tmp;
    uint64_t  tmp2;
    int       a, b, c, d;

    decompose_addr(virtual, &a, &b, &c, &d);

    tmp  = (uint64_t*)gBootParams.cr3;
    tmp2 = tmp[a];
    if (!tmp2)
    {
        tmp2   = (uint64_t)alloc_page_hi() | 3;
        tmp[a] = tmp2;
    }
    tmp  = (uint64_t*)(tmp2 & 0xfffffffffffff000);
    tmp2 = tmp[b];
    if (!tmp2)
    {
        tmp2   = (uint64_t)alloc_page_hi() | 3;
        tmp[b] = tmp2;
    }
    tmp  = (uint64_t*)(tmp2 & 0xfffffffffffff000);
    tmp2 = tmp[c];
    if (!tmp2)
    {
        tmp2   = (uint64_t)alloc_page_hi() | 3;
        tmp[c] = tmp2;
    }
    tmp    = (uint64_t*)(tmp2 & 0xfffffffffffff000);
    tmp[d] = alloc_page_hi() | 1;
}

void valloc(void* addr, size_t size)
{
    int npages;
    size += (((uint64_t)addr) & (PAGESIZE - 1));
    uint64_t vaddr = ((uint64_t)addr & ~(PAGESIZE - 1));
    npages = ((size + PAGESIZE - 1) >> 12);

    for (int i = 0; i < npages; ++i)
        map_single(vaddr + i * PAGESIZE);
    __asm__ __volatile__("mov %0, %%cr3\r\n" ::"a"(gBootParams.cr3));
}
