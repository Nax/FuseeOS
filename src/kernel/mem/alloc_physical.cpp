#include <kernel/kernel.h>

static void mark_page_free(uint64_t page)
{
    PhysicalMemoryAllocator* alloc = &gKernel.pmem;
    uint64_t                 bitmap_off;

    for (int i = 0; i < PMB_COUNT; ++i)
    {
        PhysicalMemoryBlock* mblock = alloc->blocks + i;

        if (!(page >= mblock->base && page < mblock->base + mblock->npages * PAGESIZE))
            continue;
        bitmap_off = ((page - mblock->base) / PAGESIZE);
        for (int j = 0; j < PMB_MAX_ORDER; ++j)
        {
            /* Clear the page in the bitmap */
            mblock->bitmap[j][bitmap_off / 8] &= ~(1 << (bitmap_off % 8));

            /* Check if the buddy is clear */
            if ((mblock->bitmap[j][bitmap_off / 8] & (3 << (bitmap_off & ~1))) == 0)
            {
                bitmap_off /= 2;
            }
            else
            {
                return;
            }
        }
        return;
    }
}

static void mark_page_used(uint64_t page)
{
    PhysicalMemoryAllocator* alloc = &gKernel.pmem;
    uint64_t                 bitmap_off;

    for (int i = 0; i < PMB_COUNT; ++i)
    {
        PhysicalMemoryBlock* mblock = alloc->blocks + i;

        if (!(page >= mblock->base && page < mblock->base + mblock->npages * PAGESIZE))
            continue;
        bitmap_off = ((page - mblock->base) / PAGESIZE);
        for (int j = 0; j < PMB_MAX_ORDER; ++j)
        {
            if ((mblock->bitmap[j][bitmap_off / 8] & (1 << (bitmap_off % 8))))
                return;
            /* Set the page in the bitmap */
            mblock->bitmap[j][bitmap_off / 8] |= (1 << (bitmap_off % 8));
            bitmap_off /= 2;
        }
        return;
    }
}

void init_physical_memory(void)
{
    PhysicalMemoryAllocator* alloc = &gKernel.pmem;
    int                      i;
    uint64_t                 bitmap_size_pages;

    /* Allocate the buddy for every memory zone */
    i = 0;
    for (;;)
    {
        KernelBootMemRegion* reg    = gKernel.boot_params.mem_map + i;
        PhysicalMemoryBlock* mblock = alloc->blocks + i;

        if (reg->size == 0)
            break;

        mblock->base      = reg->base;
        mblock->npages    = reg->size / PAGESIZE;
        bitmap_size_pages = (mblock->npages + PAGESIZE * 8 - 1) / (PAGESIZE * 8);

        for (int j = 0; j < PMB_MAX_ORDER; ++j)
        {
            mblock->bitmap[j] = (uint8_t*)physical_to_virtual(alloc_phys_early(bitmap_size_pages));
            memset(mblock->bitmap[j], 0xff, bitmap_size_pages * PAGESIZE);
            bitmap_size_pages = (bitmap_size_pages + 1) / 2;
        }
        i++;
    }

    /* Unmark free memory */
    for (i = 0; i < 32; ++i)
    {
        KernelBootMemRegion* reg = gKernel.boot_params.mem_free + i;
        for (int j = 0; j < reg->size / PAGESIZE; ++j)
        {
            mark_page_free(reg->base + j * PAGESIZE);
        }
    }
}

uint64_t alloc_phys_pages(int npages)
{
    PhysicalMemoryAllocator* alloc = &gKernel.pmem;
    int                      i;
    uint64_t                 bitmap_size;
    uint64_t                 tmp;
    uint64_t                 page;

    if (npages != 1)
        return BADPAGE;

    i = 0;
    for (;;)
    {
        PhysicalMemoryBlock* mblock = alloc->blocks + i;

        if (mblock->npages == 0)
            break;

        uint64_t* bitmap = (uint64_t*)(mblock->bitmap[0]);

        bitmap_size = (mblock->npages + 63) / 64;
        for (uint64_t j = 0; j < bitmap_size; ++j)
        {
            tmp = bitmap[j];
            if (tmp != 0xffffffffffffffffLL)
            {
                for (int k = 0; k < 64; ++k)
                {
                    if (!(tmp & (1ull << k)))
                    {
                        page = mblock->base + ((j * 64) + k) * PAGESIZE;
                        mark_page_used(page);
                        return page;
                    }
                }
            }
        }

        i++;
    }

    return BADPAGE;
}

uint64_t alloc_phys(uint64_t size)
{
    return alloc_phys_pages((size + PAGESIZE - 1) / PAGESIZE);
}

uint64_t alloc_phys_early(int npages)
{
    uint64_t size;
    uint64_t base;

    size = npages * PAGESIZE;
    base = 0;
    for (int i = 0; i < 32; ++i)
    {
        if (gKernel.boot_params.mem_free[i].size >= size)
        {
            base = gKernel.boot_params.mem_free[i].base;
            gKernel.boot_params.mem_free[i].base += size;
            gKernel.boot_params.mem_free[i].size -= size;
            break;
        }
    }

    return base;
}
