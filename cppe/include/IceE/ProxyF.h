
// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_PROXY_F_H
#define ICEE_PROXY_F_H

#include <IceE/Config.h>

namespace IceProxy
{

namespace Ice
{

class Object;

}

}

namespace IceInternal
{

ICE_API void incRef(::IceProxy::Ice::Object*);
ICE_API void decRef(::IceProxy::Ice::Object*);

}

#include <IceE/ProxyHandle.h>

namespace Ice
{

typedef IceInternal::ProxyHandle< ::IceProxy::Ice::Object> ObjectPrx;

}

#endif
