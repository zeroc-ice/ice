
// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
    
    virtual void addAdapter(const ::std::string&, const ::Ice::ObjectPrx&, const ::Ice::Current&);

private:
    
    AdapterRegistryPtr _registry;
    Ice::ObjectAdapterPtr _adapter;
};

}

#endif
