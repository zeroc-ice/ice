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
#include <Ice/Functional.h>
#include <Ice/LoggerUtil.h>
#include <IcePack/ServerManagerI.h>
#include <IcePack/AdapterManager.h>
#include <IcePack/Activator.h>
#include <IcePack/ServerDeployer.h>

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

}

IcePack::ServerI::ServerI(const ObjectAdapterPtr& adapter, const ActivatorPrx& activator) :
    _adapter(adapter), 
    _activator(activator)
{
}

IcePack::ServerI::~ServerI()
{
}

ServerDescription
IcePack::ServerI::getServerDescription(const Current&)
{
    return _description;
}

bool
IcePack::ServerI::start(const Current&)
{
    while(true)
    {
	IceUtil::Monitor< IceUtil::Mutex>::Lock sync(*this);
	if(!_activator)
	    return false;

	switch(_state)
	{
	case Inactive:
	    _state = Activating;
	    break;

	case Activating:
	    wait(); // TODO: Timeout?
	    continue;

 	case Active:
	    return true; // Raise an exception instead?

	case Deactivating:
	    wait();
	    continue;

	case Destroyed:
	    throw ObjectNotExistException(__FILE__,__LINE__);
	}
	break;
    }

    try
    {
	bool activated = _activator->activate(ServerNameToServer(_adapter)(_description.name));
	setState(activated ? Active : Inactive);
	return activated;
    }
    catch (const SystemException& ex)
    {
	Warning out(_adapter->getCommunicator()->getLogger());
	out << "activation failed for server `" << _description.name << "':\n";
	out << ex;

	setState(Inactive);
	return false;
    }
}

void
IcePack::ServerI::terminationCallback(const Current&)
{
    //
    // Callback from the activator indicating that the server
    // stopped. Change state to deactivating while we mark the server
    // adapters as inactive.
    //
    setState(Deactivating);

    //
    // Mark each adapter as inactive. _adapters is immutable when
    // state == Deactivating.
    //
    for(Adapters::iterator p = _adapters.begin(); p != _adapters.end(); ++p)
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

void
IcePack::ServerI::setState(ServerState state, const Current&)
{
    IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);

    if(state == Destroyed && (_state == Active || _state == Deactivating))
	throw ServerNotInactiveException();

    _state = state;

    notifyAll();
}

IcePack::ServerManagerI::ServerManagerI(const ObjectAdapterPtr& adapter, 
					const AdapterManagerPrx& adapterManager,
					const ActivatorPrx& activator) :
    _adapter(adapter),
    _adapterManager(adapterManager),
    _activator(activator)
{
}

IcePack::ServerManagerI::~ServerManagerI()
{
}

ServerPrx
IcePack::ServerManagerI::create(const string& name, const string& path, const string& libraryPath,
				const string& descriptor, const Current&)
{
    IceUtil::Mutex::Lock sync(*this);

    ServerPrx server = ServerNameToServer(_adapter)(name);
    try
    {
	server->ice_ping();
	throw ServerExistsException();
    }
    catch (const ObjectNotExistException&)
    {
    }

    //
    // Creates the server. Set its state to Activating so that we can
    // safelly register the adapters without any race conditions. If a
    // request comes in for an adapter we've just registerd, the
    // server won't be started as long as we are not in the Inactive
    // state.
    //
    // TODO: the server isn't fully initialized here. Is this really
    // valid to add the servant to the object adapter? If not, how do
    // we handle the race condition because of registered adapters
    // having a proxy on the server?
    //
    ServerPtr serverI = new ServerI(_adapter, _activator);
    serverI->_description.name = name;
    serverI->_description.path = path;
    serverI->_description.libraryPath = libraryPath;
    serverI->_description.descriptor = descriptor;
    serverI->_state = Activating;
    server = ServerPrx::uncheckedCast(_adapter->add(serverI, server->ice_getIdentity()));

    //
    // Deploy the server.
    //
    try
    {
	ServerDeployer deployer(_adapter->getCommunicator(), serverI, server);
	deployer.setAdapterManager(_adapterManager);

	deployer.parse();
	deployer.deploy();
    }
    catch(const DeploymentException&)
    {
	_adapter->remove(server->ice_getIdentity());
	serverI->setState(Destroyed);
	throw;
    }

    serverI->setState(Inactive);

    _serverNames.insert(name);

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
    // Mark the server as destroyed.
    //
    ServerPtr serverI = ServerPtr::dynamicCast(_adapter->proxyToServant(server).get());
    assert(serverI);
    serverI->setState(Destroyed);

    //
    // Undeploy the server.
    //
    try
    {
	ServerDeployer deployer(_adapter->getCommunicator(), serverI, server);
	deployer.setAdapterManager(_adapterManager);

	deployer.parse();
	deployer.undeploy();
    }
    catch(const DeploymentException& ex)
    {
    }

    _adapter->remove(server->ice_getIdentity());

    //
    // Remove the server name from our internal server name set.
    //
    _serverNames.erase(_serverNames.find(name));
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

