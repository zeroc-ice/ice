// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
#include <IcePack/NodeRegistryI.h>
#include <IcePack/LocatorI.h>
#include <IcePack/LocatorRegistryI.h>
#include <IcePack/AdminI.h>
#include <IcePack/ExceptionFactory.h>
#include <IcePack/TraceLevels.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
using namespace IcePack;

IcePack::Registry::Registry(const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

IcePack::Registry::~Registry()
{
    if(_dbEnv)
    {
	try
	{
	    _dbEnv->close();
	}
	catch(const Freeze::DBException& ex)
	{
	    Ice::Error out(_communicator->getLogger());
	    out << ex << ": " << ex.message;
	}
	catch(const Ice::Exception& ex)
	{
	    Ice::Error out(_communicator->getLogger());
	    out << ex;
	}
	catch(...)
	{
	    Ice::Error out(_communicator->getLogger());
	    out << "unknown exception";
	}
	_dbEnv = 0;
    }
}

bool
IcePack::Registry::start(bool nowarn)
{
    assert(_communicator);
    Ice::PropertiesPtr properties = _communicator->getProperties();

    //
    // Initialize the database environment.
    //
    string dbPath = properties->getProperty("IcePack.Registry.Data");
    if(dbPath.empty())
    {
	Ice::Error out(_communicator->getLogger());
	out << "property `IcePack.Registry.Data' is not set";
	return false;
    }
    else
    {
	struct stat filestat;
	if(stat(dbPath.c_str(), &filestat) != 0 || !S_ISDIR(filestat.st_mode))
	{
	    Ice::Error out(_communicator->getLogger());
	    out << "property `IcePack.Registry.Data' is not set to a valid directory path";
	    return false;	    
	}
    }
    _dbEnv = Freeze::initialize(_communicator, dbPath);

    //
    // Check that required properties are set and valid.
    //
    string registryEndpoints = properties->getProperty("IcePack.Registry.Internal.Endpoints");
    if(registryEndpoints.empty())
    {
	Ice::Error out(_communicator->getLogger());
	out << "property `IcePack.Registry.Internal.Endpoints' is not set";
	return false;
    }

    string locatorEndpoints = properties->getProperty("IcePack.Registry.Locator.Endpoints");
    if(locatorEndpoints.empty())
    {
	Ice::Error out(_communicator->getLogger());
	out << "property `IcePack.Registry.Locator.Endpoints' is not set";
	return false;
    }

    string locatorRegistryEndpoints = properties->getProperty("IcePack.Registry.LocatorRegistry.Endpoints");
    if(locatorRegistryEndpoints.empty())
    {
	Ice::Error out(_communicator->getLogger());
	out << "property `IcePack.Registry.LocatorRegistry.Endpoints' is not set";
	return false;
    }

    string adminEndpoints = properties->getProperty("IcePack.Registry.Admin.Endpoints");
    if(!adminEndpoints.empty())
    {
	if(!nowarn)
	{
	    Ice::Warning out(_communicator->getLogger());
	    out << "administrative endpoints `IcePack.Registry.Admin.Endpoints' enabled";
	}
    }
    
    _communicator->setDefaultLocator(0);

    //
    // Register IcePack exception factory with the communicator.
    //
    Ice::UserExceptionFactoryPtr(new ExceptionFactory(_communicator));
    
    TraceLevelsPtr traceLevels = new TraceLevels(properties, _communicator->getLogger());

    //
    // Create the internal registries (node, server, adapter).
    //
    Ice::ObjectAdapterPtr registryAdapter = 
	_communicator->createObjectAdapterWithEndpoints("IcePackRegistryAdapter", registryEndpoints);

    AdapterFactoryPtr adapterFactory = new AdapterFactory(registryAdapter, traceLevels, _dbEnv);    

    AdapterRegistryPtr adapterRegistry = new AdapterRegistryI(_dbEnv->openDB("adapterregistry", true), traceLevels);
    registryAdapter->add(adapterRegistry, Ice::stringToIdentity("IcePack/AdapterRegistry"));

    ServerRegistryPtr serverRegistry = new ServerRegistryI(_dbEnv->openDB("serverregistry", true), traceLevels);
    registryAdapter->add(serverRegistry, Ice::stringToIdentity("IcePack/ServerRegistry"));    

    NodeRegistryPtr nodeRegistry = new NodeRegistryI(_dbEnv->openDB("noderegistry", true), adapterRegistry,
						     adapterFactory, traceLevels);
    registryAdapter->add(nodeRegistry, Ice::stringToIdentity("IcePack/NodeRegistry"));
    
    //
    // Create the locator registry adapter and the locator
    // registry servant.
    //
    // NOTE: the locator registry uses the registry object adapter
    // to activate standalone object adapters.
    //
    Ice::ObjectAdapterPtr locatorRegistryAdapter =
	_communicator->createObjectAdapterWithEndpoints("IcePackLocatorRegistryAdapter",locatorRegistryEndpoints);
    
    Ice::Identity locatorRegistryId;
    locatorRegistryId.category = "IcePack";
    locatorRegistryId.name = IceUtil::generateUUID();
    
    Ice::LocatorRegistryPrx locatorRegistryPrx = Ice::LocatorRegistryPrx::uncheckedCast(
	locatorRegistryAdapter->add(new LocatorRegistryI(adapterRegistry, registryAdapter), locatorRegistryId));
    
    //
    // Create the locator registry adapter and the locator
    // registry servant.
    //
    Ice::ObjectAdapterPtr locatorAdapter = 
	_communicator->createObjectAdapterWithEndpoints("IcePackLocatorAdapter", locatorEndpoints);
    
    Ice::Identity locatorId;
    locatorId.category = "IcePack";
    locatorId.name = "Locator";
    
    Ice::LocatorPrx locatorPrx = Ice::LocatorPrx::uncheckedCast(
	locatorAdapter->add(new LocatorI(adapterRegistry, locatorRegistryPrx), locatorId));
    
    //
    // Create the admin adapter and admin servant.
    //
    Ice::ObjectAdapterPtr adminAdapter = 
	_communicator->createObjectAdapterWithEndpoints("IcePackAdminAdapter", adminEndpoints);

    AdminPtr admin = new AdminI(_communicator, nodeRegistry, serverRegistry, adapterRegistry);
    adminAdapter->add(admin, Ice::stringToIdentity("IcePack/Admin"));
    
    //
    // Register the IcePackRegistryAdapter adapter and
    // IcePackAdminAdapter with the adapter registry so that they can
    // be located by clients.
    //
    try
    {
	adapterRegistry->remove("IcePackRegistryAdapter");
    }
    catch(const AdapterNotExistException&)
    {
    }
    try
    {
	adapterRegistry->remove("IcePackAdminAdapter");
    }
    catch(const AdapterNotExistException&)
    {
    }
    
    adapterRegistry->add("IcePackRegistryAdapter", AdapterPrx::uncheckedCast(
			     locatorRegistryAdapter->addWithUUID(new StandaloneAdapterI())));
    adapterRegistry->add("IcePackAdminAdapter", AdapterPrx::uncheckedCast(
			     locatorRegistryAdapter->addWithUUID(new StandaloneAdapterI())));
    
    //
    // Set the locator on the registry and admin adapter. This should
    // cause the adapters to register their endpoints with the locator
    // registry.
    //
    registryAdapter->setLocator(locatorPrx);
    adminAdapter->setLocator(locatorPrx);
    
    _communicator->setDefaultLocator(locatorPrx);

    //
    // We are ready to go!
    //
    // NOTE: the locator registry adapter must be activated before the
    // registry adapter since the registry adapter needs the locator
    // to work to register itself on activation.
    //
    locatorRegistryAdapter->activate();
    locatorAdapter->activate();
    registryAdapter->activate();
    adminAdapter->activate();
    
    return true;
}
