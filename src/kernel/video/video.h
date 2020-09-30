#ifndef KERNEL_VIDEO_H
#define KERNEL_VIDEO_H

#include <stdint.h>

#define FONT_W 8
#define FONT_H 8

typedef void (*VideoPutcharFunc)(char, uint16_t, uint16_t);
typedef void (*VideoScrollFunc)(void);

struct Video
{
    uint16_t width;
    uint16_t height;
    uint8_t  bpp;
    uint32_t advance;

    uint16_t text_cursor_x;
    uint16_t text_cursor_y;
    uint16_t text_max_x;
    uint16_t text_max_y;

    volatile void* framebuffer;

    VideoPutcharFunc func_putchar;
    VideoScrollFunc  func_scroll;
};

void video_init(void);
void video_set_graphical_mode(
    uint64_t phys_framebuffer, uint16_t width, uint16_t height, uint8_t bpp, uint32_t advance);

void video_putchar(char c);
void video_putchar_text(char c, uint16_t x, uint16_t y);
void video_putchar_gfx(char c, uint16_t x, uint16_t y);

void video_scroll_text(void);
void video_scroll_gfx(void);

#endif
