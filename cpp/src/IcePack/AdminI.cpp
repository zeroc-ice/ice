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
#include <IcePack/ServerManager.h>
#include <IcePack/AdapterManager.h>
#include <IcePack/ApplicationDeployer.h>
#include <IcePack/ServerDeployer.h>

using namespace std;
using namespace Ice;
using namespace IcePack;

IcePack::AdminI::AdminI(const CommunicatorPtr& communicator, const ServerManagerPrx& serverManager,
			const AdapterManagerPrx& adapterManager) :
    _communicator(communicator),
    _serverManager(serverManager),
    _adapterManager(adapterManager)
{
}

void
IcePack::AdminI::addApplication(const string& descriptor, const Targets& targets, const Current&)
{
    ApplicationDeployer deployer(_communicator, this, targets);
    deployer.parse(descriptor);
    deployer.deploy();
}

void
IcePack::AdminI::removeApplication(const string& descriptor, const Current&)
{
    ApplicationDeployer deployer(_communicator, this, Targets());
    deployer.parse(descriptor);
    deployer.deploy();
}

void
IcePack::AdminI::addServer(const string& name, const string& path, const string& ldpath, const string& descriptor,
			   const Targets& targets, const Current&)
{
    ServerDeployer deployer(_communicator, name, path, ldpath, targets);
    deployer.setServerManager(_serverManager);
    deployer.setAdapterManager(_adapterManager);
    deployer.parse(descriptor);
    deployer.deploy();
}

ServerDescription
IcePack::AdminI::getServerDescription(const string& name, const Current&)
{
    ServerPrx server = _serverManager->findByName(name);
    if(server)
    {
	try
	{
	    return server->getServerDescription();
	}
	catch(const ObjectNotExistException&)
	{
	}
    }
    throw ServerNotExistException();
}

ServerState
IcePack::AdminI::getServerState(const string& name, const Current&)
{
    ServerPrx server = _serverManager->findByName(name);
    if(server)
    {
	try
	{
	    return server->getState();
	}
	catch(const ObjectNotExistException&)
	{
	}
    }
    throw ServerNotExistException();
}

bool
IcePack::AdminI::startServer(const string& name, const Current&)
{
    ServerPrx server = _serverManager->findByName(name);
    if(server)
    {
	try
	{
	    return server->start();
	}
	catch(const ObjectNotExistException&)
	{
	}
    }
    throw ServerNotExistException();
}

void
IcePack::AdminI::removeServer(const string& name, const Current&)
{
    ServerPrx server = _serverManager->findByName(name);
    if(!server)
    {
	throw ServerNotExistException();
    }

    server->setState(Destroyed);
    ServerDescription desc = server->getServerDescription();
    
    ServerDeployer deployer(_communicator, desc.name, desc.path, "", desc.targets);
    deployer.setServerManager(_serverManager);
    deployer.setAdapterManager(_adapterManager);
    deployer.parse(desc.descriptor);
    deployer.undeploy();
}

ServerNames
IcePack::AdminI::getAllServerNames(const Current&)
{
    return _serverManager->getAll();
}

void 
IcePack::AdminI::addAdapterWithEndpoints(const string& name, const string& endpoints, const Current&)
{
    AdapterDescription desc;
    desc.name = name;

    //
    // Create the adapter.
    //
    AdapterPrx adapter = _adapterManager->create(desc);

    //
    // Set the adapter direct proxy.
    //
    ObjectPrx object = _communicator->stringToProxy("dummy:" + endpoints);
    adapter->setDirectProxy(object);
}

void 
IcePack::AdminI::removeAdapter(const string& name, const Current&)
{
    _adapterManager->remove(name);
}

string 
IcePack::AdminI::getAdapterEndpoints(const string& name, const Current&)
{
    AdapterPrx adapter = _adapterManager->findByName(name);
    if(adapter)
    {
	try
	{
	    return _communicator->proxyToString(adapter->getDirectProxy(false));
	}
	catch(const ObjectNotExistException&)
	{
	}
    }
    throw AdapterNotExistException();
}

AdapterNames
IcePack::AdminI::getAllAdapterNames(const Current&)
{
    return _adapterManager->getAll();
}

void
IcePack::AdminI::shutdown(const Current&)
{
    _communicator->shutdown();
}
