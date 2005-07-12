// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_CONFIG_H
#define ICEE_CONFIG_H

//
// Uncomment to build with Router support.
//
//#define ICEE_HAS_ROUTER

//
// Uncomment to build with Locator support.
//
//#define ICEE_HAS_LOCATOR

//
// Uncomment to enable batch mode on the client side.
//
//#define ICEE_HAS_BATCH


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
// If building a pure client, we cannot also build with router.
//
#if defined(ICEE_PURE_CLIENT) && defined(ICEE_HAS_ROUTER)
#  undef ICEE_HAS_ROUTER
#endif

//
// If using STLport we don't use iostreams and we don't want to
// auto-link in the STLport runtime library.
//
//#define _STLP_NO_OWN_IOSTREAMS
//#define _STLP_NO_NEW_IOSTREAMS
//#define _STLP_NO_IOSTREAMS
//#define _STLP_DONT_USE_AUTO_LINK

//
// Only supported/needed with VC6; Totally undocumented! And if you
// set it lower than 512 bytes, the program crashes.  Either leave at
// 0x200 or 0x1000
//
// I don't know whether this is necessary, or needed for VC7.
//
#if defined(_MSC_VER) && !defined(_WIN32_WCE) && (_MSC_VER < 1300) && defined(NDEBUG)
#  pragma comment(linker,"/FILEALIGN:0x200")
#endif // _MSC_VER >= 1000

//
// Endianness
//
// Most CPUs support only one endianness, with the notable exceptions
// of Itanium (IA64) and MIPS.
//
#if defined(__i386) || defined(_M_IX86) || defined (__x86_64) || defined (_M_ARM) 
#   define ICEE_LITTLE_ENDIAN
#elif defined(__sparc) || defined(__sparc__) || defined(__hppa) || defined(__ppc__) || defined(_ARCH_COM)
#   define ICEE_BIG_ENDIAN
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
#   define  ICEE_32
#elif defined(__sun) && defined(__sparcv9) || \
      defined(__linux) && defined(__x86_64) || \
      defined(__hppa) && defined(__LP64__) || \
      defined(_ARCH_COM) && defined(__64BIT__)
#   define ICEE_64
#else
#   define ICEE_32
#endif

//
// Compiler extensions to export and import symbols: see the documentation 
// for Visual C++, Sun ONE Studio 8 and HP aC++.
//
// TODO: more macros to support IBM Visual Age _Export syntax as well.
//
#if ((defined(_MSC_VER) || defined(_WIN32_WCE)) && !defined(ICEE_STATIC_LIBS)) || (defined(__HP_aCC) && defined(__HP_WINDLL))
#   define ICEE_DECLSPEC_EXPORT __declspec(dllexport)
#   define ICEE_DECLSPEC_IMPORT __declspec(dllimport)
#elif defined(__SUNPRO_CC) && (__SUNPRO_CC >= 0x550)
#   define ICEE_DECLSPEC_EXPORT __global
#   define ICEE_DECLSPEC_IMPORT
#else
#   define ICEE_DECLSPEC_EXPORT /**/
#   define ICEE_DECLSPEC_IMPORT /**/
#endif

//
// Let's use these extensions with IceE:
//
#ifdef ICEE_API_EXPORTS
#   define ICEE_API ICEE_DECLSPEC_EXPORT
#else
#   define ICEE_API ICEE_DECLSPEC_IMPORT
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
#  define _STLP_NO_OWN_IOSTREAMS
#  define _STLP_NO_NEW_IOSTREAMS
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

#ifndef _WIN32_WCE
#   if !defined(_DLL) || !defined(_MT)
#       error "Only multi-threaded DLL libraries can be used with Ice!"
#   endif
#endif

#   include <windows.h>

#ifdef _WIN32_WCE
// return type for ... (ie; not a UDT or reference to a UDT.  Will
// produce errors if applied using infix notation)
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

#include <string>

#ifndef _WIN32
#   include <pthread.h>
#   include <errno.h>
#endif

//
// If we use Visual C++ 6.0 or Embedded Visual C++ 4.0, we must use STLport
//
#if defined(_MSC_VER) && (_MSC_VER < 1300) && !defined(_STLP_BEGIN_NAMESPACE)
#   error "IceE for Visual C++ 6.0 and Embedded Visual C++ 4.0 require STLport"
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

#   if defined(ICEE_64)
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
#   ifdef INT64_MAX
const Int64 Int64Max = INT64_MAX;
#   else
const Int64 Int64Max = 0x7fffffffffffffffLL;
#   endif

#endif

#if defined(_MSC_VER)
#   define ICEE_INT64(n) n##i64
#elif defined(__HP_aCC)
#   define ICEE_INT64(n) n
#elif defined(ICEE_64)
#   define ICEE_INT64(n) n##L
#else
#   define ICEE_INT64(n) n##LL
#endif

}

//
// The Ice version.
//
#define ICEE_STRING_VERSION "1.0.0" // "A.B.C", with A=major, B=minor, C=patch
#define ICEE_INT_VERSION 10000      // AABBCC, with AA=major, BB=minor, CC=patch

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
#if defined(_WIN32)
#   include <process.h>
#else
#   include <sys/types.h>
#   include <unistd.h>
#endif
#endif

//
// Define the IceE and IceInternal namespace, so that we can use the following
// everywhere in our code:
//
// using namespace Ice;
// using namespace IceInternal;
//
namespace Ice
{
}

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

}

// TODO: Should not be inline, this is not performance critical.
#ifdef _WIN32
inline int getSystemErrno() { return GetLastError(); }
#else
inline int getSystemErrno() { return errno; }
#endif

#endif
