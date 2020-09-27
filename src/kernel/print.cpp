#include <kernel/kernel.h>

#define SCREEN_BUFFER ((short*)0xb8000)
#define MAX_X         80
#define MAX_Y         25

int cx;
int cy;

void init_screen(void)
{
    /* Map the screen buffer */
    gKernel.screenbuf = (uint16_t*)kmmap(nullptr, 0xb8000, MAX_X * MAX_Y * 2, KPROT_READ | KPROT_WRITE, 0);

    /* Clear the screen buffer */
    bzero(gKernel.screenbuf, MAX_X * MAX_Y * 2);

    /* Reset cursor pos */
    cx = 0;
    cy = 0;
}

void putchar(int c)
{
    if (cx == MAX_X)
    {
        cx = 0;
        cy++;
    }

    if (cy == MAX_Y)
    {
        cy--;
        memmove(gKernel.screenbuf, gKernel.screenbuf + MAX_X, MAX_X * (MAX_Y - 1) * 2);
        bzero(gKernel.screenbuf + MAX_X * (MAX_Y - 1), MAX_X * 2);
    }

    switch (c)
    {
    case '\n':
        cx = 0;
        cy++;
        break;
    default:
        gKernel.screenbuf[cy * MAX_X + cx] = (0x0f00 | (c & 0xff));
        cx++;
        break;
    }
}

void print(const char* str)
{
    int i = 0;
    int c;

    for (;;)
    {
        c = str[i++];
        if (!c)
            break;
        putchar(c);
    }
}

static const char* const hex_chars = "0123456789abcdef";

void puts(const char* str)
{
    print(str);
    putchar('\n');
}

void puthex8(uint8_t v)
{
    print("0x");
    for (int i = 0; i < 2; ++i)
    {
        putchar(hex_chars[v >> 4]);
        v <<= 4;
    }
}

void puthex16(uint16_t v)
{
    print("0x");
    for (int i = 0; i < 4; ++i)
    {
        putchar(hex_chars[v >> 12]);
        v <<= 4;
    }
}

void puthex32(uint32_t v)
{
    print("0x");
    for (int i = 0; i < 8; ++i)
    {
        putchar(hex_chars[v >> 28]);
        v <<= 4;
    }
}

void puthex64(uint64_t v)
{
    print("0x");
    for (int i = 0; i < 16; ++i)
    {
        putchar(hex_chars[v >> 60]);
        v <<= 4;
    }
}
