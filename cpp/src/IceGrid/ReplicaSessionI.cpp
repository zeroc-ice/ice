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

ReplicaSessionI::ReplicaSessionI(const DatabasePtr& database, 
				 const WellKnownObjectsManagerPtr& wellKnownObjects,
				 const string& name, 
				 const RegistryInfo& info,
				 const InternalRegistryPrx& proxy,
				 const DatabaseObserverPrx& databaseObserver) :
    _database(database),
    _wellKnownObjects(wellKnownObjects),
    _traceLevels(database->getTraceLevels()),
    _name(name),
    _internalRegistry(InternalRegistryPrx::uncheckedCast(proxy->ice_timeout(_database->getSessionTimeout() * 1000))),
    _databaseObserver(databaseObserver),
    _info(info),
    _timestamp(IceUtil::Time::now()),
    _destroy(false)
{
    __setNoDelete(true);
    try
    {
	_database->addReplica(name, this);
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
    return _database->getSessionTimeout();
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
    {
	Lock sync(*this);
	if(_destroy)
	{
	    throw Ice::ObjectNotExistException(__FILE__, __LINE__);
	}
	_replicaWellKnownObjects = objects;
    }
    _wellKnownObjects->registerWellKnownObjects(objects);
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
ReplicaSessionI::receivedUpdate(const string& update, int serial, const Ice::Current&)
{
    _database->replicaReceivedUpdate(_name, update, serial);
}

void
ReplicaSessionI::destroy(const Ice::Current& current)
{
    {
	Lock sync(*this);
	if(_destroy)
	{
	    throw Ice::ObjectNotExistException(__FILE__, __LINE__);
	}	
	_destroy = true;
    }
    _database->removeReplica(_name, this);

    _wellKnownObjects->unregisterWellKnownObjects(_replicaWellKnownObjects);
    bool shutdown = !current.adapter;
    if(shutdown)
    {
	ObjectInfo info;
	info.type = InternalRegistry::ice_staticId();
	info.proxy = _internalRegistry;
	_database->addObject(info, true);
    }
    else
    {
	_wellKnownObjects->updateReplicatedWellKnownObjects(); // No need to update these if we're shutting down.
    }

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
    return _replicaEndpoints[name];
}
