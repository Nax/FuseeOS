#include <kernel/kernel.h>

static const char* const hex_chars = "0123456789abcdef";

void putchar(int c)
{
    video_putchar(c);
    out8(0xe9, c);
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

static void _putu(int64_t v)
{
    if (v >= 10)
        _putu(v / 10);
    else if (v == 0)
        return;
    putchar(hex_chars[v % 10]);
}

void putu(int64_t v)
{
    if (v == 0)
        putchar('0');
    else
        _putu(v);
}
