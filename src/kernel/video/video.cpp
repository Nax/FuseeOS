#include <kernel/kernel.h>

void video_init(void)
{
    Video* video = &gKernel.video;

    video->text_cursor_x = 0;
    video->text_cursor_y = 0;
    video->text_max_x    = 80;
    video->text_max_y    = 25;
    video->framebuffer   = (volatile void*)physical_to_virtual(0xb8000);

    video->func_putchar = &video_putchar_text;
    video->func_scroll  = &video_scroll_text;

    for (size_t i = 0; i < video->text_max_y; ++i)
        video_scroll_text();
}

void video_set_graphical_mode(uint64_t phys_framebuffer, uint16_t width, uint16_t height, uint8_t bpp, uint32_t advance)
{
    Video* video = &gKernel.video;

    video->framebuffer = (volatile void*)physical_to_virtual(phys_framebuffer);
    video->width       = width;
    video->height      = height;
    video->bpp         = bpp;
    video->advance     = advance;

    video->text_cursor_x = 0;
    video->text_cursor_y = 0;
    video->text_max_x    = width / FONT_W;
    video->text_max_y    = height / FONT_H;

    video->func_putchar = &video_putchar_gfx;
    video->func_scroll  = &video_scroll_gfx;
}

void video_putchar(char c)
{
    Video* video = &gKernel.video;

    if (video->text_cursor_x == video->text_max_x)
    {
        video->text_cursor_x = 0;
        video->text_cursor_y++;
    }

    if (video->text_cursor_y == video->text_max_y)
    {
        video->text_cursor_y--;
        video->func_scroll();
    }

    switch (c)
    {
    case '\n':
        video->text_cursor_x = 0;
        video->text_cursor_y++;
        break;
    default:
        video->func_putchar(c, video->text_cursor_x, video->text_cursor_y);
        video->text_cursor_x++;
        break;
    }
}
