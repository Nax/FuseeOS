#ifndef _STDINT_H
#define _STDINT_H 1

#if defined(__INT8_TYPE__)
typedef __INT8_TYPE__ int8_t;
#else
typedef signed char    int8_t;
#endif

#if defined(__UINT8_TYPE__)
typedef __UINT8_TYPE__ uint8_t;
#else
typedef unsigned char  uint8_t;
#endif

#if defined(__INT16_TYPE__)
typedef __INT16_TYPE__ int16_t;
#else
typedef short          int16_t;
#endif

#if defined(__UINT16_TYPE__)
typedef __UINT16_TYPE__ uint16_t;
#else
typedef unsigned short uint16_t;
#endif

#if defined(__INT32_TYPE__)
typedef __INT32_TYPE__ int32_t;
#else
typedef int            int32_t;
#endif

#if defined(__UINT32_TYPE__)
typedef __UINT32_TYPE__ uint32_t;
#else
typedef unsigned int   uint32_t;
#endif

#if defined(__INT64_TYPE__)
typedef __INT64_TYPE__ int64_t;
#else
typedef long           int64_t;
#endif

#if defined(__UINT64_TYPE__)
typedef __UINT64_TYPE__ uint64_t;
#else
typedef unsigned long  uint64_t;
#endif

typedef int8_t  int_least8_t;
typedef int16_t int_least16_t;
typedef int32_t int_least32_t;
typedef int64_t int_least64_t;

typedef uint8_t  uint_least8_t;
typedef uint16_t uint_least16_t;
typedef uint32_t uint_least32_t;
typedef uint64_t uint_least64_t;

typedef int8_t  int_fast8_t;
typedef int16_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef int64_t int_fast64_t;

typedef uint8_t  uint_fast8_t;
typedef uint16_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
typedef uint64_t uint_fast64_t;

typedef int64_t  intptr_t;
typedef uint64_t uintptr_t;

typedef int64_t  intmax_t;
typedef uint64_t uintmax_t;

#define INT8_MAX  127
#define INT16_MAX 32767
#define INT32_MAX 2147483647
#define INT64_MAX 9223372036854775807L

#define UINT8_MAX  255
#define UINT16_MAX 65535
#define UINT32_MAX 4294967295U
#define UINT64_MAX 18446744073709551615UL

#define INT8_MIN  -128
#define INT16_MIN -32768
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN (-9223372036854775807L - 1)

#define INT_FAST8_MAX  127
#define INT_FAST16_MAX 32767
#define INT_FAST32_MAX 2147483647
#define INT_FAST64_MAX 9223372036854775807L

#define UINT_FAST8_MAX  255
#define UINT_FAST16_MAX 65535
#define UINT_FAST32_MAX 4294967295U
#define UINT_FAST64_MAX 18446744073709551615UL

#define INT_FAST8_MIN  -128
#define INT_FAST16_MIN -32768
#define INT_FAST32_MIN (-2147483647 - 1)
#define INT_FAST64_MIN (-9223372036854775807L - 1)

#define INT_LEAST8_MAX  127
#define INT_LEAST16_MAX 32767
#define INT_LEAST32_MAX 2147483647
#define INT_LEAST64_MAX 9223372036854775807L

#define UINT_LEAST8_MAX  255
#define UINT_LEAST16_MAX 65535
#define UINT_LEAST32_MAX 4294967295U
#define UINT_LEAST64_MAX 18446744073709551615UL

#define INT_LEAST8_MIN  -128
#define INT_LEAST16_MIN -32768
#define INT_LEAST32_MIN (-2147483647 - 1)
#define INT_LEAST64_MIN (-9223372036854775807L - 1)

#define INTPTR_MAX  9223372036854775807L
#define INTPTR_MIN  (-9223372036854775807L - 1)
#define UINTPTR_MAX 18446744073709551615UL

#define INTMAX_MAX  9223372036854775807L
#define INTMAX_MIN  (-9223372036854775807L - 1)
#define UINTMAX_MAX 18446744073709551615UL

#endif
