// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_ROUTER_INFO_F_H
#define ICEE_ROUTER_INFO_F_H

#include <IceE/Config.h>

#ifdef ICEE_HAS_ROUTER

#include <IceE/Handle.h>
#include <IceE/Shared.h>

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

#endif
