// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_LOCATOR_I_H
#define ICE_PACK_LOCATOR_I_H

#include <Ice/Locator.h>
#include <Ice/ProxyF.h>
#include <IcePack/AdapterManagerF.h>

namespace IcePack
{

class LocatorI : public ::Ice::Locator
{
public:
    LocatorI(const AdapterManagerPrx&, const ::Ice::LocatorRegistryPrx&);

    virtual ::Ice::ObjectPrx findAdapterByName(const std::string&, const ::Ice::Current&);

    virtual ::Ice::LocatorRegistryPrx getRegistry(const ::Ice::Current&);

private:
    
    AdapterManagerPrx _adapters;
    ::Ice::LocatorRegistryPrx _registry;
};

}

#endif
