// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_LOCATOR_I_H
#define ICE_PACK_LOCATOR_I_H

#include <IcePack/Internal.h>
#include <Ice/Locator.h>

namespace IcePack
{

class LocatorI : public ::Ice::Locator
{
public:

    LocatorI(const AdapterRegistryPtr&, const ::Ice::LocatorRegistryPrx&);

    virtual ::Ice::ObjectPrx findAdapterByName(const std::string&, const ::Ice::Current&) const;

    virtual ::Ice::LocatorRegistryPrx getRegistry(const ::Ice::Current&) const;

private:
    
    AdapterRegistryPtr _adapterRegistry;
    Ice::LocatorRegistryPrx _locatorRegistry;
};

}

#endif
