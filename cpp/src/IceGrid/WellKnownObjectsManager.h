//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_WELL_KNOWN_OBJECTS_MANAGER_H
#define ICE_GRID_WELL_KNOWN_OBJECTS_MANAGER_H

#include <IceGrid/Internal.h>
#include <IceGrid/Registry.h>

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
        void registerAll(const ReplicaSessionPrxPtr&);
        void updateReplicatedWellKnownObjects();

        Ice::ObjectPrxPtr getEndpoints(const std::string&);

        LocatorPrxPtr getLocator();
        Ice::LocatorRegistryPrxPtr getLocatorRegistry();

    private:
        bool initialized() const;

        Ice::ObjectPrxPtr getWellKnownObjectReplicatedProxy(const Ice::Identity&, const std::string&);

        const std::shared_ptr<Database> _database;
        bool _initialized;

        StringObjectProxyDict _endpoints;
        ObjectInfoSeq _wellKnownObjects;

        mutable std::mutex _mutex;
    };

};

#endif
