#include <kernel/kernel.h>

#define HEAP_BASE ((void*)0xfffffff000000000)

typedef struct _ALIGN(32)
{
    unsigned int    used:1;
    size_t          size:63;
} HeapBlock;

static void* make_new_block(size_t size)
{
    HeapBlock*      blk;
    size_t extra_size;

    blk        = (HeapBlock*)((char*)HEAP_BASE + gKernel.heap_size);
    extra_size = page_round(size + sizeof(HeapBlock));
    gKernel.heap_size += extra_size;
    kmmap(blk, 0, extra_size, KPROT_READ | KPROT_WRITE, KMAP_ANONYMOUS | KMAP_FIXED);
    blk->used = 1;
    blk->size = extra_size - sizeof(HeapBlock);

    return blk + 1;
}

static void shrink(void)
{
    size_t newSize;
    HeapBlock* blk;
    void*  end;

    newSize = 0;

    blk  = (HeapBlock*)HEAP_BASE;
    end  = (char*)HEAP_BASE + gKernel.heap_size;

    while (blk < end)
    {
        if (blk->used)
            newSize = ((char*)blk + sizeof(HeapBlock) + blk->size) - (char*)HEAP_BASE;
        blk = (HeapBlock*)((char*)blk + sizeof(HeapBlock) + blk->size);
    }

    kmunmap((char*)HEAP_BASE + newSize, gKernel.heap_size - newSize);
    gKernel.heap_size = newSize;
}

void* kmalloc(size_t size)
{
    HeapBlock* blk;
    HeapBlock* best;
    void*  end;

    blk  = (HeapBlock*)HEAP_BASE;
    best = NULL;
    end  = (char*)HEAP_BASE + gKernel.heap_size;

    while (blk < end)
    {
        /* Is the HeapBlock suitable? */
        if (!blk->used && blk->size >= size)
        {
            /* Is the HeapBlock our best fit so far? */
            if (!best || best->size > blk->size) best = blk;
        }

        blk = (HeapBlock*)((char*)blk + sizeof(HeapBlock) + blk->size);
    }
    if (best)
    {
        /* We found a suitable HeapBlock */
        best->used = 1;
        return best + 1;
    }
    /* We didn't find any free HeapBlock */
    return make_new_block(size);
}

void kfree(void* addr)
{
    HeapBlock* blk;

    if (!addr)
        return;

    blk       = (HeapBlock*)addr - 1;
    blk->used = 0;

    if ((char*)addr + blk->size == (char*)HEAP_BASE + gKernel.heap_size)
        shrink();
}
