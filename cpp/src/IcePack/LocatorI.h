// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PACK_LOCATOR_I_H
#define ICE_PACK_LOCATOR_I_H

#include <IcePack/Internal.h>
#include <Ice/Locator.h>

namespace IcePack
{

class LocatorI : public Ice::Locator, public IceUtil::Mutex
{
public:

    LocatorI(const AdapterRegistryPtr&, const ObjectRegistryPtr&, const ::Ice::LocatorRegistryPrx&);

    virtual void findObjectById_async(const ::Ice::AMD_Locator_findObjectByIdPtr&, const ::Ice::Identity&, 
				      const ::Ice::Current&) const;

    virtual void findAdapterById_async(const ::Ice::AMD_Locator_findAdapterByIdPtr&, const ::std::string&, 
				       const ::Ice::Current&) const;

    virtual ::Ice::LocatorRegistryPrx getRegistry(const ::Ice::Current&) const;

    bool getDirectProxyRequest(const ::Ice::AMD_Locator_findAdapterByIdPtr&, const AdapterPrx&);
    void getDirectProxyException(const AdapterPrx&, const std::string&, const Ice::Exception&);
    void getDirectProxyCallback(const Ice::Identity&, const Ice::ObjectPrx&);
    
protected:

    const AdapterRegistryPtr _adapterRegistry;
    const ObjectRegistryPtr _objectRegistry;
    const Ice::LocatorRegistryPrx _locatorRegistry;

    typedef std::vector<Ice::AMD_Locator_findAdapterByIdPtr> PendingRequests;
    typedef std::map<Ice::Identity, PendingRequests> PendingRequestsMap;

    PendingRequestsMap _pendingRequests;
};

typedef IceUtil::Handle<LocatorI> LocatorIPtr;

}

#endif
