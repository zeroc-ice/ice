// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ROUTER_INFO_F_H
#define ICE_ROUTER_INFO_F_H

#include <Ice/Handle.h>

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
