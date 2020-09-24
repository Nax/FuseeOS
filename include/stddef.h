#ifndef _STDDEF_H
#define _STDDEF_H 1

#include <sys/_types/_ptrdiff_t.h>
#include <sys/_types/_size_t.h>
#include <sys/_types/_wchar_t.h>

#include <sys/_null.h>

#define offsetof(s, m) ((size_t)(&(((s*)0)->m)))

#endif
