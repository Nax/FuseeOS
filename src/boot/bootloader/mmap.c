#include <boot/bootloader/boot.h>

static void* zero_page_alloc()
{
    void* p;

    p = memory_alloc(4096);
    memset(p, 0, 4096);
    return p;
}

static void decompose_addr(uint64_t virtual, int* a, int* b, int* c, int* d)
{
    *d = (virtual >> 12) & 0x1ff;
    *c = (virtual >> 21) & 0x1ff;
    *b = (virtual >> 30) & 0x1ff;
    *a = (virtual >> 39) & 0x1ff;
}

static void mmap64_single(uint64_t physical, uint64_t virtual)
{
    uint64_t* tmp;
    uint64_t  tmp2;
    int       a, b, c, d;

    decompose_addr(virtual, &a, &b, &c, &d);

    tmp  = (uint64_t*)gBootParams.cr3;
    tmp2 = tmp[a];
    if (!tmp2)
    {
        tmp2   = (uint64_t)zero_page_alloc() | 3;
        tmp[a] = tmp2;
    }
    tmp  = (uint64_t*)(tmp2 & 0xfffffffffffff000);
    tmp2 = tmp[b];
    if (!tmp2)
    {
        tmp2   = (uint64_t)zero_page_alloc() | 3;
        tmp[b] = tmp2;
    }
    tmp  = (uint64_t*)(tmp2 & 0xfffffffffffff000);
    tmp2 = tmp[c];
    if (!tmp2)
    {
        tmp2   = (uint64_t)zero_page_alloc() | 3;
        tmp[c] = tmp2;
    }
    tmp    = (uint64_t*)(tmp2 & 0xfffffffffffff000);
    tmp[d] = physical | 3;
}

static void mmap64_2mb(int i, int j)
{
    uint64_t* tmp;
    uint64_t  tmp2;

    tmp  = (uint64_t*)gBootParams.cr3;
    tmp2 = tmp[0];
    if (!tmp2)
    {
        tmp2   = (uint64_t)zero_page_alloc() | 3;
        tmp[0] = tmp2;
    }
    tmp  = (uint64_t*)(tmp2 & 0xfffffffffffff000);
    tmp2 = tmp[i];
    if (!tmp2)
    {
        tmp2   = (uint64_t)zero_page_alloc() | 3;
        tmp[i] = tmp2;
    }
    tmp    = (uint64_t*)(tmp2 & 0xfffffffffffff000);
    tmp[j] = (((((uint64_t)i * 0x200) + (uint64_t)j) * 0x200000) | 0x83);
}

void mmap64(void* src, uint64_t dst, uint64_t size)
{
    uint64_t npages = ((size + 4095) / 4096);

    if (!gBootParams.cr3) { gBootParams.cr3 = zero_page_alloc(); }

    for (uint64_t i = 0; i < npages; ++i)
        mmap64_single((uint64_t)src + i * 4096, dst + i * 4096);
    __asm__ __volatile__("mov %0, %%cr3\r\n" ::"a"(gBootParams.cr3));
}

void mmap64_4GiB()
{
    if (!gBootParams.cr3) { gBootParams.cr3 = zero_page_alloc(); }

    for (uint64_t i = 0; i < 4; ++i)
    {
        for (uint64_t j = 0; j < 0x200; ++j)
        {
            mmap64_2mb(i, j);
        }
    }
    __asm__ __volatile__("mov %0, %%cr3\r\n" ::"a"(gBootParams.cr3));
}
