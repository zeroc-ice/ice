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

#if defined(WIN32)

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

inline int getSystemErrno() { return GetLastError(); }
inline int getSocketErrno() { return WSAGetLastError(); }
inline int getDNSErrno() { return WSAGetLastError(); }

#elif defined(__linux__) && defined(i386)

#   include <sys/types.h>
#   include <unistd.h>

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

inline int getSystemErrno() { return errno; }
inline int getSocketErrno() { return errno; }
extern int h_errno;
inline int getDNSErrno() { return h_errno; }

#else

#   error "Unsupported operating system or platform!"

#endif

#endif
