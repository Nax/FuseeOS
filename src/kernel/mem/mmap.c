#include <kernel/kernel.h>

typedef struct
{
    uint64_t*   dir[4];
    int         offset[4];
    int         len[4];
    int         len_current[4];
    int         index[4];
    int         state;
} PageIterator;

static void page_iter_create(PageIterator* iter, uint64_t cr3, void* vaddr, size_t len)
{
    uint64_t vpage = ((uint64_t)vaddr >> 12);
    uint64_t vlen = (((uint64_t)len + (1 << 12) - 1) >> 12);

    /* Compute initial offsets */
    /* We can pre-compute that because only the
       first chunk of each size can be misaligned */
    for (int i = 0; i < 4; ++i)
    {
        iter->offset[i] = (vpage >> (9 * i)) % 512;
        iter->len[i] = ((vlen + (1 << (9 * i)) - 1) >> (9 * i));
    }

    iter->dir[3] = (uint64_t*)physical_to_virtual(cr3);
    iter->state = 0;
}

static uint64_t* iter_compute_dir(uint64_t* dir, int index, int offset)
{
    return (uint64_t*)physical_to_virtual(dir[offset + index] & MMASK_PHYS);
}

static int iter_compute_len(int len, int offset)
{
    len += offset;
    if (len > 512)
        len = 512;
    return len - offset;
}

static int page_iter_next(PageIterator* iter, int* out_depth, uint64_t** out_dir, int* out_len)
{
    switch (iter->state)
    {
    case 0:
        *out_depth = 3;
        *out_dir = iter->dir[3] + iter->offset[3];
        *out_len = iter->len[3];
        iter->state = 1;
        return 0;
    case 1:
        for (iter->index[3] = 0; iter->index[3] < iter->len[3]; iter->index[3]++)
        {
            iter->dir[2] = iter_compute_dir(iter->dir[3], iter->index[3], iter->offset[3]);
            iter->len_current[2] = iter_compute_len(iter->len[2], iter->offset[2]);
            *out_depth = 2;
            *out_dir = iter->dir[2] + iter->offset[2];
            *out_len = iter->len_current[2];
            iter->state = 2;
            return 0;
    case 2:
            for (iter->index[2] = 0; iter->index[2] < iter->len[2]; iter->index[2]++)
            {
                iter->dir[1] = iter_compute_dir(iter->dir[2], iter->index[2], iter->offset[2]);
                iter->len_current[1] = iter_compute_len(iter->len[1], iter->offset[1]);
                *out_depth = 1;
                *out_dir = iter->dir[1] + iter->offset[1];
                *out_len = iter->len_current[1];
                iter->state = 3;
                return 0;
    case 3:
                for (iter->index[1] = 0; iter->index[1] < iter->len[1]; iter->index[1]++)
                {
                    iter->dir[0] = iter_compute_dir(iter->dir[1], iter->index[1], iter->offset[1]);
                    iter->len_current[0] = iter_compute_len(iter->len[0], iter->offset[0]);
                    *out_depth = 0;
                    *out_dir = iter->dir[0] + iter->offset[0];
                    *out_len = iter->len_current[0];
                    iter->state = 4;
                    return 0;
    case 4:
                    iter->len[0] -= iter->len_current[0];
                    iter->offset[0] = 0;
                }
                iter->len[1] -= iter->len_current[1];
                iter->offset[1] = 0;
            }
            iter->len[2] -= iter->len_current[2];
            iter->offset[2] = 0;
        }
    }
    return 1;
}

static uint64_t alloc_zero_page(void)
{
    uint64_t page;

    page = alloc_phys_pages(1);
    bzero(physical_to_virtual(page), PAGESIZE);
    return page;
}

void* physical_to_virtual(uint64_t physical)
{
    return (void*)(0xffff800000000000 | physical);
}

static uint64_t page_prot_flags(int prot)
{
    uint64_t flags = 0;

    if (prot & KPROT_WRITE)
        flags |= X86_PAGE_WRITE;
    if (prot & KPROT_USER)
        flags |= X86_PAGE_USER;
    if (!(prot & KPROT_EXECUTE))
        flags |= gKernel.nx_mask;

    return flags;
}

void kmprotect(void* addr, size_t size, int prot)
{
    PageIterator iter;
    uint64_t* page_dir;
    int page_depth;
    int page_count;

    uint64_t mask = ~((uint64_t)(X86_PAGE_USER | X86_PAGE_WRITE | gKernel.nx_mask));
    uint64_t flags = page_prot_flags(prot);
    uint64_t tmp;

    page_iter_create(&iter, (uint64_t)gKernel.cr3, addr, size);
    while (page_iter_next(&iter, &page_depth, &page_dir, &page_count) == 0)
    {
        if (page_depth == 0)
        {
            for (int i = 0; i < page_count; ++i)
            {
                tmp = page_dir[i];
                tmp &= mask;
                tmp |= flags;
                page_dir[i] = tmp;
            }
        }
    }

    cr3_write((uint64_t)gKernel.cr3);
}

void* kmapanon(void* addr, size_t size, int prot)
{
    PageIterator iter;
    uint64_t* page_dir;
    int page_depth;
    int page_count;

    uint64_t flags = page_prot_flags(prot);
    uint64_t tmp;

    page_iter_create(&iter, (uint64_t)gKernel.cr3, addr, size);
    while (page_iter_next(&iter, &page_depth, &page_dir, &page_count) == 0)
    {
        for (int i = 0; i < page_count; ++i)
        {
            if (page_depth)
            {
                tmp = page_dir[i];
                if (!(tmp & X86_PAGE_PRESENT))
                {
                    tmp = alloc_zero_page();
                    tmp |= (X86_PAGE_PRESENT | X86_PAGE_WRITE | X86_PAGE_USER);
                    page_dir[i] = tmp;
                }
            }
            else
            {
                tmp = alloc_zero_page();
                tmp |= (X86_PAGE_PRESENT | flags);
                page_dir[i] = tmp;
            }
        }
    }

    cr3_write((uint64_t)gKernel.cr3);
    return addr;
}

void* kmap(void* addr, uint64_t phys, size_t size, int prot)
{
    PageIterator iter;
    uint64_t* page_dir;
    int page_depth;
    int page_count;

    uint64_t flags = page_prot_flags(prot);
    uint64_t tmp;

    phys &= ~(0xfffULL);

    if (addr == NULL)
        addr = io_alloc(size);

    page_iter_create(&iter, (uint64_t)gKernel.cr3, addr, size);
    while (page_iter_next(&iter, &page_depth, &page_dir, &page_count) == 0)
    {
        for (int i = 0; i < page_count; ++i)
        {
            if (page_depth)
            {
                tmp = page_dir[i];
                if (!(tmp & X86_PAGE_PRESENT))
                {
                    tmp = alloc_zero_page();
                    tmp |= (X86_PAGE_PRESENT | X86_PAGE_WRITE | X86_PAGE_USER);
                    page_dir[i] = tmp;
                }
            }
            else
            {
                tmp = phys;
                tmp |= (X86_PAGE_PRESENT | flags);
                page_dir[i] = tmp;
                phys += PAGESIZE;
            }
        }
    }

    cr3_write((uint64_t)gKernel.cr3);
    return addr;
}

void kunmapanon(void* addr, size_t size)
{
    PageIterator iter;
    uint64_t* page_dir;
    int page_depth;
    int page_count;

    uint64_t tmp;

    page_iter_create(&iter, (uint64_t)gKernel.cr3, addr, size);
    while (page_iter_next(&iter, &page_depth, &page_dir, &page_count) == 0)
    {
        if (page_depth == 0)
        {
            for (int i = 0; i < page_count; ++i)
            {
                tmp = page_dir[i] & MMASK_PHYS;
                free_phys_pages(tmp, 1);
                page_dir[i] = 0;
            }
        }
    }

    cr3_write((uint64_t)gKernel.cr3);
}

static void set_kernel_half(void)
{
    for (int i = 256; i < 512; ++i)
    {
        if (!(gKernel.cr3[i] & X86_PAGE_PRESENT))
            gKernel.cr3[i] = alloc_zero_page() | X86_PAGE_PRESENT | X86_PAGE_USER;
    }
}

static void set_kernel_hierarchy_mem_prot(int depth, uint64_t* dir)
{
    for (int i = 0; i < 512; ++i)
    {
        if (dir[i] & X86_PAGE_PRESENT)
        {
            dir[i] |= X86_PAGE_WRITE;
            if (depth)
                set_kernel_hierarchy_mem_prot(depth - 1, (uint64_t*)physical_to_virtual(dir[i] & MMASK_PHYS));
        }
    }
}

void init_kernel_mem_prot(void)
{
    set_kernel_half();
    set_kernel_hierarchy_mem_prot(2, (uint64_t*)physical_to_virtual(gKernel.cr3[511] & MMASK_PHYS));
    kmprotect(&__KERNEL_SECTION_EXEC_START, &__KERNEL_SECTION_EXEC_END - &__KERNEL_SECTION_EXEC_START, KPROT_READ | KPROT_EXECUTE);
    kmprotect(&__KERNEL_SECTION_RODATA_START, &__KERNEL_SECTION_RODATA_END - &__KERNEL_SECTION_RODATA_START, KPROT_READ);
    kmprotect(&__KERNEL_SECTION_DATA_START, &__KERNEL_SECTION_DATA_END - &__KERNEL_SECTION_DATA_START, KPROT_READ | KPROT_WRITE);
    kmprotect(&__KERNEL_SECTION_BSS_START, &__KERNEL_SECTION_BSS_END - &__KERNEL_SECTION_BSS_START, KPROT_READ | KPROT_WRITE);
}
