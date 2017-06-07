// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

#ifndef ICE_OS_UWP
#   if defined(_WIN32)
#      include <process.h>
#   else
#      include <sys/types.h>
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

namespace Ice
{

typedef unsigned char Byte;
typedef short Short;
typedef int Int;
#ifdef ICE_CPP11_MAPPING
typedef long long int Long;
#else
typedef IceUtil::Int64 Long;
#endif
typedef float Float;
typedef double Double;

}

namespace IceInternal
{

ICE_API int getSystemErrno();

}

#endif
