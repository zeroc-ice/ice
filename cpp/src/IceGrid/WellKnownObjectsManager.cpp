// Copyright (c) ZeroC, Inc.

#include "WellKnownObjectsManager.h"
#include "Database.h"
#include "Glacier2/PermissionsVerifier.h"
#include "Ice/Endpoint.h"

using namespace std;
using namespace IceGrid;

WellKnownObjectsManager::WellKnownObjectsManager(const shared_ptr<Database>& database) : _database(database) {}

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
    lock_guard lock(_mutex);

    auto replicatedClientProxy = _database->getReplicaCache().getEndpoints("Client", _endpoints["Client"]);
    _database->addOrUpdateRegistryWellKnownObjects(
        {ObjectInfo{
             replicatedClientProxy->ice_identity(Ice::Identity{"Query", _database->getInstanceName()}),
             string{Query::ice_staticId()}},
         ObjectInfo{
             replicatedClientProxy->ice_identity(Ice::Identity{"Locator", _database->getInstanceName()}),
             string{Ice::Locator::ice_staticId()}},
         ObjectInfo{
             _database->getReplicaCache()
                 .getEndpoints("Server", _endpoints["Server"])
                 ->ice_identity(Ice::Identity{"LocatorRegistry", _database->getInstanceName()}),
             string{Ice::LocatorRegistry::ice_staticId()}}});
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
    return Ice::uncheckedCast<LocatorPrx>(
        getWellKnownObjectReplicatedProxy(Ice::Identity{"Locator", _database->getInstanceName()}, "Client"));
}

Ice::LocatorRegistryPrx
WellKnownObjectsManager::getLocatorRegistry()
{
    return Ice::uncheckedCast<Ice::LocatorRegistryPrx>(
        getWellKnownObjectReplicatedProxy({"LocatorRegistry", _database->getInstanceName()}, "Server"));
}

Ice::ObjectPrx
WellKnownObjectsManager::getWellKnownObjectReplicatedProxy(const Ice::Identity& id, const string& endpt)
{
    try
    {
        auto proxy = _database->getObjectProxy(id);

        Ice::EndpointSeq registryEndpoints = getEndpoints(endpt)->ice_getEndpoints();

        // Re-order the endpoints to return first the endpoint for this registry replica.
        Ice::EndpointSeq newEndpoints = registryEndpoints;
        for (const auto& endpoint : proxy->ice_getEndpoints())
        {
            // Avoid duplicates.
            if (none_of(
                    registryEndpoints.begin(),
                    registryEndpoints.end(),
                    [&endpoint](const Ice::EndpointPtr& p) { return *endpoint == *p; }))
            {
                newEndpoints.push_back(endpoint);
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
