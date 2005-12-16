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
#   include <signal.h>
#else
#   include <sys/wait.h>
#   include <signal.h>
#   include <unistd.h>
#   include <dirent.h>
#endif

#include <fstream>

using namespace std;
using namespace IceGrid;
using namespace IcePatch2;

namespace IceGrid
{

class CommandTimeoutItem : public WaitItem
{
public:
 
    CommandTimeoutItem(const TimedServerCommandPtr& command) : 
	WaitItem(), _command(command)
    {
    }
    
    virtual void expired(bool destroyed)
    {
	_command->timeout(destroyed);
    }

private:
    
    const TimedServerCommandPtr _command;
};

class LoadCommand : public ServerCommand
{
public:

    LoadCommand(const ServerIPtr& server) : 
	ServerCommand(server)
    {
    }

    virtual bool
    canExecute(ServerI::InternalServerState state)
    {
	return state == ServerI::Inactive;
    }

    virtual ServerI::InternalServerState
    nextState()
    {
	return ServerI::Loading;
    }

    virtual void
    execute()
    {
	_server->update();
    }

    void
    setUpdate(const string& application, const ServerDescriptorPtr& desc, bool clearDir)
    {
	_clearDir = clearDir;
	_application = application;
	_desc = desc;
    }

    bool 
    clearDir() const
    {
	return _clearDir;
    }

    string
    getApplication() const
    {
	return _application;
    }
    
    ServerDescriptorPtr
    getDescriptor() const
    {
	return _desc;
    }

    void
    addCallback(const AMD_Node_loadServerPtr& amdCB)
    {
	_loadCB.push_back(amdCB);
    }

    void
    failed(const Ice::Exception& ex)
    {
	for(vector<AMD_Node_loadServerPtr>::const_iterator p = _loadCB.begin(); p != _loadCB.end(); ++p)
	{
	    (*p)->ice_exception(ex);
	}	
    }

    void
    finished(const ServerPrx& proxy, const AdapterPrxDict& adapters, int at, int dt)
    {
	for(vector<AMD_Node_loadServerPtr>::const_iterator p = _loadCB.begin(); p != _loadCB.end(); ++p)
	{
	    (*p)->ice_response(proxy, adapters, at, dt);
	}
    }

private:

    vector<AMD_Node_loadServerPtr> _loadCB;
    bool _clearDir;
    string _application;
    ServerDescriptorPtr _desc;
    auto_ptr<DeploymentException> _exception;
};

class DestroyCommand : public ServerCommand
{
public:

    DestroyCommand(const ServerIPtr& server, bool kill) : 
	ServerCommand(server),
	_kill(kill)
    {
    }

    virtual bool
    canExecute(ServerI::InternalServerState state)
    {
	return
	    state == ServerI::Inactive || 
	    state == ServerI::WaitForActivation ||
	    state == ServerI::Active || 
	    state == ServerI::ActivationTimeout ||
	    state == ServerI::Deactivating || 
	    state == ServerI::DeactivatingWaitForProcess;
    }

    virtual ServerI::InternalServerState
    nextState()
    {
	return ServerI::Destroying;
    }

    virtual void
    execute()
    {
	if(_kill)
	{
	    _server->kill();
	}
	else
	{
	    _server->destroy();
	}
    }

    void
    addCallback(const AMD_Node_destroyServerPtr& amdCB)
    {
	_destroyCB.push_back(amdCB);
    }

    void
    finished()
    {
	for(vector<AMD_Node_destroyServerPtr>::const_iterator p = _destroyCB.begin(); p != _destroyCB.end(); ++p)
	{
	    (*p)->ice_response();
	}	
    }

private:

    const bool _kill;
    vector<AMD_Node_destroyServerPtr> _destroyCB;
};

class PatchCommand : public ServerCommand, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    PatchCommand(const ServerIPtr& server) : 
	ServerCommand(server), 
	_notified(false),
	_destroyed(false)
    {
    }

    virtual bool
    canExecute(ServerI::InternalServerState state)
    {
	return state == ServerI::Inactive;
    }

    virtual ServerI::InternalServerState
    nextState()
    {
	return ServerI::Patching;
    }

    virtual void
    execute()
    {
	Lock sync(*this);
	_notified = true;
	notifyAll();
    }

    bool
    waitForPatch()
    {
	Lock sync(*this);
	while(!_notified && !_destroyed)
	{
	    wait();
	}
	return _destroyed;
    }

    void
    destroyed()
    {
	Lock sync(*this);
	_destroyed = true;
	notifyAll();
    }

    void
    finished()
    {
    }

private:

    bool _notified;
    bool _destroyed;
};

class StartCommand : public TimedServerCommand
{
public:

    StartCommand(const ServerIPtr& server, const WaitQueuePtr& waitQueue, int timeout) : 
	TimedServerCommand(server, waitQueue, timeout)
    {
    }

    static bool isStarted(ServerI::InternalServerState state)
    {
	return state == ServerI::ActivationTimeout || state == ServerI::Active;
    }

    virtual bool
    canExecute(ServerI::InternalServerState state)
    {
	return state == ServerI::Inactive;
    }

    virtual ServerI::InternalServerState
    nextState()
    {
	startTimer();
	return ServerI::Activating;
    }

    virtual void
    execute()
    {
	_server->activate();
    }

    virtual void
    timeout(bool destroyed)
    {
	_server->activationFailed(destroyed);
    }

    void
    addCallback(const AMD_Server_startPtr& amdCB)
    {
	_startCB.push_back(amdCB);
    }
    
    void
    failed(const string& reason)
    {
	stopTimer();
	ServerStartException ex(_server->getId(), reason);
	for(vector<AMD_Server_startPtr>::const_iterator p = _startCB.begin(); p != _startCB.end(); ++p)
	{
	    (*p)->ice_exception(ex);
	}	
	_startCB.clear();
    }

    void
    finished()
    {
	stopTimer();
	for(vector<AMD_Server_startPtr>::const_iterator p = _startCB.begin(); p != _startCB.end(); ++p)
	{
	    (*p)->ice_response();
	}
	_startCB.clear();
    }

private:

    vector<AMD_Server_startPtr> _startCB;
};

class StopCommand : public TimedServerCommand
{
public:

    StopCommand(const ServerIPtr& server, const WaitQueuePtr& waitQueue, int timeout) : 
	TimedServerCommand(server, waitQueue, timeout)
    {
    }

    static bool isStopped(ServerI::InternalServerState state)
    {
	return state == ServerI::Inactive || state == ServerI::Patching || state == ServerI::Loading;
    }

    virtual bool
    canExecute(ServerI::InternalServerState state)
    {
	return state == ServerI::WaitForActivation || state == ServerI::ActivationTimeout || state == ServerI::Active;
    }

    virtual ServerI::InternalServerState
    nextState()
    {
	startTimer();
	return ServerI::Deactivating;
    }

    virtual void
    execute()
    {
	_server->deactivate();
    }

    virtual void
    timeout(bool)
    {
	_server->kill();
    }

    void
    addCallback(const AMD_Server_stopPtr& amdCB)
    {
	_stopCB.push_back(amdCB);
    }

    void
    finished()
    {
	stopTimer();
	for(vector<AMD_Server_stopPtr>::const_iterator p = _stopCB.begin(); p != _stopCB.end(); ++p)
	{
	    (*p)->ice_response();
	}
	_stopCB.clear();
    }

private:

    vector<AMD_Server_stopPtr> _stopCB;
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

ServerCommand::ServerCommand(const ServerIPtr& server) : _server(server)
{
}

TimedServerCommand::TimedServerCommand(const ServerIPtr& server, const WaitQueuePtr& waitQueue, int timeout) : 
    ServerCommand(server), _waitQueue(waitQueue), _timer(new CommandTimeoutItem(this)), _timeout(timeout)
{
}

void
TimedServerCommand::startTimer()
{
    _waitQueue->add(_timer, IceUtil::Time::seconds(_timeout));
}

void
TimedServerCommand::stopTimer()
{
    _waitQueue->remove(_timer);
}

ServerI::ServerI(const NodeIPtr& node, const ServerPrx& proxy, const string& serversDir, const string& id, int wt) :
    _node(node),
    _this(proxy),
    _id(id),
    _waitTime(wt),
    _serversDir(serversDir),
    _disableOnFailure(0),
    _state(ServerI::Inactive),
    _activation(ServerI::Manual),
    _pid(0)
{
    assert(_node->getActivator());
    const_cast<int&>(_disableOnFailure) = 
	_node->getCommunicator()->getProperties()->getPropertyAsIntWithDefault("IceGrid.Node.DisableOnFailure", 0);
}

ServerI::~ServerI()
{
}

void
ServerI::start_async(const AMD_Server_startPtr& amdCB, const Ice::Current&)
{
    ServerCommandPtr command;
    {
	Lock sync(*this);
	checkDestroyed();
	if(_state == Destroying)
	{
	    throw ServerStartException(_id, "The server is being destroyed.");
	}
	
	//
	// The server is disabled because it failed and if the time of
	// the failure is now past the configured duration or if the
	// server is manualy started, we re-enable the server.
	//
	if(_activation == Disabled &&
	   _failureTime != IceUtil::Time() &&
	   (amdCB || 
	    (_disableOnFailure > 0 &&
	     (_failureTime + IceUtil::Time::seconds(_disableOnFailure) < IceUtil::Time::now()))))
	{
	    _failureTime = IceUtil::Time();
	    _activation = _previousActivation;
	}

	//
	// If the amd callback is set, it's a remote start call to
	// manually activate the server. Otherwise it's a call to
	// activate the server on demand (called from ServerAdapterI).
	//
	if(_activation == Disabled)
	{
	    throw ServerStartException(_id, "The server is disabled.");
	}
	else if(_activation == Manual && !amdCB)
	{
	    throw ServerStartException(_id, "The server activation doesn't allow this activation mode.");
	}
	else if(_state == ActivationTimeout)
	{
	    throw ServerStartException(_id, "The server activation timed out.");
	}
	else if(_state == Active)
	{
	    throw ServerStartException(_id, "The server is already active.");
	}

	if(!_start)
	{
	    _start = new StartCommand(this, _node->getWaitQueue(), _activationTimeout);
	}
	if(amdCB)
	{
	    _start->addCallback(amdCB);
	}
	command = nextCommand();
    }
    if(command)
    {
	command->execute();
    }
}

void
ServerI::stop_async(const AMD_Server_stopPtr& amdCB, const Ice::Current&)
{
    ServerCommandPtr command;
    {
	Lock sync(*this);
	checkDestroyed();
	if(StopCommand::isStopped(_state))
	{
	    if(amdCB)
	    {
		amdCB->ice_response();
	    }
	    return;
	}
	if(!_stop)
	{
	    _stop = new StopCommand(this, _node->getWaitQueue(), _deactivationTimeout);
	}
	if(amdCB)
	{
	    _stop->addCallback(amdCB);
	}
	command = nextCommand();
    }
    if(command)
    {
	command->execute();
    }
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

	_failureTime = IceUtil::Time();
	_activation = enabled ? (_desc->activation  == "on-demand" ? OnDemand : Manual) : Disabled;
    }
    
    NodeObserverPrx observer = _node->getObserver();
    if(observer)
    {
	try
	{
	    observer->updateServer(_node->getName(), getDynamicInfo());
	}
	catch(const Ice::LocalException& ex)
	{
	    Ice::Warning out(_node->getTraceLevels()->logger);
	    out << "unexpected observer exception:\n" << ex;
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
ServerI::setProcess_async(const AMD_Server_setProcessPtr& amdCB, const Ice::ProcessPrx& process, const Ice::Current&)
{
    bool deact = false;
    {
	Lock sync(*this);
	_process = process;
	deact = _state == DeactivatingWaitForProcess;
    }
    amdCB->ice_response();
    if(deact)
    {
	deactivate();
    }
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
    return _desc->activation  == "on-demand" ? OnDemand : Manual;
}

const string&
ServerI::getId() const
{
    return _id;
}

DistributionDescriptor
ServerI::getDistribution() const
{
    Lock sync(*this);
    return _desc->distrib;
}

void
ServerI::load(const AMD_Node_loadServerPtr& amdCB, const string& application, const ServerDescriptorPtr& desc)
{
    stop_async(0);

    ServerCommandPtr command;
    {
	Lock sync(*this);
	if(_desc && descriptorEqual(_desc, desc))
	{
	    if(amdCB)
	    {
		AdapterPrxDict adapters;
		for(ServerAdapterDict::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
		{
		    adapters.insert(make_pair(p->first, p->second->getProxy()));
		}
		amdCB->ice_response(_this, adapters, _activationTimeout, _deactivationTimeout);
	    }
	    return;
	}
	if(!_load)
	{
	    _load = new LoadCommand(this);
	}
	_load->setUpdate(application, desc, _destroy);
	if(_destroy && _state != Destroying)
	{
	    _destroy->finished();
	    _destroy = 0;
	}
	if(amdCB)
	{
	    _load->addCallback(amdCB);
	}
	command = nextCommand();
    }
    if(command)
    {
	command->execute();
    }
}

bool
ServerI::startPatch(bool shutdown)
{
    if(shutdown)
    {
	stop_async(0);
    }
    
    ServerCommandPtr command;
    {
	Lock sync(*this);
	checkDestroyed();
	if(_state != Inactive && !shutdown)
	{
	    return false;
	}
	if(!_patch)
	{
	    _patch = new PatchCommand(this);
	}
	command = nextCommand();
    }
    if(command)
    {
	command->execute();
    }
    return true;
}

bool
ServerI::waitForPatch()
{
    PatchCommandPtr patch;
    {
	Lock sync(*this);
	if(!_patch)
	{
	    return true;
	}
	patch = _patch;
    }
    return patch->waitForPatch();
}

void
ServerI::finishPatch()
{
    setState(Inactive);
}

void
ServerI::destroy(const AMD_Node_destroyServerPtr& amdCB)
{
    ServerCommandPtr command;
    {
	Lock sync(*this);
	if(!_destroy)
	{
	    _destroy = new DestroyCommand(this, _state != Inactive && _state != Loading && _state != Patching);
	}
	if(amdCB)
	{
	    _destroy->addCallback(amdCB);
	}
	command = nextCommand();
    }
    if(command)
    {
	command->execute();
    }
}

void
ServerI::adapterActivated(const string& id)
{
    ServerCommandPtr command;
    {
	Lock sync(*this);
	_activeAdapters.insert(id);
	checkActivation();
	command = nextCommand();
    }
    if(command)
    {
	command->execute();
    }
}

void
ServerI::checkDestroyed()
{
    if(_state == Destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = _this->ice_getIdentity();
	throw ex;
    }
}

void
ServerI::adapterDeactivated(const string& id)
{
    Lock sync(*this);
    _activeAdapters.erase(id);
}

void
ServerI::activationFailed(bool destroyed)
{
    ServerCommandPtr command;
    ServerAdapterDict adapters;
    {
	Lock sync(*this);
	if(_state != ServerI::WaitForActivation)
	{
	    return;
	}

	if(!destroyed)
	{
	    setStateNoSync(ServerI::ActivationTimeout, "The server activation timed out.");
	}
	else
	{
	    setStateNoSync(ServerI::ActivationTimeout, "The node is being shutdown.");
	}

	if(_node->getTraceLevels()->server > 1)
	{
	    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
	    if(!destroyed)
	    {
		out << "server `" << _id << "' activation timed out";
	    }	
	    else
	    {
		out << "server `" << _id << "' activation failed";
	    }
	}
	adapters = _adapters;
	command = nextCommand();
    }

    for(ServerAdapterDict::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
	try
	{
	    p->second->activationFailed(destroyed);
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
    }
    if(command)
    {
	command->execute();
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

void
ServerI::activate()
{
    ServerDescriptorPtr desc;
    ServerAdapterDict adpts;
    {
	Lock sync(*this);
	assert(_state == Activating);
	desc = _desc;
	adpts = _adapters;
    }

    //
    // Compute the server command line options.
    //
    Ice::StringSeq options;
    copy(desc->options.begin(), desc->options.end(), back_inserter(options));
    options.push_back("--Ice.Config=" + _serverDir + "/config/config");

    Ice::StringSeq envs;
    transform(desc->envs.begin(), desc->envs.end(), back_inserter(envs), EnvironmentEval());

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

    string failure;
    try
    {
	int pid = _node->getActivator()->activate(desc->id, desc->exe, desc->pwd, options, envs, this);

	ServerCommandPtr command;
	{
	    Lock sync(*this);
	    assert(_state == Activating);
	    _pid = pid;
	    setStateNoSync(ServerI::WaitForActivation);
 	    checkActivation();
	    command = nextCommand();
	}
	if(command)
	{
	    command->execute();
	}
	return;
    }
    catch(const std::string& ex)
    {
	failure = ex;
    }
    catch(const Ice::SyscallException& ex)
    {
	Ice::Warning out(_node->getTraceLevels()->logger);
	out << "activation failed for server `" << _id << "':\n";
	out << ex;

	ostringstream os;
	os << ex;
	failure = os.str();
    }
    for(ServerAdapterDict::iterator r = adpts.begin(); r != adpts.end(); ++r)
    {
	try
	{
	    r->second->activationFailed(true);
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
    }    
    setState(ServerI::Inactive, failure);
}

void
ServerI::kill()
{
    {
	Lock sync(*this);
	if(_state != Destroying && _state != Deactivating && _state != DeactivatingWaitForProcess)
	{
	    return;
	}
    }

    try
    {
	_node->getActivator()->kill(_id);
    }
    catch(const Ice::SyscallException& ex)
    {
	Ice::Warning out(_node->getTraceLevels()->logger);
	out << "deactivation failed for server `" << _id << "':\n";
	out << ex;
	setState(ServerI::Inactive); // TODO: Is this really correct?
    }
}

void
ServerI::deactivate()
{
    Ice::ProcessPrx process;
    {
	Lock sync(*this);
	if(_state != Deactivating && _state != DeactivatingWaitForProcess)
	{
	    return;
	}
	if(_processRegistered && !_process)
	{
	    setStateNoSync(DeactivatingWaitForProcess);
	    return;
	}
	process = _process;
    }

    try
    {
	//
	// Deactivate the server and for the termination of the server.
	//
	_node->getActivator()->deactivate(_id, process);
	return;
    }
    catch(const Ice::Exception& ex)
    {
	Ice::Warning out(_node->getTraceLevels()->logger);
	out << "graceful server shutdown failed, killing server `" << _id << "':\n";
	out << ex;
    }

    try
    {
	//
	// If we couldn't deactivate it we kill it.
	//
	_node->getActivator()->kill(_id);
	return;
    }
    catch(const Ice::SyscallException& ex)
    {
	Ice::Warning out(_node->getTraceLevels()->logger);
	out << "deactivation failed for server `" << _id << "':\n";
	out << ex;	
	setState(ServerI::Inactive); // TODO: Is this really correct?
    }
}

void
ServerI::destroy()
{
    ServerAdapterDict adpts;
    {
	Lock sync(*this);
	assert(_state == Destroying);
	adpts = _adapters;
    }

    try
    {
	removeRecursive(_serverDir);
    }
    catch(const string& msg)
    {
	Ice::Warning out(_node->getTraceLevels()->logger);
	out << "removing server directory `" << _serverDir << "' failed:" << msg;
    }
    
    //
    // Destroy the object adapters.
    //
    for(ServerAdapterDict::const_iterator p = adpts.begin(); p != adpts.end(); ++p)
    {
	try
	{
	    p->second->destroy();
	}
	catch(const Ice::LocalException&)
	{
	}
    }    

    setState(Destroyed);
}

void
ServerI::terminated(const string& msg, int status)
{
    ServerAdapterDict adpts;
    {
	Lock sync(*this);
	adpts = _adapters;
    }

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

    ServerCommandPtr command;
    bool destroying = false;
    {
	Lock sync(*this);

	//
	// Clear the process proxy and the pid.
	//
	_process = 0;
	_pid = 0;

	if(_disableOnFailure != 0 && _activation != Disabled)
	{
	    bool failed = false;
#ifndef _WIN32
	    failed = WIFEXITED(status) && WEXITSTATUS(status) != 0;
	    if(WIFSIGNALED(status))
	    {
		int s = WTERMSIG(status);
		failed = s == SIGABRT || s == SIGILL || s == SIGBUS || s == SIGFPE || s == SIGSEGV;
	    }
#else
	    failed = status != 0;
#endif
	    if(failed)
	    {
		_previousActivation = _activation;
		_activation = Disabled;
		_failureTime = IceUtil::Time::now();
	    }
	}
	
	if(_state != ServerI::Destroying)
	{
	    ostringstream os;
	    os << "The server terminated unexpectedly";
#ifndef _WIN32
	    if(WIFEXITED(status))
	    {
		os << " with exit code " << WEXITSTATUS(status);
	    }
	    else if(WIFSIGNALED(status))
	    {
		os << " with signal " << signalToString(WTERMSIG(status));
	    }
#else
	    os << " with exit code " << status;
#endif
	    os << (msg.empty() ? "." : ":\n" + msg);
	    setStateNoSync(ServerI::Inactive, os.str());
	    command = nextCommand();
	}
	else
	{
	    destroying = true;
	}
    }

    if(command)
    {
	command->execute();
    }
    else if(destroying)
    {
	destroy();
    }
}

void
ServerI::update()
{
    ServerCommandPtr command;
    {
	Lock sync(*this);
	if(_state != ServerI::Loading)
	{
	    return;
	}

	try
	{
	    updateImpl();

	    AdapterPrxDict adapters;
	    for(ServerAdapterDict::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
	    {
		adapters.insert(make_pair(p->first, p->second->getProxy()));
	    }    
	    _load->finished(_this, adapters, _activationTimeout, _deactivationTimeout);
	}
	catch(const Ice::Exception& ex)
	{
	    _load->failed(ex);
	}
	catch(const string& msg)
	{
	    _load->failed(DeploymentException(msg));
	}
	catch(const char* msg)
	{
	    _load->failed(DeploymentException(msg));
	}
	setStateNoSync(Inactive);
	command = nextCommand();
    }
    if(command)
    {
	command->execute();
    }
}

void
ServerI::updateImpl()
{
    assert(_load);
    _application = _load->getApplication();
    _desc = _load->getDescriptor();
    _serverDir = _serversDir + "/" + _desc->id;

    if(_load->clearDir())
    {
	try
	{
	    removeRecursive(_serverDir);
	}
	catch(const string&)
	{
	}
    }

    if(_activation < Disabled)
    {
	_activation = _desc->activation  == "on-demand" ? OnDemand : Manual;
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
    updateConfigFile(_serverDir, _desc);
    knownFiles.push_back("config");
    IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(_desc);
    if(iceBox)
    {
	ServiceInstanceDescriptorSeq::const_iterator p;
	for(p = iceBox->services.begin(); p != iceBox->services.end(); ++p)
	{
	    updateConfigFile(_serverDir, ServiceDescriptorPtr::dynamicCast(p->descriptor));
	    knownFiles.push_back("config_" + p->descriptor->name);
	}
    }
    sort(knownFiles.begin(), knownFiles.end());
    
    //
    // Remove old configuration files.
    //
    Ice::StringSeq files = readDirectory(_serverDir + "/config");
    Ice::StringSeq toDel;
    set_difference(files.begin(), files.end(), knownFiles.begin(), knownFiles.end(), back_inserter(toDel));
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
    for(DbEnvDescriptorSeq::const_iterator q = _desc->dbEnvs.begin(); q != _desc->dbEnvs.end(); ++q)
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
    sort(knownDbEnvs.begin(), knownDbEnvs.end());

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
    AdapterDescriptorSeq::const_iterator r;
    for(r = _desc->adapters.begin(); r != _desc->adapters.end(); ++r)
    {
	if(!r->id.empty())
	{
	    oldAdapters.erase(addAdapter(*r, _desc));
	}
	_processRegistered |= r->registerProcess;
    }
    if(iceBox)
    {
	ServiceInstanceDescriptorSeq::const_iterator s;
	for(s = iceBox->services.begin(); s != iceBox->services.end(); ++s)
	{
	    CommunicatorDescriptorPtr svc = s->descriptor;
	    for(r = svc->adapters.begin(); r != svc->adapters.end(); ++r)
	    {
		if(!r->id.empty())
		{
		    oldAdapters.erase(addAdapter(*r, svc));
		}
		_processRegistered |= r->registerProcess;
	    }
	}
    }
    for(ServerAdapterDict::const_iterator t = oldAdapters.begin(); t != oldAdapters.end(); ++t)
    {
	try
	{
	    t->second->destroy();
	}
	catch(const Ice::LocalException&)
	{
	}
    }

}

void
ServerI::checkActivation()
{
    //assert(locked());
    if(_state == ServerI::WaitForActivation || _state == ServerI::ActivationTimeout)
    {
	for(AdapterDescriptorSeq::const_iterator p = _desc->adapters.begin(); p != _desc->adapters.end(); ++p)
	{
	    if(!p->id.empty() && p->waitForActivation && _activeAdapters.find(p->id) == _activeAdapters.end())
	    {
		return;
	    }
	}
	IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(_desc);
	if(iceBox)
	{
	    ServiceInstanceDescriptorSeq::const_iterator s;
	    for(s = iceBox->services.begin(); s != iceBox->services.end(); ++s)
	    {
		ServiceDescriptorPtr desc = ServiceDescriptorPtr::dynamicCast(s->descriptor);
		for(AdapterDescriptorSeq::const_iterator p = desc->adapters.begin(); p != desc->adapters.end(); ++p)
		{
		    if(!p->id.empty() && p->waitForActivation && _activeAdapters.find(p->id) == _activeAdapters.end())
		    {
			return;
		    }
		}
	    }
	}
	setStateNoSync(ServerI::Active);
    }
}

void
ServerI::setState(InternalServerState st, const std::string& reason)
{
    ServerCommandPtr command;
    {
	Lock sync(*this);
	setStateNoSync(st, reason);
	command = nextCommand();
    }
    if(command)
    {
	command->execute();
    }
}

ServerCommandPtr
ServerI::nextCommand()
{
    ServerCommandPtr command;
    if(_destroy && _destroy->canExecute(_state))
    {
	command = _destroy;
    }
    else if(_stop && _stop->canExecute(_state))
    {
	command = _stop;
    }
    else if(_load && _load->canExecute(_state))
    {
	command = _load;
    }
    else if(_patch && _patch->canExecute(_state))
    {
	command = _patch;
    }
    else if(_start && _start->canExecute(_state))
    {
	command = _start;
    }
    if(command)
    {
	setStateNoSync(command->nextState());
    }
    return command;
}

void
ServerI::setStateNoSync(InternalServerState st, const std::string& reason)
{
    //
    // Ensure that the given state can be switched to.
    //
    switch(st)
    {
    case Inactive:
	break;
    case Patching:
	assert(_patch && _patch->canExecute(_state));
	break;
    case Loading:
	assert(_load && _load->canExecute(_state));
	break;
    case Activating:
	assert(_start && _start->canExecute(_state));
	break;
    case WaitForActivation:
	assert(_state == Activating);
	break;
    case ActivationTimeout:
	assert(_state == WaitForActivation);
	break;
    case Active:
	assert(_state == WaitForActivation || _state == ActivationTimeout);
	break;	
    case Deactivating:
	assert(_stop && _stop->canExecute(_state));
	break;
    case DeactivatingWaitForProcess:
	assert(_state == Deactivating);
	break;
    case Destroying:
	assert(_destroy && _destroy->canExecute(_state));
	break;
    case Destroyed:
	assert(_destroy);
	break;
    }

    //
    // Change the current state.
    //
    InternalServerState previous = _state;
    _state = st;

    //
    // Check if some commands are done.
    //
    switch(_state)
    {
    case Inactive:
	if(previous == Loading)
	{
	    _load = 0;
	}
	if(previous == Patching)
	{
	    _patch = 0;
	}
	if(_start)
	{
	    _start->failed(reason);
	    _start = 0;
	}
	if(_stop)
	{
	    _stop->finished();
	    _stop = 0;
	}
	break;
    case Active:
	if(_start)
	{
	    _start->finished();
	    _start = 0;
	}
	break;
    case ActivationTimeout:
	if(_start)
	{
	    _start->failed(reason);
	    _start = 0;
	}
	break;
    case Deactivating:
	if(_start)
	{
	    _start->failed("The server is being deactivated.");
	    _start = 0;
	}
	break;
    case Destroying:
	if(_patch)
	{
	    _patch->destroyed();
	    _patch = 0;
	}
	if(_load)
	{
	    _load->failed(DeploymentException("The server is being destroyed."));
	    _load = 0;
	}
	if(_start)
	{
	    _start->failed("The server is being destroyed.");
	    _start = 0;
	}
	break;
    case Destroyed:
	if(_stop)
	{
	    _stop->finished();
	    _stop = 0;
	}
	if(_destroy)
	{
	    _destroy->finished();
	    _destroy = 0;
	}
	break;
    default:
	break;
    }

    if(_state == Destroyed && !_load)
    {
	try
	{
	    _node->getAdapter()->remove(_this->ice_getIdentity());
	}
	catch(const Ice::ObjectAdapterDeactivatedException&)
	{
	}
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
	    catch(const Ice::LocalException& ex)
	    {
		Ice::Warning out(_node->getTraceLevels()->logger);
		out << "unexpected observer exception:\n" << ex;
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
	    else if(_state == ServerI::ActivationTimeout)
	    {
		Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
		out << "changed server `" << _id << "' state to `ActivationTimeout'";
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
	    else if(_state == ServerI::DeactivatingWaitForProcess)
	    {
		Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
		out << "changed server `" << _id << "' state to `DeactivatingWaitForProcess'";
	    }
	    else if(_state == ServerI::Destroying)
	    {
		Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
		out << "changed server `" << _id << "' state to `Destroying'";
	    }
	    else if(_state == ServerI::Loading)
	    {
		Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
		out << "changed server `" << _id << "' state to `Loading'";
	    }
	    else if(_state == ServerI::Patching)
	    {
		Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
		out << "changed server `" << _id << "' state to `Loading'";
	    }
	}
    }
}

string
ServerI::addAdapter(const AdapterDescriptor& desc, const CommunicatorDescriptorPtr& comm)
{
    assert(!desc.id.empty());

    Ice::Identity id;
    id.category = _this->ice_getIdentity().category + "Adapter";
    id.name = _desc->id + "-" + desc.id;
    AdapterPrx proxy = AdapterPrx::uncheckedCast(_node->getAdapter()->createProxy(id));
    ServerAdapterIPtr servant = ServerAdapterIPtr::dynamicCast(_node->getAdapter()->find(id));
    if(!servant)
    {
	servant = new ServerAdapterI(_node, this, _desc->id, proxy, desc.id, _waitTime);
	_node->getAdapter()->add(servant, id);
    }
    _adapters.insert(make_pair(desc.id, servant));
    return desc.id;
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
		props.push_back(createProperty("IceBox.Service." + s->name, 
					       s->entry + " --Ice.Config=\"" + path + "\""));
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

    ofstream configfile;
    configfile.open(configFilePath.c_str(), ios::out);
    if(!configfile)
    {
	DeploymentException ex;
	ex.reason = "couldn't create configuration file: " + configFilePath;
	throw ex;
    }
    for(PropertyDescriptorSeq::const_iterator r = props.begin(); r != props.end(); ++r)
    {
	if(r->value.empty() && r->name.find('#') == 0)
	{
	    configfile << r->name << endl;
	}
	else
	{
	    configfile << r->name << "=" << r->value << endl;
	}
    }
    configfile.close();
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
	throw DeploymentException("couldn't create configuration file: " + file);
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
    case ServerI::Activating:
    case ServerI::Patching:
    case ServerI::Loading:
	return IceGrid::Inactive;
    case ServerI::WaitForActivation:
    case ServerI::ActivationTimeout:
	return IceGrid::Activating;
    case ServerI::Active:
	return IceGrid::Active;
    case ServerI::Deactivating:
    case ServerI::DeactivatingWaitForProcess:
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
