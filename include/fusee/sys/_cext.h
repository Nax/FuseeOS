#ifndef _INCLUDED_SYS_CEXT_H
#define _INCLUDED_SYS_CEXT_H 1

#define _ALIGN(n) __attribute__((aligned(n)))
#define _PACKED   __attribute__((packed))
#define _NORETURN __attribute__((noreturn))
#define _PURE     __attribute__((pure))

#define _LIKELY(x)   (__builtin_expect((x), 1))
#define _UNLIKELY(x) (__builtin_expect((x), 0))

#define _UNUSED(x) ((void)x)

#if defined(__cplusplus)
#define _RESTRICT __restrict__
#define _EXTERNC  extern "C"
#else
#define _RESTRICT restrict
#define _EXTERNC
#endif

#endif
