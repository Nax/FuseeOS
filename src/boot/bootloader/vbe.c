#include <boot/bootloader/boot.h>

#define EDID_DTD 54

static uint8_t      gEDID[128];
static VbeInfoBlock gVbeInfoBlock;
static VbeVideoMode gVbeVideoMode;

static void vbe_info(void)
{
    BiosArgs args;

    args.eax = 0x4f00;
    args.es  = (uintptr_t)&gVbeInfoBlock >> 4;
    args.edi = (uintptr_t)&gVbeInfoBlock & 0xf;
    bios_call(0x10, &args);
}

static int vbe_monitor_info(uint16_t* width, uint16_t* height)
{
    BiosArgs args;

    args.eax = 0x4f15;
    args.ebx = 0x01;
    args.ecx = 0x00;
    args.edx = 0x00;
    args.edi = (uintptr_t)gEDID & 0xf;
    args.es  = (uintptr_t)gEDID >> 4;
    bios_call(0x10, &args);

    if ((args.eax & 0xffff) != 0x4f) return 1;

    *width  = ((((uint16_t)gEDID[EDID_DTD + 4]) << 4) | gEDID[EDID_DTD + 2]) & 0xfff;
    *height = ((((uint16_t)gEDID[EDID_DTD + 7]) << 4) | gEDID[EDID_DTD + 5]) & 0xfff;
    return 0;
}

static void vbe_try_modes(uint16_t max_width, uint16_t max_height)
{
    BiosArgs  args;
    uint16_t* modes_ptr;
    uint16_t  mode;
    uint16_t  mode_width;
    uint16_t  mode_height;
    uint16_t  m;

    modes_ptr   = (uint16_t*)(((uintptr_t)gVbeInfoBlock.video_modes_ptr[1] << 4) + gVbeInfoBlock.video_modes_ptr[0]);
    mode        = 0xffff;
    mode_width  = 0;
    mode_height = 0;

    for (;;)
    {
        m = *modes_ptr;
        if (m == 0xffff) break;
        modes_ptr++;

        args.eax = 0x4f01;
        args.ecx = m;
        args.edi = (uintptr_t)&gVbeVideoMode & 0xf;
        args.es  = (uintptr_t)&gVbeVideoMode >> 4;
        bios_call(0x10, &args);

        if (!(gVbeVideoMode.attr & 0x90)) continue;
        if (!(gVbeVideoMode.memory_model == 4 || gVbeVideoMode.memory_model == 6)) continue;
        if (gVbeVideoMode.xres > max_width) continue;
        if (gVbeVideoMode.yres > max_height) continue;
        if (gVbeVideoMode.bpp != 32) continue;

        if (gVbeVideoMode.xres >= mode_width && gVbeVideoMode.xres >= mode_height)
        {
            mode_width  = gVbeVideoMode.xres;
            mode_height = gVbeVideoMode.yres;
            mode        = m;
        }
    }
    if (mode != 0xffff)
    {
        args.eax = 0x4f01;
        args.ecx = mode;
        args.edi = (uintptr_t)&gVbeVideoMode & 0xf;
        args.es  = (uintptr_t)&gVbeVideoMode >> 4;
        bios_call(0x10, &args);

        args.eax = 0x4f02;
        args.ebx = mode | 0x4000;
        args.edi = 0;
        args.es  = 0;
        bios_call(0x10, &args);

        video_set_graphical_mode((void*)(uintptr_t)gVbeVideoMode.physbase,
                                 gVbeVideoMode.xres,
                                 gVbeVideoMode.yres,
                                 gVbeVideoMode.bpp,
                                 gVbeVideoMode.pitch);
    }
}

void vbe_init(void)
{
    uint16_t mon_width;
    uint16_t mon_height;

    vbe_info();
    vbe_monitor_info(&mon_width, &mon_height);
    vbe_try_modes(mon_width, mon_height);
}
