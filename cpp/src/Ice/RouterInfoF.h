// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
