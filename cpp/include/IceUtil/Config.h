// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_UTIL_CONFIG_H
#define ICE_UTIL_CONFIG_H

//
// For STLport. If we compile in debug mode, we want to use the debug
// STLport library. This is done by setting _STLP_DEBUG before any
// STLport header files are included.
//
#if !defined(NDEBUG) && !defined(_STLP_DEBUG)
#   define _STLP_DEBUG
#endif

#if defined(_WIN32)

// Necessary for TryEnterCriticalSection.
#   define _WIN32_WINNT 0x0400

#   if !defined(_UNICODE)
#       error "Only unicode libraries can be used with Ice!"
#   endif

#   if !defined(_DLL) || !defined(_MT)
#       error "Only multi-threaded DLL libraries can be used with Ice!"
#   endif

#   ifdef ICE_UTIL_API_EXPORTS
#       define ICE_UTIL_API __declspec(dllexport)
#   else
#       define ICE_UTIL_API __declspec(dllimport)
#   endif

#   include <windows.h>

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

#   define SIZEOF_WCHAR_T 2

#elif defined(__linux__) && defined(i386)

#   define ICE_UTIL_API /**/
#   define HAVE_READLINE
#   define SIZEOF_WCHAR_T 4

#else

#   error "unsupported operating system or platform"

#endif

//
// Some include files we need almost everywhere.
//
#include <cassert>
#include <iostream>
#include <sstream>

#ifndef _WIN32
#   ifndef _REENTRANT
#       define _REENTRANT 1
#   endif
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
#if defined(_WIN32)
typedef __int64 Int64;
const Int64 Int64Min = -9223372036854775808i64;
const Int64 Int64Max =  9223372036854775807i64;
#define ICE_INT64(x) Int64(x##i64)
#elif defined(__linux__) && defined(i386)
typedef long long Int64;
const Int64 Int64Min = -0x7fffffffffffffffLL-1LL;
const Int64 Int64Max = 0x7fffffffffffffffLL;
#define ICE_INT64(x) Int64(x##LL)
#endif

}

//
// The Ice version.
//
#define ICE_STRING_VERSION "1.0.0" // "A.B.C", with A=major, B=minor, C=patch
#define ICE_INT_VERSION 10000      // AABBCC, with AA=major, BB=minor, CC=patch

#endif
