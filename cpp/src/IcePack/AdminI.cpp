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

#include <Ice/Ice.h>
#include <IcePack/AdminI.h>
#include <IcePack/ApplicationBuilder.h>

using namespace std;
using namespace Ice;
using namespace IcePack;

IcePack::AdminI::AdminI(const CommunicatorPtr& communicator, const NodeRegistryPtr& nodeRegistry,
			const ServerRegistryPtr& serverRegistry, const AdapterRegistryPtr& adapterRegistry,
			const ObjectRegistryPtr& objectRegistry) :
    _communicator(communicator),
    _nodeRegistry(nodeRegistry),
    _serverRegistry(serverRegistry),
    _adapterRegistry(adapterRegistry),
    _objectRegistry(objectRegistry)
{
}

IcePack::AdminI::~AdminI()
{
}

void
IcePack::AdminI::addApplication(const string& descriptor, const ServerTargets& targets, const Current&)
{
    ApplicationBuilder builder(_communicator, _nodeRegistry, targets);
    builder.parse(descriptor);
    builder.execute();
}

void
IcePack::AdminI::removeApplication(const string& descriptor, const Current&)
{
    ApplicationBuilder builder(_communicator, _nodeRegistry, ServerTargets());
    builder.parse(descriptor);
    builder.undo();
}

void
IcePack::AdminI::addServer(const string& node, const string& name, const string& path, const string& ldpath, 
			   const string& descriptor, const ServerTargets& targets, const Current&)
{
    ApplicationBuilder builder(_communicator, _nodeRegistry, targets);
    builder.addServer(name, node, descriptor, path, ldpath, "");
    builder.execute();
}

void
IcePack::AdminI::removeServer(const string& name, const Current&)
{
    ServerPrx server = _serverRegistry->findByName(name);
    try
    {
	ServerDescription desc = server->getServerDescription();
	
	ApplicationBuilder builder(_communicator, _nodeRegistry, desc.targets);
	builder.addServer(name, desc.node, desc.descriptor, desc.path, "", "");
	builder.undo();
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

ServerDescription
IcePack::AdminI::getServerDescription(const string& name, const Current&) const
{
    ServerPrx server = _serverRegistry->findByName(name);
    try
    {
	return server->getServerDescription();
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
}

ServerState
IcePack::AdminI::getServerState(const string& name, const Current&) const
{
    ServerPrx server = _serverRegistry->findByName(name);
    try
    {
	return server->getState();
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

Ice::Int
IcePack::AdminI::getServerPid(const string& name, const Current&) const
{
    ServerPrx server = _serverRegistry->findByName(name);
    try
    {
	return server->getPid();
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

bool
IcePack::AdminI::startServer(const string& name, const Current&)
{
    ServerPrx server = _serverRegistry->findByName(name);
    try
    {
	return server->start(Manual);
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

void
IcePack::AdminI::stopServer(const string& name, const Current&)
{
    ServerPrx server = _serverRegistry->findByName(name);
    try
    {
	return server->stop();
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

StringSeq
IcePack::AdminI::getAllServerNames(const Current&) const
{
    return _serverRegistry->getAll();
}

ServerActivation 
IcePack::AdminI::getServerActivation(const ::std::string& name, const Ice::Current&) const
{
    ServerPrx server = _serverRegistry->findByName(name);
    try
    {
	return server->getActivationMode();
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

void 
IcePack::AdminI::setServerActivation(const ::std::string& name, ServerActivation mode, const Ice::Current&)
{
    ServerPrx server = _serverRegistry->findByName(name);
    try
    {
	return server->setActivationMode(mode);
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

string 
IcePack::AdminI::getAdapterEndpoints(const string& id, const Current&) const
{
    AdapterPrx adapter = _adapterRegistry->findById(id);
    try
    {
	return _communicator->proxyToString(adapter->getDirectProxy(true));
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw AdapterNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

StringSeq
IcePack::AdminI::getAllAdapterIds(const Current&) const
{
    return _adapterRegistry->getAll();
}

void 
IcePack::AdminI::addObject(const Ice::ObjectPrx& proxy, const ::Ice::Current& current) const
{
    ObjectDescription desc;
    desc.proxy = proxy;
    
    try
    {
	addObjectWithType(proxy, proxy->ice_id(), current);
    }
    catch(const Ice::LocalException&)
    {
	ObjectDeploymentException ex;
	ex.reason = "Couldn't invoke on the object to get its interface.";
	ex.proxy = proxy;
	throw ex;
    }
}

void 
IcePack::AdminI::addObjectWithType(const Ice::ObjectPrx& proxy, const string& type, const ::Ice::Current&) const
{
    ObjectDescription desc;
    desc.proxy = proxy;
    desc.type = type;
    _objectRegistry->add(desc);
}

void 
IcePack::AdminI::removeObject(const Ice::ObjectPrx& proxy, const Ice::Current&) const
{
    _objectRegistry->remove(proxy);
}

bool
IcePack::AdminI::pingNode(const string& name, const Current&) const
{
    NodePrx node = _nodeRegistry->findByName(name);
    try
    {
	node->ice_ping();
	return true;
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw NodeNotExistException();
    }
    catch(const Ice::LocalException& ex)
    {
	return false;
    }
}

void
IcePack::AdminI::shutdownNode(const string& name, const Current&)
{
    NodePrx node = _nodeRegistry->findByName(name);
    try
    {
	node->shutdown();
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw NodeNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

StringSeq
IcePack::AdminI::getAllNodeNames(const Current&) const
{
    return _nodeRegistry->getAll();
}

void
IcePack::AdminI::shutdown(const Current&)
{
    _communicator->shutdown();
}
