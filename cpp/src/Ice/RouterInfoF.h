// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
