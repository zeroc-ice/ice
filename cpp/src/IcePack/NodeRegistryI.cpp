// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IcePack/NodeRegistryI.h>
#include <IcePack/AdapterFactory.h>
#include <IcePack/TraceLevels.h>
#include <Freeze/Initialize.h>

using namespace std;
using namespace IcePack;

const string NodeRegistryI::_dbName = "noderegistry";

NodeRegistryI::NodeRegistryI(const Ice::CommunicatorPtr& communicator, 
			     const string& envName, 
			     const AdapterRegistryPtr& adapterRegistry, 
			     const AdapterFactoryPtr& adapterFactory,
			     const TraceLevelsPtr& traceLevels) :
    _connectionCache(Freeze::createConnection(communicator, envName)),
    _dictCache(_connectionCache, _dbName),
    _adapterRegistry(adapterRegistry),
    _adapterFactory(adapterFactory),
    _traceLevels(traceLevels),
    _envName(envName),
    _communicator(communicator)
{
    for(StringObjectProxyDict::iterator p = _dictCache.begin(); p != _dictCache.end(); ++p)
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
NodeRegistryI::add(const string& name, const NodePrx& node, const Ice::Current& current)
{
    while(true)
    {
	NodePrx oldNode;
	try
	{
	    oldNode = findByName(name, current);
	    oldNode->ice_ping();
	    throw NodeActiveException();
	}
	catch(const NodeNotExistException&)
	{
	}
	catch(const Ice::LocalException&)
	{
	}

	IceUtil::Mutex::Lock sync(*this);

	Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
	StringObjectProxyDict dict(connection, _dbName);

	StringObjectProxyDict::iterator p = dict.find(name);
	if(p != dict.end())
	{
	    if(oldNode && oldNode != p->second)
	    {
		continue;
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

	break;
    }

    AdapterPrx adapter;
    while(!adapter)
    {
	try
	{
	    adapter = _adapterRegistry->findById("IcePack.Node." + name);
	    
	    //
	    // TODO: ensure this adapter has been created by the adapter factory. It's possible that an 
	    // adapter has been created with the same name. In such a case, the best is probably to
	    // prevent the node registration by throwing an appropriate exception. The user would then 
	    // need to remove the adapter from the adapter registry to be able to run the node.
	    //
	}    
	catch(const AdapterNotExistException&)
	{
	    //
	    // Create and register the node adapter.
	    //
	    adapter = _adapterFactory->createStandaloneAdapter("IcePackNodeAdapter." + name);
	    try
	    {
		_adapterRegistry->add("IcePack.Node." + name, adapter);
	    }
	    catch(const AdapterExistsException&)
	    {
		adapter->destroy();
		adapter = 0;
	    }
	}
    }

    //
    // Set the direct proxy of the node object adapter.
    //
    adapter->setDirectProxy(node);
}

void
NodeRegistryI::remove(const string& name, const Ice::Current&)
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
	AdapterPrx adapter = _adapterRegistry->findById("IcePack.Node." + name);
	adapter->destroy();
	_adapterRegistry->remove("IcePack.Node." + name, 0);
    }
    catch(const AdapterNotExistException&)
    {
    }
}

NodePrx
NodeRegistryI::findByName(const string& name, const Ice::Current&)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxyDict dict(connection, _dbName); 

    StringObjectProxyDict::iterator p = dict.find(name);
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
NodeRegistryI::getAll(const Ice::Current&) const
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxyDict dict(connection, _dbName); 

    Ice::StringSeq names;
    names.reserve(dict.size());

    for(StringObjectProxyDict::iterator p = dict.begin(); p != dict.end(); ++p)
    {
	names.push_back(p->first);
    }

    return names;
}
