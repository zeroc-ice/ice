// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_PROXY_FACTORY_F_H
#define ICEE_PROXY_FACTORY_F_H

#include <IceE/Handle.h>
#include <IceE/Shared.h>

namespace IceInternal
{

class ProxyFactory;
IceUtil::Shared* upCast(ProxyFactory*);
typedef IceInternal::Handle<ProxyFactory> ProxyFactoryPtr;

}

#endif
