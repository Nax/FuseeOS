#ifndef _SYS_VARIADIC_H
#define _SYS_VARIADIC_H

#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_arg(ap, type)   __builtin_va_arg(ap, type)
#define va_end(ap)         __builtin_va_end(ap)
#define va_copy(dst, src)  __builtin_va_copy(dst, src)

#define __va_list __builtin_va_list

#endif
