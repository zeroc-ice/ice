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

#if defined(_WIN32)
#   include <process.h>
#else
#   include <sys/types.h>
#   include <unistd.h>
#endif

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

#ifndef ICE_API
#   ifdef ICE_API_EXPORTS
#       define ICE_API ICE_DECLSPEC_EXPORT
#    else
#       define ICE_API ICE_DECLSPEC_IMPORT
#    endif
#endif

#ifndef ICE_PROTOCOL_API 
#   ifdef ICE_PROTOCOL_API_EXPORTS
#       define ICE_PROTOCOL_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_PROTOCOL_API ICE_DECLSPEC_IMPORT
#   endif
#endif


namespace Ice
{

typedef char Byte;
typedef short Short;
typedef int Int;
typedef IceUtil::Int64 Long;
typedef float Float;
typedef double Double;

}

// TODO: Should not be inline, this is not performance critical.
#ifdef _WIN32
inline int getSystemErrno() { return GetLastError(); }
inline int getSocketErrno() { return WSAGetLastError(); }
#else
inline int getSystemErrno() { return errno; }
inline int getSocketErrno() { return errno; }
#endif

#endif
