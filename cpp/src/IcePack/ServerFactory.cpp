// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef __sun
#define _POSIX_PTHREAD_SEMANTICS
#endif

#include <Ice/Ice.h>
#include <IceUtil/UUID.h>
#include <Freeze/Evictor.h>
#include <Freeze/Initialize.h>
#include <IcePack/ServerFactory.h>
#include <IcePack/ServerI.h>
#include <IcePack/ServerAdapterI.h>
#include <IcePack/TraceLevels.h>
#include <IcePack/DescriptorVisitor.h>

#include <map>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#   include <direct.h>
#else
#   include <unistd.h>
#   include <dirent.h>
#endif

using namespace std;
using namespace IcePack;

namespace IcePack
{

class NodeServerCleaner : public DescriptorVisitor
{
public:

    void clean(const ServerPrx&, const ServerDescriptorPtr&);

private:
    
    virtual void visitServerEnd(const ServerWrapper&, const ServerDescriptorPtr&);
    virtual bool visitServiceStart(const ServiceWrapper&, const ServiceDescriptorPtr&);
    virtual void visitDbEnv(const DbEnvWrapper&, const DbEnvDescriptor&);

    ServerPrx _currentServer;
};

};

void
NodeServerCleaner::clean(const ServerPrx& server, const ServerDescriptorPtr& descriptor)
{
    _currentServer = server;
    ServerWrapper(descriptor).visit(*this);
}

void
NodeServerCleaner::visitServerEnd(const ServerWrapper&, const ServerDescriptorPtr& server)
{
    _currentServer->removeConfigFile("config");
    _currentServer->destroy();
}

bool
NodeServerCleaner::visitServiceStart(const ServiceWrapper&, const ServiceDescriptorPtr& service)
{
    _currentServer->removeConfigFile("config_" + service->name);
    return true;
}

void
NodeServerCleaner::visitDbEnv(const DbEnvWrapper&, const DbEnvDescriptor& dbEnv)
{
    _currentServer->removeDbEnv(dbEnv, "");
}

ServerFactory::ServerFactory(const Ice::ObjectAdapterPtr& adapter, 
			     const TraceLevelsPtr& traceLevels, 
			     const string& envName,
			     const ActivatorPtr& activator,
			     const WaitQueuePtr& waitQueue) :
    _adapter(adapter),
    _traceLevels(traceLevels),
    _activator(activator),
    _waitQueue(waitQueue)
{
    Ice::PropertiesPtr properties = _adapter->getCommunicator()->getProperties();
    _waitTime = properties->getPropertyAsIntWithDefault("IcePack.Node.WaitTime", 60);
    
    _serversDir = properties->getProperty("IcePack.Node.Data");
    _serversDir = _serversDir + (_serversDir[_serversDir.length() - 1] == '/' ? "" : "/") + "servers/";

    //
    // Create and install the freeze evictor for server objects.
    //
    properties->setProperty("Freeze.Evictor." + envName + ".servers.SaveSizeTrigger", "1");
    _serverEvictor = Freeze::createEvictor(_adapter, envName, "servers", 0);
    _serverEvictor->setSize(10000);

    //
    // Create and install the freeze evictor for server adapter objects.
    //
    properties->setProperty("Freeze.Evictor." + envName + ".serveradapters.SaveSizeTrigger", "1");
    _serverAdapterEvictor = Freeze::createEvictor(_adapter, envName, "serveradapters", 0);
    _serverAdapterEvictor->setSize(10000);

    //
    // Install the server object factory.
    // 
    _adapter->getCommunicator()->addObjectFactory(this, "::IcePack::Server");
    _adapter->getCommunicator()->addObjectFactory(this, "::IcePack::ServerAdapter");

    //
    // Install the evictors.
    //
    _adapter->addServantLocator(_serverEvictor, "IcePackServer");
    _adapter->addServantLocator(_serverAdapterEvictor, "IcePackServerAdapter");
}

//
// Ice::ObjectFactory::create method implementation
//
Ice::ObjectPtr
ServerFactory::create(const string& type)
{
    if(type == "::IcePack::Server")
    {
	return new ServerI(this, _traceLevels, _activator, _waitTime, _serversDir);
    }
    else if(type == "::IcePack::ServerAdapter")
    {
	return new ServerAdapterI(this, _traceLevels, _waitTime);
    }
    else
    {
	assert(false);
	return 0; // Keep the compiler happy.
    }
}

//
// Ice::ObjectFactory::destroy method implementation
//
void 
ServerFactory::destroy()
{
    _adapter = 0;
    _serverEvictor = 0;
    _serverAdapterEvictor = 0;
    _traceLevels = 0;
    _activator = 0;
}

void
ServerFactory::checkConsistency()
{
    try
    {
	Ice::CommunicatorPtr communicator = _adapter->getCommunicator();

	//
	// Make sure that all the servers in this node server database are registered with the 
	// IcePack server registry. If a server isn't registered with the registry, we remove
	// it from the node and also delete any resources associated with it (config files,
	// db envs, ...).
	//
	ServerRegistryPrx serverRegistry = ServerRegistryPrx::checkedCast(
            communicator->stringToProxy("IcePack/ServerRegistry@IcePack.Registry.Internal"));
	
	Freeze::EvictorIteratorPtr p = _serverEvictor->getIterator("", 50);
	while(p->hasNext())
	{
	    ServerPrx server = ServerPrx::uncheckedCast(_adapter->createProxy(p->next()));
	    ServerDescriptorPtr descriptor = server->getDescriptor();
	    try
	    {
		if(Ice::proxyIdentityEqual(serverRegistry->findByName(descriptor->name), server))
		{
		    continue;
		}
	    }
	    catch(const ServerNotExistException&)
	    {
	    }

	    NodeServerCleaner().clean(server, descriptor);
	}
	
	//
	// Make sure all the adapters in this node adapter database are registered with the 
	// IcePack adapter registry. If an adapter isn't registered with the registry, we 
	// remove it from this node.
	//
	AdapterRegistryPrx adapterRegistry = AdapterRegistryPrx::checkedCast(
            communicator->stringToProxy("IcePack/AdapterRegistry@IcePack.Registry.Internal"));
	
	p = _serverAdapterEvictor->getIterator("", 50);
	while(p->hasNext())
	{
	    ServerAdapterPrx adapter = ServerAdapterPrx::uncheckedCast(_adapter->createProxy(p->next()));
	    try
	    {
		if(Ice::proxyIdentityEqual(adapterRegistry->findById(adapter->getId()), adapter))
		{
		    continue;
		}
	    }
	    catch(const AdapterNotExistException&)
	    {
	    }

	    adapter->destroy();
	}
    }
    catch(const Ice::LocalException& ex)
    {
	ostringstream os;
	os << "couldn't contact the IcePack registry for the consistency check:\n" << ex;
	_traceLevels->logger->warning(os.str());
	return;
    }

}

//
// Create a new server servant and new server adapter servants from
// the given description.
//
ServerPrx
ServerFactory::createServer(const string& name, const ServerDescriptorPtr& desc)
{
    //
    // Create the server object.
    //
    ServerPtr serverI = new ServerI(this, _traceLevels, _activator, _waitTime, _serversDir);
    
    serverI->name = name;
    serverI->activation = Manual;
    serverI->processRegistered = false;
    serverI->descriptor = desc;

    string path = _serversDir + name;
#ifdef _WIN32
    if(_mkdir(path.c_str()) != 0)
#else
    if(mkdir(path.c_str(), 0755) != 0)
#endif
    {
	DeploymentException ex;
	ex.reason = "couldn't create directory " + path + ": " + strerror(getSystemErrno());
	throw ex;
    }

#ifdef _WIN32
    _mkdir(string(path + "/config").c_str());
    _mkdir(string(path + "/dbs").c_str());
#else
    mkdir(string(path + "/config").c_str(), 0755);
    mkdir(string(path + "/dbs").c_str(), 0755);
#endif
    
    Ice::Identity id;
    id.category = "IcePackServer";
    id.name = name + "-" + IceUtil::generateUUID();
    
    _serverEvictor->add(serverI, id);

    if(_traceLevels->server > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
	out << "created server `" << name << "'";
    }
    
    return ServerPrx::uncheckedCast(_adapter->createProxy(id));
}

//
// Create a new server adapter servant with the given name and server
// and add it the evictor database.
//
ServerAdapterPrx
ServerFactory::createServerAdapter(const string& adapterId, const ServerPrx& server)
{
    ServerAdapterPtr adapterI = new ServerAdapterI(this, _traceLevels, _waitTime);
    adapterI->id = adapterId;
    adapterI->svr = server;

    Ice::Identity id;
    id.category = "IcePackServerAdapter";
    id.name = adapterId + "-" + IceUtil::generateUUID();
    
    _serverAdapterEvictor->add(adapterI, id);

    if(_traceLevels->adapter > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
	out << "created server adapter `" << adapterId << "'";
    }
    
    return ServerAdapterPrx::uncheckedCast(_adapter->createProxy(id));
}

const WaitQueuePtr&
ServerFactory::getWaitQueue() const
{
    return _waitQueue;
}

const Freeze::EvictorPtr&
ServerFactory::getServerEvictor() const
{
    return _serverEvictor;
}

const Freeze::EvictorPtr&
ServerFactory::getServerAdapterEvictor() const
{
    return _serverAdapterEvictor;
}

void
ServerFactory::destroy(const ServerPtr& server, const Ice::Identity& ident)
{
    try
    {
	_serverEvictor->remove(ident);

	if(_traceLevels->server > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
	    out << "destroyed server `" << server->name << "'";
	}
    }
    catch(const Freeze::DatabaseException& ex)
    {
	ostringstream os;
	os << "couldn't destroy server `" << server->name << "':\n" << ex;
	_traceLevels->logger->warning(os.str());
    }
    catch(const Freeze::EvictorDeactivatedException&)
    {
	assert(false);
    }

    string path = _serversDir + server->name;
    rmdir(string(path + "/config").c_str());
    rmdir(string(path + "/dbs").c_str());
    rmdir(path.c_str());
}

void
ServerFactory::destroy(const ServerAdapterPtr& adapter, const Ice::Identity& ident)
{
    try
    {
	_serverAdapterEvictor->remove(ident);

	if(_traceLevels->adapter > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
	    out << "destroyed server adapter `" << adapter->id << "'";
	}
    }
    catch(const Freeze::DatabaseException& ex)
    {
	ostringstream os;
	os << "couldn't destroy server adapter `" << adapter->id << "':\n" << ex;
	_traceLevels->logger->warning(os.str());
    }
    catch(const Freeze::EvictorDeactivatedException&)
    {
	assert(false);
    }
}

