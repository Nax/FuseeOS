#include <libboot/libboot.h>

extern const uint16_t gFont[];

void video_gfx_putchar(char c, uint16_t x, uint16_t y)
{
    BootVideo*        video = &gBootParams.video;
    volatile uint8_t* ptr;
    const uint16_t*   font_ptr;
    uint32_t          v;

    ptr      = (volatile uint8_t*)video->framebuffer + (y * FONT_H * video->pitch) + x * FONT_W * 4;
    font_ptr = gFont + ((uint8_t)c * FONT_H);
    for (int j = 0; j < FONT_H; ++j)
    {
        for (int i = 0; i < FONT_W; ++i)
        {
            v                            = (font_ptr[j] & (1 << i)) ? 0xffffffff : 0x00000000;
            ((volatile uint32_t*)ptr)[i] = v;
        }
        ptr += video->pitch;
    }
}

void video_gfx_scroll(void) {}
