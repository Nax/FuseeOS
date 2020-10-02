#include <boot/bootloader/boot.h>

void screen_init(void)
{
    BiosArgs args;

    /* Set video mode 0x03 */
    args.eax = 0x0003;
    bios_call(0x10, &args);

    /* Hide cursor */
    args.eax = 0x0100;
    args.ecx = 0x2607;
    bios_call(0x10, &args);

    video_init();
}
