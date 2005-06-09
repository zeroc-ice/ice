// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/Database.h>

using namespace std;
using namespace IceGrid;

NodeSessionI::NodeSessionI(const DatabasePtr& database, const RegistryObserverPrx& observer, const string& name, 
			   const NodePrx& node) : 
    _database(database),
    _observer(observer),
    _name(name),
    _node(node),
    _startTime(IceUtil::Time::now()),
    _timestamp(_startTime),
    _destroy(false)
{
    __setNoDelete(true);
    try
    {
	_database->addNode(name, this);
	
	try
	{
	    _observer->nodeUp(_name);
	}
	catch(const Ice::LocalException& ex)
	{
	    // TODO: Log a warning?
	    cerr << ex << endl;
	}
    }
    catch(...)
    {
	__setNoDelete(false);
	throw;
    }
    __setNoDelete(false);
}

void
NodeSessionI::keepAlive(const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroy)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    _timestamp = IceUtil::Time::now();
}

Ice::StringSeq
NodeSessionI::getServers(const Ice::Current& current)
{
    return _database->getAllNodeServers(_name);
}

void
NodeSessionI::destroy(const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroy)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    _destroy = true;

    _database->removeNode(_name);

    try
    {
	_observer->nodeDown(_name);
    }
    catch(const Ice::LocalException&)
    {
	// TODO: Log a warning?
    }
    
    try
    {
	current.adapter->remove(current.id);
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
    }
}

const NodePrx&
NodeSessionI::getNode() const
{
    return _node;
}

IceUtil::Time
NodeSessionI::timestamp() const
{
    Lock sync(*this);
    if(_destroy)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    return _timestamp;
}
