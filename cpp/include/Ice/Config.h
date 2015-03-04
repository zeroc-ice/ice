// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

#ifndef ICE_OS_WINRT
#   if defined(_WIN32)
#      include <process.h>
#   else
#      include <sys/types.h>
#      include <unistd.h>
#      include <cstddef>
#   endif
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

namespace Ice
{

typedef unsigned char Byte;
typedef short Short;
typedef int Int;
typedef IceUtil::Int64 Long;
typedef float Float;
typedef double Double;

}

namespace IceInternal
{

// TODO: Should not be inline, this is not performance critical.
#ifdef _WIN32
inline int getSystemErrno() { return GetLastError(); }
#else
inline int getSystemErrno() { return errno; }
#endif

}

#endif
