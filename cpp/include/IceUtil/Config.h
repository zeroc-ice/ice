// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_CONFIG_H
#define ICE_UTIL_CONFIG_H

//
// Endianness
//
// Most CPUs support only one endianness, with the notable exceptions
// of Itanium (IA64) and MIPS.
//
#ifdef __GLIBC__
# include <endian.h>
#endif

#if defined(__i386)     || defined(_M_IX86) || defined(__x86_64)  || \
    defined(_M_X64)     || defined(_M_IA64) || defined(__alpha__) || \
    defined(__ARMEL__) || defined(_M_ARM_FP) || \
    defined(__MIPSEL__) || (defined(__BYTE_ORDER) && (__BYTE_ORDER == __LITTLE_ENDIAN))
#   define ICE_LITTLE_ENDIAN
#elif defined(__sparc) || defined(__sparc__) || defined(__hppa)      || \
      defined(__ppc__) || defined(__powerpc) || defined(_ARCH_COM) || \
      defined(__MIPSEB__) || (defined(__BYTE_ORDER) && (__BYTE_ORDER == __BIG_ENDIAN))
#   define ICE_BIG_ENDIAN
#else
#   error "Unknown architecture"
#endif

//
// 32 or 64 bit mode?
//
#if defined(__sun) && (defined(__sparcv9) || defined(__x86_64))    || \
      defined(__linux) && defined(__x86_64)                        || \
      defined(__hppa) && defined(__LP64__)                         || \
      defined(_ARCH_COM) && defined(__64BIT__)                     || \
      defined(__alpha__)                                           || \
      defined(_WIN64)
#   define ICE_64
#else
#   define ICE_32
#endif

//
// Check for C++ 11 support
//
#if (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)) && defined(__GXX_EXPERIMENTAL_CXX0X__)) || \
    (defined(__clang__) && (__clang_major__ >= 4) && __cplusplus >= 201103) || \
    (defined(_MSC_VER) && (_MSC_VER >= 1600))
#   define ICE_CPP11
#endif

#if defined(ICE_CPP11) && !defined(_MSC_VER)

// Visual Studio does not support noexcept yet
#   define ICE_NOEXCEPT noexcept
#   define ICE_NOEXCEPT_FALSE noexcept(false)
#else
#   define ICE_NOEXCEPT throw()
#   define ICE_NOEXCEPT_FALSE /**/
#endif


#if defined(_MSC_VER) && (_MSC_VER >= 1700) // Visual Studio 2012 or later
//
// Check if building for WinRT
//
#   include <winapifamily.h>
#   if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
#      define ICE_OS_WINRT
#      define ICE_STATIC_LIBS
#   endif
//
// Windows provides native condition variables on Vista and later,
// and Visual Studio 2012 no longer supports Windows XP or Windows Server 2003.
//
// You can also "switch-on" this macro to use native condition variables with
// other C++ compilers on Windows.
//
#   define ICE_HAS_WIN32_CONDVAR 
#endif

//
// Compiler extensions to export and import symbols: see the documentation 
// for Visual C++, Solaris Studio and HP aC++.
//
#if (defined(_MSC_VER) && !defined(ICE_STATIC_LIBS)) || \
    (defined(__HP_aCC) && defined(__HP_WINDLL))
#   define ICE_DECLSPEC_EXPORT __declspec(dllexport)
#   define ICE_DECLSPEC_IMPORT __declspec(dllimport)
#   define ICE_HAS_DECLSPEC_IMPORT_EXPORT
#elif defined(__SUNPRO_CC)
#   define ICE_DECLSPEC_EXPORT __global
#   define ICE_DECLSPEC_IMPORT /**/
#else
#   define ICE_DECLSPEC_EXPORT /**/
#   define ICE_DECLSPEC_IMPORT /**/
#endif

//
// Let's use these extensions with IceUtil:
//
#ifdef ICE_UTIL_API_EXPORTS
#   define ICE_UTIL_API ICE_DECLSPEC_EXPORT
#else
#   define ICE_UTIL_API ICE_DECLSPEC_IMPORT
#endif


#if defined(_MSC_VER)
#   define ICE_DEPRECATED_API __declspec(deprecated)
#elif defined(__GNUC__)
#   define ICE_DEPRECATED_API __attribute__((deprecated))
#else
#   define ICE_DEPRECATED_API /**/
#endif

#ifdef _WIN32
#   if !defined(ICE_STATIC_LIBS) && defined(_MSC_VER) && (!defined(_DLL) || !defined(_MT))
#       error "Only multi-threaded DLL libraries can be used with Ice!"
#   endif

#   include <windows.h>
#endif

//
// Some include files we need almost everywhere.
//
#include <cassert>
#include <iostream>
#include <sstream>

#ifndef _WIN32
#   include <pthread.h>
#   include <errno.h>
#endif

#ifdef _MSC_VER
//
// Move some warnings to level 4
//
#   pragma warning( 4 : 4250 ) // ... : inherits ... via dominance
#   pragma warning( 4 : 4251 ) // class ... needs to have dll-interface to be used by clients of class ..
#endif

namespace IceUtil
{

//
// By deriving from this class, other classes are made non-copyable.
//
class ICE_UTIL_API noncopyable
{
protected:

    noncopyable() { }
    ~noncopyable() { } // May not be virtual! Classes without virtual 
                       // operations also derive from noncopyable.

private:

    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
};

//
// Int64 typedef
//
#ifdef _MSC_VER
//
// With Visual C++, long is always 32-bit
//
typedef __int64 Int64;
#elif defined(ICE_64)
typedef long Int64;
#else
typedef long long Int64;
#endif

}

//
// ICE_INT64: macro for Int64 literal values
//
#if defined(_MSC_VER)
#   define ICE_INT64(n) n##i64
#elif defined(ICE_64)
#   define ICE_INT64(n) n##L
#else
#   define ICE_INT64(n) n##LL
#endif

//
// The Ice version.
//
#define ICE_STRING_VERSION "3.5.0" // "A.B.C", with A=major, B=minor, C=patch
#define ICE_INT_VERSION 30500      // AABBCC, with AA=major, BB=minor, CC=patch

#endif
