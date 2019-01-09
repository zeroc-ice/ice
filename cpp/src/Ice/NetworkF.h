// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_NETWORK_F_H
#define ICE_NETWORK_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

#ifdef ICE_OS_UWP
struct ICE_API Address;
#else
union Address;
#endif

}

#endif
