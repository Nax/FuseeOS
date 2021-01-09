#include <kernel/mem.h>
#include <kernel/mem/HeapAlloc.h>

#define HEAP_BASE ((void*)0xfffffff000000000)

void* HeapAlloc::alloc(std::size_t size)
{
    Block* blk;
    Block* best;
    void*  end;

    blk  = (Block*)HEAP_BASE;
    best = nullptr;
    end  = (char*)HEAP_BASE + _size;

    while (blk < end)
    {
        /* Is the block suitable? */
        if (!blk->used && blk->size >= size)
        {
            /* Is the block our best fit so far? */
            if (!best || best->size > blk->size) best = blk;
        }

        blk = (Block*)((char*)blk + sizeof(Block) + blk->size);
    }
    if (best)
    {
        /* We found a suitable block */
        best->used = true;
        return best + 1;
    }
    /* We didn't find any free block */
    return make_new_block(size);
}

void HeapAlloc::free(void* addr)
{
    Block* blk;

    blk       = (Block*)addr - 1;
    blk->used = false;

    if ((char*)addr + blk->size == (char*)HEAP_BASE + _size)
        shrink();
}

void* HeapAlloc::make_new_block(std::size_t size)
{
    Block*      blk;
    std::size_t extra_size;

    blk        = (Block*)((char*)HEAP_BASE + _size);
    extra_size = page_round(size + sizeof(Block));
    _size += extra_size;
    kmmap(blk, 0, extra_size, KPROT_READ | KPROT_WRITE, KMAP_ANONYMOUS | KMAP_FIXED);
    blk->used = true;
    blk->size = extra_size - sizeof(Block);

    return blk + 1;
}

void HeapAlloc::shrink()
{
    std::size_t newSize;
    Block* blk;
    void*  end;

    newSize = 0;

    blk  = (Block*)HEAP_BASE;
    end  = (char*)HEAP_BASE + _size;

    while (blk < end)
    {
        if (blk->used)
            newSize = ((char*)blk + sizeof(Block) + blk->size) - (char*)HEAP_BASE;
        blk = (Block*)((char*)blk + sizeof(Block) + blk->size);
    }

    kmunmap((char*)HEAP_BASE + newSize, _size - newSize);
    _size = newSize;
}
