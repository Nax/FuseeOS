#include <kernel/kernel.h>

void video_putchar_text(char c, uint16_t x, uint16_t y)
{
    Video*             v   = &gKernel.video;
    volatile uint16_t* buf = (volatile uint16_t*)v->framebuffer;

    buf[y * v->text_max_x + x] = 0x0f00 | (c & 0xff);
}

void video_scroll_text(void)
{
    Video*             v   = &gKernel.video;
    volatile uint16_t* buf = (volatile uint16_t*)v->framebuffer;
    size_t             tmp;

    tmp = v->text_max_x * (v->text_max_y - 1);
    for (size_t i = 0; i < tmp; ++i)
    {
        buf[i] = buf[i + v->text_max_x];
    }
    for (size_t i = 0; i < v->text_max_x; ++i)
    {
        buf[tmp + i] = 0;
    }
}
