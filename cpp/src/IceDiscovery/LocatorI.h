// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef LOCATORI_H
#define LOCATORI_H

#include <Ice/Locator.h>
#include <Ice/ProxyF.h>

#include <set>

namespace IceDiscovery
{

class LocatorRegistryI : public Ice::LocatorRegistry, private IceUtil::Mutex
{
public:

    LocatorRegistryI(const Ice::CommunicatorPtr&);
    
    virtual void 
    setAdapterDirectProxy_async(const Ice::AMD_LocatorRegistry_setAdapterDirectProxyPtr&, const std::string&, 
                                const Ice::ObjectPrx&, const Ice::Current&);

    virtual void
    setReplicatedAdapterDirectProxy_async(const Ice::AMD_LocatorRegistry_setReplicatedAdapterDirectProxyPtr&,
                                          const std::string&, const std::string&, const Ice::ObjectPrx&, 
                                          const Ice::Current&);

    virtual void 
    setServerProcessProxy_async(const Ice::AMD_LocatorRegistry_setServerProcessProxyPtr&, const std::string&, 
                                const Ice::ProcessPrx&, const Ice::Current&);

    Ice::ObjectPrx findObject(const Ice::Identity&) const;
    Ice::ObjectPrx findAdapter(const std::string&, bool&) const;

private:

    const Ice::ObjectPrx _wellKnownProxy;
    std::map<std::string, Ice::ObjectPrx> _adapters;
    std::map<std::string, std::set<std::string> > _replicaGroups;
};
typedef IceInternal::Handle<LocatorRegistryI> LocatorRegistryIPtr;

class LookupI;
typedef IceInternal::Handle<LookupI> LookupIPtr;

class LocatorI : public Ice::Locator
{
public:

    LocatorI(const LookupIPtr&, const Ice::LocatorRegistryPrx&);

    virtual void 
    findObjectById_async(const Ice::AMD_Locator_findObjectByIdPtr&, const Ice::Identity&, 
                         const Ice::Current&) const;

    virtual void 
    findAdapterById_async(const Ice::AMD_Locator_findAdapterByIdPtr&, const std::string&, 
                          const Ice::Current&) const;

    virtual Ice::LocatorRegistryPrx getRegistry(const Ice::Current&) const;

private:
    
    LookupIPtr _lookup;
    Ice::LocatorRegistryPrx _registry;
};

};

#endif
