// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/UUID.h>
#include <Ice/Ice.h>
#include <Freeze/Freeze.h>

#include <IceGrid/Registry.h>
#include <IceGrid/AdapterI.h>
#include <IceGrid/AdapterFactory.h>
#include <IceGrid/ApplicationRegistryI.h>
#include <IceGrid/ServerRegistryI.h>
#include <IceGrid/AdapterRegistryI.h>
#include <IceGrid/ObjectRegistryI.h>
#include <IceGrid/NodeRegistryI.h>
#include <IceGrid/LocatorI.h>
#include <IceGrid/LocatorRegistryI.h>
#include <IceGrid/AdminI.h>
#include <IceGrid/QueryI.h>
#include <IceGrid/TraceLevels.h>

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
using namespace IceGrid;

string
intToString(int v)
{
    ostringstream os;
    os << v;
    return os.str();
}

Registry::Registry(const CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

Registry::~Registry()
{
}

bool
Registry::start(bool nowarn)
{
    assert(_communicator);
    PropertiesPtr properties = _communicator->getProperties();

    //
    // Initialize the database environment.
    //
    string dbPath = properties->getProperty("IceGrid.Registry.Data");
    if(dbPath.empty())
    {
	Error out(_communicator->getLogger());
	out << "property `IceGrid.Registry.Data' is not set";
	return false;
    }
    else
    {
	struct stat filestat;
	if(stat(dbPath.c_str(), &filestat) != 0 || !S_ISDIR(filestat.st_mode))
	{
	    Error out(_communicator->getLogger());
	    SyscallException ex(__FILE__, __LINE__);
	    ex.error = getSystemErrno();
	    out << "property `IceGrid.Registry.Data' is set to an invalid path:\n" << ex;
	    return false;
	}
    }

    //
    // Check that required properties are set and valid.
    //
    if(properties->getProperty("IceGrid.Registry.Client.Endpoints").empty())
    {
	Error out(_communicator->getLogger());
	out << "property `IceGrid.Registry.Client.Endpoints' is not set";
	return false;
    }

    if(properties->getProperty("IceGrid.Registry.Server.Endpoints").empty())
    {
	Error out(_communicator->getLogger());
	out << "property `IceGrid.Registry.Server.Endpoints' is not set";
	return false;
    }

    if(properties->getProperty("IceGrid.Registry.Internal.Endpoints").empty())
    {
	Error out(_communicator->getLogger());
	out << "property `IceGrid.Registry.Internal.Endpoints' is not set";
	return false;
    }

    if(!properties->getProperty("IceGrid.Registry.Admin.Endpoints").empty())
    {
	if(!nowarn)
	{
	    Warning out(_communicator->getLogger());
	    out << "administrative endpoints `IceGrid.Registry.Admin.Endpoints' enabled";
	}
    }

    properties->setProperty("Ice.PrintProcessId", "0");
    properties->setProperty("Ice.Warn.Leaks", "0");
    properties->setProperty("Ice.ServerIdleTime", "0");
    properties->setProperty("IceGrid.Registry.Client.AdapterId", "");
    properties->setProperty("IceGrid.Registry.Server.AdapterId", "");
    properties->setProperty("IceGrid.Registry.Admin.AdapterId", "");
    properties->setProperty("IceGrid.Registry.Internal.AdapterId", "IceGrid.Registry.Internal");

    //
    // Setup thread pool size for each thread pool.
    //
    int nThreadPool = 0;
    if(properties->getPropertyAsInt("IceGrid.Registry.Client.ThreadPool.Size") == 0 &&
       properties->getPropertyAsInt("IceGrid.Registry.Client.ThreadPool.SizeMax") == 0)
    {
	++nThreadPool;
    }
    if(properties->getPropertyAsInt("IceGrid.Registry.Server.ThreadPool.Size") == 0 &&
       properties->getPropertyAsInt("IceGrid.Registry.Server.ThreadPool.SizeMax") == 0)
    {
	++nThreadPool;
    }
    if(properties->getPropertyAsInt("IceGrid.Registry.Admin.ThreadPool.Size") == 0 &&
       properties->getPropertyAsInt("IceGrid.Registry.Admin.ThreadPool.SizeMax") == 0)	
    {
	++nThreadPool;
    }    

    int size = properties->getPropertyAsIntWithDefault("Ice.ThreadPool.Server.Size", 10);
    if(size < nThreadPool)
    {
	size = nThreadPool;
    }
    int sizeMax = properties->getPropertyAsIntWithDefault("Ice.ThreadPool.Server.SizeMax", size * 10);
    if(sizeMax < size)
    {
	sizeMax = size;
    }
    int sizeWarn = properties->getPropertyAsIntWithDefault("Ice.ThreadPool.Server.SizeWarn", sizeMax * 80 / 100);

    if(properties->getPropertyAsInt("IceGrid.Registry.Client.ThreadPool.Size") == 0 &&
       properties->getPropertyAsInt("IceGrid.Registry.Client.ThreadPool.SizeMax") == 0)
    {
	properties->setProperty("IceGrid.Registry.Client.ThreadPool.Size", intToString(size / nThreadPool));
	properties->setProperty("IceGrid.Registry.Client.ThreadPool.SizeMax", intToString(sizeMax / nThreadPool));
	properties->setProperty("IceGrid.Registry.Client.ThreadPool.SizeWarn", intToString(sizeWarn / nThreadPool));
    }
    if(properties->getPropertyAsInt("IceGrid.Registry.Server.ThreadPool.Size") == 0 &&
       properties->getPropertyAsInt("IceGrid.Registry.Server.ThreadPool.SizeMax") == 0)
    {
	properties->setProperty("IceGrid.Registry.Server.ThreadPool.Size", intToString(size / nThreadPool));
	properties->setProperty("IceGrid.Registry.Server.ThreadPool.SizeMax", intToString(sizeMax / nThreadPool));
	properties->setProperty("IceGrid.Registry.Server.ThreadPool.SizeWarn", intToString(sizeWarn / nThreadPool));
    }
    if(properties->getPropertyAsInt("IceGrid.Registry.Admin.ThreadPool.Size") == 0 &&
       properties->getPropertyAsInt("IceGrid.Registry.Admin.ThreadPool.SizeMax") == 0)	
    {
	properties->setProperty("IceGrid.Registry.Admin.ThreadPool.Size", intToString(size / nThreadPool));
	properties->setProperty("IceGrid.Registry.Admin.ThreadPool.SizeMax", intToString(sizeMax / nThreadPool));
	properties->setProperty("IceGrid.Registry.Admin.ThreadPool.SizeWarn", intToString(sizeWarn / nThreadPool));
    }    

    int clientSize = properties->getPropertyAsInt("IceGrid.Registry.Client.ThreadPool.Size") * 2;
    int serverSize = properties->getPropertyAsInt("IceGrid.Registry.Server.ThreadPool.Size") * 2;
    properties->setProperty("IceGrid.Registry.Internal.ThreadPool.Size", intToString(clientSize + serverSize));

    int clientSizeMax = properties->getPropertyAsInt("IceGrid.Registry.Client.ThreadPool.SizeMax") * 2;
    if(clientSizeMax < clientSize)
    {
	clientSizeMax = clientSize;
    }
    int serverSizeMax = properties->getPropertyAsInt("IceGrid.Registry.Server.ThreadPool.SizeMax") * 2;
    if(serverSizeMax < serverSize)
    {
	serverSizeMax = serverSize;
    }
    properties->setProperty("IceGrid.Registry.Internal.ThreadPool.SizeMax", 
			    intToString(clientSizeMax + serverSizeMax));

    int clientSizeWarn = properties->getPropertyAsIntWithDefault("IceGrid.Registry.Client.ThreadPool.SizeWarn", 
								 clientSizeMax * 80 / 100) * 2;
    int serverSizeWarn = properties->getPropertyAsIntWithDefault("IceGrid.Registry.Server.ThreadPool.SizeWarn", 
								 serverSizeMax * 80 / 100) * 2;
    properties->setProperty("IceGrid.Registry.Internal.ThreadPool.SizeWarn", 
			    intToString(clientSizeWarn + serverSizeWarn));

    TraceLevelsPtr traceLevels = new TraceLevels(properties, _communicator->getLogger());

    _communicator->setDefaultLocator(0);

    //
    // Create the object adapters.
    //
    ObjectAdapterPtr serverAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Server");
    ObjectAdapterPtr clientAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Client");
    ObjectAdapterPtr adminAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Admin");
    ObjectAdapterPtr registryAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Internal");

    //
    // Create the internal registries (node, server, adapter, object).
    //
    const string envName = "Registry";
    properties->setProperty("Freeze.DbEnv.Registry.DbHome", dbPath);
    AdapterFactoryPtr adapterFactory = new AdapterFactory(registryAdapter, traceLevels, envName);
    ObjectRegistryPtr objectRegistry = new ObjectRegistryI(_communicator, envName, traceLevels);
    AdapterRegistryPtr adapterRegistry = new AdapterRegistryI(_communicator, envName, traceLevels);
    ServerRegistryPtr serverRegistry = new ServerRegistryI(_communicator, envName, traceLevels);
    ApplicationRegistryPtr appReg = new ApplicationRegistryI(_communicator, serverRegistry, envName, traceLevels);
    NodeRegistryPtr nodeReg = new NodeRegistryI(_communicator, envName, adapterRegistry, adapterFactory, traceLevels);

    registryAdapter->add(objectRegistry, stringToIdentity("IceGrid/ObjectRegistry"));
    registryAdapter->add(adapterRegistry, stringToIdentity("IceGrid/AdapterRegistry"));
    registryAdapter->add(serverRegistry, stringToIdentity("IceGrid/ServerRegistry"));    
    registryAdapter->add(appReg, stringToIdentity("IceGrid/ApplicationRegistry"));
    registryAdapter->add(nodeReg, stringToIdentity("IceGrid/NodeRegistry"));

    //
    // Create the locator registry and locator interfaces.
    //
    bool dynamicReg = properties->getPropertyAsInt("IceGrid.Registry.DynamicRegistration") > 0;
    ObjectPtr locatorRegistry = new LocatorRegistryI(adapterRegistry, serverRegistry, adapterFactory, dynamicReg);
    ObjectPrx obj = serverAdapter->add(locatorRegistry, stringToIdentity("IceGrid/" + IceUtil::generateUUID()));
    LocatorRegistryPrx locatorRegistryPrx = LocatorRegistryPrx::uncheckedCast(obj->ice_collocationOptimization(false));
    ObjectPtr locator = new LocatorI(adapterRegistry, objectRegistry, locatorRegistryPrx); 
    clientAdapter->add(locator, stringToIdentity("IceGrid/Locator"));

    //
    // Create the query interface and register it with the object registry.
    //
    QueryPtr query = new QueryI(_communicator, objectRegistry);
    clientAdapter->add(query, stringToIdentity("IceGrid/Query"));
    ObjectPrx queryPrx = clientAdapter->createDirectProxy(stringToIdentity("IceGrid/Query"));
    try
    {
	objectRegistry->remove(queryPrx->ice_getIdentity());
    }
    catch(const ObjectNotExistException&)
    {
    }	
    IceGrid::ObjectDescriptor desc;
    desc.proxy = queryPrx;
    desc.type = "::IceGrid::Query";	
    objectRegistry->add(desc);

    //
    // Create the admin interface and register it with the object registry.
    //
    ObjectPtr admin = new AdminI(_communicator, nodeReg, appReg, serverRegistry, adapterRegistry, objectRegistry, 
				 this);
    adminAdapter->add(admin, stringToIdentity("IceGrid/Admin"));    
    ObjectPrx adminPrx = adminAdapter->createDirectProxy(stringToIdentity("IceGrid/Admin"));
    try
    {
	objectRegistry->remove(adminPrx->ice_getIdentity());
    }
    catch(const ObjectNotExistException&)
    {
    }
    desc.proxy = adminPrx;
    desc.type = "::IceGrid::Admin";	
    objectRegistry->add(desc);

    //
    // Register the IceGrid.Registry.Internal adapter with the adapter registry.
    //
    try
    {
	adapterRegistry->remove("IceGrid.Registry.Internal", 0);
    }
    catch(const AdapterNotExistException&)
    {
    }
    obj = registryAdapter->addWithUUID(new StandaloneAdapterI());
    registryAdapter->activate();
    AdapterPrx adapter = AdapterPrx::uncheckedCast(registryAdapter->createDirectProxy(obj->ice_getIdentity())); 
    adapterRegistry->add("IceGrid.Registry.Internal", adapter);
    adapter->setDirectProxy(adapter);
    
    //
    // Setup a internal locator to be used by the IceGrid registry itself. This locator is 
    // registered with the registry object adapter which is using an independant threadpool.
    //
    locator = new LocatorI(adapterRegistry, objectRegistry, locatorRegistryPrx);
    registryAdapter->add(locator, stringToIdentity("IceGrid/Locator"));
    obj = registryAdapter->createDirectProxy(stringToIdentity("IceGrid/Locator"));
    _communicator->setDefaultLocator(LocatorPrx::uncheckedCast(obj->ice_collocationOptimization(false)));

    //
    // We are ready to go!
    //
    serverAdapter->activate();
    clientAdapter->activate();
    if(adminAdapter)
    {
	adminAdapter->activate();
    }
    
    return true;
}

void
Registry::shutdown()
{
    _communicator->shutdown();
}
