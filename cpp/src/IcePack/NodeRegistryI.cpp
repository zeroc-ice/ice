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

using namespace std;
using namespace IcePack;

IcePack::NodeRegistryI::NodeRegistryI(const Freeze::DBPtr& db, 
				      const AdapterRegistryPtr& adapterRegistry,
				      const AdapterFactoryPtr& adapterFactory,
				      const TraceLevelsPtr& traceLevels) :
    _dict(db),
    _adapterRegistry(adapterRegistry),
    _adapterFactory(adapterFactory),
    _traceLevels(traceLevels)
{
    for(StringObjectProxyDict::const_iterator p = _dict.begin(); p != _dict.end(); ++p)
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
	catch(const Ice::LocalException& ex)
	{
	}
    }
}

void
IcePack::NodeRegistryI::add(const string& name, const NodePrx& node, const Ice::Current&)
{
    IceUtil::Mutex::Lock sync(*this);

    StringObjectProxyDict::iterator p = _dict.find(name);
    if(p != _dict.end())
    {
	try
	{
	    p->second->ice_ping();
	    throw NodeActiveException();
	}
	catch(const Ice::LocalException&)
	{
	    //
	    // Node not active.
	    //
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
	_dict.insert(make_pair(name, node));
	
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
    catch(AdapterNotExistException&)
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

    StringObjectProxyDict::iterator p = _dict.find(name);
    if(p == _dict.end())
    {
	throw NodeNotExistException();
    }
    
    _dict.erase(p);

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
    StringObjectProxyDict::const_iterator p = _dict.find(name);
    if(p != _dict.end())
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
    Ice::StringSeq names;
    names.reserve(_dict.size());

    for(StringObjectProxyDict::const_iterator p = _dict.begin(); p != _dict.end(); ++p)
    {
	names.push_back(p->first);
    }

    return names;
}
