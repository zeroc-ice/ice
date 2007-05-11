// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_CONFIG_H
#define ICEE_CONFIG_H

//
// Comment this out if you want to build without Router support.
//
#define ICEE_HAS_ROUTER

//
// Comment this out if you want to build without Locator support.
//
#define ICEE_HAS_LOCATOR

//
// Comment this out if you want to build without batch mode on the client side.
//
#define ICEE_HAS_BATCH

//
// Comment this out if you want to build without wstring and string converter
// support.
//
#define ICEE_HAS_WSTRING

//
// Uncomment this if want the pure client library built with
// only the blocking concurrency model.
// If commented, both blocking and thread-per-connections support
// will be included in client library.
//
//#define ICEE_PURE_BLOCKING_CLIENT

// ***********************************************************************
//
// User should not change anything below this line!
//
// ***********************************************************************

//
// Unless we're building a pure client batch mode cannot be disabled.
//
#if !defined(ICEE_PURE_CLIENT) && !defined(ICEE_HAS_BATCH)
#  define ICEE_HAS_BATCH
#endif

//
// Unless we are building a pure client we cannot build pure blocking
// client.
//
#if !defined(ICEE_PURE_CLIENT) && defined(ICEE_PURE_BLOCKING_CLIENT)
#   undef ICEE_PURE_BLOCKING_CLIENT
#endif

//
// Use poll()/WSAEventSelect for timeouts. 
//
// Using WSAEventSelect for timeouts on Windows is necessary because
// when a socket times out, it can't be re-used (and we need the
// transceiver read() call to periodically timeout since it can't be
// unblocked by shutting down the socket.)
//
// Also, on Windows CE, socket timeouts (SO_SNDTIMEO and SO_RCVTIMEO)
// are not supported so we have to use WSAEventSelect to implement
// timeouts.
//
// This macro can also be defined for non-Windows platforms to use
// poll() for timeouts rather than using socket timeouts.
//
#if defined(_WIN32)
#   define ICEE_USE_SELECT_OR_POLL_FOR_TIMEOUTS
#endif

//
// Only supported/needed with VC6; Totally undocumented! And if you
// set it lower than 512 bytes, the program crashes.  Either leave at
// 0x200 or 0x1000
//
// I don't know whether this is necessary, or needed for VC7.
//
#if defined(_MSC_VER) && !defined(_WIN32_WCE) && (_MSC_VER < 1300) && defined(NDEBUG)
#  pragma comment(linker,"/FILEALIGN:0x200")
#endif

//
// Endianness
//
// Most CPUs support only one endianness, with the notable exceptions
// of Itanium (IA64) and MIPS.
//
#if defined(__i386)  || defined(_M_IX86)    || defined (__x86_64) || \
    defined (_M_ARM) || defined(__MIPSEL__) || defined (__ARMEL__)
#   define ICE_LITTLE_ENDIAN
#elif defined(__sparc) || defined(__sparc__) || defined(__hppa) || \
      defined(__ppc__) || defined(_ARCH_COM) || defined(__MIPSEB__)
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
#if ((defined(_MSC_VER) || defined(_WIN32_WCE)) && !defined(ICEE_STATIC_LIBS)) || (defined(__HP_aCC) && defined(__HP_WINDLL))
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
// Let's use these extensions with IceE:
//
#ifdef ICE_API_EXPORTS
#   define ICE_API ICE_DECLSPEC_EXPORT
#else
#   define ICE_API ICE_DECLSPEC_IMPORT
#endif

//
// For STLport. If we compile in debug mode, we want to use the debug
// STLport library. This is done by setting _STLP_DEBUG before any
// STLport header files are included.
//
// TODO: figure out why IceE does not compile with _SLTP_DEBUG using
// the Intel compiler.
//
#ifdef _WIN32_WCE
#  define _STLP_NO_IOSTREAMS
#else
#  if !defined(NDEBUG) && !defined(_STLP_DEBUG) && !defined(__INTEL_COMPILER)
#     define _STLP_DEBUG
#  endif
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

#   include <windows.h>

#if defined(_WIN32_WCE) && defined(_MSC_VER)
    //
    // return type for ... (ie; not a UDT or reference to a UDT.  Will
    // produce errors if applied using infix notation)
    //
#   pragma warning( disable : 4284 )
#endif

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
#endif

//
// Some include files we need almost everywhere.
//
#include <cassert>

#include <string>

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
// Int64 typedef
//
#if defined(__BCPLUSPLUS__) || defined(_MSC_VER)
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
#if defined(__BCPLUSPLUS__) || defined(_MSC_VER)
#   define ICE_INT64(n) n##i64
#elif defined(ICE_64)
#   define ICE_INT64(n) n##L
#else
#   define ICE_INT64(n) n##LL
#endif

//
// The Ice-E version.
//
#define ICEE_STRING_VERSION "1.2.0" // "A.B.C", with A=major, B=minor, C=patch
#define ICEE_INT_VERSION 10200      // AABBCC, with AA=major, BB=minor, CC=patch

//
// Some include files we need almost everywhere
//
#if !defined(_WIN32_WCE)
#include <cerrno>
#endif
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <map>

#if !defined(_WIN32_WCE)
#   if defined(_WIN32)
#       include <process.h>
#   else
#       include <sys/types.h>
#       include <unistd.h>
#   endif
#endif

//
// Define the IceInternal namespace, so that we can use the following
// everywhere in our code:
//
// using namespace IceInternal;
//

namespace IceInternal
{
}

namespace Ice
{

typedef unsigned char Byte;
typedef short Short;
typedef int Int;
typedef IceUtil::Int64 Long;
typedef float Float;
typedef double Double;

#ifdef ICEE_HAS_WSTRING
typedef std::wstring Wstring;
#else
typedef std::string Wstring;
#endif

}

// TODO: Should not be inline, this is not performance critical.
#ifdef _WIN32
inline int getSystemErrno() { return GetLastError(); }
#else
inline int getSystemErrno() { return errno; }
#endif

#endif
