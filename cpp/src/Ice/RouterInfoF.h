// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
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
