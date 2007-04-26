
// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_PROXY_F_H
#define ICEE_PROXY_F_H

#include <IceE/Shared.h>
#include <IceE/Config.h>
#include <IceE/ProxyHandle.h>

namespace IceProxy
{

namespace Ice
{

class Object;

}

}

namespace IceInternal
{

inline ::IceProxy::Ice::Object* 
upCast(::IceProxy::Ice::Object* o)
{
    return o;
}

}

namespace Ice
{

typedef IceInternal::ProxyHandle< ::IceProxy::Ice::Object> ObjectPrx;

}

#endif
