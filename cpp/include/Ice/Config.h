// **********************************************************************
//
// Copyright (c) 2002
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
namespace Ice
{
}

namespace IceInternal
{
}

#if defined(_WIN32)

#   ifdef ICE_API_EXPORTS
#       define ICE_API __declspec(dllexport)
#   else
#       define ICE_API __declspec(dllimport)
#   endif

#   ifdef ICE_PROTOCOL_API_EXPORTS
#       define ICE_PROTOCOL_API __declspec(dllexport)
#   else
#       define ICE_PROTOCOL_API __declspec(dllimport)
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

// TODO: Should not be inline, this is not performance critical.
inline int getSystemErrno() { return GetLastError(); }
inline int getSocketErrno() { return WSAGetLastError(); }
inline int getDNSErrno() { return WSAGetLastError(); }

#elif defined(__linux__) && defined(i386)

#   include <sys/types.h>
#   include <unistd.h>

#   define ICE_API /**/
#   define ICE_PROTOCOL_API /**/

namespace Ice
{

typedef char Byte;
typedef short Short;
typedef int Int;
typedef long long Long;
typedef float Float;
typedef double Double;

}

// TODO: Should not be inline, this is not performance critical.
inline int getSystemErrno() { return errno; }
inline int getSocketErrno() { return errno; }
extern int h_errno;
inline int getDNSErrno() { return h_errno; }

#else

#   error "Unsupported operating system or platform!"

#endif

#endif
