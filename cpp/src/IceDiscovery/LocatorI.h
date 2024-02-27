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

    class LocatorRegistryI : public Ice::LocatorRegistry
    {
    public:
        LocatorRegistryI(const Ice::CommunicatorPtr&);

        virtual void setAdapterDirectProxyAsync(std::string,
                                                Ice::ObjectPrxPtr,
                                                std::function<void()>,
                                                std::function<void(std::exception_ptr)>,
                                                const Ice::Current&);

        virtual void setReplicatedAdapterDirectProxyAsync(std::string,
                                                          std::string,
                                                          Ice::ObjectPrxPtr,
                                                          std::function<void()>,
                                                          std::function<void(std::exception_ptr)>,
                                                          const Ice::Current&);

        virtual void setServerProcessProxyAsync(std::string,
                                                Ice::ProcessPrxPtr,
                                                std::function<void()>,
                                                std::function<void(std::exception_ptr)>,
                                                const Ice::Current&);
        Ice::ObjectPrxPtr findObject(const Ice::Identity&) const;
        Ice::ObjectPrxPtr findAdapter(const std::string&, bool&) const;

    private:
        const Ice::ObjectPrxPtr _wellKnownProxy;
        std::map<std::string, Ice::ObjectPrxPtr> _adapters;
        std::map<std::string, std::set<std::string>> _replicaGroups;
        mutable std::mutex _mutex;
    };
    using LocatorRegistryIPtr = std::shared_ptr<LocatorRegistryI>;

    class LookupI;
    using LookupIPtr = std::shared_ptr<LookupI>;

    class LocatorI : public Ice::Locator
    {
    public:
        LocatorI(const LookupIPtr&, const Ice::LocatorRegistryPrxPtr&);

        virtual void findObjectByIdAsync(Ice::Identity,
                                         std::function<void(const Ice::ObjectPrxPtr&)>,
                                         std::function<void(std::exception_ptr)>,
                                         const Ice::Current&) const;

        virtual void findAdapterByIdAsync(std::string,
                                          std::function<void(const Ice::ObjectPrxPtr&)>,
                                          std::function<void(std::exception_ptr)>,
                                          const Ice::Current&) const;

        virtual Ice::LocatorRegistryPrxPtr getRegistry(const Ice::Current&) const;

    private:
        LookupIPtr _lookup;
        Ice::LocatorRegistryPrxPtr _registry;
    };

};

#endif
