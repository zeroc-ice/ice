//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
#   include <cstddef>
#endif

#ifdef __APPLE__
#   include <dispatch/dispatch.h>
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

/** The mapping for the Slice byte type. */
typedef unsigned char Byte;
/** The mapping for the Slice short type. */
typedef short Short;
/** The mapping for the Slice int type. */
typedef int Int;
/** The mapping for the Slice long type. */
typedef long long int Long;
/** The mapping for the Slice float type. */
typedef float Float;
/** The mapping for the Slice double type. */
typedef double Double;

}

namespace IceInternal
{

ICE_API int getSystemErrno();

}

#endif
