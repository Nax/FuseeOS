#include <libboot/libboot.h>
#include <stddef.h>

#define VIDEO_MODE_TEXT 0
#define VIDEO_MODE_GFX  1

static const BootVideModeHandler gVideoHandlers[] = {
    {&video_text_putchar, &video_text_scroll},
    {&video_gfx_putchar, &video_gfx_scroll},
};

void video_init(void)
{
    BootVideo* video = &gBootParams.video;

    video->mode          = VIDEO_MODE_TEXT;
    video->text_cursor_x = 0;
    video->text_cursor_y = 0;
    video->text_max_x    = 80;
    video->text_max_y    = 25;
    video->framebuffer   = (volatile void*)0xb8000;

    for (size_t i = 0; i < video->text_max_y; ++i)
        video_text_scroll();
}

void video_set_graphical_mode(void* framebuffer, uint16_t width, uint16_t height, uint8_t bpp, uint32_t pitch)
{
    BootVideo* video = &gBootParams.video;

    video->mode          = VIDEO_MODE_GFX;
    video->framebuffer   = (volatile void*)framebuffer;
    video->width         = width;
    video->height        = height;
    video->bpp           = bpp;
    video->pitch         = pitch;
    video->text_cursor_x = 0;
    video->text_cursor_y = 0;
    video->text_max_x    = width / FONT_W;
    video->text_max_y    = height / FONT_H;
}

void video_putchar(char c)
{
    BootVideo*                 video   = &gBootParams.video;
    const BootVideModeHandler* handler = &gVideoHandlers[video->mode];

    if (video->text_cursor_x == video->text_max_x)
    {
        video->text_cursor_x = 0;
        video->text_cursor_y++;
    }

    if (video->text_cursor_y == video->text_max_y)
    {
        video->text_cursor_y--;
        handler->scroll();
    }

    switch (c)
    {
    case '\n':
        video->text_cursor_x = 0;
        video->text_cursor_y++;
        break;
    default:
        handler->putchar(c, video->text_cursor_x, video->text_cursor_y);
        video->text_cursor_x++;
        break;
    }
}
