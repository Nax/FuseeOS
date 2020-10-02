#include <libboot/libboot.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define TYPE_CHAR      0
#define TYPE_SHORT     1
#define TYPE_INT       2
#define TYPE_LONG      3
#define TYPE_LONG_LONG 4
#define TYPE_SIZE_T    5
#define TYPE_INTMAX_T  6

static const char* const hex_chars = "0123456789abcdef";

static void putchar(uint8_t c)
{
    video_putchar(c);
    __asm__ __volatile__("outb %0, $0xe9\r\n" ::"a"(c));
}

static void print(const char* str)
{
    int i = 0;
    int c;

    for (;;)
    {
        c = str[i++];
        if (!c) break;
        putchar(c);
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

static const char* fmt_udec(uintmax_t num)
{
    static char buffer[21];

    uint64_t divisor;
    int      cursor;
    int      v;

    memset(buffer, 0, sizeof(buffer));
    cursor  = 0;
    divisor = 10000000000000000000ULL;

    for (int i = 0; i < 20; ++i)
    {
        v = ((num / divisor) % 10);
        divisor /= 10;
        if (v || cursor) buffer[cursor++] = hex_chars[v];
    }
    if (!cursor) buffer[0] = '0';
    return buffer;
}

static const char* fmt_hex(uintmax_t num)
{
    static char buffer[17];

    int cursor;
    int v;
    int shift;

    memset(buffer, 0, sizeof(buffer));
    cursor = 0;
    shift  = 60;

    for (int i = 0; i < 16; ++i)
    {
        v = (num >> shift) & 0xf;
        shift -= 4;
        if (v || cursor) buffer[cursor++] = hex_chars[v];
    }
    if (!cursor) buffer[0] = '0';
    return buffer;
}

static uintmax_t integer_arg(va_list list, int type)
{
    switch (type)
    {
    case TYPE_CHAR:
        return (unsigned char)va_arg(list, unsigned int);
    case TYPE_SHORT:
        return (unsigned short)va_arg(list, unsigned int);
    case TYPE_INT:
        return va_arg(list, unsigned int);
    case TYPE_LONG:
        return va_arg(list, unsigned long);
    case TYPE_LONG_LONG:
        return va_arg(list, unsigned long long);
    case TYPE_SIZE_T:
        return va_arg(list, size_t);
    case TYPE_INTMAX_T:
        return va_arg(list, uintmax_t);
    }
    return 0;
}

void boot_printf(const char* fmt, ...)
{
    va_list     ap;
    size_t      len;
    char        buffer[1024];
    const char* substr;
    int         arg_type;
    char        c;
    char        cbuf[2];
    size_t      i;
    size_t      j;
    int         format_over;

    i = 0;
    j = 0;
    va_start(ap, fmt);
    for (;;)
    {
        c = fmt[i++];
        if (!c) break;
        if (c != '%')
        {
            buffer[j++] = c;
            continue;
        }
        format_over = 0;
        arg_type    = TYPE_INT;
        while (!format_over)
        {
            c = fmt[i++];
            if (!c) goto end;

            switch (c)
            {
            case 'h':
                arg_type = (arg_type == TYPE_CHAR) ? TYPE_SHORT : TYPE_CHAR;
                break;
            case 'l':
                arg_type = (arg_type == TYPE_LONG) ? TYPE_LONG_LONG : TYPE_LONG;
                break;
            case 'q':
                arg_type = TYPE_LONG_LONG;
                break;
            case 'z':
            case 'Z':
                arg_type = TYPE_SIZE_T;
                break;
            case 'j':
                arg_type = TYPE_LONG_LONG;
                break;
            case '%':
                substr      = "%";
                format_over = 1;
                break;
            case 's':
                substr      = va_arg(ap, const char*);
                format_over = 1;
                break;
            case 'c':
                cbuf[0]     = (char)va_arg(ap, int);
                cbuf[1]     = 0;
                substr      = cbuf;
                format_over = 1;
                break;
            case 'x':
                substr      = fmt_hex(integer_arg(ap, arg_type));
                format_over = 1;
                break;
            case 'u':
                substr      = fmt_udec(integer_arg(ap, arg_type));
                format_over = 1;
                break;
            default:
                break;
            }
        }
        /* Append the substr */
        len = strlen(substr);
        memcpy(buffer + j, substr, len);
        j += len;
    }
end:
    va_end(ap);
    buffer[j] = 0;
    print(buffer);
}
