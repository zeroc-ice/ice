// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PROXY_FACTORY_F_H
#define ICE_PROXY_FACTORY_F_H

#include <Ice/Handle.h>

namespace __Ice
{

class ProxyFactory;
void incRef(ProxyFactory*);
void decRef(ProxyFactory*);
typedef __Ice::Handle<ProxyFactory> ProxyFactory_ptr;

}

#endif
