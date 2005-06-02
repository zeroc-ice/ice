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

#include <IcePack/Registry.h>
#include <IcePack/AdapterI.h>
#include <IcePack/AdapterFactory.h>
#include <IcePack/ApplicationRegistryI.h>
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
    string dbPath = properties->getProperty("IcePack.Registry.Data");
    if(dbPath.empty())
    {
	Error out(_communicator->getLogger());
	out << "property `IcePack.Registry.Data' is not set";
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
	    out << "property `IcePack.Registry.Data' is set to an invalid path:\n" << ex;
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

    if(properties->getProperty("IcePack.Registry.Internal.Endpoints").empty())
    {
	Error out(_communicator->getLogger());
	out << "property `IcePack.Registry.Internal.Endpoints' is not set";
	return false;
    }

    if(!properties->getProperty("IcePack.Registry.Admin.Endpoints").empty())
    {
	if(!nowarn)
	{
	    Warning out(_communicator->getLogger());
	    out << "administrative endpoints `IcePack.Registry.Admin.Endpoints' enabled";
	}
    }

    properties->setProperty("Ice.PrintProcessId", "0");
    properties->setProperty("Ice.Warn.Leaks", "0");
    properties->setProperty("Ice.ServerIdleTime", "0");
    properties->setProperty("IcePack.Registry.Client.AdapterId", "");
    properties->setProperty("IcePack.Registry.Server.AdapterId", "");
    properties->setProperty("IcePack.Registry.Admin.AdapterId", "");
    properties->setProperty("IcePack.Registry.Internal.AdapterId", "IcePack.Registry.Internal");

    //
    // Setup thread pool size for each thread pool.
    //
    int nThreadPool = 0;
    if(properties->getPropertyAsInt("IcePack.Registry.Client.ThreadPool.Size") == 0 &&
       properties->getPropertyAsInt("IcePack.Registry.Client.ThreadPool.SizeMax") == 0)
    {
	++nThreadPool;
    }
    if(properties->getPropertyAsInt("IcePack.Registry.Server.ThreadPool.Size") == 0 &&
       properties->getPropertyAsInt("IcePack.Registry.Server.ThreadPool.SizeMax") == 0)
    {
	++nThreadPool;
    }
    if(properties->getPropertyAsInt("IcePack.Registry.Admin.ThreadPool.Size") == 0 &&
       properties->getPropertyAsInt("IcePack.Registry.Admin.ThreadPool.SizeMax") == 0)	
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

    if(properties->getPropertyAsInt("IcePack.Registry.Client.ThreadPool.Size") == 0 &&
       properties->getPropertyAsInt("IcePack.Registry.Client.ThreadPool.SizeMax") == 0)
    {
	properties->setProperty("IcePack.Registry.Client.ThreadPool.Size", intToString(size / nThreadPool));
	properties->setProperty("IcePack.Registry.Client.ThreadPool.SizeMax", intToString(sizeMax / nThreadPool));
	properties->setProperty("IcePack.Registry.Client.ThreadPool.SizeWarn", intToString(sizeWarn / nThreadPool));
    }
    if(properties->getPropertyAsInt("IcePack.Registry.Server.ThreadPool.Size") == 0 &&
       properties->getPropertyAsInt("IcePack.Registry.Server.ThreadPool.SizeMax") == 0)
    {
	properties->setProperty("IcePack.Registry.Server.ThreadPool.Size", intToString(size / nThreadPool));
	properties->setProperty("IcePack.Registry.Server.ThreadPool.SizeMax", intToString(sizeMax / nThreadPool));
	properties->setProperty("IcePack.Registry.Server.ThreadPool.SizeWarn", intToString(sizeWarn / nThreadPool));
    }
    if(properties->getPropertyAsInt("IcePack.Registry.Admin.ThreadPool.Size") == 0 &&
       properties->getPropertyAsInt("IcePack.Registry.Admin.ThreadPool.SizeMax") == 0)	
    {
	properties->setProperty("IcePack.Registry.Admin.ThreadPool.Size", intToString(size / nThreadPool));
	properties->setProperty("IcePack.Registry.Admin.ThreadPool.SizeMax", intToString(sizeMax / nThreadPool));
	properties->setProperty("IcePack.Registry.Admin.ThreadPool.SizeWarn", intToString(sizeWarn / nThreadPool));
    }    

    int clientSize = properties->getPropertyAsInt("IcePack.Registry.Client.ThreadPool.Size") * 2;
    int serverSize = properties->getPropertyAsInt("IcePack.Registry.Server.ThreadPool.Size") * 2;
    properties->setProperty("IcePack.Registry.Internal.ThreadPool.Size", intToString(clientSize + serverSize));

    int clientSizeMax = properties->getPropertyAsInt("IcePack.Registry.Client.ThreadPool.SizeMax") * 2;
    if(clientSizeMax < clientSize)
    {
	clientSizeMax = clientSize;
    }
    int serverSizeMax = properties->getPropertyAsInt("IcePack.Registry.Server.ThreadPool.SizeMax") * 2;
    if(serverSizeMax < serverSize)
    {
	serverSizeMax = serverSize;
    }
    properties->setProperty("IcePack.Registry.Internal.ThreadPool.SizeMax", 
			    intToString(clientSizeMax + serverSizeMax));

    int clientSizeWarn = properties->getPropertyAsIntWithDefault("IcePack.Registry.Client.ThreadPool.SizeWarn", 
								 clientSizeMax * 80 / 100) * 2;
    int serverSizeWarn = properties->getPropertyAsIntWithDefault("IcePack.Registry.Server.ThreadPool.SizeWarn", 
								 serverSizeMax * 80 / 100) * 2;
    properties->setProperty("IcePack.Registry.Internal.ThreadPool.SizeWarn", 
			    intToString(clientSizeWarn + serverSizeWarn));

    TraceLevelsPtr traceLevels = new TraceLevels(properties, _communicator->getLogger());

    _communicator->setDefaultLocator(0);

    //
    // Create the object adapters.
    //
    ObjectAdapterPtr serverAdapter = _communicator->createObjectAdapter("IcePack.Registry.Server");
    ObjectAdapterPtr clientAdapter = _communicator->createObjectAdapter("IcePack.Registry.Client");
    ObjectAdapterPtr adminAdapter = _communicator->createObjectAdapter("IcePack.Registry.Admin");
    ObjectAdapterPtr registryAdapter = _communicator->createObjectAdapter("IcePack.Registry.Internal");

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

    registryAdapter->add(objectRegistry, stringToIdentity("IcePack/ObjectRegistry"));
    registryAdapter->add(adapterRegistry, stringToIdentity("IcePack/AdapterRegistry"));
    registryAdapter->add(serverRegistry, stringToIdentity("IcePack/ServerRegistry"));    
    registryAdapter->add(appReg, stringToIdentity("IcePack/ApplicationRegistry"));
    registryAdapter->add(nodeReg, stringToIdentity("IcePack/NodeRegistry"));

    //
    // Create the locator registry and locator interfaces.
    //
    bool dynamicReg = properties->getPropertyAsInt("IcePack.Registry.DynamicRegistration") > 0;
    ObjectPtr locatorRegistry = new LocatorRegistryI(adapterRegistry, serverRegistry, adapterFactory, dynamicReg);
    ObjectPrx obj = serverAdapter->add(locatorRegistry, stringToIdentity("IcePack/" + IceUtil::generateUUID()));
    LocatorRegistryPrx locatorRegistryPrx = LocatorRegistryPrx::uncheckedCast(obj->ice_collocationOptimization(false));
    ObjectPtr locator = new LocatorI(adapterRegistry, objectRegistry, locatorRegistryPrx); 
    const string locatorIdProperty = "IcePack.Registry.LocatorIdentity";
    Identity locatorId = stringToIdentity(properties->getPropertyWithDefault(locatorIdProperty, "IcePack/Locator"));
    clientAdapter->add(locator, locatorId);

    //
    // Create the query interface and register it with the object registry.
    //
    QueryPtr query = new QueryI(_communicator, objectRegistry);
    const string queryIdProperty = "IcePack.Registry.QueryIdentity";
    Identity queryId = stringToIdentity(properties->getPropertyWithDefault(queryIdProperty, "IcePack/Query"));
    clientAdapter->add(query, queryId);
    ObjectPrx queryPrx = clientAdapter->createDirectProxy(queryId);
    try
    {
	objectRegistry->remove(queryPrx->ice_getIdentity());
    }
    catch(const ObjectNotExistException&)
    {
    }	
    IcePack::ObjectDescriptor desc;
    desc.proxy = queryPrx;
    desc.type = "::IcePack::Query";	
    objectRegistry->add(desc);

    //
    // Create the admin interface and register it with the object registry.
    //
    ObjectPtr admin = new AdminI(_communicator, nodeReg, appReg, serverRegistry, adapterRegistry, objectRegistry, this);
    const string adminIdProperty = "IcePack.Registry.AdminIdentity";
    Identity adminId = stringToIdentity(properties->getPropertyWithDefault(adminIdProperty, "IcePack/Admin"));
    adminAdapter->add(admin, adminId);
    ObjectPrx adminPrx = adminAdapter->createDirectProxy(adminId);
    try
    {
	objectRegistry->remove(adminPrx->ice_getIdentity());
    }
    catch(const ObjectNotExistException&)
    {
    }
    desc.proxy = adminPrx;
    desc.type = "::IcePack::Admin";	
    objectRegistry->add(desc);

    //
    // Register the IcePack.Registry.Internal adapter with the adapter registry.
    //
    try
    {
	adapterRegistry->remove("IcePack.Registry.Internal", 0);
    }
    catch(const AdapterNotExistException&)
    {
    }
    obj = registryAdapter->addWithUUID(new StandaloneAdapterI());
    registryAdapter->activate();
    AdapterPrx adapter = AdapterPrx::uncheckedCast(registryAdapter->createDirectProxy(obj->ice_getIdentity())); 
    adapterRegistry->add("IcePack.Registry.Internal", adapter);
    adapter->setDirectProxy(adapter);
    
    //
    // Setup a internal locator to be used by the IcePack registry itself. This locator is 
    // registered with the registry object adapter which is using an independant threadpool.
    //
    locator = new LocatorI(adapterRegistry, objectRegistry, locatorRegistryPrx);
    registryAdapter->add(locator, locatorId);
    obj = registryAdapter->createDirectProxy(locatorId);
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
