// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_WELL_KNOWN_OBJECTS_MANAGER_H
#define ICEGRID_WELL_KNOWN_OBJECTS_MANAGER_H

#include "IceGrid/Registry.h"
#include "Internal.h"

namespace IceGrid
{
    class Database;

    class WellKnownObjectsManager
    {
    public:
        WellKnownObjectsManager(const std::shared_ptr<Database>&);

        void add(const Ice::ObjectPrx&, const std::string&);
        void addEndpoint(const std::string&, const Ice::ObjectPrx&);
        void finish();

        void registerAll();
        void registerAll(const ReplicaSessionPrx&);
        void updateReplicatedWellKnownObjects();

        std::optional<Ice::ObjectPrx> getEndpoints(const std::string&);

        LocatorPrx getLocator();
        Ice::LocatorRegistryPrx getLocatorRegistry();

    private:
        [[nodiscard]] bool initialized() const;

        Ice::ObjectPrx getWellKnownObjectReplicatedProxy(const Ice::Identity&, const std::string&);

        const std::shared_ptr<Database> _database;
        bool _initialized{false};

        StringObjectProxyDict _endpoints;
        ObjectInfoSeq _wellKnownObjects;

        mutable std::mutex _mutex;
    };

};

#endif
