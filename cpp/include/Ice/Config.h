// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
// Automatically link Ice[D|++11|++11D].lib with Visual C++
//

#if !defined(ICE_BUILDING_ICE) && defined(ICE_API_EXPORTS)
#   define ICE_BUILDING_ICE
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICE)
#   pragma comment(lib, ICE_LIBNAME("Ice"))
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
#   if defined(ICE_STATIC_LIBS)
#       define ICE_API /**/
#   elif defined(ICE_API_EXPORTS)
#       define ICE_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif

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
