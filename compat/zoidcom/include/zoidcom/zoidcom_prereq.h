/****************************************
* zoidcom_prereq.h
* prerequisites -- HovercraftUniverse modernization shim
*
* This is NOT the original ZoidCom library. It is a from-scratch,
* API-compatible replacement written for the "revival" porting effort,
* because ZoidCom's vendor is gone and only VC9 binaries survive.
* See docs/porting/zoidcom-compat.md for the full design rationale.
*
* This header intentionally mirrors HovercraftUniverse/dependencies/zoidcom/
* include/zoidcom/zoidcom_prereq.h closely (same typedefs/macros) so that
* game code written against the real ZoidCom API surface compiles unchanged.
*****************************************/

#ifndef _ZOIDCOM_PREREQ_H
#define _ZOIDCOM_PREREQ_H

#include <cassert>
#include <cstddef>
#include <cstring>
#include <climits>

#ifdef WIN32
  #define ZCOM_PLATFORM_WIN32
#endif

#ifdef __linux__
  #define ZCOM_PLATFORM_LINUX
#endif

#if __APPLE__
  #define ZCOM_PLATFORM_MAC
#endif

#ifdef _BIG_ENDIAN
  #define ZCOM_BIG_ENDIAN
#else
  #define ZCOM_LITTLE_ENDIAN
#endif

// This shim is always built as a plain static library (hu_zoidcom_compat),
// never as a DLL, so there is no import/export dance to do -- unlike the
// real ZoidCom prereq header, ZCOM_API/ZCOM_TAPI are always empty here.
#define ZCOM_API
#define ZCOM_TAPI

#ifndef NULL
  #define NULL 0
#endif

// data types (identical to the real ZoidCom typedefs)
typedef unsigned char        zU8;
#define zU8_MAX              UCHAR_MAX
#define zU8_MIN              0
typedef signed char          zS8;
#define zS8_MAX              SCHAR_MAX
#define zS8_MIN              SCHAR_MIN
typedef unsigned short       zU16;
#define zU16_MAX             USHRT_MAX
#define zU16_MIN             0
typedef signed short         zS16;
#define zS16_MAX             SHRT_MAX
#define zS16_MIN             SHRT_MIN
typedef unsigned int         zU32;
#define zU32_MAX             UINT_MAX
#define zU32_MIN             0
typedef signed int           zS32;
#define zS32_MAX             INT_MAX
#define zS32_MIN             INT_MIN
typedef unsigned long long   zU64;
#define zU64_MAX             0xffffffffffffffffULL
#define zU64_MIN             0
typedef signed long long     zS64;
#define zS64_MAX             0x7fffffffffffffffLL
#define zS64_MIN             0x8000000000000000LL
typedef float                zFloat;
typedef double                zDouble;

#endif
