#ifndef _STRING_H
#define _STRING_H 1

#include <sys/_cext.h>
#include <sys/_null.h>
#include <sys/_types/_size_t.h>

_EXTERNC void* memcpy(void* _RESTRICT dst, const void* _RESTRICT src, size_t size);
_EXTERNC void* memmove(void* dst, const void* src, size_t size);
_EXTERNC void* memset(void* dst, int c, size_t size);
_EXTERNC int   memcmp(const void* s1, const void* s2, size_t n);

_EXTERNC size_t strlen(const char* str);
_EXTERNC int    strcmp(const char* s1, const char* s2);

#endif
