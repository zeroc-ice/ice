// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/ServerI.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/Activator.h>
#include <IceGrid/NodeI.h>
#include <IceGrid/Util.h>
#include <IceGrid/ServerAdapterI.h>
#include <IceGrid/WaitQueue.h>
#include <IceGrid/DescriptorHelper.h>

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

struct EnvironmentEval : std::unary_function<string, string>
{
    string
    operator()(const std::string& value)
    {
	string::size_type assignment = value.find("=");
	if(assignment == string::npos || assignment >= value.size() - 1)
	{
	    return value;
	}

	string v = value.substr(assignment + 1);
	assert(v.size());
	string::size_type beg = 0;
	string::size_type end;
#ifdef _WIN32
	char buf[32767];
	while((beg = v.find("%", beg)) != string::npos && beg < v.size() - 1)
	{
	    end = v.find("%", beg + 1);
	    if(end == string::npos)
	    {
		break;
	    }
	    string variable = v.substr(beg + 1, end - beg - 1);
	    int ret = GetEnvironmentVariable(variable.c_str(), buf, sizeof(buf));
	    string valstr = (ret > 0 && ret < sizeof(buf)) ? string(buf) : "";
	    v.replace(beg, end - beg + 1, valstr);
 	    beg += valstr.size();
	}
#else
	while((beg = v.find("$", beg)) != string::npos && beg < v.size() - 1)
	{
	    string variable;
	    if(v[beg + 1] == '{')
	    {
		end = v.find("}");
		if(end == string::npos)
		{
		    break;
		}
		variable = v.substr(beg + 2, end - beg - 2);
	    }
	    else
	    {
		end = beg + 1;
		while((isalnum(v[end]) || v[end] == '_')  && end < v.size())
		{
		    ++end;
		}
		variable = v.substr(beg + 1, end - beg - 1);
		--end;
	    }

	    char* val = getenv(variable.c_str());
	    string valstr = val ? string(val) : "";
	    v.replace(beg, end - beg + 1, valstr);
	    beg += valstr.size();
	}
#endif
	return value.substr(0, assignment) + "=" + v;
    }
};

}

ServerI::ServerI(const NodeIPtr& node, const ServerPrx& proxy, const string& serversDir, const string& id, int wt) :
    _node(node),
    _this(proxy),
    _id(id),
    _waitTime(wt),
    _serversDir(serversDir),
    _state(ServerI::Inactive),
    _activation(ServerI::Manual),
    _pid(0)
{
    assert(_node->getActivator());
}

ServerI::~ServerI()
{
}

void
ServerI::update(const string& application,
		const ServerDescriptorPtr& descriptor, 
		bool load, 
		AdapterPrxDict& adapters,
		int& activationTimeout,
		int& deactivationTimeout,
		const Ice::Current& current)
{
    //
    // First we make sure that we're not going to update for nothing
    // by comparing the 2 descriptors.
    //
    {
	Lock sync(*this);
	if(_desc && descriptorEqual(_desc, descriptor))
	{
	    getAdaptersAndTimeouts(adapters, activationTimeout, deactivationTimeout);
	    return;
	}
    }

    startUpdating(true);
    
    //
    // If the server is inactive we can update its descriptor and its directory.
    //
    try
    {
	Lock sync(*this);
	updateImpl(application, descriptor, load, current);
	getAdaptersAndTimeouts(adapters, activationTimeout, deactivationTimeout);
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
	case ServerI::WaitForActivationTimeout: 
	case ServerI::Active:
	{	    
	    setStateNoSync(ServerI::Deactivating, "The server is being stopped.");
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
	case ServerI::WaitForActivationTimeout: 
 	case ServerI::Active:
	{
	    stop = true;
	    setStateNoSync(ServerI::Destroying, "The server is being destroyed.");
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
    for(ServerAdapterDict::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
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
    // Unregister from the object adapter.
    //
    current.adapter->remove(current.id);
}

void
ServerI::terminated(const Ice::Current& current)
{
    InternalServerState newState = ServerI::Inactive; // Initialize to keep the compiler happy.
    ServerAdapterDict adpts;
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
	case ServerI::WaitForActivationTimeout: 
	case ServerI::Active:
	{
	    setStateNoSync(ServerI::Deactivating, "The server terminated.");
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
	// Clear the process proxy and the pid.
	//
	_process = 0;
	_pid = 0;
	break;
    }

    if(newState != ServerI::Destroyed)
    {
	//
	// The server has terminated, set its adapter direct proxies to
	// null to cause the server re-activation if one of its adapter
	// direct proxy is requested.
	//
	for(ServerAdapterDict::iterator p = adpts.begin(); p != adpts.end(); ++p)
	{
	    try
	    {
		p->second->setDirectProxy(0);
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
ServerI::setEnabled(bool enabled, const ::Ice::Current&)
{
    {
	Lock sync(*this);
	if(enabled && _activation < Disabled || !enabled && _activation == Disabled)
	{
	    return;
	}
	_activation = enabled ? (_desc->activation  == "on-demand" ? OnDemand : Manual) : Disabled;
    }
    
    NodeObserverPrx observer = _node->getObserver();
    if(observer)
    {
	try
	{
	    observer->updateServer(_node->getName(), getDynamicInfo());
	}
	catch(const Ice::LocalException&)
	{
	}
    }
}

bool
ServerI::isEnabled(const ::Ice::Current&) const
{
    Lock sync(*this);
    return _activation < Disabled;
}

void
ServerI::setProcess(const ::Ice::ProcessPrx& proc, const ::Ice::Current&)
{
    Lock sync(*this);
    _process = Ice::ProcessPrx::uncheckedCast(proc->ice_timeout(_deactivationTimeout * 1000));
    notifyAll();
}

ServerDescriptorPtr
ServerI::getDescriptor() const
{
    Lock sync(*this);
    return _desc;
}

string
ServerI::getApplication() const
{
    Lock sync(*this);    
    return _application;
}

ServerI::ServerActivation
ServerI::getActivationMode() const
{
    Lock sync(*this);
    return _activation;
}

bool
ServerI::startInternal(ServerActivation act, const AMD_Server_startPtr& amdCB)
{
    ServerDescriptorPtr desc;
    ServerAdapterDict adpts;
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
		    if(_activation == Disabled)
		    {
			amdCB->ice_exception(ServerStartException(_id, "The server is disabled."));
		    }
		    else
		    {
			ServerStartException ex(_id, "The server activation doesn't allow this activation mode.");
			amdCB->ice_exception(ex);
		    }
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
	case ServerI::WaitForActivationTimeout:
	{
	    if(amdCB)
	    {
		amdCB->ice_exception(ServerStartException(_id, "The server activation timed out."));
	    }
	    return false;
	}
 	case ServerI::Active:
	{
	    if(amdCB)
	    {
		amdCB->ice_response();
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
	adpts = _adapters;
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
    transform(desc->envs.begin(), desc->envs.end(), back_inserter(envs), EnvironmentEval());

    string pwd = desc->pwd;
    if(pwd.empty())
    {
	pwd = _serverDir;
    }

    //
    // Clear the adapters direct proxy (this is usefull if the server
    // was manually activated).
    //
    for(ServerAdapterDict::iterator p = adpts.begin(); p != adpts.end(); ++p)
    {
	try
	{
	    p->second->clear();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
    }

    try
    {
	int pid = _node->getActivator()->activate(desc->id, desc->exe, pwd, options, envs, _this);
	
	Lock sync(*this);
	_pid = pid;
	_node->getWaitQueue()->add(new WaitForActivationItem(this), IceUtil::Time::seconds(_activationTimeout));
	setStateNoSync(ServerI::WaitForActivation);
	checkActivation();
	notifyAll();
	return true;
    }
    catch(const std::string& ex)
    {	
	setState(ServerI::Inactive, ex);
	return false;
    }
    catch(const Ice::SyscallException& ex)
    {
	Ice::Warning out(_node->getTraceLevels()->logger);
	out << "activation failed for server `" << _id << "':\n";
	out << ex;

	ostringstream os;
	os << ex;
	setState(ServerI::Inactive, os.str());
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
    ServerAdapterDict adapters;
    {
	Lock sync(*this);
	if(_state != ServerI::WaitForActivation)
	{
	    return;
	}

	setStateNoSync(ServerI::WaitForActivationTimeout, "The server activation timed out.");

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

    for(ServerAdapterDict::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
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
    ServerAdapterDict adapters;
    {
	Lock sync(*this);
	if(_state == ServerI::Inactive && _activation != Disabled)
	{
	    return;
	}
	adapters = _adapters;
	serverInfos.push_back(getDynamicInfo());
    }

    //
    // Add adapters info.
    //
    for(ServerAdapterDict::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
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

bool
ServerI::startUpdating(bool deactivate)
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
		continue;
	    }
	    else if(_state == ServerI::Inactive)
	    {
		_state = ServerI::Updating;
		return true;
	    }
	}

	//
	// If the server is active we stop it and try again to update it.
	//
	if(deactivate)
	{
	    stop();
	}
	else
	{
	    return false;
	}
    }
}

void
ServerI::finishUpdating()
{
    Lock sync(*this);
    if(_state == ServerI::Updating)
    {
	_state = ServerI::Inactive;
	notifyAll();
    }
}

const string&
ServerI::getId() const
{
    return _id;
}

void
ServerI::checkActivation()
{
    //assert(locked());
    if(_state == ServerI::WaitForActivation || _state == ServerI::WaitForActivationTimeout)
    {
	for(AdapterDescriptorSeq::const_iterator p = _desc->adapters.begin(); p != _desc->adapters.end(); ++p)
	{
	    if(!p->id.empty() && p->waitForActivation && _activeAdapters.find(p->id) == _activeAdapters.end())
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
	Lock sync(*this);
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
		bool notify = timedWait(IceUtil::Time::seconds(_deactivationTimeout));
		if(!notify)
		{
		    break;
		}
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
ServerI::setState(InternalServerState st, const std::string& reason)
{
    Lock sync(*this);
    setStateNoSync(st, reason);
    notifyAll();
}

void
ServerI::setStateNoSync(InternalServerState st, const std::string& reason)
{
    InternalServerState previous = _state;
    _state = st;

    if(!_startCB.empty() && _state != ServerI::WaitForActivation)
    {
	for(vector<AMD_Server_startPtr>::const_iterator p = _startCB.begin(); p != _startCB.end(); ++p)
	{
	    if(_state == ServerI::Active)
	    {
		(*p)->ice_response();
	    }
	    else
	    {	
		if(!reason.empty())
		{
		    (*p)->ice_exception(ServerStartException(_id, reason));
		}
		else
		{
		    (*p)->ice_exception(ServerStartException(_id, "The server didn't successfully start."));
		}
	    }
	}
	_startCB.clear();

	_node->getWaitQueue()->notifyAllWaitingOn(this);
    }

    if(toServerState(previous) != toServerState(_state))
    {
	NodeObserverPrx observer = _node->getObserver();
	if(observer)
	{
	    try
	    {
		observer->updateServer(_node->getName(), getDynamicInfo());
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
	    else if(_state == ServerI::WaitForActivationTimeout)
	    {
		Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
		out << "changed server `" << _id << "' state to `WaitForActivationTimeout'";
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
ServerI::updateImpl(const string& app, const ServerDescriptorPtr& descriptor, bool load, const Ice::Current& current)
{
    _application = app;
    _desc = descriptor;
    _serverDir = _serversDir + "/" + descriptor->id;
    if(_activation < Disabled)
    {
	_activation = descriptor->activation  == "on-demand" ? OnDemand : Manual;
    }

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
	if(find(contents.begin(), contents.end(), "ditrib") == contents.end())
	{
	    throw "can't find `distrib' directory in `" + _serverDir + "'";
	}
    }
    catch(const string&)
    {
	//
	// TODO: log message?
	//

	createDirectory(_serverDir);
	createDirectory(_serverDir + "/config");
	createDirectory(_serverDir + "/dbs");
	createDirectory(_serverDir + "/distrib");
    }

    //
    // Update the configuration file(s) of the server if necessary.
    //
    Ice::StringSeq knownFiles;
    updateConfigFile(_serverDir, descriptor, load);
    knownFiles.push_back("config");
    IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(descriptor);
    if(iceBox)
    {
	for(ServiceInstanceDescriptorSeq::const_iterator p = iceBox->services.begin(); p != iceBox->services.end(); 
	    ++p)
	{
	    updateConfigFile(_serverDir, ServiceDescriptorPtr::dynamicCast(p->descriptor), load);
	    knownFiles.push_back("config_" + p->descriptor->name);
	}
    }

    //
    // Remove old configuration files.
    //
    Ice::StringSeq configFiles = readDirectory(_serverDir + "/config");
    Ice::StringSeq toDel;
    set_difference(configFiles.begin(),configFiles.end(), knownFiles.begin(), knownFiles.end(), back_inserter(toDel));
    Ice::StringSeq::const_iterator p;
    for(p = toDel.begin(); p != toDel.end(); ++p)
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
    for(DbEnvDescriptorSeq::const_iterator q = descriptor->dbEnvs.begin(); q != descriptor->dbEnvs.end(); ++q)
    {
	updateDbEnv(_serverDir, *q);
	knownDbEnvs.push_back(q->name);
    }
    if(iceBox)
    {
	ServiceInstanceDescriptorSeq::const_iterator s;
	for(s = iceBox->services.begin(); s != iceBox->services.end(); ++s)
	{
	    CommunicatorDescriptorPtr svc = s->descriptor;
	    for(DbEnvDescriptorSeq::const_iterator q = svc->dbEnvs.begin(); q != svc->dbEnvs.end(); ++q)
	    {
		updateDbEnv(_serverDir, *q);
		knownDbEnvs.push_back(q->name);
	    }
	}
    }

    //
    // Remove old database environments.
    //
    Ice::StringSeq dbEnvs = readDirectory(_serverDir + "/dbs");
    toDel.clear();
    set_difference(dbEnvs.begin(), dbEnvs.end(), knownDbEnvs.begin(), knownDbEnvs.end(), back_inserter(toDel));
    for(p = toDel.begin(); p != toDel.end(); ++p)
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
    ServerAdapterDict oldAdapters;
    oldAdapters.swap(_adapters);
    for(AdapterDescriptorSeq::const_iterator r = descriptor->adapters.begin(); r != descriptor->adapters.end(); ++r)
    {
	if(!r->id.empty())
	{
	    oldAdapters.erase(addAdapter(*r, descriptor, current));
	}
	_processRegistered |= r->registerProcess;
    }
    if(iceBox)
    {
	ServiceInstanceDescriptorSeq::const_iterator s;
	for(s = iceBox->services.begin(); s != iceBox->services.end(); ++s)
	{
	    CommunicatorDescriptorPtr svc = s->descriptor;
	    for(AdapterDescriptorSeq::const_iterator t = svc->adapters.begin(); t != svc->adapters.end(); ++t)
	    {
		if(!t->id.empty())
		{
		    oldAdapters.erase(addAdapter(*t, svc, current));
		}
		_processRegistered |= t->registerProcess;
	    }
	}
    }
    for(ServerAdapterDict::const_iterator t = oldAdapters.begin(); t != oldAdapters.end(); ++t)
    {
	try
	{
	    t->second->destroy(current);
	}
	catch(const Ice::LocalException&)
	{
	}
    }
}

string
ServerI::addAdapter(const AdapterDescriptor& desc, const CommunicatorDescriptorPtr& comm, const Ice::Current& current)
{
    assert(!desc.id.empty());

    Ice::Identity id;
    id.category = "IceGridServerAdapter";
    id.name = _desc->id + "-" + desc.id;
    AdapterPrx proxy = AdapterPrx::uncheckedCast(current.adapter->createProxy(id));
    ServerAdapterIPtr servant = ServerAdapterIPtr::dynamicCast(current.adapter->find(id));
    if(!servant)
    {
	servant = new ServerAdapterI(_node, this, _desc->id, proxy, desc.id, _waitTime);
	current.adapter->add(servant, id);
    }
    _adapters.insert(make_pair(desc.id, servant));
    return desc.id;
}

void
ServerI::updateConfigFile(const string& serverDir, const CommunicatorDescriptorPtr& descriptor, bool rewrite)
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
    for(AdapterDescriptorSeq::const_iterator q = descriptor->adapters.begin(); q != descriptor->adapters.end(); ++q)
    {
	if(!q->id.empty() || q->registerProcess)
	{
	    props.push_back(createProperty("# Object adapter " + q->name));
	    if(!q->id.empty())
	    {
		props.push_back(createProperty(q->name + ".AdapterId", q->id));
		if(!q->replicaGroupId.empty())
		{
		    props.push_back(createProperty(q->name + ".ReplicaGroupId", q->replicaGroupId));
		}
	    }
	    if(q->registerProcess)
	    {
		props.push_back(createProperty(q->name + ".RegisterProcess", "1"));
	    }
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
    // Only update the properties on the disk if they are different or
    // if we're asked to (properties are always re-written after the
    // server is loaded/updated for the first time after the node was
    // started, if the node data directory patch has changed, the
    // changes will be taken into account into the configuration
    // files.)
    //
    if(rewrite ||
       set<PropertyDescriptor>(orig.begin(), orig.end()) != set<PropertyDescriptor>(props.begin(), props.end()))
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
	    configfile << p->name << "=" << p->value << endl;
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
	catch(const string&)
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

void
ServerI::getAdaptersAndTimeouts(AdapterPrxDict& adapters, int& activationTimeout, int& deactivationTimeout) const
{
    for(ServerAdapterDict::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
    {
	adapters.insert(make_pair(p->first, p->second->getProxy()));
    }    
    activationTimeout = _activationTimeout;
    deactivationTimeout = _deactivationTimeout;
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
    case ServerI::Activating:
    case ServerI::Updating:
	return IceGrid::Inactive;
    case ServerI::WaitForActivation:
    case ServerI::WaitForActivationTimeout:
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

ServerDynamicInfo
ServerI::getDynamicInfo() const
{
    ServerDynamicInfo info;
    info.id = _id;
    info.state = toServerState(_state);
    
    //
    // NOTE: this must be done only for the active state. Otherwise, we could get a 
    // deadlock since getPid() will lock the activator and since this method might 
    // be called from the activator locked.
    //
    info.pid = _pid;
    info.enabled = _activation < Disabled;
    return info;
}
