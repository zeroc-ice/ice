//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

#ifdef ICE_CPP11_MAPPING
    virtual void
    setAdapterDirectProxyAsync(std::string,
                               std::shared_ptr<Ice::ObjectPrx>,
                               std::function<void()>,
                               std::function<void(std::exception_ptr)>,
                               const Ice::Current&);

    virtual void
    setReplicatedAdapterDirectProxyAsync(std::string,
                                         std::string,
                                         std::shared_ptr<Ice::ObjectPrx>,
                                         std::function<void()>,
                                         std::function<void(std::exception_ptr)>,
                                         const Ice::Current&);

    virtual void
    setServerProcessProxyAsync(std::string,
                               std::shared_ptr<Ice::ProcessPrx>,
                               std::function<void()>,
                               std::function<void(std::exception_ptr)>,
                               const Ice::Current&);
#else
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
#endif
    Ice::ObjectPrxPtr findObject(const Ice::Identity&) const;
    Ice::ObjectPrxPtr findAdapter(const std::string&, bool&) const;

private:

    const Ice::ObjectPrxPtr _wellKnownProxy;
    std::map<std::string, Ice::ObjectPrxPtr> _adapters;
    std::map<std::string, std::set<std::string> > _replicaGroups;
};
ICE_DEFINE_PTR(LocatorRegistryIPtr, LocatorRegistryI);

class LookupI;
ICE_DEFINE_PTR(LookupIPtr, LookupI);

class LocatorI : public Ice::Locator
{
public:

    LocatorI(const LookupIPtr&, const Ice::LocatorRegistryPrxPtr&);

#ifdef ICE_CPP11_MAPPING
    virtual void
    findObjectByIdAsync(Ice::Identity,
                        std::function<void(const std::shared_ptr<Ice::ObjectPrx>&)>,
                        std::function<void(std::exception_ptr)>,
                        const Ice::Current&) const;

    virtual void
    findAdapterByIdAsync(std::string,
                         std::function<void(const std::shared_ptr<Ice::ObjectPrx>&)>,
                         std::function<void(std::exception_ptr)>,
                         const Ice::Current&) const;
#else
    virtual void
    findObjectById_async(const Ice::AMD_Locator_findObjectByIdPtr&, const Ice::Identity&,
                         const Ice::Current&) const;

    virtual void
    findAdapterById_async(const Ice::AMD_Locator_findAdapterByIdPtr&, const std::string&,
                          const Ice::Current&) const;
#endif
    virtual Ice::LocatorRegistryPrxPtr getRegistry(const Ice::Current&) const;

private:

    LookupIPtr _lookup;
    Ice::LocatorRegistryPrxPtr _registry;
};

};

#endif
