
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

#include <Ice/Locator.h>
#include <Ice/ProxyF.h>
#include <IcePack/AdapterManagerF.h>

namespace IcePack
{

class LocatorRegistryI : public Ice::LocatorRegistry
{
public: // TODO: ML: Add empty line.
    LocatorRegistryI(const AdapterManagerPrx&);
    
    virtual void addAdapter(const ::std::string&, const ::Ice::ObjectPrx&, const ::Ice::Current&);

private:
    
    AdapterManagerPrx _adapters;
};

}

#endif
