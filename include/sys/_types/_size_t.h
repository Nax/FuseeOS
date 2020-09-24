#ifndef _SYS__TYPES__SIZE_T_H
#define _SYS__TYPES__SIZE_T_H 1

#if defined(__SIZE_TYPE__)
typedef __SIZE_TYPE__ size_t;
#else
typedef long size_t;
#endif

#endif
