// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

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
#if defined(__linux) && defined(__sparc__)
//
// We are a linux sparc, which forces 32 bit usr land, no matter 
// the architecture
//
#   define  ICE_32
#elif defined(__sun) && (defined(__sparcv9) || defined(__x86_64))  || \
      defined(__linux) && defined(__x86_64)                        || \
      defined(__hppa) && defined(__LP64__)                         || \
      defined(_ARCH_COM) && defined(__64BIT__)                     || \
      defined(__alpha__)                                           || \
      defined(_WIN64)
#   define ICE_64
#else
#   define ICE_32
#endif

#if defined(_MSC_VER) && _MSC_VER > 1600
#   include <winapifamily.h>
#   if defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_APP
#      define ICE_OS_WINRT
#      define ICE_STATIC_LIBS
#   endif
#endif

//
// Compiler extensions to export and import symbols: see the documentation 
// for Visual C++, Sun ONE Studio 8 and HP aC++.
//
// TODO: more macros to support IBM Visual Age _Export syntax as well.
//
#if (defined(_MSC_VER) && !defined(ICE_STATIC_LIBS)) || \
    (defined(__HP_aCC) && defined(__HP_WINDLL))
#   define ICE_DECLSPEC_EXPORT __declspec(dllexport)
#   define ICE_DECLSPEC_IMPORT __declspec(dllimport)
#elif defined(__SUNPRO_CC) && (__SUNPRO_CC >= 0x550)
#   define ICE_DECLSPEC_EXPORT __global
#   define ICE_DECLSPEC_IMPORT
#else
#   define ICE_DECLSPEC_EXPORT /**/
#   define ICE_DECLSPEC_IMPORT /**/
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#   define ICE_DEPRECATED_API __declspec(deprecated)
#elif defined(__GNUC__)
#   define ICE_DEPRECATED_API __attribute__((deprecated))
#else
#   define ICE_DEPRECATED_API /**/
#endif

//
// Let's use these extensions with IceUtil:
//
#ifdef ICE_UTIL_API_EXPORTS
#   define ICE_UTIL_API ICE_DECLSPEC_EXPORT
#else
#   define ICE_UTIL_API ICE_DECLSPEC_IMPORT
#endif

//
// For STLport. If we compile in debug mode, we want to use the debug
// STLport library. This is done by setting _STLP_DEBUG before any
// STLport header files are included.
//
// TODO: figure out why IceUtil does not compile with _SLTP_DEBUG using
// the Intel compiler.
//
#if !defined(NDEBUG) && !defined(_STLP_DEBUG) && !defined(__INTEL_COMPILER)
#   define _STLP_DEBUG
#endif

#if defined(_WIN32)

#   ifndef _WIN32_WINNT
        //
        // Necessary for TryEnterCriticalSection (see IceUtil/Mutex.h).
        //
#       if defined(_MSC_VER) && _MSC_VER < 1500
#           define _WIN32_WINNT 0x0400
#       endif
#   elif _WIN32_WINNT < 0x0400
#       error "TryEnterCricalSection requires _WIN32_WINNT >= 0x0400"
#   endif

#   if !defined(ICE_STATIC_LIBS) && defined(_MSC_VER) && (!defined(_DLL) || !defined(_MT))
#       error "Only multi-threaded DLL libraries can be used with Ice!"
#   endif

#   include <windows.h>

#   ifdef _MSC_VER
//     '...' : forcing value to bool 'true' or 'false' (performance warning)
#      pragma warning( disable : 4800 )
//     ... identifier was truncated to '255' characters in the debug information
#      pragma warning( disable : 4786 )
//     'this' : used in base member initializer list
#      pragma warning( disable : 4355 )
//     class ... needs to have dll-interface to be used by clients of class ...
#      pragma warning( disable : 4251 )
//     ... : inherits ... via dominance
#      pragma warning( disable : 4250 )
//     non dll-interface class ... used as base for dll-interface class ...
#      pragma warning( disable : 4275 )
//      ...: decorated name length exceeded, name was truncated
#      pragma warning( disable : 4503 )  
#   endif

    //
    // For STLport. Define _STLP_NEW_PLATFORM_SDK if a PSDK newer than the PSDK included with VC6.
    //
#   if !defined(_STLP_NEW_PLATFORM_SDK) && WINVER > 0x0400
#       define _STLP_NEW_PLATFORM_SDK 1
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

//
// If we use Visual C++ 6.0, we must use STLport
//
#if defined(_MSC_VER) && (_MSC_VER < 1300) && !defined(_STLP_BEGIN_NAMESPACE)
#   error "Ice for Visual C++ 6.0 requires STLport"
#endif

//
// By deriving from this class, other classes are made non-copyable.
//
namespace IceUtil
{

//
// TODO: Constructor and destructor should not be inlined, as they are
// not performance critical.
//
// TODO: Naming conventions?
//
class noncopyable
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
// On Windows, long is always 32-bit
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
#define ICE_STRING_VERSION "3.4.2" // "A.B.C", with A=major, B=minor, C=patch
#define ICE_INT_VERSION 30402      // AABBCC, with AA=major, BB=minor, CC=patch

