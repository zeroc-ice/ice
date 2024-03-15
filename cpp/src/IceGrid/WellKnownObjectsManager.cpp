//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Endpoint.h>
#include <Glacier2/PermissionsVerifier.h>
#include <IceGrid/WellKnownObjectsManager.h>
#include <IceGrid/Database.h>

using namespace std;
using namespace IceGrid;

WellKnownObjectsManager::WellKnownObjectsManager(const shared_ptr<Database>& database)
    : _database(database),
      _initialized(false)
{
}

void
WellKnownObjectsManager::add(const Ice::ObjectPrx& proxy, const string& type)
{
    assert(!_initialized);
    ObjectInfo info = {proxy, type};
    _wellKnownObjects.push_back(std::move(info));
}

void
WellKnownObjectsManager::addEndpoint(const string& name, const Ice::ObjectPrx& proxy)
{
    _endpoints.insert(make_pair(name, proxy));
}

void
WellKnownObjectsManager::finish()
{
    lock_guard lock(_mutex);
    _initialized = true;
}

void
WellKnownObjectsManager::registerAll(const ReplicaSessionPrx& session)
{
    if (!initialized())
    {
        return;
    }

    // If initialized, the endpoints and well known objects are immutable.
    try
    {
        session->setEndpoints(_endpoints);
        session->registerWellKnownObjects(_wellKnownObjects);
    }
    catch (const Ice::LocalException&)
    {
        // The session is already gone, ignore, this will be detected by the keep alive thread.
    }
}

void
WellKnownObjectsManager::registerAll()
{
    if (!initialized())
    {
        return;
    }

    // If initialized, the endpoints and well known objects are immutable.
    updateReplicatedWellKnownObjects();
    _database->addOrUpdateRegistryWellKnownObjects(_wellKnownObjects);
}

void
WellKnownObjectsManager::updateReplicatedWellKnownObjects()
{
    if (!initialized())
    {
        return;
    }

    // Update replicated objects.
    Ice::Identity id;
    id.category = _database->getInstanceName();
    ObjectInfo info;
    ObjectInfoSeq objects;

    lock_guard lock(_mutex);

    auto replicatedClientProxy = _database->getReplicaCache().getEndpoints("Client", _endpoints["Client"]);

    id.name = "Query";
    info.type = Query::ice_staticId();
    info.proxy = replicatedClientProxy->ice_identity(id);
    objects.push_back(info);

    id.name = "Locator";
    info.type = Ice::Locator::ice_staticId();
    info.proxy = replicatedClientProxy->ice_identity(id);
    objects.push_back(info);

    id.name = "LocatorRegistry";
    info.type = Ice::LocatorRegistry::ice_staticId();
    info.proxy = _database->getReplicaCache().getEndpoints("Server", _endpoints["Server"])->ice_identity(id);
    objects.push_back(info);

    _database->addOrUpdateRegistryWellKnownObjects(objects);
}

bool
WellKnownObjectsManager::initialized() const
{
    lock_guard lock(_mutex);
    return _initialized;
}

std::optional<Ice::ObjectPrx>
WellKnownObjectsManager::getEndpoints(const string& name)
{
    lock_guard lock(_mutex);
    return _endpoints[name];
}

LocatorPrx
WellKnownObjectsManager::getLocator()
{
    return LocatorPrx{
        getWellKnownObjectReplicatedProxy(Ice::Identity{"Locator", _database->getInstanceName()}, "Client")};
}

Ice::LocatorRegistryPrx
WellKnownObjectsManager::getLocatorRegistry()
{
    return Ice::LocatorRegistryPrx{
        getWellKnownObjectReplicatedProxy({"LocatorRegistry", _database->getInstanceName()}, "Server")};
}

Ice::ObjectPrx
WellKnownObjectsManager::getWellKnownObjectReplicatedProxy(const Ice::Identity& id, const string& endpt)
{
    try
    {
        auto proxy = _database->getObjectProxy(id);
        Ice::EndpointSeq registryEndpoints = getEndpoints(endpt)->ice_getEndpoints();

        // Re-order the endpoints to return first the endpoint for this registry replica.
        Ice::EndpointSeq endpoints = proxy->ice_getEndpoints();
        Ice::EndpointSeq newEndpoints = registryEndpoints;
        for (Ice::EndpointSeq::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
        {
            if (find(registryEndpoints.begin(), registryEndpoints.end(), *p) == registryEndpoints.end())
            {
                newEndpoints.push_back(*p);
            }
        }
        return proxy->ice_endpoints(newEndpoints);
    }
    catch (const ObjectNotRegisteredException&)
    {
        // If for some reasons the object isn't registered, we compute the endpoints with the replica cache. For
        // slaves, this will however only return the slave endpoints.
        return _database->getReplicaCache().getEndpoints(endpt, getEndpoints(endpt))->ice_identity(id);
    }
}
