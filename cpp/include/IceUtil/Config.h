// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_CONFIG_H
#define ICE_UTIL_CONFIG_H

//
// Use the system headers as preferred way to detect endianness
// and fallback to architecture based checks.
//
//
#include <stdlib.h>

#if defined(__GLIBC__)
#   include <endian.h>
#elif defined(__APPLE__)
#   include <machine/endian.h>
#elif defined(__FreeBSD__)
#   include <sys/endian.h>
#endif

#if (defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && (__BYTE_ORDER == __LITTLE_ENDIAN)) || \
    (defined(_BYTE_ORDER) && defined(_LITTLE_ENDIAN) && (_BYTE_ORDER == _LITTLE_ENDIAN))

#   define ICE_LITTLE_ENDIAN

#elif (defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && (__BYTE_ORDER == __BIG_ENDIAN)) || \
      (defined(_BYTE_ORDER) && defined(_BIG_ENDIAN) && (_BYTE_ORDER == _BIG_ENDIAN))

#   define ICE_BIG_ENDIAN

#elif defined(__i386)      || \
      defined(_M_IX86)     || \
      defined(__x86_64)    || \
      defined(_M_X64)      || \
      defined(_M_IA64)     || \
      defined(__alpha__)   || \
      defined(__ARMEL__)   || \
      defined(_M_ARM_FP)   || \
      defined(__arm64)     || \
      defined(__MIPSEL__)

#   define ICE_LITTLE_ENDIAN

#elif defined(__sparc)   || \
      defined(__sparc__) || \
      defined(__hppa)    || \
      defined(__ppc__)   || \
      defined(__powerpc) || \
      defined(_ARCH_COM) || \
      defined(__MIPSEB__)

#   define ICE_BIG_ENDIAN

#else

#   error "Unknown architecture"

#endif

#ifdef _MSC_VER

#   ifdef _WIN64
#      define ICE_64
#   else
#      define ICE_32
#   endif

#else

    //
    // Use system headers as preferred way to detect 32 or 64 bit mode and
    // fallback to architecture based checks
    //
#   include <stdint.h>

#   if defined(__WORDSIZE) && (__WORDSIZE == 64)
#      define ICE_64
#   elif defined(__WORDSIZE) && (__WORDSIZE == 32)
#      define ICE_32
#   elif defined(__sun) && (defined(__sparcv9) || defined(__x86_64))  || \
         defined(__linux) && defined(__x86_64)                        || \
         defined(__APPLE__) && defined(__x86_64)                      || \
         defined(__hppa) && defined(__LP64__)                         || \
         defined(_ARCH_COM) && defined(__64BIT__)                     || \
         defined(__alpha__)                                           || \
         defined(_WIN64)
#      define ICE_64
#   else
#      define ICE_32
#   endif
#endif

//
// Check for C++ 11 support
//
// For GCC, we recognize --std=c++0x only for GCC version 4.5 and greater,
// as C++11 support in prior releases was too limited.
//
#if (__cplusplus >= 201103) || \
    ((defined(__GNUC__) && defined(__GXX_EXPERIMENTAL_CXX0X__) && ((__GNUC__* 100) + __GNUC_MINOR__) >= 405)) || \
    (defined(_MSC_VER) && (_MSC_VER >= 1600))
#   define ICE_CPP11
#endif


#if defined(ICE_CPP11) && (!defined(_MSC_VER) || (_MSC_VER >= 1900))
#   define ICE_NOEXCEPT noexcept
#   define ICE_NOEXCEPT_FALSE noexcept(false)
#else
#   define ICE_NOEXCEPT throw()
#   define ICE_NOEXCEPT_FALSE /**/
#endif

//
// Visual Studio 2012 or later, without Windows XP/2003 support
//
#if defined(_MSC_VER) && (_MSC_VER >= 1700) && !defined(_USING_V110_SDK71_)

//
// Check if building for WinRT
//
#   include <winapifamily.h>
#   if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
#      define ICE_OS_WINRT
#      define ICE_STATIC_LIBS
#   endif

#endif

//
// Support for thread-safe function local static initialization
// (a.k.a. "magic statics")
//
#if defined(__GNUC__) || defined(__clang__)
#   define ICE_HAS_THREAD_SAFE_LOCAL_STATIC
#endif

//
// Compiler extensions to export and import symbols: see the documentation
// for Visual Studio, Solaris Studio and GCC.
//
#if defined(_WIN32)
#   define ICE_DECLSPEC_EXPORT __declspec(dllexport)
#   define ICE_DECLSPEC_IMPORT __declspec(dllimport)
//
//  ICE_HAS_DECLSPEC_IMPORT_EXPORT defined only for compilers with distinct
//  declspec for IMPORT and EXPORT
#   define ICE_HAS_DECLSPEC_IMPORT_EXPORT
#elif defined(__GNUC__)
#   define ICE_DECLSPEC_EXPORT __attribute__((visibility ("default")))
#   define ICE_DECLSPEC_IMPORT __attribute__((visibility ("default")))
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
#if defined(ICE_UTIL_API_EXPORTS)
#   define ICE_UTIL_API ICE_DECLSPEC_EXPORT
#elif defined(ICE_STATIC_LIBS)
#   define ICE_UTIL_API /**/
#else
#   define ICE_UTIL_API ICE_DECLSPEC_IMPORT
#endif


#if defined(_MSC_VER)
#   define ICE_DEPRECATED_API(msg) __declspec(deprecated(msg))
#elif defined(__clang__)
#   if __has_extension(attribute_deprecated_with_message)
#       define ICE_DEPRECATED_API(msg) __attribute__((deprecated(msg)))
#   else
#       define ICE_DEPRECATED_API(msg) __attribute__((deprecated))
#   endif
#elif defined(__GNUC__)
#   if (__GNUC__ > 4 || (__GNUC__ == 4 &&  __GNUC_MINOR__ >= 5))
// The message option was introduced in GCC 4.5
#      define ICE_DEPRECATED_API(msg) __attribute__((deprecated(msg)))
#   else
#      define ICE_DEPRECATED_API(msg) __attribute__((deprecated))
#   endif
#else
#   define ICE_DEPRECATED_API(msg) /**/
#endif

#ifdef _WIN32
#   include <windows.h>

#   if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x600)
//
// Windows provides native condition variables on Vista and later
//
#      ifndef ICE_HAS_WIN32_CONDVAR
#          define ICE_HAS_WIN32_CONDVAR
#      endif
#   endif
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

#ifdef __APPLE__
#   include <TargetConditionals.h>
#endif

#if defined(_AIX) && defined(_LARGE_FILES)
    // defines macros such as open that we want to use consistently everywhere
#   include <fcntl.h>
#endif

#ifdef __IBMCPP__
// TODO: better fix for this warning
#   pragma report(disable, "1540-0198") // private inheritance without private keyword
#endif


#if !defined(ICE_BUILDING_ICE_UTIL) && defined(ICE_UTIL_API_EXPORTS)
#   define ICE_BUILDING_ICE_UTIL
#endif

#if defined(_MSC_VER)
#   if !defined(ICE_STATIC_LIBS) && (!defined(_DLL) || !defined(_MT))
#       error "Only multi-threaded DLL libraries can be used with Ice!"
#   endif
//
//  Automatically link with IceUtil[D].lib
//
#   if !defined(ICE_BUILDING_ICE_UTIL)
#      if defined(_DEBUG) && !defined(ICE_OS_WINRT)
#          pragma comment(lib, "IceUtilD.lib")
#      else
#          pragma comment(lib, "IceUtil.lib")
#      endif
#   endif
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
// Int64 typedef and ICE_INT64 macro for Int64 literal values
//
// Note that on Windows, long is always 32-bit
//
#if defined(_WIN32) && defined(_MSC_VER)
typedef __int64 Int64;
#    define ICE_INT64(n) n##i64
#    define ICE_INT64_FORMAT "%lld"
#elif defined(ICE_64) && !defined(_WIN32)
typedef long Int64;
#    define ICE_INT64(n) n##L
#    define ICE_INT64_FORMAT "%ld"
#else
typedef long long Int64;
#    define ICE_INT64(n) n##LL
#    define ICE_INT64_FORMAT "%lld"
#endif

}

//
// The Ice version.
//
#define ICE_STRING_VERSION "3.6.3" // "A.B.C", with A=major, B=minor, C=patch
#define ICE_INT_VERSION 30603      // AABBCC, with AA=major, BB=minor, CC=patch

#endif
