// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>

#include <IceGrid/ReplicaSessionI.h>
#include <IceGrid/Database.h>
#include <IceGrid/WellKnownObjectsManager.h>

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
				 const string& name, 
				 const RegistryInfo& info,
				 const InternalRegistryPrx& proxy,
				 int timeout) :
    _database(database),
    _wellKnownObjects(wellKnownObjects),
    _traceLevels(database->getTraceLevels()),
    _name(name),
    _internalRegistry(InternalRegistryPrx::uncheckedCast(proxy->ice_timeout(timeout * 1000))),
    _info(info),
    _timeout(timeout),
    _timestamp(IceUtil::Time::now()),
    _destroy(false)
{
    __setNoDelete(true);
    try
    {
	_database->getReplicaCache().add(name, this);
	ObserverTopicPtr obsv = _database->getObserverTopic(RegistryObserverTopicName);
	RegistryObserverTopicPtr::dynamicCast(obsv)->registryUp(_info);
    }
    catch(...)
    {
	__setNoDelete(false);
	throw;
    }
    __setNoDelete(false);
}

void
ReplicaSessionI::keepAlive(const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroy)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    _timestamp = IceUtil::Time::now();

    if(_traceLevels->replica > 2)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
	out << "replica `" << _name << "' keep alive ";
    }
}

int
ReplicaSessionI::getTimeout(const Ice::Current& current) const
{
    return _timeout;
}

void
ReplicaSessionI::setDatabaseObserver(const DatabaseObserverPrx& observer, const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroy)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }	
    _observer = observer;
    _database->getObserverTopic(ApplicationObserverTopicName)->subscribe(_observer, _name);
    _database->getObserverTopic(AdapterObserverTopicName)->subscribe(_observer, _name);
    _database->getObserverTopic(ObjectObserverTopicName)->subscribe(_observer, _name);
}

void
ReplicaSessionI::setEndpoints(const StringObjectProxyDict& endpoints, const Ice::Current& current)
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
ReplicaSessionI::registerWellKnownObjects(const ObjectInfoSeq& objects, const Ice::Current& current)
{
    int serial;
    {
	Lock sync(*this);
	if(_destroy)
	{
	    throw Ice::ObjectNotExistException(__FILE__, __LINE__);
	}
	_replicaWellKnownObjects = objects;
	serial = _database->addOrUpdateObjectsInDatabase(objects);
    }

    //
    // We wait for the replica to receive the database replication
    // updates. This is to ensure that the replica well-known objects
    // are correctly setup when the replica starts accepting requests
    // from clients (if the replica is being started).
    //
    _database->getObserverTopic(ObjectObserverTopicName)->waitForSyncedSubscribers(serial, _name);
}

void
ReplicaSessionI::setAdapterDirectProxy(const string& adapterId,
				       const string& replicaGroupId, 
				       const Ice::ObjectPrx& proxy,
				       const Ice::Current&)
{
    _database->setAdapterDirectProxy(adapterId, replicaGroupId, proxy);
}

void
ReplicaSessionI::receivedUpdate(TopicName topicName, int serial, const string& failure, const Ice::Current&)
{
    ObserverTopicPtr topic = _database->getObserverTopic(topicName);
    if(topic)
    {
	topic->receivedUpdate(_name, serial, failure);
    }
}

void
ReplicaSessionI::destroy(const Ice::Current& current)
{
    const bool shutdown = !current.adapter; // adapter is null if we're shutting down, see InternalRegistryI.cpp

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
	_database->getObserverTopic(ApplicationObserverTopicName)->unsubscribe(_observer, _name);
	_database->getObserverTopic(AdapterObserverTopicName)->unsubscribe(_observer, _name);
	_database->getObserverTopic(ObjectObserverTopicName)->unsubscribe(_observer, _name);
    }

    if(!_replicaWellKnownObjects.empty())
    {
	if(shutdown) // Don't remove the replica proxy from the database if the registry is being shutdown.
	{
	    ObjectInfoSeq::iterator p = find(_replicaWellKnownObjects.begin(), _replicaWellKnownObjects.end(), 
					     _internalRegistry->ice_getIdentity());
	    if(p != _replicaWellKnownObjects.end())
	    {
		_replicaWellKnownObjects.erase(p);
	    }
	}
	_database->removeObjectsInDatabase(_replicaWellKnownObjects);
    }

    if(!shutdown)
    {
	_wellKnownObjects->updateReplicatedWellKnownObjects(); // No need to update these if we're shutting down.
    }

    //
    // Notify the observer that the registry is down.
    //
    ObserverTopicPtr obsv = _database->getObserverTopic(RegistryObserverTopicName);
    RegistryObserverTopicPtr::dynamicCast(obsv)->registryDown(_name);

    //
    // Remove the replica from the cache. This must be done last. As
    // soon as the replica is removed another session might be
    // created.
    //
    _database->getReplicaCache().remove(_name, shutdown);

    if(current.adapter)
    {
	try
	{
	    current.adapter->remove(current.id);
	}
	catch(const Ice::ObjectAdapterDeactivatedException&)
	{
	}
    }
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
