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
#include <Freeze/DB.h>
#include <Freeze/Evictor.h>
#include <Ice/Functional.h>
#include <Ice/LoggerUtil.h>
#include <IcePack/ServerManagerI.h>
#include <IcePack/AdapterManager.h>
#include <IcePack/Activator.h>
#include <IcePack/ServerDeployer.h>
#include <IcePack/TraceLevels.h>
#include <IceBox/IceBox.h>

using namespace std;
using namespace Ice;
using namespace IcePack;

namespace IcePack
{

class ServerNameToServer
{
public:

    ServerNameToServer(const ObjectAdapterPtr& adapter) :
	_adapter(adapter)
    {
    }

    ServerPrx
    operator()(const string& name)
    {
	Identity ident;
	ident.category = "server";
	ident.name = name;
	return ServerPrx::uncheckedCast(_adapter->createProxy(ident));
    }

private:

    ObjectAdapterPtr _adapter;
};

class ServerFactory : public ObjectFactory
{
public:

    ServerFactory(const ObjectAdapterPtr& adapter, const TraceLevelsPtr& traceLevels, const ActivatorPtr& activator) :
	_adapter(adapter),
	_traceLevels(traceLevels),
	_activator(activator)
    {
    }

    virtual Ice::ObjectPtr 
    create(const std::string& type)
    {
	assert(type == "::IcePack::Server");
	return new ServerI(_adapter, _traceLevels, _activator);
    }

    virtual void 
    destroy()
    {
	_adapter = 0;
	_activator = 0;
    }

private:
    
    ObjectAdapterPtr _adapter;
    TraceLevelsPtr _traceLevels;
    ActivatorPtr _activator;
};

}

IcePack::ServerI::ServerI(const ObjectAdapterPtr& adapter, const TraceLevelsPtr& traceLevels, 
			  const ActivatorPtr& activator) :
    _adapter(adapter), 
    _traceLevels(traceLevels),
    _activator(activator),
    _state(Inactive),
    _pid(0)
{
    assert(_activator);
}

IcePack::ServerI::~ServerI()
{
}

ServerDescription
IcePack::ServerI::getServerDescription(const Current&)
{
    return description;
}

bool
IcePack::ServerI::start(const Current&)
{
    while(true)
    {
	IceUtil::Monitor< IceUtil::Mutex>::Lock sync(*this);
	switch(_state)
	{
	case Inactive:
	{
	    _state = Activating;
	    break;
	}
	case Activating:
	{
	    wait(); // TODO: Timeout?
	    continue;
	}
 	case Active:
	case Deactivating:
	{
	    return true; // Raise an exception instead?
	}
	case Destroyed:
	{
	    throw ObjectNotExistException(__FILE__,__LINE__);
	}
	}

	if(_traceLevels->serverMgr > 2)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->serverMgrCat);
	    out << "changed server `" << description.name << "' state to `Activating'";
	}
	assert(_state == Activating);
	break;
    }

    try
    {
	int pid  = _activator->activate(ServerNameToServer(_adapter)(description.name));
	setState((pid != 0) ? Active : Inactive);
	setPid(pid);
	return pid != 0;
    }
    catch (const SystemException& ex)
    {
	Warning out(_adapter->getCommunicator()->getLogger());
	out << "activation failed for server `" << description.name << "':\n";
	out << ex;

	setState(Inactive);
	return false;
    }
}

void
IcePack::ServerI::stop(const Current&)
{
    while(true)
    {
	IceUtil::Monitor< IceUtil::Mutex>::Lock sync(*this);
	switch(_state)
	{
	case Inactive:
	{
	    return;
	}
	case Activating:
	{
	    wait(); // TODO: Timeout?
	    continue;
	}
 	case Active:
	{	    
	    _state = Deactivating;
	    break;
	}
	case Deactivating:
	{
	    wait();
	    continue;
	}
	case Destroyed:
	{
	    throw ObjectNotExistException(__FILE__,__LINE__);
	}
	}

	if(_traceLevels->serverMgr > 2)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->serverMgrCat);
	    out << "changed server `" << description.name << "' state to `Deactivating'";
	}
	assert(_state == Deactivating);
	break;
    }

    Ice::PropertiesPtr properties = _adapter->getCommunicator()->getProperties();
    Int waitTime = properties->getPropertyAsIntWithDefault("IcePack.Activation.WaitTime", 60);

    //
    // If the server is an icebox, first try to use the IceBox service
    // manager to shutdown the server.
    //
    bool deactivate = true;

    if(description.isIceBox)
    {
	try
	{
	    Ice::ObjectPrx object = _adapter->getCommunicator()->stringToProxy(
		description.name + ".ServiceManager@" + description.name + ".ServiceManagerAdapter");

	    if(object)
	    {
		IceBox::ServiceManagerPrx serviceManager = 
		    IceBox::ServiceManagerPrx::uncheckedCast(object->ice_timeout(waitTime));

		if(serviceManager)
		{
		    serviceManager->shutdown();
		    
		    //
		    // No need to deactivate the process by sending a signal
		    // since we successfully called shutdown on the service
		    // manager.
		    //
		    deactivate = false;
		}
	    }
	}
	catch(const Ice::LocalException& ex)
	{
	    if(_traceLevels->serverMgr > 1)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->serverMgrCat);
		out << "couldn't contact the IceBox `" << description.name << "' service manager:\n";
		out << ex;
	    }
	}

    }

    if(deactivate)
    {
	//
	// Deactivate the server by sending a SIGTERM.
	//
	try
	{
	    _activator->deactivate(ServerNameToServer(_adapter)(description.name));
	}
	catch (const SystemException& ex)
	{
	    Warning out(_adapter->getCommunicator()->getLogger());
	    out << "deactivation failed for server `" << description.name << "':\n";
	    out << ex;
	    
	    setState(Active);
	    return;
	}
    }

    //
    // Wait for the server to be inactive (the activator monitors the
    // process and should notify us when it detects the process
    // termination by calling the terminationCallback() method).
    //
    {
	IceUtil::Monitor< IceUtil::Mutex>::Lock sync(*this);
	while(true)
	{
	    if(_state == Inactive)
	    {
		//
		// State changed to inactive, the server has been
		// correctly deactivated, we can return.
		//
		return;
	    }
	    
	    //
	    // Wait for a state change.
	    //
	    bool notify = timedWait(IceUtil::Time::seconds(waitTime));
	    if(!notify)
	    {
		//
		// Timeout.
		//
		assert(_state == Deactivating);
		break;
	    }
	}
    }
    
    if(_traceLevels->serverMgr > 1)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->serverMgrCat);
	out << "gracefull server shutdown failed, killing server `" << description.name << "'";
    }
    
    //
    // The server is still not inactive, kill it.
    //
    try
    {
	_activator->kill(ServerNameToServer(_adapter)(description.name));
    }
    catch (const SystemException& ex)
    {
	Warning out(_adapter->getCommunicator()->getLogger());
	out << "deactivation failed for server `" << description.name << "':\n";
	out << ex;
	
	setState(Active);
    }
}

void
IcePack::ServerI::terminationCallback(const Current&)
{
    while(true)
    {
	IceUtil::Monitor< IceUtil::Mutex>::Lock sync(*this);
	switch(_state)
	{
	case Inactive:
	case Activating:
	case Destroyed:
	{
	    assert(false);
	}
 	case Active:
	{
	    _state = Deactivating;

	    if(_traceLevels->serverMgr > 2)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->serverMgrCat);
		out << "changed server `" << description.name << "' state to `Deactivating'";
	    }
	    break;
	}
	case Deactivating:
	{
	    // Deactivation was initiated by stop().
	    break;
	}
	}
	assert(_state == Deactivating);
	break;
    }

    //
    // Mark each adapter as inactive. adapters is immutable when
    // state == Deactivating.
    //
    for(Adapters::iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
	(*p)->markAsInactive();
    }

    setState(Inactive);
}

ServerState
IcePack::ServerI::getState(const Current&)
{
    IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    return _state;
}

Ice::Int
IcePack::ServerI::getPid(const Current&)
{
    IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    return _pid;
}

void
IcePack::ServerI::setState(ServerState state)
{
    IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);

    if(state == Destroyed && (_state == Active || _state == Deactivating))
    {
	throw ServerNotInactiveException();
    }

    _state = state;

    if(_traceLevels->serverMgr > 1)
    {
	if(_state == Active)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->serverMgrCat);
	    out << "changed server `" << description.name << "' state to `Active'";
	}
	else if(_state == Inactive)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->serverMgrCat);
	    out << "changed server `" << description.name << "' state to `Inactive'";
	}
	else if(_traceLevels->serverMgr > 2)
	{
	    if(_state == Activating)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->serverMgrCat);
		out << "changed server `" << description.name << "' state to `Activating'";
	    }
	    else if(_state == Deactivating)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->serverMgrCat);
		out << "changed server `" << description.name << "' state to `Deactivating'";
	    }
	}
    }    

    notifyAll();
}

void
IcePack::ServerI::setPid(int pid)
{
    IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    _pid = pid;
}

IcePack::ServerManagerI::ServerManagerI(const ObjectAdapterPtr& adapter,
					const TraceLevelsPtr& traceLevels,
					const Freeze::DBEnvironmentPtr& dbEnv,
					const AdapterManagerPrx& adapterManager,
					const ActivatorPtr& activator) :
    _adapter(adapter),
    _traceLevels(traceLevels),
    _adapterManager(adapterManager),
    _activator(activator)
{
    ObjectFactoryPtr serverFactory = new ServerFactory(adapter, _traceLevels, activator);
    adapter->getCommunicator()->addObjectFactory(serverFactory, "::IcePack::Server");

    Freeze::DBPtr dbServers = dbEnv->openDB("servers", true);
    _evictor = dbServers->createEvictor(Freeze::SaveUponEviction);
    _evictor->setSize(1000);
    _adapter->addServantLocator(_evictor, "server");

    //
    // Cache the server names for getAll(). This will load all the
    // server objects at the begining and might cause slow startup.
    //
    Freeze::EvictorIteratorPtr p = _evictor->getIterator();
    while(p->hasNext())
    {
	ServerPrx s = ServerPrx::checkedCast(_adapter->createProxy(p->next()));
	assert(s);
	ServerDescription desc = s->getServerDescription();
	_serverNames.insert(desc.name);
    }
}

IcePack::ServerManagerI::~ServerManagerI()
{
}

ServerPrx
IcePack::ServerManagerI::create(const ServerDescription& desc, const Current&)
{
    IceUtil::Mutex::Lock sync(*this);

    ServerPrx server = ServerNameToServer(_adapter)(desc.name);
    try
    {
	server->ice_ping();
	throw ServerExistsException();
    }
    catch (const ObjectNotExistException&)
    {
    }
    
    ServerPtr serverI = new ServerI(_adapter, _traceLevels, _activator);
    serverI->description = desc;
    for(AdapterNames::const_iterator p = desc.adapters.begin(); p != desc.adapters.end(); ++p)
    {
	AdapterPrx adapter = _adapterManager->findByName(*p);
	if(adapter)
	{
	    serverI->adapters.push_back(adapter);
	}
    }

    _evictor->createObject(server->ice_getIdentity(), serverI);

    _serverNames.insert(desc.name);

    if(_traceLevels->serverMgr > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->serverMgrCat);
	out << "added server `" << desc.name << "'";
    }

    return server;
}

ServerPrx
IcePack::ServerManagerI::findByName(const string& name, const Current&)
{
    IceUtil::Mutex::Lock sync(*this);

    ServerPrx server = ServerNameToServer(_adapter)(name);
    try
    {
	server->ice_ping();
	return server;
    }
    catch(const ObjectNotExistException&)
    {
	return 0;
    }
}

void
IcePack::ServerManagerI::remove(const string& name, const Current&)
{
    IceUtil::Mutex::Lock sync(*this);

    ServerPrx server = ServerNameToServer(_adapter)(name);
    try
    {
	server->ice_ping();
    }
    catch(const ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }

    //
    // Stop the server before removing it.
    //
    server->stop();

    _evictor->destroyObject(server->ice_getIdentity());

    _serverNames.erase(_serverNames.find(name));

    if(_traceLevels->serverMgr > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->serverMgrCat);
	out << "removed server `" << name << "'";
    }
}

ServerNames
IcePack::ServerManagerI::getAll(const Current&)
{
    IceUtil::Mutex::Lock sync(*this);

    ServerNames names;
    names.reserve(_serverNames.size());
    copy(_serverNames.begin(), _serverNames.end(), back_inserter(names));

    return names;
}
