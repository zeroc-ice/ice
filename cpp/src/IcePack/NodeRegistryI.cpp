// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <IcePack/NodeRegistryI.h>
#include <IcePack/AdapterFactory.h>
#include <IcePack/TraceLevels.h>
#include <Freeze/Initialize.h>

using namespace std;
using namespace IcePack;

IcePack::NodeRegistryI::NodeRegistryI(const Ice::CommunicatorPtr& communicator,
				      const string& envName, 
				      const string& dbName, 
				      const AdapterRegistryPtr& adapterRegistry,
				      const AdapterFactoryPtr& adapterFactory,
				      const TraceLevelsPtr& traceLevels) :
    _connectionCache(Freeze::createConnection(communicator, envName)),
    _dictCache(_connectionCache, dbName),
    _adapterRegistry(adapterRegistry),
    _adapterFactory(adapterFactory),
    _traceLevels(traceLevels),
    _envName(envName),
    _communicator(communicator),
    _dbName(dbName)
{
    for(StringObjectProxyDict::const_iterator p = _dictCache.begin(); p != _dictCache.end(); ++p)
    {
	NodePrx node = NodePrx::uncheckedCast(p->second);
	try
	{
	    node->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	    remove(p->first);
	}
	catch(const Ice::LocalException&)
	{
	}
    }
}

void
IcePack::NodeRegistryI::add(const string& name, const NodePrx& node, const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(*this);

    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxyDict dict(connection, _dbName); 

    StringObjectProxyDict::iterator p = dict.find(name);
    if(p != dict.end())
    {
	try
	{
	    sync.release();
	    p->second->ice_ping();
	    sync.acquire();
	    throw NodeActiveException();
	}
	catch(const Ice::LocalException&)
	{
	    //
	    // Node not active.
	    //
	    sync.acquire();
	}
	p.set(node);

	if(_traceLevels->nodeRegistry > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->nodeRegistryCat);
	    out << "updated node `" << name << "' proxy";
	}
    }
    else
    {
	dict.put(pair<const string, const Ice::ObjectPrx>(name, node));
	
	if(_traceLevels->nodeRegistry > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->nodeRegistryCat);
	    out << "added node `" << name << "'";
	}
    }

    try
    {
	_adapterRegistry->findById("IcePack.Node-" + name);

	//
	// TODO: ensure this adapter has been created by the adapter
	// factory. It's possible that an adapter has been created
	// with the same name. In such a case, the best is probably to
	// prevent the node registration by throwing an appropriate
	// exception. The user would then need to remove the adapter
	// from the adapter registry to be able to run the node.
	//
    }    
    catch(const AdapterNotExistException&)
    {
	//
	// Create and register the node adapter.
	//
	AdapterPrx adapter = _adapterFactory->createStandaloneAdapter("IcePackNodeAdapter." + name);
	try
	{
	    _adapterRegistry->add("IcePack.Node-" + name, adapter);
	}
	catch(const AdapterExistsException&)
	{
	    adapter->destroy();
	}
    }
}

void
IcePack::NodeRegistryI::remove(const string& name, const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(*this);

    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxyDict dict(connection, _dbName); 


    StringObjectProxyDict::iterator p = dict.find(name);
    if(p == dict.end())
    {
	throw NodeNotExistException();
    }
    
    dict.erase(p);

    if(_traceLevels->nodeRegistry > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->nodeRegistryCat);
	out << "removed node `" << name << "'";
    }

    //
    // Remove the adapter from the adapter registry.
    //
    try
    {
	AdapterPrx adapter = _adapterRegistry->findById("IcePack.Node-" + name);
	adapter->destroy();
	_adapterRegistry->remove("IcePack.Node-" + name);
    }
    catch(const AdapterNotExistException&)
    {
    }
}

NodePrx
IcePack::NodeRegistryI::findByName(const string& name, const Ice::Current&)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxyDict dict(connection, _dbName); 

    StringObjectProxyDict::const_iterator p = dict.find(name);
    if(p != dict.end())
    {
	try
	{
	    return NodePrx::checkedCast(p->second);
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	catch(const Ice::LocalException&)
	{
	    return NodePrx::uncheckedCast(p->second);
	}
    }
    throw NodeNotExistException();
}

Ice::StringSeq
IcePack::NodeRegistryI::getAll(const Ice::Current&) const
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxyDict dict(connection, _dbName); 

    Ice::StringSeq names;
    names.reserve(dict.size());

    for(StringObjectProxyDict::const_iterator p = dict.begin(); p != dict.end(); ++p)
    {
	names.push_back(p->first);
    }

    return names;
}
