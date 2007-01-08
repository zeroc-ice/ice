// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_WELL_KNOWN_OBJECTS_MANAGER_H
#define ICE_GRID_WELL_KNOWN_OBJECTS_MANAGER_H

#include <IceGrid/Internal.h>

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
    
private:

    bool initialized() const;

    const DatabasePtr _database;
    bool _initialized;

    StringObjectProxyDict _endpoints;
    ObjectInfoSeq _wellKnownObjects;
};
typedef IceUtil::Handle<WellKnownObjectsManager> WellKnownObjectsManagerPtr;

};

#endif
