// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_CONFIG_H
#define ICE_CONFIG_H

#include <IceUtil/Config.h>

#if defined(WIN32)

#   if !defined(_DLL) || !defined(_MT)
#       error "Only multi-threaded DLL libraries can be used with Ice!"
#   endif

#   ifdef ICE_API_EXPORTS
#       define ICE_API __declspec(dllexport)
#   else
#       define ICE_API __declspec(dllimport)
#   endif

#   include <process.h>

namespace Ice
{

typedef char Byte;
typedef short Short;
typedef int Int;
typedef __int64 Long;
typedef float Float;
typedef double Double;

}

#   define _UNICODE

#elif defined(__linux__) && defined(i386)

#   include <sys/types.h>
#   include <unistd.h>

#   define ICE_API /**/
#   define HAVE_READLINE

namespace Ice
{

typedef char Byte;
typedef short Short;
typedef int Int;
typedef long long Long;
typedef float Float;
typedef double Double;

}

#else

#   error "Unsupported operating system or platform!"

#endif

//
// Some include files we need almost everywhere
//
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <map>

//
// Define the Ice and IceInternal namespace, so that we can use the following
// everywhere in our code:
//
// using namespace Ice;
// using namespace IceInternal;
//
namespace Ice { }
namespace IceInternal { }

#endif
