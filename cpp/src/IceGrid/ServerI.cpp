// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef __sun
#define _POSIX_PTHREAD_SEMANTICS
#endif

#include <Ice/Ice.h>
#include <IceGrid/ServerI.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/Activator.h>
#include <IceGrid/NodeI.h>
#include <IceGrid/Util.h>
#include <IceGrid/ServerAdapterI.h>
#include <IceGrid/WaitQueue.h>

#include <IcePatch2/Util.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#   include <direct.h>
#else
#   include <unistd.h>
#   include <dirent.h>
#endif

#include <fstream>

using namespace std;
using namespace IceGrid;
using namespace IcePatch2;

namespace IceGrid
{

class WaitForActivationItem : public WaitItem
{
public:
 
    WaitForActivationItem(const ServerIPtr& server) : 
	WaitItem(server),
	_server(server)
    {
    }
    
    virtual void execute()
    {
    }

    virtual void expired(bool destroyed)
    {
	_server->activationFailed(!destroyed);
    }

private:
    
    const ServerIPtr _server;
};

}

ServerI::ServerI(const NodeIPtr& node, const ServerPrx& proxy, const string& serversDir, const string& id, int wt) :
    _node(node),
    _this(proxy),
    _id(id),
    _waitTime(wt),
    _serversDir(serversDir),
    _state(ServerI::Inactive)
{
    assert(_node->getActivator());
}

ServerI::~ServerI()
{
}

void
ServerI::load(const ServerDescriptorPtr& descriptor, StringAdapterPrxDict& adapters, int& activationTimeout,
	      int& deactivationTimeout, const Ice::Current& current)
{
    startUpdating();

    //
    // If the server is inactive we can update its descriptor and its directory.
    //
    try
    {
	update(descriptor, adapters, activationTimeout, deactivationTimeout, current);
    }
    catch(const string& msg)
    {
	finishUpdating();
	DeploymentException ex;
	ex.reason = msg;
	throw ex;
    }
    catch(...)
    {
	finishUpdating();
	throw;
    }

    finishUpdating();
}

void
ServerI::start_async(const AMD_Server_startPtr& amdCB, const Ice::Current& current)
{
    startInternal(Manual, amdCB);
}

void
ServerI::stop(const Ice::Current&)
{
    while(true)
    {
	Lock sync(*this);
	switch(_state)
	{
	case ServerI::Inactive:
	case ServerI::Updating:
	{
	    return;
	}
	case ServerI::Activating:
	case ServerI::Deactivating:
	{
	    wait(); // TODO: Timeout?
	    continue;
	}
	case ServerI::WaitForActivation:
 	case ServerI::Active:
	{	    
	    setStateNoSync(ServerI::Deactivating);
	    break;
	}
	case ServerI::Destroying:
	case ServerI::Destroyed:
	{
	    Ice::ObjectNotExistException ex(__FILE__,__LINE__);
	    ex.id = _this->ice_getIdentity();
	    throw ex;
	}
	}

	assert(_state == ServerI::Deactivating);
	break;
    }

    stopInternal(false);
}

void
ServerI::patch(const ::Ice::Current& current)
{
    //
    // Patch the server data.
    //
    startUpdating();
    try
    {
	ServerDescriptorPtr desc = getDescriptor(current);
	for(PatchDescriptorSeq::const_iterator p = desc->patchs.begin(); p != desc->patchs.end(); ++p)
	{
	    _node->patch(this, p->destination);
	}
    }
    catch(...)
    {
	finishUpdating();
	throw;
    }
    finishUpdating();
}

void
ServerI::sendSignal(const string& signal, const Ice::Current& current)
{
    _node->getActivator()->sendSignal(_id, signal);
}

void
ServerI::writeMessage(const string& message, Ice::Int fd, const Ice::Current& current)
{
    IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    if(_process != 0)
    {
	try
	{
	    _process->writeMessage(message, fd);
	}
	catch(const Ice::LocalException&)
	{
	}
    }
}

void
ServerI::destroy(const Ice::Current& current)
{
    bool stop = false;

    while(true)
    {
	Lock sync(*this);
	switch(_state)
	{
	case ServerI::Inactive:
	{
	    setStateNoSync(ServerI::Destroyed);
	    break;
	}
	case ServerI::WaitForActivation:
 	case ServerI::Active:
	{
	    stop = true;
	    setStateNoSync(ServerI::Destroying);
	    break;
	}
	case ServerI::Activating:
	case ServerI::Deactivating:
	case ServerI::Updating:
	{
	    wait(); // TODO: Timeout?
	    continue;
	}
	case ServerI::Destroying:
	case ServerI::Destroyed:
	{
	    Ice::ObjectNotExistException ex(__FILE__,__LINE__);
	    ex.id = _this->ice_getIdentity();
	    throw ex;
	}
	}

	assert(_state == ServerI::Destroyed || _state == ServerI::Destroying);
	break;
    }

    if(stop)
    {
	stopInternal(true);
    }

    //
    // Destroy the object adapters.
    //
    for(map<string,  ServerAdapterIPtr>::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
    {
	try
	{
	    p->second->destroy(current);
	}
	catch(const Ice::LocalException&)
	{
	}
    }

    //
    // Delete the server directory from the disk.
    //
    try
    {
	removeRecursive(_serverDir);
    }
    catch(const string& msg)
    {
	Ice::Warning out(_node->getTraceLevels()->logger);
	out << "couldn't remove directory `" + _serverDir + "':\n" + msg;
    }
    
    //
    // Unregister from the object adapter.
    //
    current.adapter->remove(current.id);
}

void
ServerI::terminated(const Ice::Current& current)
{
    InternalServerState newState = ServerI::Inactive; // Initialize to keep the compiler happy.
    map<string, ServerAdapterIPtr> adpts;
    while(true)
    {
	Lock sync(*this);
	switch(_state)
	{
	case ServerI::Inactive:
	case ServerI::Updating:
	{
	    assert(false);
	}
	case ServerI::Activating:
	{
	    wait(); // TODO: Timeout?
	    continue;
	}
	case ServerI::WaitForActivation:
	case ServerI::Active:
	{
	    setStateNoSync(ServerI::Deactivating);
	    newState = ServerI::Inactive;
	    break;
	}
	case ServerI::Deactivating:
	{
	    //
	    // Deactivation was initiated by the stop method.
	    //
	    newState = ServerI::Inactive;
	    break;
	}
	case ServerI::Destroying:
	{
	    //
	    // Deactivation was initiated by the destroy method.
	    //
	    newState = ServerI::Destroyed;
	    break;
	}
	case ServerI::Destroyed:
	{
	    assert(false);
	}
	}

	assert(_state == ServerI::Deactivating || _state == ServerI::Destroying);
	adpts = _adapters;

	//
	// Clear the process proxy.
	//
	_process = 0;
	break;
    }

    if(newState != ServerI::Destroyed)
    {
	//
	// The server has terminated, set its adapter direct proxies to
	// null to cause the server re-activation if one of its adapter
	// direct proxy is requested.
	//
	for(map<string, ServerAdapterIPtr>::iterator p = adpts.begin(); p != adpts.end(); ++p)
	{
	    try
	    {
		p->second->setDirectProxy(0, current);
	    }
	    catch(const Ice::ObjectNotExistException&)
	    {
	    }
	}
    }

    setState(newState);
}

ServerState
ServerI::getState(const Ice::Current&) const
{
    Lock sync(*this);
    return toServerState(_state);
}

Ice::Int
ServerI::getPid(const Ice::Current&) const
{
    return _node->getActivator()->getServerPid(_id);
}

void 
ServerI::setActivationMode(ServerActivation mode, const ::Ice::Current&)
{
    Lock sync(*this);
    _activation = mode;
}

ServerActivation 
ServerI::getActivationMode(const ::Ice::Current&) const
{
    Lock sync(*this);
    return _activation;
}

ServerDescriptorPtr
ServerI::getDescriptor(const Ice::Current&) const
{
    Lock sync(*this);
    return _desc;
}

void
ServerI::setProcess(const ::Ice::ProcessPrx& proc, const ::Ice::Current&)
{
    IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
    _process = proc;
    notifyAll();
}

bool
ServerI::startInternal(ServerActivation act, const AMD_Server_startPtr& amdCB)
{
    ServerDescriptorPtr desc;
    while(true)
    {
	Lock sync(*this);
	switch(_state)
	{
	case ServerI::Inactive:
	{
	    if(act < _activation)
	    {
		if(amdCB)
		{
		    amdCB->ice_response(false);
		}
		return false;
	    }

	    setStateNoSync(ServerI::Activating);
	    break;
	}
	case ServerI::Activating:
	case ServerI::Deactivating:
	case ServerI::Updating:
	{
	    wait(); // TODO: Timeout?
	    continue;
	}
	case ServerI::WaitForActivation:
	{
	    if(amdCB)
	    {
		_startCB.push_back(amdCB);
	    }
	    return true;
	}
 	case ServerI::Active:
	{
	    if(amdCB)
	    {
		amdCB->ice_response(true);
	    }
	    return true;
	}
	case ServerI::Destroying:
	case ServerI::Destroyed:
	{
	    Ice::ObjectNotExistException ex(__FILE__,__LINE__);
	    ex.id = _this->ice_getIdentity();
	    throw ex;
	}
	}

	assert(_state == ServerI::Activating);

	desc = _desc;
	if(amdCB)
	{
	    _startCB.push_back(amdCB);
	}
	break;
    }

    //
    // Compute the server command line options.
    //
    Ice::StringSeq options;
    copy(desc->options.begin(), desc->options.end(), back_inserter(options));
    options.push_back("--Ice.Config=" + _serverDir + "/config/config");

    Ice::StringSeq envs;
    copy(desc->envs.begin(), desc->envs.end(), back_inserter(envs));

    try
    {
	bool started  = _node->getActivator()->activate(desc->id, desc->exe, desc->pwd, options, envs, _this);
	if(!started)
	{
	    setState(ServerI::Inactive);
	    return false;
	}
	else
	{
	    Lock sync(*this);
	    _node->getWaitQueue()->add(new WaitForActivationItem(this), IceUtil::Time::seconds(_activationTimeout));
	    setStateNoSync(ServerI::WaitForActivation);
	    checkActivation();
	    notifyAll();
	    return true;
	}
    }
    catch(const Ice::SyscallException& ex)
    {
	Ice::Warning out(_node->getTraceLevels()->logger);
	out << "activation failed for server `" << _id << "':\n";
	out << ex;

	setState(ServerI::Inactive);
	return false;
    }
}

void
ServerI::adapterActivated(const string& id)
{
    Lock sync(*this);
    _activeAdapters.insert(id);
    checkActivation();
}

void
ServerI::adapterDeactivated(const string& id)
{
    Lock sync(*this);
    _activeAdapters.erase(id);
}

void
ServerI::activationFailed(bool timeout)
{
    map<string, ServerAdapterIPtr> adapters;
    {
	Lock sync(*this);
	if(_state != ServerI::WaitForActivation)
	{
	    return;
	}

	for(vector<AMD_Server_startPtr>::const_iterator p = _startCB.begin(); p != _startCB.end(); ++p)
	{
	    (*p)->ice_response(false);
	}
	_startCB.clear();
	
	if(_node->getTraceLevels()->server > 1)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
	    if(timeout)
	    {
		out << "server `" << _id << "' activation timed out";
	    }	
	    else
	    {
		out << "server `" << _id << "' activation failed";
	    }
	}
	adapters = _adapters;
    }

    for(map<string, ServerAdapterIPtr>::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
	try
	{
	    p->second->activationFailed(timeout);
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
    }
}

void
ServerI::addDynamicInfo(ServerDynamicInfoSeq& serverInfos, AdapterDynamicInfoSeq& adapterInfos) const
{
    //
    // Add server info if it's not inactive.
    //
    ServerDynamicInfo info;
    map<string, ServerAdapterIPtr> adapters;
    {
	Lock sync(*this);
	if(_state == ServerI::Inactive)
	{
	    return;
	}
	adapters = _adapters;
	info.state = toServerState(_state);
    }
    info.id = _id;
    info.pid = info.state == IceGrid::Active ? getPid() : 0;
    serverInfos.push_back(info);

    //
    // Add adapters info.
    //
    for(map<string, ServerAdapterIPtr>::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
	try
	{
	    AdapterDynamicInfo adapter;
	    adapter.id = p->first;
	    adapter.proxy = p->second->getDirectProxy();
	    adapterInfos.push_back(adapter);
	}
	catch(const AdapterNotActiveException&)
	{
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
    }
}

void
ServerI::startUpdating()
{
    while(true)
    {
	{
	    Lock sync(*this);
	    if(_state == ServerI::Destroying || _state == ServerI::Destroyed)
	    {
		Ice::ObjectNotExistException ex(__FILE__,__LINE__);
		ex.id = _this->ice_getIdentity();
		throw ex;
	    }
	    else if(_state == ServerI::Updating)
	    {
		wait(); // Only one update at a time!
	    }
	    else if(_state == ServerI::Inactive)
	    {
		_state = ServerI::Updating;
		return;
	    }
	}

	//
	// If the server is active we stop it and try again to update it.
	//
	stop();
    }
}

void
ServerI::finishUpdating()
{
    Lock sync(*this);
    assert(_state == ServerI::Updating);
    _state = ServerI::Inactive;
    notifyAll();
}

void
ServerI::checkActivation()
{
    //assert(locked());
    if(_state == ServerI::WaitForActivation)
    {
	for(AdapterDescriptorSeq::const_iterator p = _desc->adapters.begin(); p != _desc->adapters.end(); ++p)
	{
	    if(p->waitForActivation && _activeAdapters.find(p->id) == _activeAdapters.end())
	    {
		return;
	    }
	}

	setStateNoSync(ServerI::Active);
	notifyAll();
    }
}

void
ServerI::stopInternal(bool kill)
{
    Ice::ProcessPrx process;
    if(!kill)
    {
	IceUtil::Monitor< ::IceUtil::Mutex>::Lock sync(*this);
	if(!_process && _processRegistered)
	{
	    while(!_process)
	    {
		if(_state == ServerI::Inactive || _state == ServerI::Destroyed)
		{
		    //
		    // State changed to inactive or destroyed, the server
		    // has been correctly deactivated, we can return.
		    //
		    return;
		}

		//
		// Wait for the process to be set.
		//
		timedWait(IceUtil::Time::seconds(_deactivationTimeout));
	    }
	}
	process = _process;
    }

    try
    {
	//
	// Deactivate the server.
	//
	if(kill)
	{
	    _node->getActivator()->kill(_id);
	}
	else
	{
	    _node->getActivator()->deactivate(_id, process);
	}

	//
	// Wait for the server to be inactive (the activator monitors
	// the process and should notify us when it detects the
	// process termination by calling the terminated() method).
	//
	Lock sync(*this);

#ifndef NDEBUG
	InternalServerState oldState = _state;
#endif

	while(true)
	{
	    if(_state == ServerI::Inactive || _state == ServerI::Destroyed)
	    {
		//
		// State changed to inactive or destroyed, the server
		// has been correctly deactivated, we can return.
		//
		return;
	    }
	    
	    //
	    // Wait for a notification.
	    //
	    bool notify = timedWait(IceUtil::Time::seconds(_deactivationTimeout));
	    if(!notify)
	    {
		assert(oldState == _state);
		break;
	    }
	}

	if(_node->getTraceLevels()->server > 1)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
	    out << "graceful server shutdown timed out, killing server `" << _id << "'";
	}
    }
    catch(const Ice::Exception& ex)
    {
	Ice::Warning out(_node->getTraceLevels()->logger);
	out << "graceful server shutdown failed, killing server `" << _id << "':\n";
	out << ex;
    }

    //
    // The server is still not inactive, kill it.
    //
    try
    {
	_node->getActivator()->kill(_id);
    }
    catch(const Ice::SyscallException& ex)
    {
	Ice::Warning out(_node->getTraceLevels()->logger);
	out << "deactivation failed for server `" << _id << "':\n";
	out << ex;
	
	setState(ServerI::Active);
    }
}

void
ServerI::setState(InternalServerState st)
{
    Lock sync(*this);
    setStateNoSync(st);
    notifyAll();
}

void
ServerI::setStateNoSync(InternalServerState st)
{
    InternalServerState previous = _state;
    _state = st;

    if(!_startCB.empty() && _state != ServerI::WaitForActivation)
    {
	for(vector<AMD_Server_startPtr>::const_iterator p = _startCB.begin(); p != _startCB.end(); ++p)
	{
	    (*p)->ice_response(_state == ServerI::Active);
	}
	_startCB.clear();

	_node->getWaitQueue()->notifyAllWaitingOn(this);
    }

    if(toServerState(previous) != toServerState(_state))
    {
	NodeObserverPrx observer = _node->getObserver();
	if(observer)
	{
	    ServerDynamicInfo info;
	    info.id = _id;
	    info.state = toServerState(st);
	    
	    //
	    // NOTE: this must be done only for the active state. Otherwise, we could get a 
	    // deadlock since getPid() will lock the activator and since this method might 
	    // be called from the activator locked.
	    //
	    info.pid = st == ServerI::Active ? getPid() : 0;
	    
	    try
	    {
		observer->updateServer(_node->getName(), info);
	    }
	    catch(const Ice::LocalException&)
	    {
	    }
	}
    }

    if(_node->getTraceLevels()->server > 1)
    {
	if(_state == ServerI::Active)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
	    out << "changed server `" << _id << "' state to `Active'";
	}
	else if(_state == ServerI::Inactive)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
	    out << "changed server `" << _id << "' state to `Inactive'";
	}
	else if(_state == ServerI::Destroyed)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
	    out << "changed server `" << _id << "' state to `Destroyed'";
	}
	else if(_node->getTraceLevels()->server > 2)
	{
	    if(_state == ServerI::WaitForActivation)
	    {
		Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
		out << "changed server `" << _id << "' state to `WaitForActivation'";
	    }
	    else if(_state == ServerI::Activating)
	    {
		Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
		out << "changed server `" << _id << "' state to `Activating'";
	    }
	    else if(_state == ServerI::Deactivating)
	    {
		Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
		out << "changed server `" << _id << "' state to `Deactivating'";
	    }
	    else if(_state == ServerI::Destroying)
	    {
		Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
		out << "changed server `" << _id << "' state to `Destroying'";
	    }
	}
    }
}

void
ServerI::update(const ServerDescriptorPtr& descriptor, StringAdapterPrxDict& adapters, int& activationTimeout,
		int& deactivationTimeout, const Ice::Current& current)
{
    Lock sync(*this);
    _desc = descriptor;
    _serverDir = _serversDir + "/" + descriptor->id;
    _activation = descriptor->activation  == "on-demand" ? OnDemand : Manual;

    istringstream at(_desc->activationTimeout);
    if(!(at >> _activationTimeout) || !at.eof() || _activationTimeout == 0)
    {
	_activationTimeout = _waitTime;
    }
    istringstream dt(_desc->deactivationTimeout);
    if(!(dt >> _deactivationTimeout) || !dt.eof() || _deactivationTimeout == 0)
    {
	_deactivationTimeout = _waitTime;
    }
    activationTimeout = _activationTimeout;
    deactivationTimeout = _deactivationTimeout;

    //
    // Make sure the server directories exists.
    //
    try
    {
	Ice::StringSeq contents = readDirectory(_serverDir);
	if(find(contents.begin(), contents.end(), "config") == contents.end())
	{
	    throw "can't find `config' directory in `" + _serverDir + "'";
	}
	if(find(contents.begin(), contents.end(), "dbs") == contents.end())
	{
	    throw "can't find `dbs' directory in `" + _serverDir + "'";
	}
    }
    catch(const string& msg)
    {
	//
	// TODO: log message?
	//

	createDirectory(_serverDir);
	createDirectory(_serverDir + "/config");
	createDirectory(_serverDir + "/dbs");
    }

    //
    // Update the configuration file(s) of the server if necessary.
    //
    Ice::StringSeq knownFiles;
    updateConfigFile(_serverDir, descriptor);
    knownFiles.push_back("config");
    IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(descriptor);
    if(iceBox)
    {
	for(ServiceInstanceDescriptorSeq::const_iterator p = iceBox->services.begin(); p != iceBox->services.end(); ++p)
	{
	    updateConfigFile(_serverDir, ServiceDescriptorPtr::dynamicCast(p->descriptor));
	    knownFiles.push_back("config_" + p->descriptor->name);
	}
    }

    //
    // Remove old configuration files.
    //
    Ice::StringSeq configFiles = readDirectory(_serverDir + "/config");
    Ice::StringSeq toDel;
    set_difference(configFiles.begin(), configFiles.end(), knownFiles.begin(), knownFiles.end(), back_inserter(toDel));
    for(Ice::StringSeq::const_iterator p = toDel.begin(); p != toDel.end(); ++p)
    {
	if(p->find("config_") == 0)
	{
	    try
	    {
		remove(_serverDir + "/config/" + *p);
	    }
	    catch(const string& msg)
	    {
		Ice::Warning out(_node->getTraceLevels()->logger);
		out << "couldn't remove file `" + _serverDir + "/config/" + *p + "':\n" + msg;
	    }
	}
    }

    //
    // Update the database environments if necessary.
    //
    Ice::StringSeq knownDbEnvs;
    for(DbEnvDescriptorSeq::const_iterator p = descriptor->dbEnvs.begin(); p != descriptor->dbEnvs.end(); ++p)
    {
	updateDbEnv(_serverDir, *p);
	knownDbEnvs.push_back(p->name);
    }

    //
    // Remove old database environments.
    //
    Ice::StringSeq dbEnvs = readDirectory(_serverDir + "/dbs");
    toDel.clear();
    set_difference(dbEnvs.begin(), dbEnvs.end(), knownDbEnvs.begin(), knownDbEnvs.end(), back_inserter(toDel));
    for(Ice::StringSeq::const_iterator p = toDel.begin(); p != toDel.end(); ++p)
    {
	try
	{
	    removeRecursive(_serverDir + "/dbs/" + *p);
	}
	catch(const string& msg)
	{
	    Ice::Warning out(_node->getTraceLevels()->logger);
	    out << "couldn't remove directory `" + _serverDir + "/dbs/" + *p + "':\n" + msg;
	}
    }

    //
    // Create the object adapter objects if necessary.
    //
    _processRegistered = false;
    map<string, ServerAdapterIPtr> oldAdapters;
    oldAdapters.swap(_adapters);
    for(AdapterDescriptorSeq::const_iterator p = descriptor->adapters.begin(); p != descriptor->adapters.end(); ++p)
    {
	adapters.insert(make_pair(p->id, addAdapter(*p, current)));
	oldAdapters.erase(p->id);
    }
    if(iceBox)
    {
	for(ServiceInstanceDescriptorSeq::const_iterator p = iceBox->services.begin(); p != iceBox->services.end(); ++p)
	{
	    ServiceDescriptorPtr s = ServiceDescriptorPtr::dynamicCast(p->descriptor);
	    for(AdapterDescriptorSeq::const_iterator q = s->adapters.begin(); q != s->adapters.end(); ++q)
	    {
		adapters.insert(make_pair(q->id, addAdapter(*q, current)));
		oldAdapters.erase(q->id);
	    }
	}
    }
    for(map<string, ServerAdapterIPtr>::const_iterator p = oldAdapters.begin(); p != oldAdapters.end(); ++p)
    {
	try
	{
	    p->second->destroy(current);
	}
	catch(const Ice::LocalException&)
	{
	}
    }
}

AdapterPrx
ServerI::addAdapter(const AdapterDescriptor& descriptor, const Ice::Current& current)
{
    Ice::Identity id;
    id.category = "IceGridServerAdapter";
    id.name = _desc->id + "-" + descriptor.id + "-" + descriptor.name;
    AdapterPrx proxy = AdapterPrx::uncheckedCast(current.adapter->createProxy(id));
    ServerAdapterIPtr servant = ServerAdapterIPtr::dynamicCast(current.adapter->find(id));
    if(!servant)
    {
	servant = new ServerAdapterI(_node, this, _desc->id, proxy, descriptor.id, _waitTime);
	current.adapter->add(servant, id);
    }
    _adapters.insert(make_pair(descriptor.id, servant));
    _processRegistered |= descriptor.registerProcess;
    return proxy;
}

void
ServerI::updateConfigFile(const string& serverDir, const CommunicatorDescriptorPtr& descriptor)
{
    string configFilePath;

    PropertyDescriptorSeq props;
    if(ServerDescriptorPtr::dynamicCast(descriptor))
    {
	ServerDescriptorPtr serverDesc = ServerDescriptorPtr::dynamicCast(descriptor);

	configFilePath = serverDir + "/config/config";

	//
	// Add server properties.
	//
	props.push_back(createProperty("# Server configuration"));
	props.push_back(createProperty("Ice.ProgramName", serverDesc->id));
	copy(descriptor->properties.begin(), descriptor->properties.end(), back_inserter(props));

	//
	// Add service properties.
	//
	string servicesStr;
	IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(descriptor);
	if(iceBox)
	{
	    ServiceInstanceDescriptorSeq::const_iterator p;
	    for(p = iceBox->services.begin(); p != iceBox->services.end(); ++p)
	    {
		ServiceDescriptorPtr s = ServiceDescriptorPtr::dynamicCast(p->descriptor);
		const string path = serverDir + "/config/config_" + s->name;
		props.push_back(createProperty("IceBox.Service." + s->name, s->entry + " --Ice.Config=" + path));
		servicesStr += s->name + " ";
	    }
	    props.push_back(createProperty("IceBox.LoadOrder", servicesStr));
	}
    }
    else
    {	
	ServiceDescriptorPtr serviceDesc = ServiceDescriptorPtr::dynamicCast(descriptor);
	assert(serviceDesc);
	configFilePath = serverDir + "/config/config_" + serviceDesc->name;
	props.push_back(createProperty("# Service configuration"));
	copy(descriptor->properties.begin(), descriptor->properties.end(), back_inserter(props));
    }

    //
    // Add database environment properties.
    //
    for(DbEnvDescriptorSeq::const_iterator p = descriptor->dbEnvs.begin(); p != descriptor->dbEnvs.end(); ++p)
    {
	const string path = serverDir + "/dbs/" + p->name;
	props.push_back(createProperty("# Database environment " + p->name));
	props.push_back(createProperty("Freeze.DbEnv." + p->name + ".DbHome", path));
    }

    //
    // Add object adapter properties.
    //
    for(AdapterDescriptorSeq::const_iterator p = descriptor->adapters.begin(); p != descriptor->adapters.end(); ++p)
    {
	props.push_back(createProperty("# Object adapter " + p->name));
	props.push_back(createProperty(p->name + ".AdapterId", p->id));
	if(p->registerProcess)
	{
	    props.push_back(createProperty(p->name + ".RegisterProcess", "1"));
	}
    }

    //
    // Load the previous configuration properties.
    //
    PropertyDescriptorSeq orig;
    try
    {
	Ice::PropertiesPtr origProps = Ice::createProperties();
	origProps->load(configFilePath);
	Ice::PropertyDict all = origProps->getPropertiesForPrefix("");
	for(Ice::PropertyDict::const_iterator p = all.begin(); p != all.end(); ++p)
	{
	    orig.push_back(createProperty(p->first, p->second));
	}
    }
    catch(const Ice::LocalException&)
    {
    }

    // 
    // Only update the properties on the disk if they are different.
    //
    if(set<PropertyDescriptor>(orig.begin(), orig.end()) != set<PropertyDescriptor>(props.begin(), props.end()))
    {
	ofstream configfile;
	configfile.open(configFilePath.c_str(), ios::out);
	if(!configfile)
	{
	    DeploymentException ex;
	    ex.reason = "couldn't create configuration file: " + configFilePath;
	    throw ex;
	}
	
	for(PropertyDescriptorSeq::const_iterator p = props.begin(); p != props.end(); ++p)
	{
	    configfile << p->name;
	    if(!p->value.empty())
	    {
		configfile << "=" << p->value;
	    }
	    configfile << endl;
	}
	configfile.close();
    }
}

void
ServerI::updateDbEnv(const string& serverDir, const DbEnvDescriptor& dbEnv)
{
    string dbEnvHome = dbEnv.dbHome;
    if(dbEnvHome.empty())
    {
	dbEnvHome = serverDir + "/dbs/" + dbEnv.name;
	try
	{
	    IcePatch2::createDirectory(dbEnvHome);
	}
	catch(const string& message)
	{
	}
    }

    //
    // TODO: only write the configuration file if necessary.
    //

    string file = dbEnvHome + "/DB_CONFIG";
    ofstream configfile;
    configfile.open(file.c_str(), ios::out);
    if(!configfile)
    {
	DeploymentException ex;
	ex.reason = "couldn't create configuration file: " + file;
	throw ex;
    }

    for(PropertyDescriptorSeq::const_iterator p = dbEnv.properties.begin(); p != dbEnv.properties.end(); ++p)
    {
	if(!p->name.empty())
	{
	    configfile << p->name;
	    if(!p->value.empty())
	    {
		configfile << " " << p->value;
	    }
	    configfile << endl;
	}
    }
    configfile.close();
}

PropertyDescriptor
ServerI::createProperty(const string& name, const string& value)
{
    PropertyDescriptor prop;
    prop.name = name;
    prop.value = value;
    return prop;
}

ServerState
ServerI::toServerState(InternalServerState st) const
{
    switch(st)
    {
    case ServerI::Inactive:
	return IceGrid::Inactive;
    case ServerI::Activating:
	return IceGrid::Activating;
    case ServerI::WaitForActivation:
	return IceGrid::Activating;
    case ServerI::Active:
	return IceGrid::Active;
    case ServerI::Deactivating:
	return IceGrid::Deactivating;
    case ServerI::Destroying:
	return IceGrid::Destroying;
    case ServerI::Destroyed:
	return IceGrid::Destroyed;
    default:
	assert(false);
	return IceGrid::Destroyed;
    }
}

