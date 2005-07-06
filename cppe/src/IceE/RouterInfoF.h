// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_ROUTER_INFO_F_H
#define ICEE_ROUTER_INFO_F_H

#include <IceE/Config.h>

#ifdef ICEE_HAS_ROUTER

#include <IceE/Handle.h>

namespace IceInternal
{

class RouterManager;
void incRef(RouterManager*);
void decRef(RouterManager*);
typedef Handle<RouterManager> RouterManagerPtr;

class RouterInfo;
void incRef(RouterInfo*);
void decRef(RouterInfo*);
typedef Handle<RouterInfo> RouterInfoPtr;

}

#endif

#endif
