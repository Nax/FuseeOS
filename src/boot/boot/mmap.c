#include <boot/boot/boot.h>

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
    uint32_t  tmp2;
    int       a, b, c, d;

    decompose_addr(virtual, &a, &b, &c, &d);

    tmp  = (uint64_t*)g_kernel_params.cr4;
    tmp2 = tmp[a];
    if (!tmp2)
    {
        tmp2   = (uint32_t)zero_page_alloc() | 3;
        tmp[a] = tmp2;
    }
    tmp  = (uint64_t*)(tmp2 & 0xfffff000);
    tmp2 = tmp[b];
    if (!tmp2)
    {
        tmp2   = (uint32_t)zero_page_alloc() | 3;
        tmp[b] = tmp2;
    }
    tmp  = (uint64_t*)(tmp2 & 0xfffff000);
    tmp2 = tmp[c];
    if (!tmp2)
    {
        tmp2   = (uint32_t)zero_page_alloc() | 3;
        tmp[c] = tmp2;
    }
    tmp    = (uint64_t*)(tmp2 & 0xfffff000);
    tmp[d] = physical | 3;
}

void mmap64(void* src, uint64_t dst, uint64_t size)
{
    uint32_t npages = ((size + 4095) / 4096);

    if (!g_kernel_params.cr4)
        g_kernel_params.cr4 = zero_page_alloc();

    for (uint32_t i = 0; i < npages; ++i)
        mmap64_single((uint32_t)src + i * 4096, dst + i * 4096);
}