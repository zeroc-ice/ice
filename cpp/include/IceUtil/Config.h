// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_UTIL_CONFIG_H
#define ICE_UTIL_CONFIG_H

#if defined(WIN32)

// Necessary for TryEnterCriticalSection
#   define _WIN32_WINNT 0x0400

#   if !defined(_UNICODE)
#       error "Only UNICODE libraries can be used with Ice!"
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
// Some include files we need almost everywhere
//
#include <cassert>
#include <iostream>
#include <sstream>

#ifndef WIN32
#    ifndef _REENTRANT
#        define _REENTRANT 1
#    endif
#    include <pthread.h>
#    include <errno.h>
#endif

//
// By deriving from this class, other classes are made non-copyable
//
namespace IceUtil
{

class noncopyable
{
protected:

    noncopyable() { }
    ~noncopyable() { } // May not be virtual! Classes without virtual operations also derive from noncopyable.

private:

    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
};

}

//
// The Ice version
//
#define ICE_STRING_VERSION "0.0.1"
#define ICE_INT_VERSION 0x00000001

#endif
