// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
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
#if defined(__i386) || defined(_M_IX86) || defined (__x86_64)
#   define ICE_LITTLE_ENDIAN
#elif defined(__sparc) || defined(__sparc__) || defined(__hppa) || defined(__ppc__) || defined(_ARCH_COM)
#   define ICE_BIG_ENDIAN
#else
#   error "Unknown architecture"
#endif

//
// 32 or 64 bit mode?
//
#if defined(__linux) && defined(__sparc__)
//
// We are a linux sparc, which forces 32 bit usr land, no matter the architecture
//
#   define  ICE_32
#elif defined(__sun) && defined(__sparcv9) || \
      defined(__linux) && defined(__x86_64) || \
      defined(__hppa) && defined(__LP64__) || \
      defined(_ARCH_COM) && defined(__64BIT__)
#   define ICE_64
#else
#   define ICE_32
#endif

//
// Compiler extensions to export and import symbols: see the documentation 
// for Visual C++, Sun ONE Studio 8 and HP aC++.
//
// TODO: more macros to support IBM Visual Age _Export syntax as well.
//
#if defined(_MSC_VER) || (defined(__HP_aCC) && defined(__HP_WINDLL))
#   define ICE_DECLSPEC_EXPORT __declspec(dllexport)
#   define ICE_DECLSPEC_IMPORT __declspec(dllimport)
#elif defined(__SUNPRO_CC) && (__SUNPRO_CC >= 0x550)
#   define ICE_DECLSPEC_EXPORT __global
#   define ICE_DECLSPEC_IMPORT
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

//
// Comment out the following block if you want to run on Windows 9x
// or Windows NT 3.51.
//
#   ifndef _WIN32_WINNT
        //
        // Necessary for TryEnterCriticalSection.
        //
#       define _WIN32_WINNT 0x0400
#   endif

#   if !defined(_DLL) || !defined(_MT)
#       error "Only multi-threaded DLL libraries can be used with Ice!"
#   endif

#   include <windows.h>

//
// MFC applications that include afxwin.h before this header will cause
// windows.h to skip inclusion of winsock.h, so we include it here if
// necessary.
// 
#   ifndef _WINSOCKAPI_
#      include <winsock2.h>
#   endif

// '...' : forcing value to bool 'true' or 'false' (performance warning)
#   pragma warning( disable : 4800 )
// ... identifier was truncated to '255' characters in the debug information
#   pragma warning( disable : 4786 )
// 'this' : used in base member initializer list
#   pragma warning( disable : 4355 )
// class ... needs to have dll-interface to be used by clients of class ...
#   pragma warning( disable : 4251 )
// ... : inherits ... via dominance
#   pragma warning( disable : 4250 )
// non dll-interface class ... used as base for dll-interface class ...
#   pragma warning( disable : 4275 )
//  ...: decorated name length exceeded, name was truncated
#   pragma warning( disable : 4503 )  

#elif (defined(__sun) && defined(__sparc)) || (defined(__hpux))
#   include <inttypes.h>
#else
//
// The ISO C99 standard specifies that in C++ implementations the
// macros for minimum/maximum integer values should only be defined if
// explicitly requested with __STDC_LIMIT_MACROS.
//
#   ifndef  __STDC_LIMIT_MACROS
#      define __STDC_LIMIT_MACROS
#   endif
#   include <stdint.h>
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
    ~noncopyable() { } // May not be virtual! Classes without virtual operations also derive from noncopyable.

private:

    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
};

//
// Some definitions for 64-bit integers.
//
#if defined(_MSC_VER)

typedef __int64 Int64;
const Int64 Int64Min = -9223372036854775808i64;
const Int64 Int64Max =  9223372036854775807i64;

#elif defined(__SUNPRO_CC)

#   if defined(ICE_64)
typedef long Int64;
const Int64 Int64Min = -0x7fffffffffffffffL-1L;
const Int64 Int64Max = 0x7fffffffffffffffL;
#   else
typedef long long Int64;
const Int64 Int64Min = -0x7fffffffffffffffLL-1LL;
const Int64 Int64Max = 0x7fffffffffffffffLL;
#   endif

#else

//
// Assumes ISO C99 types
//
typedef int64_t Int64;
#   ifdef INT64_MIN
const Int64 Int64Min = INT64_MIN;
#   else
const Int64 Int64Min = -0x7fffffffffffffffLL-1LL;
#   endif
#   ifdef INT64_MIN
const Int64 Int64Max = INT64_MAX;
#   else
const Int64 Int64Max = 0x7fffffffffffffffLL;
#   endif

#endif

#if defined(_MSC_VER)
#   define ICE_INT64(n) n##i64
#elif defined(__HP_aCC)
#   define ICE_INT64(n) n
#elif defined(ICE_64)
#   define ICE_INT64(n) n##L
#else
#   define ICE_INT64(n) n##LL
#endif

}

//
// The Ice version.
//
#define ICE_STRING_VERSION "2.0.0" // "A.B.C", with A=major, B=minor, C=patch
#define ICE_INT_VERSION 20000      // AABBCC, with AA=major, BB=minor, CC=patch

#endif
