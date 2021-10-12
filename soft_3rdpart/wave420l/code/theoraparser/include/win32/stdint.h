/* stdint.h - integer types

   Copyright 2003, 2006, 2007 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _STDINT_H
#define _STDINT_H


#ifndef __int8_t_defined
#define __int8_t_defined
typedef __int8  int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;
#endif

#ifndef __uint8_t_defined
#define __uint8_t_defined
typedef unsigned __int8  uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#endif

/* Minimum-width integer types */

#ifndef __int_least8_defined
#define __int_least8_defined
typedef __int8  int_least8_t;
typedef __int16 int_least16_t;
typedef __int32 int_least32_t;
typedef __int64 int_least64_t;
#endif

#ifndef __uint_least8_t_defined
#define __uint_least8_t_defined
typedef unsigned __int8  uint_least8_t;
typedef unsigned __int16 uint_least16_t;
typedef unsigned __int32 uint_least32_t;
typedef unsigned __int64 uint_least64_t;
#endif

/* Fastest minimum-width integer types */

#ifndef __int_fast8_t_defined
#define __int_fast8_t_defined
typedef __int8  int_fast8_t;
typedef __int32 int_fast16_t;
typedef __int32 int_fast32_t;
typedef __int64 int_fast64_t;
#endif

#ifndef __uint_fast8_t_defined
#define __uint_fast8_t_defined
typedef unsigned __int8  uint_fast8_t;
typedef unsigned __int32 uint_fast16_t;
typedef unsigned __int32 uint_fast32_t;
typedef unsigned __int64 uint_fast64_t;
#endif

/* Integer types capable of holding object pointers */

#if !defined(__intptr_t_defined) && !defined(_INTPTR_T_DEFINED)
#define __intptr_t_defined
#define _INTPTR_T_DEFINED
typedef int intptr_t;
#endif

#if !defined(__uintptr_t_defined) && !defined(_UINTPTR_T_DEFINED)
#define __uintptr_t_defined
#define _UINTPTR_T_DEFINED
typedef unsigned int uintptr_t;
#endif


/* Greatest-width integer types */

typedef __int64 intmax_t;
typedef unsigned __int64 uintmax_t;

/* Limits of exact-width integer types */

#if !defined(INT8_MIN)
#define INT8_MIN (-128)
#define INT16_MIN (-32768)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN (-9223372036854775807LL - 1LL)
#endif

#if !defined(INT8_MAX)
#define INT8_MAX (127)
#define INT16_MAX (32767)
#define INT32_MAX (2147483647)
#define INT64_MAX (9223372036854775807LL)
#endif

#if !defined(UINT8_MAX)
#define UINT8_MAX (255)
#define UINT16_MAX (65535)
#define UINT32_MAX (4294967295U)
#define UINT64_MAX (18446744073709551615ULL)
#endif

/* Limits of minimum-width integer types */
#if !defined(INT_LEAST8_MIN)
#define INT_LEAST8_MIN (-128)
#define INT_LEAST16_MIN (-32768)
#define INT_LEAST32_MIN (-2147483647 - 1)
#define INT_LEAST64_MIN (-9223372036854775807LL - 1LL)
#endif

#if !defined(INT_LEAST8_MAX)
#define INT_LEAST8_MAX (127)
#define INT_LEAST16_MAX (32767)
#define INT_LEAST32_MAX (2147483647)
#define INT_LEAST64_MAX (9223372036854775807LL)
#endif

#if !defined(INT_LEAST8_MAX)
#define UINT_LEAST8_MAX (255)
#define UINT_LEAST16_MAX (65535)
#define UINT_LEAST32_MAX (4294967295U)
#define UINT_LEAST64_MAX (18446744073709551615ULL)
#endif

#endif /* _STDINT_H */
