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
    virtual Ice::LocatorRegistryPrxPtr getRegistry(const Ice::Current&) const;

private:

    LookupIPtr _lookup;
    Ice::LocatorRegistryPrxPtr _registry;
};

};

#endif
