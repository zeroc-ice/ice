// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/PermissionsVerifier.h>
#include <IceGrid/WellKnownObjectsManager.h>
#include <IceGrid/Database.h>

using namespace std;
using namespace IceGrid;

WellKnownObjectsManager::WellKnownObjectsManager(const DatabasePtr& database) : 
    _database(database), _initialized(false)
{
}

void
WellKnownObjectsManager::add(const Ice::ObjectPrx& proxy, const string& type)
{
    assert(!_initialized);
    ObjectInfo info;
    info.type = type;
    info.proxy = proxy;
    _wellKnownObjects.push_back(info);
}

void
WellKnownObjectsManager::addEndpoint(const string& name, const Ice::ObjectPrx& proxy)
{
    _endpoints.insert(make_pair(name, proxy));
}

void
WellKnownObjectsManager::finish()
{
    Lock sync(*this);
    _initialized = true;
}

void
WellKnownObjectsManager::registerAll(const ReplicaSessionPrx& session)
{
    if(!initialized())
    {
        return;
    }

    assert(session);

    //
    // If initialized, the endpoints and well known objects are immutable.
    //
    session->setEndpoints(_endpoints);
    session->registerWellKnownObjects(_wellKnownObjects);
}

void
WellKnownObjectsManager::registerAll()
{
    if(!initialized())
    {
        return;
    }

    //
    // If initialized, the endpoints and well known objects are immutable.
    //
    updateReplicatedWellKnownObjects();
    _database->addOrUpdateRegistryWellKnownObjects(_wellKnownObjects);
}

void
WellKnownObjectsManager::updateReplicatedWellKnownObjects()
{
    if(!initialized())
    {
        return;
    }

    //
    // Update replicated objects.
    //
    Ice::Identity id;
    id.category = _database->getInstanceName();
    ObjectInfo info;
    ObjectInfoSeq objects;
    
    Lock sync(*this);

    Ice::ObjectPrx replicatedClientProxy = _database->getReplicaCache().getEndpoints("Client", _endpoints["Client"]);

    id.name = "Query";
    info.type = Query::ice_staticId();
    info.proxy = replicatedClientProxy->ice_identity(id);
    objects.push_back(info);

    id.name = "Locator";
    info.type = Ice::Locator::ice_staticId();
    info.proxy = replicatedClientProxy->ice_identity(id);
    objects.push_back(info);

    _database->addOrUpdateRegistryWellKnownObjects(objects);
}

bool
WellKnownObjectsManager::initialized() const
{
    Lock sync(*this);
    return _initialized;
}

Ice::ObjectPrx
WellKnownObjectsManager::getEndpoints(const string& name)
{
    Lock sync(*this);
    return _endpoints[name];
}
