// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
