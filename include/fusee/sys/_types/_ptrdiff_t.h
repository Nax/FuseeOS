#ifndef _SYS__TYPES__PTRDIFF_T_H
#define _SYS__TYPES__PTRDIFF_T_H 1

#if defined(__PTRDIFF_TYPE__)
typedef __PTRDIFF_TYPE__ ptrdiff_t;
#else
typedef long ptrdiff_t;
#endif

#endif
