#include <kernel/arch/x86/emu8086.h>
#include <kernel/arch/x86/vbe.h>
#include <kernel/kernel.h>

static uint16_t vbe_pick_mode(uint16_t modes_seg, uint16_t modes_base)
{
    Emu8086BiosArgs args;
    VbeVideoMode    mode_info;
    uint16_t        modes[64];

    emu8086_read(modes, modes_seg, modes_base, sizeof(modes));
    for (int i = 0; i < 64; ++i)
    {
        if (modes[i] == 0xffff)
            break;

        args.eax = 0x4f01;
        args.ecx = modes[i];
        args.es  = 0x0c;
        args.edi = 0x0000;

        emu8086_bios_int(0x10, &args);
        emu8086_read(&mode_info, 0x0c, 0x0000, sizeof(mode_info));

        if (!(mode_info.attr & 0x90))
            continue;

        if (!(mode_info.memory_model == 4 || mode_info.memory_model == 6))
            continue;

        putu(mode_info.xres);
        putchar('x');
        putu(mode_info.yres);
        putchar(' ');
        putu(mode_info.bpp);
        puts("bpp");

        if (mode_info.xres == 1920 && mode_info.yres == 1080 && mode_info.bpp == 24)
        {
            return modes[i];
        }
    }
    return 0xffff;
}

void vbe_init(void)
{
    Emu8086BiosArgs args;
    VbeInfoBlock    info;
    char            oem[256];
    uint16_t        mode;

    args.eax = 0x4f00;
    args.es  = 0x0c;
    args.edi = 0x0000;
    emu8086_bios_int(0x10, &args);
    emu8086_read(&info, 0x0c, 0x0000, sizeof(info));
    emu8086_read(oem, info.oem_ptr[1], info.oem_ptr[0], sizeof(oem));
    oem[255] = 0;

    print("VBE: ");
    print(oem);
    putchar('\n');

    mode = vbe_pick_mode(info.video_modes_ptr[1], info.video_modes_ptr[0]);

    args.eax = 0x4f02;
    args.ebx = mode | 0x4000;
    args.es  = 0;
    args.edi = 0;
    emu8086_bios_int(0x10, &args);
}
