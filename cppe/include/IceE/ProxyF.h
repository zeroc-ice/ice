
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

namespace IceEProxy
{

namespace IceE
{

class Object;

}

}

namespace IceEDelegate
{

namespace IceE
{

class Object;

}

}

namespace IceEInternal
{

ICEE_API void incRef(::IceEProxy::IceE::Object*);
ICEE_API void decRef(::IceEProxy::IceE::Object*);

ICEE_API void incRef(::IceEDelegate::IceE::Object*);
ICEE_API void decRef(::IceEDelegate::IceE::Object*);

}


#include <IceE/ProxyHandle.h>

namespace IceE
{

typedef IceEInternal::ProxyHandle< ::IceEProxy::IceE::Object> ObjectPrx;

}

#endif
