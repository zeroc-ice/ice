//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_LOCATOR_REGISTRY_I_H
#define ICE_GRID_LOCATOR_REGISTRY_I_H

#include <Ice/Locator.h>
#include <IceGrid/Internal.h>

namespace IceGrid
{

    class Database;
    class ReplicaSessionManager;
    class TraceLevels;

    class LocatorRegistryI final : public Ice::LocatorRegistry, public std::enable_shared_from_this<LocatorRegistryI>
    {
    public:
        LocatorRegistryI(const std::shared_ptr<Database>&, bool, bool, ReplicaSessionManager&);

        void setAdapterDirectProxyAsync(std::string,
                                        Ice::ObjectPrxPtr,
                                        std::function<void()>,
                                        std::function<void(std::exception_ptr)>,
                                        const Ice::Current&) override;

        void setReplicatedAdapterDirectProxyAsync(std::string,
                                                  std::string,
                                                  Ice::ObjectPrxPtr,
                                                  std::function<void()>,
                                                  std::function<void(std::exception_ptr)>,
                                                  const Ice::Current&) override;

        void setServerProcessProxyAsync(std::string,
                                        Ice::ProcessPrxPtr,
                                        std::function<void()>,
                                        std::function<void(std::exception_ptr)>,
                                        const Ice::Current&) override;

        void setAdapterDirectProxy(std::string,
                                   std::string,
                                   Ice::ObjectPrxPtr,
                                   std::function<void()>,
                                   std::function<void(std::exception_ptr)>);

        const std::shared_ptr<TraceLevels>& getTraceLevels() const;

    private:
        const std::shared_ptr<Database> _database;
        const bool _dynamicRegistration;
        const bool _master;
        ReplicaSessionManager& _session;
    };

}

#endif
