#ifndef _SYS__NULL_H
#define _SYS__NULL_H 1

#if defined(__cplusplus)
#if (__cplusplus >= 201103L)
#define NULL nullptr
#else
#define NULL 0
#endif
#else
#define NULL ((void*)0)
#endif

#endif
