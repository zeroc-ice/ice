// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePack/AdminI.h>
#include <IcePack/ApplicationBuilder.h>

using namespace std;
using namespace Ice;
using namespace IcePack;

IcePack::AdminI::AdminI(const CommunicatorPtr& communicator, const NodeRegistryPtr& nodeRegistry,
			const ServerRegistryPtr& serverRegistry, const AdapterRegistryPtr& adapterRegistry) :
    _communicator(communicator),
    _nodeRegistry(nodeRegistry),
    _serverRegistry(serverRegistry),
    _adapterRegistry(adapterRegistry)
{
}

IcePack::AdminI::~AdminI()
{
}

void
IcePack::AdminI::addApplication(const string& descriptor, const Targets& targets, const Current&)
{
    ApplicationBuilder builder(_communicator, _nodeRegistry, targets);
    builder.parse(descriptor);
    builder.execute();
}

void
IcePack::AdminI::removeApplication(const string& descriptor, const Current&)
{
    ApplicationBuilder builder(_communicator, _nodeRegistry, Targets());
    builder.parse(descriptor);
    builder.undo();
}

void
IcePack::AdminI::addServer(const string& node, const string& name, const string& path, const string& ldpath, 
			   const string& descriptor, const Targets& targets, const Current&)
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
	
	ApplicationBuilder builder(_communicator, _nodeRegistry, desc.theTargets);
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

string 
IcePack::AdminI::getAdapterEndpoints(const string& name, const Current&) const
{
    AdapterPrx adapter = _adapterRegistry->findByName(name);
    try
    {
	return _communicator->proxyToString(adapter->getDirectProxy(false));
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
IcePack::AdminI::getAllAdapterNames(const Current&) const
{
    return _adapterRegistry->getAll();
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
