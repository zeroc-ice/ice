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


#ifndef ICE_PROXY_FACTORY_F_H
#define ICE_PROXY_FACTORY_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class ProxyFactory;
void incRef(ProxyFactory*);
void decRef(ProxyFactory*);
typedef IceInternal::Handle<ProxyFactory> ProxyFactoryPtr;

}

#endif
