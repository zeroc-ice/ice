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

#include <IceGrid/RegistryI.h>
#include <IceGrid/LocatorI.h>
#include <IceGrid/LocatorRegistryI.h>
#include <IceGrid/AdminI.h>
#include <IceGrid/QueryI.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/Database.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/ReapThread.h>

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

namespace IceStorm
{
};

namespace IceGrid
{

string
intToString(int v)
{
    ostringstream os;
    os << v;
    return os.str();
}

};

RegistryI::RegistryI(const CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

RegistryI::~RegistryI()
{
}

bool
RegistryI::start(bool nowarn)
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
    const string clientThreadPool("IceGrid.Registry.Client.ThreadPool");
    if(properties->getPropertyAsInt(clientThreadPool + ".Size") == 0 &&
       properties->getPropertyAsInt(clientThreadPool + ".SizeMax") == 0)
    {
	++nThreadPool;
    }
    const string serverThreadPool("IceGrid.Registry.Server.ThreadPool");
    if(properties->getPropertyAsInt(serverThreadPool + ".Size") == 0 &&
       properties->getPropertyAsInt(serverThreadPool + ".SizeMax") == 0)
    {
	++nThreadPool;
    }
    const string adminThreadPool("IceGrid.Registry.Admin.ThreadPool");
    if(properties->getPropertyAsInt(adminThreadPool + ".Size") == 0 &&
       properties->getPropertyAsInt(adminThreadPool + ".SizeMax") == 0)	
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

    if(properties->getPropertyAsInt(clientThreadPool + ".Size") == 0 &&
       properties->getPropertyAsInt(clientThreadPool + ".SizeMax") == 0)
    {
	properties->setProperty(clientThreadPool + ".Size", IceGrid::intToString(size / nThreadPool));
	properties->setProperty(clientThreadPool + ".SizeMax", IceGrid::intToString(sizeMax / nThreadPool));
	properties->setProperty(clientThreadPool + ".SizeWarn", IceGrid::intToString(sizeWarn / nThreadPool));
    }
    if(properties->getPropertyAsInt(serverThreadPool + ".Size") == 0 &&
       properties->getPropertyAsInt(serverThreadPool + ".SizeMax") == 0)
    {
	properties->setProperty(serverThreadPool + ".Size", IceGrid::intToString(size / nThreadPool));
	properties->setProperty(serverThreadPool + ".SizeMax", IceGrid::intToString(sizeMax / nThreadPool));
	properties->setProperty(serverThreadPool + ".SizeWarn", IceGrid::intToString(sizeWarn / nThreadPool));
    }
    if(properties->getPropertyAsInt(adminThreadPool + ".Size") == 0 &&
       properties->getPropertyAsInt(adminThreadPool + ".SizeMax") == 0)	
    {
	properties->setProperty(adminThreadPool + ".Size", IceGrid::intToString(size / nThreadPool));
	properties->setProperty(adminThreadPool + ".SizeMax", IceGrid::intToString(sizeMax / nThreadPool));
	properties->setProperty(adminThreadPool + ".SizeWarn", IceGrid::intToString(sizeWarn / nThreadPool));
    }    

    int clientSize = properties->getPropertyAsInt(clientThreadPool + ".Size") * 2;
    int serverSize = properties->getPropertyAsInt(serverThreadPool + ".Size") * 2;
    const string internalThreadPool("IceGrid.Registry.Internal.ThreadPool");
    properties->setProperty(internalThreadPool + ".Size", IceGrid::intToString(clientSize + serverSize));

    int clientSizeMax = properties->getPropertyAsInt(clientThreadPool + ".SizeMax") * 2;
    if(clientSizeMax < clientSize)
    {
	clientSizeMax = clientSize;
    }
    int serverSizeMax = properties->getPropertyAsInt(serverThreadPool + ".SizeMax") * 2;
    if(serverSizeMax < serverSize)
    {
	serverSizeMax = serverSize;
    }
    properties->setProperty(internalThreadPool + ".SizeMax", IceGrid::intToString(clientSizeMax + serverSizeMax));

    int clientSizeWarn = 
	properties->getPropertyAsIntWithDefault(clientThreadPool + ".SizeWarn", clientSizeMax * 80 / 100) * 2;
    int serverSizeWarn = 
	properties->getPropertyAsIntWithDefault(serverThreadPool + ".SizeWarn", serverSizeMax * 80 / 100) * 2;
    properties->setProperty(internalThreadPool + ".SizeWarn", IceGrid::intToString(clientSizeWarn + serverSizeWarn));

    _nodeSessionTimeout = properties->getPropertyAsIntWithDefault("IceGrid.Registry.NodeSessionTimeout", 10) * 1000;

    TraceLevelsPtr traceLevels = new TraceLevels(properties, _communicator->getLogger(), false);

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
    _database = new Database(_communicator, registryAdapter, envName, _nodeSessionTimeout, traceLevels);

    //
    // Create the locator registry and locator interfaces.
    //
    bool dynamicReg = properties->getPropertyAsInt("IceGrid.Registry.DynamicRegistration") > 0;
    ObjectPtr locatorRegistry = new LocatorRegistryI(_database, dynamicReg);
    ObjectPrx obj = serverAdapter->add(locatorRegistry, stringToIdentity("IceGrid/" + IceUtil::generateUUID()));
    LocatorRegistryPrx locatorRegistryPrx = LocatorRegistryPrx::uncheckedCast(obj->ice_collocationOptimization(false));
    ObjectPtr locator = new LocatorI(_database, locatorRegistryPrx); 
    const string locatorIdProperty = "IceGrid.Registry.LocatorIdentity";
    Identity locatorId = stringToIdentity(properties->getPropertyWithDefault(locatorIdProperty, "IceGrid/Locator"));
    clientAdapter->add(locator, locatorId);

    //
    // Create the query interface and register it with the object registry.
    //
    QueryPtr query = new QueryI(_communicator, _database);
    const string queryIdProperty = "IceGrid.Registry.QueryIdentity";
    Identity queryId = stringToIdentity(properties->getPropertyWithDefault(queryIdProperty, "IceGrid/Query"));
    clientAdapter->add(query, queryId);
    ObjectPrx queryPrx = clientAdapter->createDirectProxy(queryId);
    try
    {
	_database->removeObjectDescriptor(queryPrx->ice_getIdentity());
    }
    catch(const ObjectNotExistException&)
    {
    }	
    ObjectDescriptor desc;
    desc.proxy = queryPrx;
    desc.type = "::IceGrid::Query";	
    _database->addObjectDescriptor(desc);

    //
    // Create the admin interface and register it with the object registry.
    //
    ObjectPtr admin = new AdminI(_communicator, _database, this);
    const string adminIdProperty = "IceGrid.Registry.AdminIdentity";
    Identity adminId = stringToIdentity(properties->getPropertyWithDefault(adminIdProperty, "IceGrid/Admin"));
    adminAdapter->add(admin, adminId);
    ObjectPrx adminPrx = adminAdapter->createDirectProxy(adminId);
    try
    {
	_database->removeObjectDescriptor(adminPrx->ice_getIdentity());
    }
    catch(const ObjectNotExistException&)
    {
    }
    desc.proxy = adminPrx;
    desc.type = "::IceGrid::Admin";	
    _database->addObjectDescriptor(desc);

    //
    // Set the IceGrid.Registry.Internal adapter direct proxy directly in the database.
    //
    registryAdapter->add(this, stringToIdentity("IceGrid/Registry"));
    registryAdapter->activate();
    Ice::Identity dummy = Ice::stringToIdentity("dummy");
    _database->setAdapterDirectProxy("IceGrid.Registry.Internal", registryAdapter->createDirectProxy(dummy));
    
    //
    // Setup a internal locator to be used by the IceGrid registry itself. This locator is 
    // registered with the registry object adapter which is using an independant threadpool.
    //
    locator = new LocatorI(_database, locatorRegistryPrx);
    registryAdapter->add(locator, locatorId);
    obj = registryAdapter->createDirectProxy(locatorId);
    _communicator->setDefaultLocator(LocatorPrx::uncheckedCast(obj->ice_collocationOptimization(false)));

    //
    // Start the reaper thread. The default value for the node session
    // timeout is 10 seconds.
    //
    _reaper = new ReapThread(_nodeSessionTimeout);
    _reaper->start();

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
RegistryI::stop()
{
    _reaper->terminate();
    _reaper->getThreadControl().join();
}

NodeSessionPrx
RegistryI::registerNode(const std::string& name, const NodePrx& node, const Ice::Current& c)
{
    NodePrx n = NodePrx::uncheckedCast(node->ice_timeout(_nodeSessionTimeout));
    NodeSessionIPtr session = new NodeSessionI(_database, name, n);
    NodeSessionPrx proxy = NodeSessionPrx::uncheckedCast(c.adapter->addWithUUID(session));
    _reaper->add(proxy, session);
    return proxy;
}

NodeObserverPrx
RegistryI::getNodeObserver(const Ice::Current& current)
{
    return 0;
}

void
RegistryI::shutdown(const Ice::Current& current)
{
    _communicator->shutdown();
}
