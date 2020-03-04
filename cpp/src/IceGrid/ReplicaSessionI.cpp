//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>

#include <IceGrid/ReplicaSessionI.h>
#include <IceGrid/Database.h>
#include <IceGrid/WellKnownObjectsManager.h>
#include <IceGrid/PlatformInfo.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

static bool
operator==(const ObjectInfo& info, const Ice::Identity& id)
{
    return info.proxy->ice_getIdentity() == id;
}

}

shared_ptr<ReplicaSessionI>
ReplicaSessionI::create(const shared_ptr<Database>& database,
                        const shared_ptr<WellKnownObjectsManager>& wellKnownObjects,
                        const shared_ptr<InternalReplicaInfo>& info,
                        const shared_ptr<InternalRegistryPrx>& proxy,
                        chrono::seconds timeout)
{

    shared_ptr<ReplicaSessionI> replicaSession(new ReplicaSessionI(database, wellKnownObjects, info, proxy, timeout));

    try
    {
        database->getReplicaCache().add(info->name, replicaSession);

        auto obsv = database->getObserverTopic(TopicName::RegistryObserver);
        static_pointer_cast<RegistryObserverTopic>(obsv)->registryUp(toRegistryInfo(info));

        replicaSession->_proxy =
            Ice::uncheckedCast<ReplicaSessionPrx>(database->getInternalAdapter()->addWithUUID(replicaSession));
    }
    catch(const ReplicaActiveException&)
    {
        throw;
    }
    catch(const std::exception&)
    {
        auto obsv = database->getObserverTopic(TopicName::RegistryObserver);
        static_pointer_cast<RegistryObserverTopic>(obsv)->registryDown(info->name);

        database->getReplicaCache().remove(info->name, false);

        throw;
    }

    return replicaSession;
}

ReplicaSessionI::ReplicaSessionI(const shared_ptr<Database>& database,
                                 const shared_ptr<WellKnownObjectsManager>& wellKnownObjects,
                                 const shared_ptr<InternalReplicaInfo>& info,
                                 const shared_ptr<InternalRegistryPrx>& proxy,
                                 chrono::seconds timeout) :
    _database(database),
    _wellKnownObjects(wellKnownObjects),
    _traceLevels(database->getTraceLevels()),
    _internalRegistry(proxy),
    _info(info),
    _timeout(timeout),
    _timestamp(chrono::steady_clock::now()),
    _destroy(false)
{
}

void
ReplicaSessionI::keepAlive(const Ice::Current&)
{
    lock_guard lock(_mutex);
    if(_destroy)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    _timestamp = chrono::steady_clock::now();

    if(_traceLevels->replica > 2)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
        out << "replica `" << _info->name << "' keep alive ";
    }
}

int
ReplicaSessionI::getTimeout(const Ice::Current&) const
{
    return secondsToInt(_timeout);
}

void
ReplicaSessionI::setDatabaseObserver(shared_ptr<DatabaseObserverPrx> observer,
                                     IceUtil::Optional<StringLongDict> slaveSerials,
                                     const Ice::Current&)
{
    //
    // If it's a read only master, we don't setup the observer to not
    // modify the replica database.
    //
    if(_database->isReadOnly())
    {
        return;
    }

    //
    // If the slave provides serials (Ice >= 3.5.1), we check the
    // serials and disable database synchronization if the slave has
    // earlier updates.
    //
    if(slaveSerials)
    {
        StringLongDict masterSerials = _database->getSerials();
        bool error = false;
        for(StringLongDict::const_iterator p = slaveSerials->begin(); p != slaveSerials->end(); ++p)
        {
            Ice::Long serial = masterSerials[p->first];
            if(serial < p->second)
            {
                error = true;
                break;
            }
        }
        if(error)
        {
            ostringstream os;
            os << "database from replica `" << _info->name << "' contains earlier updates:\n";
            for(StringLongDict::const_iterator p = slaveSerials->begin(); p != slaveSerials->end(); ++p)
            {
                Ice::Long serial = masterSerials[p->first];
                os << "database `" << p->first << "': ";
                os << "master serial = " << serial << ", replica serial = " << p->second << '\n';
            }
            os << "(database replication is disabled for this replica, please check the\n";
            os << " master and slave database with an administrative client and either:\n";
            os << " - restart the slave with --initdb-from-replica=Master\n";
            os << " - restart the master with --initdb-from-replica=" << _info->name;

            Ice::Error out(_traceLevels->logger);
            out << os.str();
            throw DeploymentException(os.str());
        }
    }

    int serialApplicationObserver;
    int serialAdapterObserver;
    int serialObjectObserver;

    const auto applicationObserver = _database->getObserverTopic(TopicName::ApplicationObserver);
    const auto adapterObserver = _database->getObserverTopic(TopicName::AdapterObserver);
    const auto objectObserver = _database->getObserverTopic(TopicName::ObjectObserver);

    {
        lock_guard lock(_mutex);
        if(_destroy)
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__);
        }
        _observer = observer;

        serialApplicationObserver = applicationObserver->subscribe(_observer, _info->name);
        serialAdapterObserver = adapterObserver->subscribe(_observer, _info->name);
        serialObjectObserver = objectObserver->subscribe(_observer, _info->name);
    }

    applicationObserver->waitForSyncedSubscribers(serialApplicationObserver, _info->name);
    adapterObserver->waitForSyncedSubscribers(serialAdapterObserver, _info->name);
    objectObserver->waitForSyncedSubscribers(serialObjectObserver, _info->name);
}

void
ReplicaSessionI::setEndpoints(StringObjectProxyDict endpoints, const Ice::Current&)
{
    {
        lock_guard lock(_mutex);
        if(_destroy)
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__);
        }
        _replicaEndpoints = move(endpoints);
    }
    _wellKnownObjects->updateReplicatedWellKnownObjects();
}

void
ReplicaSessionI::registerWellKnownObjects(ObjectInfoSeq objects, const Ice::Current&)
{
    int serial;
    {
        lock_guard lock(_mutex);
        if(_destroy)
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__);
        }
        _replicaWellKnownObjects = objects;
        serial = _database->addOrUpdateRegistryWellKnownObjects(objects);
    }

    //
    // We wait for the replica to receive the database replication
    // updates. This is to ensure that the replica well-known objects
    // are correctly setup when the replica starts accepting requests
    // from clients (if the replica is being started).
    //
    _database->getObserverTopic(TopicName::ObjectObserver)->waitForSyncedSubscribers(serial, _info->name);
}

void
ReplicaSessionI::setAdapterDirectProxy(string adapterId,
                                       string replicaGroupId,
                                       shared_ptr<Ice::ObjectPrx> proxy,
                                       const Ice::Current&)
{
    if(_database->getCommunicator()->getProperties()->getPropertyAsInt("IceGrid.Registry.DynamicRegistration") <= 0)
    {
        throw AdapterNotExistException();
    }
    _database->setAdapterDirectProxy(adapterId, replicaGroupId, proxy);
}

void
ReplicaSessionI::receivedUpdate(TopicName topicName, int serial, string failure, const Ice::Current&)
{
    auto topic = _database->getObserverTopic(topicName);
    if(topic)
    {
        topic->receivedUpdate(_info->name, serial, failure);
    }
}

void
ReplicaSessionI::destroy(const Ice::Current&)
{
    destroyImpl(false);
}

chrono::steady_clock::time_point
ReplicaSessionI::timestamp() const
{
    lock_guard lock(_mutex);
    if(_destroy)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    return _timestamp;
}

void
ReplicaSessionI::shutdown()
{
    destroyImpl(true);
}

const shared_ptr<InternalRegistryPrx>&
ReplicaSessionI::getInternalRegistry() const
{
    return _internalRegistry;
}

const shared_ptr<InternalReplicaInfo>&
ReplicaSessionI::getInfo() const
{
    return _info;
}

shared_ptr<ReplicaSessionPrx>
ReplicaSessionI::getProxy() const
{
    return _proxy;
}

shared_ptr<Ice::ObjectPrx>
ReplicaSessionI::getEndpoint(const std::string& name)
{
    lock_guard lock(_mutex);
    if(_destroy)
    {
        return nullptr;
    }
    return _replicaEndpoints[name];
}

bool
ReplicaSessionI::isDestroyed() const
{
    lock_guard lock(_mutex);
    return _destroy;
}

void
ReplicaSessionI::destroyImpl(bool shutdown)
{
    {
        lock_guard lock(_mutex);
        if(_destroy)
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__);
        }
        _destroy = true;
    }

    if(_observer)
    {
        _database->getObserverTopic(TopicName::ApplicationObserver)->unsubscribe(_observer, _info->name);
        _database->getObserverTopic(TopicName::AdapterObserver)->unsubscribe(_observer, _info->name);
        _database->getObserverTopic(TopicName::ObjectObserver)->unsubscribe(_observer, _info->name);
    }

    // Don't remove the replica proxy from the database if the registry is being shutdown.
    if(!_replicaWellKnownObjects.empty())
    {
        if(shutdown) // Don't remove the replica proxy from the database if the registry is being shutdown.
        {
            Ice::Identity id = { "Registry-" + _info->name, _internalRegistry->ice_getIdentity().category };
            auto p = find(_replicaWellKnownObjects.begin(), _replicaWellKnownObjects.end(), id);
            if(p != _replicaWellKnownObjects.end())
            {
                _replicaWellKnownObjects.erase(p);
            }
        }
        _database->removeRegistryWellKnownObjects(_replicaWellKnownObjects);
    }

    if(!shutdown)
    {
        _wellKnownObjects->updateReplicatedWellKnownObjects(); // No need to update these if we're shutting down.
    }

    //
    // Notify the observer that the registry is down.
    //
    auto obsv = _database->getObserverTopic(TopicName::RegistryObserver);
    static_pointer_cast<RegistryObserverTopic>(obsv)->registryDown(_info->name);

    //
    // Remove the replica from the cache. This must be done last. As
    // soon as the replica is removed another session might be
    // created.
    //
    _database->getReplicaCache().remove(_info->name, shutdown);

    if(!shutdown)
    {
        try
        {
            _database->getInternalAdapter()->remove(_proxy->ice_getIdentity());
        }
        catch(const Ice::ObjectAdapterDeactivatedException&)
        {
        }
    }
}
