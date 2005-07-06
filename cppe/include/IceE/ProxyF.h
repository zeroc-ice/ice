
// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
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

namespace IceDelegate
{

namespace Ice
{

class Object;

}

}

namespace IceInternal
{

ICEE_API void incRef(::IceProxy::Ice::Object*);
ICEE_API void decRef(::IceProxy::Ice::Object*);

ICEE_API void incRef(::IceDelegate::Ice::Object*);
ICEE_API void decRef(::IceDelegate::Ice::Object*);

}

#include <IceE/ProxyHandle.h>

namespace Ice
{

typedef IceInternal::ProxyHandle< ::IceProxy::Ice::Object> ObjectPrx;

}

#endif
