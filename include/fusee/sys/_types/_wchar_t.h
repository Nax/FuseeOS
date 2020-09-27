#ifndef _SYS__TYPES__WCHAR_T_H
#define _SYS__TYPES__WCHAR_T_H 1

#ifndef __cplusplus
#if defined(__WCHAR_TYPE__)
typedef __WCHAR_TYPE__ wchar_t;
#else
typedef int wchar_t;
#endif
#endif

#endif
