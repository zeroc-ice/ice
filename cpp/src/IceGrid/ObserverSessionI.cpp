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

using namespace std;
using namespace IceGrid;

ObserverSessionI::ObserverSessionI(const string& userId, 
				   RegistryObserverTopic& registryObserverTopic,
				   NodeObserverTopic& nodeObserverTopic) :
    _userId(userId), 
    _destroyed(false),
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
    _registryObserverTopic.subscribe(registryObserver); 
    _nodeObserverTopic.subscribe(nodeObserver);
}

void
ObserverSessionI::destroy(const Ice::Current&)
{
    //
    // Unsubscribe from the topics.
    //
    _registryObserverTopic.unsubscribe(_registryObserver);
    _nodeObserverTopic.unsubscribe(_nodeObserver);
}

LocalObserverSessionI::LocalObserverSessionI(const string& userId, 
					     RegistryObserverTopic& registryObserverTopic,
					     NodeObserverTopic& nodeObserverTopic) :
    ObserverSessionI(userId, registryObserverTopic, nodeObserverTopic),
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
						   RegistryObserverTopic& registryObserverTopic,
						   NodeObserverTopic& nodeObserverTopic) :
    ObserverSessionI(userId, registryObserverTopic, nodeObserverTopic)
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
