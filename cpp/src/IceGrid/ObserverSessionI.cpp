// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/ObserverSessionI.h>
#include <IceGrid/Database.h>

using namespace std;
using namespace IceGrid;

ObserverSessionI::ObserverSessionI(const string& userId, 
				   const DatabasePtr& database,
				   RegistryObserverTopic& registryObserverTopic,
				   NodeObserverTopic& nodeObserverTopic) :
    _userId(userId), 
    _updating(false),
    _destroyed(false),
    _database(database),
    _registryObserverTopic(registryObserverTopic), 
    _nodeObserverTopic(nodeObserverTopic)
{
}

void
ObserverSessionI::setObservers(const RegistryObserverPrx& registryObserver, 
			       const NodeObserverPrx& nodeObserver, 
			       const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }

    _registryObserver = registryObserver;
    _nodeObserver = nodeObserver;

    //
    // Subscribe to the topics.
    //
    _registryObserverTopic.subscribe(_registryObserver); 
    _nodeObserverTopic.subscribe(_nodeObserver);
}

void
ObserverSessionI::setObserversByIdentity(const Ice::Identity& registryObserver, 
					 const Ice::Identity& nodeObserver,
					 const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }

    _registryObserver = RegistryObserverPrx::uncheckedCast(current.con->createProxy(registryObserver));
    _nodeObserver = NodeObserverPrx::uncheckedCast(current.con->createProxy(nodeObserver));

    //
    // Subscribe to the topics.
    //
    _registryObserverTopic.subscribe(_registryObserver); 
    _nodeObserverTopic.subscribe(_nodeObserver);
}

void
ObserverSessionI::startUpdate(int serial, const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }

    _database->lock(serial, this, _userId);
    _updating = true;
}

void
ObserverSessionI::addApplication(const ApplicationDescriptor& app, const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }

    if(!_updating)
    {
	throw AccessDenied();
    }
    _database->addApplicationDescriptor(this, app);
}

void
ObserverSessionI::updateApplication(const ApplicationUpdateDescriptor& update, const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }

    if(!_updating)
    {
	throw AccessDenied();
    }
    _database->updateApplicationDescriptor(this, update);
}

void
ObserverSessionI::syncApplication(const ApplicationDescriptor& app, const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }

    if(!_updating)
    {
	throw AccessDenied();
    }
    _database->syncApplicationDescriptor(this, app);
}

void
ObserverSessionI::removeApplication(const string& name, const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }

    if(!_updating)
    {
	throw AccessDenied();
    }
    _database->removeApplicationDescriptor(this, name);
}

void
ObserverSessionI::finishUpdate(const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }

    if(!_updating)
    {
	throw AccessDenied();
    }
    _database->unlock(this);
    _updating = false;
}

void
ObserverSessionI::destroy(const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }
    if(_updating)
    {
	_database->unlock(this);
	_updating = false;
    }

    //
    // Unsubscribe from the topics.
    //
    _registryObserverTopic.unsubscribe(_registryObserver);
    _nodeObserverTopic.unsubscribe(_nodeObserver);
}

LocalObserverSessionI::LocalObserverSessionI(const string& userId,
					     const DatabasePtr& database,
					     RegistryObserverTopic& registryObserverTopic,
					     NodeObserverTopic& nodeObserverTopic) :
    ObserverSessionI(userId, database, registryObserverTopic, nodeObserverTopic),
    _timestamp(IceUtil::Time::now())
{
}

void
LocalObserverSessionI::keepAlive(const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }

    _timestamp = IceUtil::Time::now();
}

IceUtil::Time
LocalObserverSessionI::timestamp() const
{
    Lock sync(*this);
    return _timestamp;
}

Glacier2ObserverSessionI::Glacier2ObserverSessionI(const string& userId,
						   const DatabasePtr& database,
						   RegistryObserverTopic& registryObserverTopic,
						   NodeObserverTopic& nodeObserverTopic) :
    ObserverSessionI(userId, database, registryObserverTopic, nodeObserverTopic)
{
}

void
Glacier2ObserverSessionI::keepAlive(const Ice::Current&)
{
}

IceUtil::Time
Glacier2ObserverSessionI::timestamp() const
{
    return IceUtil::Time();
}
