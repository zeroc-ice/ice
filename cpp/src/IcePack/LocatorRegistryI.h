
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

#ifndef ICE_PACK_LOCATOR_REGISTRY_I_H
#define ICE_PACK_LOCATOR_REGISTRY_I_H

#include <IcePack/Internal.h>
#include <Ice/Locator.h>

namespace IcePack
{

class LocatorRegistryI : public Ice::LocatorRegistry
{
public:

    LocatorRegistryI(const AdapterRegistryPtr&, const Ice::ObjectAdapterPtr&);
    
    virtual void setAdapterDirectProxy(const ::std::string&, const ::Ice::ObjectPrx&, const ::Ice::Current&);

private:
    
    AdapterRegistryPtr _registry;
    Ice::ObjectAdapterPtr _adapter;
};

}

#endif
