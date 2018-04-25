// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_WELL_KNOWN_OBJECTS_MANAGER_H
#define ICE_GRID_WELL_KNOWN_OBJECTS_MANAGER_H

#include <IceGrid/Internal.h>
#include <IceGrid/Registry.h>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class WellKnownObjectsManager : public IceUtil::Mutex, public IceUtil::Shared
{
public:

    WellKnownObjectsManager(const DatabasePtr&);

    void add(const Ice::ObjectPrx&, const std::string&);
    void addEndpoint(const std::string&, const Ice::ObjectPrx&);
    void finish();

    void registerAll();
    void registerAll(const ReplicaSessionPrx&);
    void updateReplicatedWellKnownObjects();

    Ice::ObjectPrx getEndpoints(const std::string&);

    LocatorPrx getLocator();
    Ice::LocatorRegistryPrx getLocatorRegistry();

private:

    bool initialized() const;

    Ice::ObjectPrx getWellKnownObjectReplicatedProxy(const Ice::Identity&, const std::string&);

    const DatabasePtr _database;
    bool _initialized;

    StringObjectProxyDict _endpoints;
    ObjectInfoSeq _wellKnownObjects;
};
typedef IceUtil::Handle<WellKnownObjectsManager> WellKnownObjectsManagerPtr;

};

#endif
