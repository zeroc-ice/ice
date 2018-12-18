// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_ROUTER_INFO_F_H
#define ICE_ROUTER_INFO_F_H

#include <IceUtil/Shared.h>

#include <Ice/Handle.h>

namespace IceInternal
{

class RouterManager;
IceUtil::Shared* upCast(RouterManager*);
typedef Handle<RouterManager> RouterManagerPtr;

class RouterInfo;
IceUtil::Shared* upCast(RouterInfo*);
typedef Handle<RouterInfo> RouterInfoPtr;

}

#endif
