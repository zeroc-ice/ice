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

#include <IceUtil/UUID.h>
#include <Ice/Ice.h>
#include <Freeze/Freeze.h>

#include <IcePack/Registry.h>
#include <IcePack/AdapterI.h>
#include <IcePack/AdapterFactory.h>
#include <IcePack/ServerRegistryI.h>
#include <IcePack/AdapterRegistryI.h>
#include <IcePack/ObjectRegistryI.h>
#include <IcePack/NodeRegistryI.h>
#include <IcePack/LocatorI.h>
#include <IcePack/LocatorRegistryI.h>
#include <IcePack/AdminI.h>
#include <IcePack/QueryI.h>
#include <IcePack/TraceLevels.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#   include <direct.h>
#   define S_ISDIR(mode) ((mode) & _S_IFDIR)
#   define S_ISREG(mode) ((mode) & _S_IFREG)
#else
#   include <unistd.h>
#endif

using namespace std;
using namespace Ice;
using namespace IcePack;

IcePack::Registry::Registry(const CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

IcePack::Registry::~Registry()
{
}

bool
IcePack::Registry::start(bool nowarn, bool requiresInternalEndpoints)
{
    assert(_communicator);
    PropertiesPtr properties = _communicator->getProperties();

    //
    // Initialize the database environment.
    //
    _envName = properties->getProperty("IcePack.Registry.Data");
    if(_envName.empty())
    {
	Error out(_communicator->getLogger());
	out << "property `IcePack.Registry.Data' is not set";
	return false;
    }
    else
    {
	struct stat filestat;
	if(stat(_envName.c_str(), &filestat) != 0 || !S_ISDIR(filestat.st_mode))
	{
	    Error out(_communicator->getLogger());
	    out << "property `IcePack.Registry.Data' is not set to a valid directory path";
	    return false;	    
	}
    }

    //
    // Check that required properties are set and valid.
    //
    if(properties->getProperty("IcePack.Registry.Client.Endpoints").empty())
    {
	Error out(_communicator->getLogger());
	out << "property `IcePack.Registry.Client.Endpoints' is not set";
	return false;
    }

    if(properties->getProperty("IcePack.Registry.Server.Endpoints").empty())
    {
	Error out(_communicator->getLogger());
	out << "property `IcePack.Registry.Server.Endpoints' is not set";
	return false;
    }

    bool hasInternalEndpoints = !properties->getProperty("IcePack.Registry.Internal.Endpoints").empty();
    if(requiresInternalEndpoints && !hasInternalEndpoints)
    {
	Error out(_communicator->getLogger());
	out << "property `IcePack.Registry.Internal.Endpoints' is not set";
	return false;
    }
    else if(!requiresInternalEndpoints && !hasInternalEndpoints)
    {
	if(!nowarn)
	{
	    Warning out(_communicator->getLogger());
	    out << "internal endpoints `IcePack.Registry.Internal.Endpoints' disabled";
	}
    }    

    if(!properties->getProperty("IcePack.Registry.Admin.Endpoints").empty())
    {
	if(!nowarn)
	{
	    Warning out(_communicator->getLogger());
	    out << "administrative endpoints `IcePack.Registry.Admin.Endpoints' enabled";
	}
    }

    if(properties->getPropertyAsIntWithDefault("Ice.ThreadPool.Server.Size", 5) <= 5)
    {
	properties->setProperty("Ice.ThreadPool.Server.Size", "5");
    }

    _communicator->setDefaultLocator(0);

    properties->setProperty("Ice.PrintProcessId", "0");
    properties->setProperty("Ice.Warn.Leaks", "0");
    properties->setProperty("Ice.ServerIdleTime", "0");

    TraceLevelsPtr traceLevels = new TraceLevels(properties, _communicator->getLogger());

    //
    // Create the internal registries (node, server, adapter, object).
    //
    properties->setProperty("IcePack.Registry.Internal.AdapterId", "IcePack.Registry.Internal");

    ObjectAdapterPtr registryAdapter = _communicator->createObjectAdapter("IcePack.Registry.Internal");

    AdapterFactoryPtr adapterFactory = new AdapterFactory(registryAdapter, traceLevels, _envName);    
    
    ObjectRegistryPtr objectRegistry = new ObjectRegistryI(_communicator, _envName,
							   "objectregistry", "objectregistry-types",
							   traceLevels);

    AdapterRegistryPtr adapterRegistry = new AdapterRegistryI(_communicator, _envName, "adapterregistry", traceLevels);

    ServerRegistryPtr serverRegistry = new ServerRegistryI(_communicator, _envName, "serverregistry", traceLevels);

    NodeRegistryPtr nodeRegistry = new NodeRegistryI(_communicator, _envName, "noderegistry", 
						     adapterRegistry, adapterFactory, traceLevels);

    registryAdapter->add(objectRegistry, stringToIdentity("IcePack/ObjectRegistry"));
    registryAdapter->add(adapterRegistry, stringToIdentity("IcePack/AdapterRegistry"));
    registryAdapter->add(serverRegistry, stringToIdentity("IcePack/ServerRegistry"));    
    registryAdapter->add(nodeRegistry, stringToIdentity("IcePack/NodeRegistry"));

    //
    // Create the locator registry adapter and servant.
    //
    ObjectAdapterPtr serverAdapter = _communicator->createObjectAdapter("IcePack.Registry.Server");    
    
    Identity locatorRegistryId;
    locatorRegistryId.category = "IcePack";
    locatorRegistryId.name = IceUtil::generateUUID();
    
    ObjectPtr locatorRegistry = new LocatorRegistryI(adapterRegistry, serverAdapter);
    LocatorRegistryPrx locatorRegistryPrx = LocatorRegistryPrx::uncheckedCast(
	serverAdapter->add(locatorRegistry, locatorRegistryId));

    //
    // Create the locator adapter and servant.
    //
    ObjectAdapterPtr clientAdapter = _communicator->createObjectAdapter("IcePack.Registry.Client");

    ObjectPtr locator = new LocatorI(adapterRegistry, objectRegistry, locatorRegistryPrx); 
    LocatorPrx locatorPrx = LocatorPrx::uncheckedCast(
	clientAdapter->add(locator, stringToIdentity("IcePack/Locator"))->ice_collocationOptimization(false));

    QueryPtr query = new QueryI(_communicator, objectRegistry);
    clientAdapter->add(query, stringToIdentity("IcePack/Query"));

    ObjectAdapterPtr adminAdapter;

    if(!properties->getProperty("IcePack.Registry.Admin.Endpoints").empty())
    {
	//
	// Create the admin adapter and servant.
	//
	properties->setProperty("IcePack.Registry.Admin.AdapterId", "IcePack.Registry.Admin");
	adminAdapter = _communicator->createObjectAdapter("IcePack.Registry.Admin");
	
	AdminPtr admin = new AdminI(_communicator, nodeRegistry, serverRegistry, adapterRegistry, objectRegistry);
	adminAdapter->add(admin, stringToIdentity("IcePack/Admin"));
    
	//
	// Register the IcePack/Admin object with the object registry.
	//
	AdminPrx adminPrx = AdminPrx::uncheckedCast(
	    adminAdapter->createDirectProxy(stringToIdentity("IcePack/Admin")));

	try
	{
	    objectRegistry->remove(adminPrx);
	}
	catch(const ObjectNotExistException&)
	{
	}
	
	IcePack::ObjectDescription desc;
	desc.proxy = adminPrx;
	desc.type = "::IcePack::Admin";
	
	objectRegistry->add(desc);
    }

    //
    // Register the IcePack/Query object with the object registry.
    //
    QueryPrx queryPrx = QueryPrx::uncheckedCast(clientAdapter->createDirectProxy(stringToIdentity("IcePack/Query")));
    try
    {
	objectRegistry->remove(queryPrx);
    }
    catch(const ObjectNotExistException&)
    {
    }
	
    IcePack::ObjectDescription desc;
    desc.proxy = queryPrx;
    desc.type = "::IcePack::Query";	
    objectRegistry->add(desc);

    if(!properties->getProperty("IcePack.Registry.Internal.Endpoints").empty())
    {
	//
	// Register the IcePack.Registry.Internal adapter with the adapter
	// registry and set the locator on the registry adapter.
	//
	try
	{
	    adapterRegistry->remove("IcePack.Registry.Internal");
	}
	catch(const AdapterNotExistException&)
	{
	}
	adapterRegistry->add("IcePack.Registry.Internal", AdapterPrx::uncheckedCast(
				 serverAdapter->addWithUUID(new StandaloneAdapterI())));    
	
	registryAdapter->setLocator(locatorPrx);
    }

    _communicator->setDefaultLocator(locatorPrx);
	
    //
    // We are ready to go!
    //
    // NOTE: the locator registry adapter must be activated before the
    // registry adapter since the registry adapter needs the locator
    // to work to register itself on activation.
    //
    serverAdapter->activate();
    clientAdapter->activate();
    registryAdapter->activate();

    if(adminAdapter)
    {
	adminAdapter->activate();
    }
    
    return true;
}
