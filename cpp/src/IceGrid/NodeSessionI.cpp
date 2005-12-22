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

NodeSessionI::NodeSessionI(const DatabasePtr& database, const string& name, const NodePrx& node, 
			   const NodeInfo& info) : 
    _database(database),
    _traceLevels(database->getTraceLevels()),
    _name(name),
    _node(node),
    _info(info),
    _startTime(IceUtil::Time::now()),
    _timestamp(_startTime),
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
