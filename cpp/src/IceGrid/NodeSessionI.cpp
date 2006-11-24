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
			   const NodeInfo& info,
			   int timeout) :
    _database(database),
    _traceLevels(database->getTraceLevels()),
    _name(name),
    _node(NodePrx::uncheckedCast(node->ice_timeout(timeout * 1000))),
    _info(info),
    _timeout(timeout),
    _timestamp(IceUtil::Time::now()),
    _destroy(false)
{
    __setNoDelete(true);
    try
    {
	_database->getNode(name, true)->setSession(this);

	ObjectInfo info;
	info.type = Node::ice_staticId();
	info.proxy = _node;
	_database->addInternalObject(info, true); // Add or update previous node proxy.
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
    return _timeout;
}

NodeObserverPrx
NodeSessionI::getObserver(const Ice::Current& current) const
{
    return NodeObserverTopicPtr::dynamicCast(_database->getObserverTopic(NodeObserverTopicName))->getPublisher();
}

void
NodeSessionI::loadServers(const Ice::Current& current) const
{
    //
    // Get the server proxies to load them on the node.
    //
    Ice::StringSeq servers = _database->getNode(_name)->getServers();
    for(Ice::StringSeq::const_iterator p = servers.begin(); p != servers.end(); ++p)
    {
	try
	{
	    _database->getServer(*p)->load();
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
    return _database->getNode(_name)->getServers();
}

void
NodeSessionI::waitForApplicationUpdate_async(const AMD_NodeSession_waitForApplicationUpdatePtr& cb, 
					     const std::string& application, 
					     int revision, 
					     const Ice::Current&) const
{
    _database->waitForApplicationUpdate(cb, application, revision);
}

void
NodeSessionI::destroy(const Ice::Current& current)
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

    Ice::StringSeq servers = _database->getNode(_name)->getServers();
    for(Ice::StringSeq::const_iterator p = servers.begin(); p != servers.end(); ++p)
    {
	try
	{
	    _database->getServer(*p)->unload();
	}
	catch(const Ice::UserException&)
	{
	    // Ignore.
	}
    }

    //
    // If the registry isn't being shutdown we remove the node
    // internal proxy from the database.
    // 
    if(!shutdown)
    {
	_database->removeInternalObject(_node->ice_getIdentity());
    }

    //
    // Next we notify the observer.
    //
    NodeObserverTopicPtr::dynamicCast(_database->getObserverTopic(NodeObserverTopicName))->nodeDown(_name);
    

    //
    // Finally, we clear the session, this must be done last. As soon
    // as the node entry session is set to 0 another session might be
    // created.
    //
    _database->getNode(_name)->setSession(0);

    if(!shutdown)
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

bool
NodeSessionI::isDestroyed() const
{
    Lock sync(*this);
    return _destroy;
}
