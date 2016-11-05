// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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

ReplicaSessionI::ReplicaSessionI(const DatabasePtr& database,
                                 const WellKnownObjectsManagerPtr& wellKnownObjects,
                                 const InternalReplicaInfoPtr& info,
                                 const InternalRegistryPrx& proxy,
                                 int timeout) :
    _database(database),
    _wellKnownObjects(wellKnownObjects),
    _traceLevels(database->getTraceLevels()),
    _internalRegistry(proxy),
    _info(info),
    _timeout(timeout),
    _timestamp(IceUtil::Time::now(IceUtil::Time::Monotonic)),
    _destroy(false)
{
    __setNoDelete(true);
    try
    {
        _database->getReplicaCache().add(info->name, this);

        ObserverTopicPtr obsv = _database->getObserverTopic(RegistryObserverTopicName);
        RegistryObserverTopicPtr::dynamicCast(obsv)->registryUp(toRegistryInfo(_info));

        _proxy = ReplicaSessionPrx::uncheckedCast(_database->getInternalAdapter()->addWithUUID(this));
    }
    catch(const ReplicaActiveException&)
    {
        __setNoDelete(false);
        throw;
    }
    catch(...)
    {
        ObserverTopicPtr obsv = _database->getObserverTopic(RegistryObserverTopicName);
        RegistryObserverTopicPtr::dynamicCast(obsv)->registryDown(_info->name);

        _database->getReplicaCache().remove(_info->name, false);

        __setNoDelete(false);
        throw;
    }
    __setNoDelete(false);
}

void
ReplicaSessionI::keepAlive(const Ice::Current&)
{
    Lock sync(*this);
    if(_destroy)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    _timestamp = IceUtil::Time::now(IceUtil::Time::Monotonic);

    if(_traceLevels->replica > 2)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
        out << "replica `" << _info->name << "' keep alive ";
    }
}

int
ReplicaSessionI::getTimeout(const Ice::Current&) const
{
    return _timeout;
}

void
ReplicaSessionI::setDatabaseObserver(const DatabaseObserverPrx& observer,
                                     const IceUtil::Optional<StringLongDict>& slaveSerials,
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

    const ObserverTopicPtr applicationObserver = _database->getObserverTopic(ApplicationObserverTopicName);
    const ObserverTopicPtr adapterObserver = _database->getObserverTopic(AdapterObserverTopicName);
    const ObserverTopicPtr objectObserver = _database->getObserverTopic(ObjectObserverTopicName);

    {
        Lock sync(*this);
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
ReplicaSessionI::setEndpoints(const StringObjectProxyDict& endpoints, const Ice::Current&)
{
    {
        Lock sync(*this);
        if(_destroy)
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__);
        }
        _replicaEndpoints = endpoints;
    }
    _wellKnownObjects->updateReplicatedWellKnownObjects();
}

void
ReplicaSessionI::registerWellKnownObjects(const ObjectInfoSeq& objects, const Ice::Current&)
{
    int serial;
    {
        Lock sync(*this);
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
    _database->getObserverTopic(ObjectObserverTopicName)->waitForSyncedSubscribers(serial, _info->name);
}

void
ReplicaSessionI::setAdapterDirectProxy(const string& adapterId,
                                       const string& replicaGroupId,
                                       const Ice::ObjectPrx& proxy,
                                       const Ice::Current&)
{
    if(_database->getCommunicator()->getProperties()->getPropertyAsInt("IceGrid.Registry.DynamicRegistration") <= 0)
    {
        throw AdapterNotExistException();
    }
    _database->setAdapterDirectProxy(adapterId, replicaGroupId, proxy);
}

void
ReplicaSessionI::receivedUpdate(TopicName topicName, int serial, const string& failure, const Ice::Current&)
{
    ObserverTopicPtr topic = _database->getObserverTopic(topicName);
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

IceUtil::Time
ReplicaSessionI::timestamp() const
{
    Lock sync(*this);
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

const InternalRegistryPrx&
ReplicaSessionI::getInternalRegistry() const
{
    return _internalRegistry;
}

const InternalReplicaInfoPtr&
ReplicaSessionI::getInfo() const
{
    return _info;
}

ReplicaSessionPrx
ReplicaSessionI::getProxy() const
{
    return _proxy;
}

Ice::ObjectPrx
ReplicaSessionI::getEndpoint(const std::string& name)
{
    Lock sync(*this);
    if(_destroy)
    {
        return 0;
    }
    return _replicaEndpoints[name];
}

bool
ReplicaSessionI::isDestroyed() const
{
    Lock sync(*this);
    return _destroy;
}

void
ReplicaSessionI::destroyImpl(bool shutdown)
{
    {
        Lock sync(*this);
        if(_destroy)
        {
            throw Ice::ObjectNotExistException(__FILE__, __LINE__);
        }
        _destroy = true;
    }

    if(_observer)
    {
        _database->getObserverTopic(ApplicationObserverTopicName)->unsubscribe(_observer, _info->name);
        _database->getObserverTopic(AdapterObserverTopicName)->unsubscribe(_observer, _info->name);
        _database->getObserverTopic(ObjectObserverTopicName)->unsubscribe(_observer, _info->name);
    }

    // Don't remove the replica proxy from the database if the registry is being shutdown.
    if(!_replicaWellKnownObjects.empty())
    {
        if(shutdown) // Don't remove the replica proxy from the database if the registry is being shutdown.
        {
            Ice::Identity id;
            id.category = _internalRegistry->ice_getIdentity().category;
            id.name = "Registry-" + _info->name;
            ObjectInfoSeq::iterator p = find(_replicaWellKnownObjects.begin(), _replicaWellKnownObjects.end(), id);
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
    ObserverTopicPtr obsv = _database->getObserverTopic(RegistryObserverTopicName);
    RegistryObserverTopicPtr::dynamicCast(obsv)->registryDown(_info->name);

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
