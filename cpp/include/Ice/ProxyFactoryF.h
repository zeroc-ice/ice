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

class ProxyFactoryI;
void ICE_API incRef(ProxyFactoryI*);
void ICE_API decRef(ProxyFactoryI*);
typedef __Ice::Handle<ProxyFactoryI> ProxyFactory;

}

#endif
