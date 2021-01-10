#include <libboot/libboot.h>
#include <initram.h>
#include <string.h>

char* initram_lookup(const char* name)
{
    char*             base;
    InitRamHeader*    h;
    InitRamFileEntry* fe;

    base = gBootParams.initram;
    h    = (InitRamHeader*)base;
    fe   = (InitRamFileEntry*)(base + h->fh_off);

    for (int i = 0; i < h->fh_cnt; ++i)
    {
        if (memcmp(name, base + h->sh_off + fe[i].name_off, fe[i].name_size) == 0)
            return base + h->data_off + fe[i].data_off;
    }

    return NULL;
}
