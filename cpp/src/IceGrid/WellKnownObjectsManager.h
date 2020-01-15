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

    void add(const std::shared_ptr<Ice::ObjectPrx>&, const std::string&);
    void addEndpoint(const std::string&, const std::shared_ptr<Ice::ObjectPrx>&);
    void finish();

    void registerAll();
    void registerAll(const std::shared_ptr<ReplicaSessionPrx>&);
    void updateReplicatedWellKnownObjects();

    std::shared_ptr<Ice::ObjectPrx> getEndpoints(const std::string&);

    std::shared_ptr<LocatorPrx> getLocator();
    std::shared_ptr<Ice::LocatorRegistryPrx> getLocatorRegistry();

private:

    bool initialized() const;

    std::shared_ptr<Ice::ObjectPrx> getWellKnownObjectReplicatedProxy(const Ice::Identity&, const std::string&);

    const std::shared_ptr<Database> _database;
    bool _initialized;

    StringObjectProxyDict _endpoints;
    ObjectInfoSeq _wellKnownObjects;

    mutable std::mutex _mutex;
};

};

#endif
