#include <boot/bootloader/boot.h>

void memory_detect(void)
{
    BiosArgs      args;
    BiosMemRegion mem;
    int           free_curs;
    int           map_curs;

    args.ebx  = 0;
    free_curs = 0;
    map_curs  = 0;

    for (;;)
    {
        args.eax = 0xe820;
        args.ecx = 24;
        args.edx = 0x534d4150;
        args.edi = (uintptr_t)&mem;
        args.es  = 0;

        if (bios_call(0x15, &args)) break;

        if (args.eax != 0x534d4150) break;

        if (mem.size != 0 && mem.type == 1)
        {
            gBootParams.mem_map[map_curs].base = mem.base;
            gBootParams.mem_map[map_curs].size = mem.size;
            map_curs++;

            if (mem.base >= 0x100000)
            {
                gBootParams.mem_free[free_curs].base = mem.base;
                gBootParams.mem_free[free_curs].size = mem.size;
                free_curs++;
            }
        }

        if (args.ebx == 0) break;
    }
}

void* memory_alloc(uint32_t size)
{
    uint64_t npages;
    uint64_t base;

    npages = (size + PAGESIZE - 1) / PAGESIZE;
    size   = npages * PAGESIZE;

    base = 0;
    for (int i = 0; i < 32; ++i)
    {
        if (gBootParams.mem_free[i].size >= size)
        {
            base = gBootParams.mem_free[i].base;
            gBootParams.mem_free[i].base += size;
            gBootParams.mem_free[i].size -= size;
            break;
        }
    }

    return (void*)base;
}
