// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_CONFIG_H
#define ICE_CONFIG_H

#if defined(WIN32)

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

#   ifdef ICE_EXPORTS
#       define ICE_API __declspec(dllexport)
#   else
#       define ICE_API __declspec(dllimport)
#   endif

#   ifndef _MT
#       error "Only multi-threaded runtime libraries may be linked with Ice!"
#   endif

namespace Ice
{

typedef char Byte;
typedef short Short;
typedef int Int;
typedef __int64 Long;
typedef float Float;
typedef double Double;

}

namespace __Ice
{

const bool bigendian = false;

}

#   define _UNICODE

#elif defined(__linux__) && defined(i386)

#   define ICE_API /**/

namespace Ice
{

typedef char Byte;
typedef short Short;
typedef int Int;
typedef long long Long;
typedef float Float;
typedef double Double;

}

namespace __Ice
{

const bool bigendian = false;

}

#else

#   error "unsupported operating system or platform"

#endif

//
// Some include files we need almost everywhere
//
#include <cerrno>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

//
// Ice uses JThreads/C++
//
#include <JTC/JTC.h>

//
// By deriving from this class (private inheritance), other classes
// are made non-copyable
//
namespace __Ice
{

class noncopyable
{
protected:

    noncopyable() { }
    ~noncopyable() { }

private:

    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
};

}

#endif
