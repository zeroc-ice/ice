// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>

#include <IceGrid/NodeSessionI.h>
#include <IceGrid/Database.h>
#include <IceGrid/Topics.h>

using namespace std;
using namespace IceGrid;

NodeSessionI::NodeSessionI(const DatabasePtr& database, 
			   const string& name, 
			   const NodePrx& node, 
			   const NodeInfo& info) :
    _database(database),
    _traceLevels(database->getTraceLevels()),
    _name(name),
    _node(NodePrx::uncheckedCast(node->ice_timeout(_database->getSessionTimeout() * 1000))),
    _info(info),
    _timestamp(IceUtil::Time::now()),
    _destroy(false)
{
    __setNoDelete(true);
    try
    {
	_database->addNode(name, this);
    }
    catch(...)
    {
	__setNoDelete(false);
	throw;
    }
    __setNoDelete(false);
}

void
NodeSessionI::keepAlive(const LoadInfo& load, const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroy)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    _timestamp = IceUtil::Time::now();
    _load = load;

    if(_traceLevels->node > 2)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->nodeCat);
	out << "node `" << _name << "' keep alive ";
	out << "(load = " << _load.avg1 << ", " << _load.avg5 << ", " << _load.avg15 << ")";
    }
}

int
NodeSessionI::getTimeout(const Ice::Current& current) const
{
    return _database->getSessionTimeout();
}

NodeObserverPrx
NodeSessionI::getObserver(const Ice::Current& current) const
{
    NodeObserverTopicPtr topic = NodeObserverTopicPtr::dynamicCast(_database->getObserverTopic(NodeObserverTopicName));
    if(topic)
    {
	return topic->getPublisher();
    }
    else
    {
	return 0;
    }
}

void
NodeSessionI::loadServers(const Ice::Current& current) const
{
    //
    // Get the server proxies to load them on the node.
    //
    Ice::StringSeq servers = _database->getAllNodeServers(_name);
    for(Ice::StringSeq::const_iterator p = servers.begin(); p != servers.end(); ++p)
    {
	try
	{
	    _database->getServer(*p);
	}
	catch(const Ice::UserException&)
	{
	    // Ignore.
	}
    }
}

Ice::StringSeq
NodeSessionI::getServers(const Ice::Current& current) const
{
    return _database->getAllNodeServers(_name);
}

void
NodeSessionI::waitForApplicationReplication_async(const AMD_NodeSession_waitForApplicationReplicationPtr& cb, 
						  const std::string& application, 
						  int revision, 
						  const Ice::Current&) const
{
    _database->waitForApplicationReplication(cb, application, revision);
}

void
NodeSessionI::destroy(const Ice::Current& current)
{
    //
    // TODO: XXX: If we set destroy to true now, it's possible that
    // the node calls keepAlive on the sesion and tries to create the
    // session after getting the ONE and before the node is removed 
    // from the db...
    //
    {
	Lock sync(*this);
	if(_destroy)
	{
	    throw Ice::ObjectNotExistException(__FILE__, __LINE__);
	}	
	_destroy = true;
    }

    _database->removeNode(_name, this, !current.adapter);

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

const NodePrx&
NodeSessionI::getNode() const
{
    return _node;
}

const NodeInfo&
NodeSessionI::getInfo() const
{
    return _info;
}

const LoadInfo&
NodeSessionI::getLoadInfo() const
{
    Lock sync(*this);
    return _load;
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
