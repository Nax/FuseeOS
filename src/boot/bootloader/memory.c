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

        if (bios_call(0x15, &args))
            break;

        if (args.eax != 0x534d4150)
            break;

        if (mem.size != 0 && mem.type == 1)
        {
            gBootParams.mem_map[map_curs].base = mem.base;
            gBootParams.mem_map[map_curs].size = mem.size;
            map_curs++;

            /* Low 16 MiB is lomem, a temp place */
            if (mem.base + mem.size > 0x1000000)
            {
                if (mem.base < 0x1000000)
                {
                    mem.size -= (0x1000000 - mem.base);
                    mem.base = 0x1000000;
                }
                gBootParams.mem_free[free_curs].base = mem.base;
                gBootParams.mem_free[free_curs].size = mem.size;
                free_curs++;
            }
        }

        if (args.ebx == 0)
            break;
    }
}

static void map_identity_2mib(int i, int j)
{
    uint64_t* tmp;
    uint64_t  tmp2;

    tmp  = (uint64_t*)gBootParams.cr3;
    tmp2 = tmp[0];
    if (!tmp2)
    {
        tmp2   = (uint64_t)alloc_page_lo() | 3;
        tmp[0] = tmp2;
    }
    tmp  = (uint64_t*)(tmp2 & 0xfffffffffffff000);
    tmp2 = tmp[i];
    if (!tmp2)
    {
        tmp2   = (uint64_t)alloc_page_lo() | 3;
        tmp[i] = tmp2;
    }
    tmp    = (uint64_t*)(tmp2 & 0xfffffffffffff000);
    tmp[j] = (((((uint64_t)i * 0x200) + (uint64_t)j) * 0x200000) | 0x83);
}

void memory_identity_map(void)
{
    gBootParams.cr3 = (char*)alloc_page_hi();

    for (uint64_t i = 0; i < 4; ++i)
    {
        for (uint64_t j = 0; j < 0x200; ++j)
        {
            map_identity_2mib(i, j);
        }

        /* We need to apply the paging as we go */
        __asm__ __volatile__("mov %0, %%cr3\r\n" ::"a"(gBootParams.cr3));
    }
}
