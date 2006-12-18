// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
#include <IcePatch2/OS.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#  include <direct.h>
#  include <signal.h>
#else
#  include <sys/wait.h>
#  include <pwd.h> // for getpwnam
#  include <signal.h>
#  include <unistd.h>
#  include <dirent.h>
#endif

#include <fstream>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

#ifndef _WIN32
void
chownRecursive(const string& path, uid_t uid, gid_t gid)
{
    struct dirent **namelist = 0;
    DIR* d;
    if((d = opendir(path.c_str())) == 0)
    {
	throw "cannot read directory `" + path + "':\n" + IcePatch2::lastError();
    }

    struct dirent* entry;
    int n = 0;
    while((entry = readdir(d)) != 0)
    {
        namelist = (struct dirent**)realloc((void*)namelist, (size_t)((n + 1) * sizeof(struct dirent*)));
        if(namelist == 0)
        {
	    closedir(d);
	    throw "cannot read directory `" + path + "':\n" + IcePatch2::lastError();
        }

        size_t entrysize = sizeof(struct dirent) - sizeof(entry->d_name) + strlen(entry->d_name) + 1;
        namelist[n] = (struct dirent*)malloc(entrysize);
        if(namelist[n] == 0)
        {
	    closedir(d);
	    throw "cannot read directory `" + path + "':\n" + IcePatch2::lastError();
        }
        memcpy(namelist[n], entry, entrysize);
        ++n;
    }

    if(closedir(d))
    {
	throw "cannot read directory `" + path + "':\n" + IcePatch2::lastError();
    }

    for(int i = 0; i < n; ++i)
    {
	string name = namelist[i]->d_name;
	assert(!name.empty());
	free(namelist[i]);

	if(name != ".." && name != ".")
	{
	    name = path + "/" + name;
	    if(chown(name.c_str(), uid, gid) != 0)
	    {
		throw "can't change permissions on `" + name + "':\n" + IcePatch2::lastError();
	    }

	    OS::structstat buf;
	    if(OS::osstat(name, &buf) == -1)
	    {
		throw "cannot stat `" + name + "':\n" + IcePatch2::lastError();
	    }
	    
	    if(S_ISDIR(buf.st_mode))
	    {
		chownRecursive(name, uid, gid);
	    }
	}
    }

    free(namelist);
}
#endif

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

class DelayedStart : public WaitItem
{
public:
 
    DelayedStart(const ServerIPtr& server, const TraceLevelsPtr& traceLevels) : 
	_server(server),
	_traceLevels(traceLevels)
    {
    }
    
    virtual void expired(bool destroyed)
    {
	if(!destroyed)
	{
	    try
	    {
		_server->start(ServerI::Always);
	    }
	    catch(const ServerStartException& ex)
	    {
		Ice::Error out(_traceLevels->logger);
		out << "couldn't reactivate server `" << _server->getId() 
		    << "' with `always' activation mode after failure:\n" 
		    << ex.reason;
	    }
	    catch(const Ice::ObjectNotExistException&)
	    {
	    }
	}
    }

private:

    const ServerIPtr _server;
    const TraceLevelsPtr _traceLevels;
};

class WaitForApplicationUpdateCB : public AMI_NodeSession_waitForApplicationUpdate
{
public:

    WaitForApplicationUpdateCB(const ServerIPtr& server) : _server(server)
    {
    }

    virtual void
    ice_response()
    {
	_server->activate();
    }

    virtual void
    ice_exception(const Ice::Exception&)
    {
	_server->activate();
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
	    string valstr = (ret > 0 && ret < sizeof(buf)) ? string(buf) : string("");
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

ServerCommand::~ServerCommand()
{
}

TimedServerCommand::TimedServerCommand(const ServerIPtr& server, const WaitQueuePtr& waitQueue, int timeout) : 
    ServerCommand(server), _waitQueue(waitQueue), _timeout(timeout)
{
}

void
TimedServerCommand::startTimer()
{
    _timer = new CommandTimeoutItem(this);
    _waitQueue->add(_timer, IceUtil::Time::seconds(_timeout));
}

void
TimedServerCommand::stopTimer()
{
    if(_timer)
    {
	_waitQueue->remove(_timer);
	_timer = 0;
    }
}

LoadCommand::LoadCommand(const ServerIPtr& server) : 
    ServerCommand(server)
{
}

bool
LoadCommand::canExecute(ServerI::InternalServerState state)
{
    return state == ServerI::Inactive;
}

ServerI::InternalServerState
LoadCommand::nextState()
{
    return ServerI::Loading;
}

void
LoadCommand::execute()
{
    _server->update();
}

void
LoadCommand::setUpdate(const ServerInfo& info, bool clearDir)
{
    _clearDir = clearDir;
    _info = info;
}

ServerInfo
LoadCommand::getServerInfo() const
{
    return _info;
}

bool 
LoadCommand::clearDir() const
{
    return _clearDir;
}

void
LoadCommand::addCallback(const AMD_Node_loadServerPtr& amdCB)
{
    _loadCB.push_back(amdCB);
}

void
LoadCommand::failed(const Ice::Exception& ex)
{
    for(vector<AMD_Node_loadServerPtr>::const_iterator p = _loadCB.begin(); p != _loadCB.end(); ++p)
    {
        (*p)->ice_exception(ex);
    }	
}

void
LoadCommand::finished(const ServerPrx& proxy, const AdapterPrxDict& adapters, int at, int dt)
{
    for(vector<AMD_Node_loadServerPtr>::const_iterator p = _loadCB.begin(); p != _loadCB.end(); ++p)
    {
        (*p)->ice_response(proxy, adapters, at, dt);
    }
}

DestroyCommand::DestroyCommand(const ServerIPtr& server, bool kill) : 
    ServerCommand(server),
    _kill(kill)
{
}

bool
DestroyCommand::canExecute(ServerI::InternalServerState state)
{
    return state == ServerI::Inactive;
}

ServerI::InternalServerState
DestroyCommand::nextState()
{
    return ServerI::Destroying;
}

void
DestroyCommand::execute()
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
DestroyCommand::addCallback(const AMD_Node_destroyServerPtr& amdCB)
{
    _destroyCB.push_back(amdCB);
}

void
DestroyCommand::finished()
{
    for(vector<AMD_Node_destroyServerPtr>::const_iterator p = _destroyCB.begin(); p != _destroyCB.end(); ++p)
    {
        (*p)->ice_response();
    }
}

PatchCommand::PatchCommand(const ServerIPtr& server) : 
    ServerCommand(server), 
    _notified(false),
    _destroyed(false)
{
}

bool
PatchCommand::canExecute(ServerI::InternalServerState state)
{
    return state == ServerI::Inactive;
}

ServerI::InternalServerState
PatchCommand::nextState()
{
    return ServerI::Patching;
}

void
PatchCommand::execute()
{
    Lock sync(*this);
    _notified = true;
    notifyAll();
}

bool
PatchCommand::waitForPatch()
{
    Lock sync(*this);
    while(!_notified && !_destroyed)
    {
        wait();
    }
    return _destroyed;
}

void
PatchCommand::destroyed()
{
    Lock sync(*this);
    _destroyed = true;
    notifyAll();
}

void
PatchCommand::finished()
{
}

StartCommand::StartCommand(const ServerIPtr& server, const WaitQueuePtr& waitQueue, int timeout) : 
    TimedServerCommand(server, waitQueue, timeout)
{
}

bool
StartCommand::canExecute(ServerI::InternalServerState state)
{
    return state == ServerI::Inactive;
}

ServerI::InternalServerState
StartCommand::nextState()
{
    startTimer();
    return ServerI::Activating;
}

void
StartCommand::execute()
{
    _server->activate();
}

void
StartCommand::timeout(bool destroyed)
{
    _server->activationFailed(destroyed);
}

void
StartCommand::addCallback(const AMD_Server_startPtr& amdCB)
{
    _startCB.push_back(amdCB);
}
    
void
StartCommand::failed(const string& reason)
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
StartCommand::finished()
{
    stopTimer();
    for(vector<AMD_Server_startPtr>::const_iterator p = _startCB.begin(); p != _startCB.end(); ++p)
    {
        (*p)->ice_response();
    }
    _startCB.clear();
}

StopCommand::StopCommand(const ServerIPtr& server, const WaitQueuePtr& waitQueue, int timeout) : 
    TimedServerCommand(server, waitQueue, timeout)
{
}

bool 
StopCommand::isStopped(ServerI::InternalServerState state)
{
    return state == ServerI::Inactive || state == ServerI::Patching || state == ServerI::Loading;
}

bool
StopCommand::canExecute(ServerI::InternalServerState state)
{
    return state == ServerI::WaitForActivation || state == ServerI::ActivationTimeout || state == ServerI::Active;
}

ServerI::InternalServerState
StopCommand::nextState()
{
    startTimer();
    return ServerI::Deactivating;
}

void
StopCommand::execute()
{
    _server->deactivate();
}

void
StopCommand::timeout(bool)
{
    _server->kill();
}

void
StopCommand::addCallback(const AMD_Server_stopPtr& amdCB)
{
    _stopCB.push_back(amdCB);
}

void
StopCommand::failed(const string& reason)
{
    stopTimer();
    ServerStopException ex(_server->getId(), reason);
    for(vector<AMD_Server_stopPtr>::const_iterator p = _stopCB.begin(); p != _stopCB.end(); ++p)
    {
        (*p)->ice_exception(ex);
    }	
    _stopCB.clear();
}

void
StopCommand::finished()
{
    stopTimer();
    for(vector<AMD_Server_stopPtr>::const_iterator p = _stopCB.begin(); p != _stopCB.end(); ++p)
    {
        (*p)->ice_response();
    }
    _stopCB.clear();
}

ServerI::ServerI(const NodeIPtr& node, const ServerPrx& proxy, const string& serversDir, const string& id, int wt) :
    _node(node),
    _this(proxy),
    _id(id),
    _waitTime(wt),
    _serverDir(serversDir + "/" + id),
    _disableOnFailure(0),
    _state(ServerI::Inactive),
    _activation(ServerI::Disabled),
    _failureTime(IceUtil::Time::now()), // Ensure that _activation gets initialized in updateImpl().
    _pid(0)
{
    assert(_node->getActivator());
    const_cast<int&>(_disableOnFailure) = 
	_node->getCommunicator()->getProperties()->getPropertyAsIntWithDefault("IceGrid.Node.DisableOnFailure", 0);

    //
    // Read the server application uuid and revision. This is to
    // ensure that a registry won't try to load an older server
    // definition than the one for which this server was loaded.
    //
    string idFilePath = _serverDir + "/revision";
    ifstream is(idFilePath.c_str());
    if(is.good())
    {
	char line[1024];
	is.getline(line, 1024); // Ignore comments
	is.getline(line, 1024);
	is.getline(line, 1024);
	string ignore;
	is >> ignore >> _info.uuid;
	is >> ignore >> _info.revision;
    }
    else
    {
	_info.revision = 0;
    }
}

ServerI::~ServerI()
{
}

void
ServerI::start_async(const AMD_Server_startPtr& amdCB, const Ice::Current&)
{
    start(Manual, amdCB);
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
	    throw ServerStopException(_id, "The server is already inactive.");
	}
	else if(_state == Destroying)
	{
	    throw ServerStopException(_id, "The server is being destroyed.");
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
    Lock sync(*this);
    checkDestroyed();
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
    checkDestroyed();
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
    bool activate = false;
    {
	Lock sync(*this);
	checkDestroyed();
	assert(_info.descriptor);

	if(enabled && _activation == Disabled)
	{
	    _failureTime = IceUtil::Time();
	    _activation = toServerActivation(_info.descriptor->activation);
	    activate = _state == Inactive && _activation == Always;
	}
	else if(!enabled && (_activation != Disabled || _failureTime != IceUtil::Time()))
	{
	    _failureTime = IceUtil::Time();
	    _activation = Disabled;
	    if(_timer)
	    {
		_node->getWaitQueue()->remove(_timer);
		_timer = 0;
	    }
	}
	else
	{
	    return; // Nothing to change!
	}

	_node->observerUpdateServer(getDynamicInfo());
    }

    if(activate)
    {
	try
	{
	    start(Always);
	}
	catch(const ServerStartException& ex)
	{
	    Ice::Error out(_node->getTraceLevels()->logger);
	    out << "couldn't reactivate server `" << _id << "' with `always' activation mode:\n" 
		<< ex.reason;
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
    }
}

bool
ServerI::isEnabled(const ::Ice::Current&) const
{
    Lock sync(*this);
    checkDestroyed();
    return _activation != Disabled;
}

void
ServerI::setProcess_async(const AMD_Server_setProcessPtr& amdCB, const Ice::ProcessPrx& process, const Ice::Current&)
{
    bool deact = false;
    {
	Lock sync(*this);
	checkDestroyed();
	_process = process;
	deact = _state == DeactivatingWaitForProcess;
    }
    amdCB->ice_response();
    if(deact)
    {
	deactivate();
    }
}

Ice::Long
ServerI::getOffsetFromEnd(const string& filename, int count, const Ice::Current&) const
{
    return _node->getFileCache()->getOffsetFromEnd(getFilePath(filename), count);
}

bool
ServerI::read(const string& filename, Ice::Long pos, int size, Ice::Long& newPos, Ice::StringSeq& lines, 
	      const Ice::Current&) const
{
    return _node->getFileCache()->read(getFilePath(filename), pos, size, newPos, lines);
}

bool
ServerI::isAdapterActivatable(const string& id) const
{
    Lock sync(*this);
    if(!_info.descriptor || _activation == Disabled)
    {
	return false;
    }

    if(_info.descriptor->activation == "manual" || 
       _info.descriptor->activation == "session" && _info.sessionId.empty())
    {
	return false;
    }

    if(_state <= WaitForActivation)
    {
	if(_activatedAdapters.find(id) != _activatedAdapters.end())
	{
	    return false; // The adapter was already activated once.
	}
	return true;
    }
    else if(_state < Deactivating)
    {
	return false; // The server is active or its activation timed out.
    }
    else if(_state < Destroying)
    {
	return true; // The server is being deactivated.
    }
    else
    {
	return false;
    }
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
    return _info.descriptor ? _info.descriptor->distrib : DistributionDescriptor();
}

void
ServerI::start(ServerActivation activation, const AMD_Server_startPtr& amdCB)
{
    ServerCommandPtr command;
    {
	Lock sync(*this);
	checkDestroyed();

	//
	// Eventually re-enable the server if it's disabled because of a failure.
	//
	if(_disableOnFailure > 0 && _failureTime != IceUtil::Time())
	{
	    if(activation == Manual ||
	       (_failureTime + IceUtil::Time::seconds(_disableOnFailure) < IceUtil::Time::now()))
	    {
		_activation = _previousActivation;
		_failureTime = IceUtil::Time();
	    }
	}

	//
	// Check the current activation mode and the requested activation.
	//
	if(_activation == Disabled)
	{
	    throw ServerStartException(_id, "The server is disabled.");
	}
	else if(_activation != Always && activation == Always)
	{
	    assert(!amdCB);
	    return; // Nothing to do.
	}
	else if(_activation == Manual && activation != Manual)
	{
	    throw ServerStartException(_id, "The server activation doesn't allow this activation mode.");
	}
	else if(_activation == Session && _info.sessionId.empty())
	{
	    throw ServerStartException(_id, "The server is not owned by a session.");
	}	

	//
	// Check the current state.
	//
	if(_state == ActivationTimeout)
	{
	    throw ServerStartException(_id, "The server activation timed out.");
	}
	else if(_state == Active)
	{
	    if(activation == Always)
	    {
		return; // Nothing to do, it's already active (and we
			// don't want to throw because it would be
			// considered as an error.)
	    }
	    throw ServerStartException(_id, "The server is already active.");
	}
	else if(_state == Destroying)
	{
	    throw ServerStartException(_id, "The server is being destroyed.");
	}

	if(_timer)
	{
	    _node->getWaitQueue()->remove(_timer);
	    _timer = 0;
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

ServerCommandPtr
ServerI::load(const AMD_Node_loadServerPtr& amdCB, const ServerInfo& info, bool fromMaster)
{
    Lock sync(*this);
    checkDestroyed();
    
    //
    // Don't reload the server if:
    //
    // - the application uuid, the application revision and the
    //   session id didn't change.
    //
    // - the load command if from a slave and the given descriptor
    //   is from another application or doesn't have the same
    //   version.
    //
    // - the descriptor and the session id didn't change.
    //
    // In any case, we update the server application revision if
    // it needs to be updated.
    //
    if(!_info.uuid.empty() && !fromMaster)
    {
	if(_info.uuid != info.uuid)
	{
	    DeploymentException ex;
	    ex.reason = "server descriptor from replica is from another application (`" + info.uuid + "')";
	    throw ex;
	}
	else if(_info.revision != info.revision)
	{
	    ostringstream os;
	    os << "server descriptor from replica has different version:\n";
	    os << "current revision: " << _info.revision << "\n";
	    os << "replica revision: " << info.revision;
	    throw DeploymentException(os.str());
	}
    }

    //
    // Otherwise, if the following conditions are met:
    //
    // - the server is already loaded.
    // - the descriptor is from the master and the session id didn't change or it's coming from a slave.
    // - the descriptor is the same as the one loaded.
    //
    // we don't re-load the server. We just return the server
    // proxy and the proxies of its adapters.
    // 
    if(_info.descriptor &&
       (!fromMaster || _info.sessionId == info.sessionId) &&
       (_info.uuid == info.uuid && _info.revision == info.revision || 
	descriptorEqual(_node->getCommunicator(), _info.descriptor, info.descriptor)))
    {
	if(_info.uuid != info.uuid || _info.revision != info.revision)
	{
	    _info.uuid = info.uuid;
	    _info.revision = info.revision;
	    updateRevisionFile();
	}

	if(amdCB)
	{
	    AdapterPrxDict adapters;
	    for(ServerAdapterDict::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
	    {
		adapters.insert(make_pair(p->first, p->second->getProxy()));
	    }
	    amdCB->ice_response(_this, adapters, _activationTimeout, _deactivationTimeout);
	}
	return 0;
    }

    assert(fromMaster || _info.uuid.empty() || _info.uuid == info.uuid && _info.revision == info.revision);
    if(!StopCommand::isStopped(_state) && !_stop)
    {
	_stop = new StopCommand(this, _node->getWaitQueue(), _deactivationTimeout);
    }
    if(!_load)
    {
	_load = new LoadCommand(this);
    }
    _load->setUpdate(info, _destroy);
    if(_destroy && _state != Destroying)
    {
	_destroy->finished();
	_destroy = 0;
    }
    if(amdCB)
    {
	_load->addCallback(amdCB);
    }
    return nextCommand();
}

ServerCommandPtr
ServerI::destroy(const AMD_Node_destroyServerPtr& amdCB, const string& uuid, int revision)
{
    Lock sync(*this);
    checkDestroyed();

    if(!uuid.empty()) // Empty if from checkConsistency.
    {
	if(_info.uuid.empty())
	{
	    amdCB->ice_response();
	    return 0; // Server doesn't exist.
	}
	else if(_info.uuid != uuid)
	{
	    DeploymentException ex;
	    ex.reason = "server descriptor from replica is from another application (`" + uuid + "')";
	    throw ex;
	}
	else if(_info.revision > revision)
	{
	    ostringstream os;
	    os << "server descriptor from replica is too old:\n";
	    os << "current revision: " << _info.revision << "\n";
	    os << "replica revision: " << revision;
	    throw DeploymentException(os.str());
	}
    }

    if(!StopCommand::isStopped(_state) && !_stop)
    {
	_stop = new StopCommand(this, _node->getWaitQueue(), _deactivationTimeout);
    }
    if(!_destroy)
    {
	//_destroy = new DestroyCommand(this, _state != Inactive && _state != Loading && _state != Patching);
	_destroy = new DestroyCommand(this, false);
    }
    if(amdCB)
    {
	_destroy->addCallback(amdCB);
    }
    return nextCommand();
}

bool
ServerI::startPatch(bool shutdown)
{
    ServerCommandPtr command;
    {
	Lock sync(*this);
	checkDestroyed();
	if(!StopCommand::isStopped(_state))
	{
	    if(!shutdown)
	    {
		return false;
	    }
	    else if(!_stop)
	    {
		_stop = new StopCommand(this, _node->getWaitQueue(), _deactivationTimeout);
	    }
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
#ifndef _WIN32
    {
	Lock sync(*this);
	try
	{
	    chownRecursive(_serverDir + "/distrib", _uid, _gid);
	}
	catch(const string& msg)
	{
	    Ice::Warning out(_node->getTraceLevels()->logger);
	    out << msg;
	}
    }
#endif
    setState(Inactive);
}

void
ServerI::adapterActivated(const string& id)
{
    ServerCommandPtr command;
    {
	Lock sync(*this);
	if(_state != ServerI::Activating && 
	   _state != ServerI::WaitForActivation &&
	   _state != ServerI::ActivationTimeout)
	{
	    return;
	}
	_activatedAdapters.insert(id);
	checkActivation();
	command = nextCommand();
    }
    if(command)
    {
	command->execute();
    }
}

void
ServerI::adapterDeactivated(const string& id)
{
    ServerCommandPtr command;
    {
	Lock sync(*this);
	if(_state == Active && _serverLifetimeAdapters.find(id) != _serverLifetimeAdapters.end())
	{
	    setStateNoSync(Deactivating);
	}
	command = nextCommand();
    }
    if(command)
    {
	command->execute();
    }
}

void
ServerI::checkDestroyed() const
{
    if(_state == Destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = _this->ice_getIdentity();
	throw ex;
    }
}

void
ServerI::disableOnFailure()
{
    //
    // If the server is already disabled, nothing to do.
    //
    if(_activation == Disabled)
    {
	return;
    }

    //
    // If disable on failure is configured or if the activation mode
    // is always and the server wasn't active at the time of the
    // failure we disable the server.
    //
    if(_disableOnFailure != 0 || _activation == Always && (_state == Activating || _state == WaitForActivation))
    {
	_previousActivation = _activation;
	_activation = Disabled;
	_failureTime = IceUtil::Time::now();
    }
}

void
ServerI::enableAfterFailure(bool force)
{
    if(_disableOnFailure == 0 || _failureTime == IceUtil::Time())
    {
	return;
    }

    if(force ||
       _disableOnFailure > 0 && (_failureTime + IceUtil::Time::seconds(_disableOnFailure) < IceUtil::Time::now()))
    {
	_activation = _previousActivation;
	_failureTime = IceUtil::Time();
    }

    if(_timer)
    {
	_node->getWaitQueue()->remove(_timer);
	_timer = 0;
    }
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
	    p->second->activationFailed(destroyed ? "server destroyed" : "server activation timed out");
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
ServerI::activate()
{
    ServerInfo info;
    ServerAdapterDict adpts;
    bool waitForReplication;

#ifndef _WIN32
    uid_t uid;
    gid_t gid;
#endif
    {
	Lock sync(*this);
	assert(_state == Activating && _info.descriptor);
	info = _info;
	adpts = _adapters;

	//
	// The first time the server is started, we ensure that the
	// replication of its descriptor is completed. This is to make
	// sure all the replicas are up to date when the server
	// starts for the first time with a given descriptor.
	//
	waitForReplication = _waitForReplication;
	_waitForReplication = false;

	_process = 0;
	
#ifndef _WIN32
	uid = _uid;
	gid = _gid;
#endif
    }

    //
    // We first ensure that the application is replicated on all the
    // registries before to start the server. We only do this each
    // time the server is updated or the initialy loaded on the node.
    //
    if(waitForReplication)
    {
	NodeSessionPrx session = _node->getMasterNodeSession();
	if(session)
	{
	    AMI_NodeSession_waitForApplicationUpdatePtr cb = new WaitForApplicationUpdateCB(this);
	    _node->getMasterNodeSession()->waitForApplicationUpdate_async(cb, info.uuid, info.revision);
	    return;
	}
    }

    //
    // Compute the server command line options.
    //
    ServerDescriptorPtr desc = info.descriptor;
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
#ifndef _WIN32
	int pid = _node->getActivator()->activate(desc->id, desc->exe, desc->pwd, uid, gid, options, envs, this);
#else
	int pid = _node->getActivator()->activate(desc->id, desc->exe, desc->pwd, options, envs, this);
#endif
	ServerCommandPtr command;
	{
	    Lock sync(*this);
	    assert(_state == Activating);
	    _pid = pid;
	    setStateNoSync(ServerI::WaitForActivation);
	    checkActivation();
	    command = nextCommand();
	    notifyAll(); // Terminated might be waiting for the state change.
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

    {
	Lock sync(*this);
	disableOnFailure();
	setStateNoSync(ServerI::Deactivating, failure);
    }
    
    for(ServerAdapterDict::iterator r = adpts.begin(); r != adpts.end(); ++r)
    {
	try
	{
	    r->second->activationFailed(failure);
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
    }

    setState(ServerI::Inactive);
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

	//
	// If a process object is supposed to be registered and it's
	// not set yet, we wait for the server to set this process
	// object before attempting to deactivate the server again.
	//
	if(_processRegistered && !_process)
	{
	    setStateNoSync(ServerI::DeactivatingWaitForProcess);
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

    _node->removeServer(this, _info.application, _info.descriptor->applicationDistrib);
    
    try
    {
	IcePatch2::removeRecursive(_serverDir);
    }
    catch(const string& msg)
    {
	Ice::Warning out(_node->getTraceLevels()->logger);
	out << "removing server directory `" << _serverDir << "' failed: " << msg;
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
 	while(_state == ServerI::Activating)
 	{
 	    wait(); // Wait for activate() to set the state to WaitForActivation
 	}

	adpts = _adapters;
	_activatedAdapters.clear();
	_pid = 0;

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
	    disableOnFailure();
	}

	if(_state != ServerI::Deactivating && 
	   _state != ServerI::DeactivatingWaitForProcess && 
	   _state != ServerI::Destroying)
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
	    os << (msg.empty() ? string(".") : ":\n" + msg);
	    setStateNoSync(ServerI::Deactivating, os.str());
	}
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

    bool doDestroy = false;
    ServerCommandPtr command;
    {
	Lock sync(*this);
	if(_state == ServerI::Destroying)
	{
	    doDestroy = true;
	}
	else
	{
	    setStateNoSync(ServerI::Inactive);
	    command = nextCommand();
	}
    }
    if(doDestroy)
    {
	destroy();
    }
    else if(command)
    {
	command->execute();
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

	ServerInfo oldInfo = _info;
	try
	{
	    if(_load->clearDir())
	    {
		//
		// The server was explicitely destroyed then updated,
		// we first need to cleanup the directory to remove
		// any user created files.
		//
		try
		{
		    IcePatch2::removeRecursive(_serverDir);
		}
		catch(const string&)
		{
		}
	    }

	    try
	    {
		updateImpl(_load->getServerInfo());
	    }
	    catch(const Ice::Exception& ex)
	    {
		ostringstream os;
		os << ex;
		throw DeploymentException(os.str());
	    }
	    catch(const string& msg)
	    {
		throw DeploymentException(msg);
	    }
	    catch(const char* msg)
	    {
		throw DeploymentException(msg);
	    }

	    if(oldInfo.descriptor)
	    {
		_node->removeServer(this, oldInfo.application, oldInfo.descriptor->applicationDistrib);
	    }
	    _node->addServer(this, _info.application, _info.descriptor->applicationDistrib);

	    AdapterPrxDict adapters;
	    for(ServerAdapterDict::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
	    {
		adapters.insert(make_pair(p->first, p->second->getProxy()));
	    }    
	    _load->finished(_this, adapters, _activationTimeout, _deactivationTimeout);
	}
	catch(const DeploymentException& ex)
	{
	    //
	    // Rollback old descriptor.
	    //
	    try
	    {
		updateImpl(oldInfo);
	    }
	    catch(const Ice::Exception& e)
	    {
		Ice::Warning out(_node->getTraceLevels()->logger);
		out << "update failed:\n" << ex.reason << "\nand couldn't rollback old descriptor:\n" << e;
	    }
	    catch(const string& msg)
	    {
		Ice::Warning out(_node->getTraceLevels()->logger);
		out << "update failed:\n" << ex.reason << "\nand couldn't rollback old descriptor:\n" << msg;
	    }
	    catch(const char* msg)
	    {
		Ice::Warning out(_node->getTraceLevels()->logger);
		out << "update failed:\n" << ex.reason << "\nand couldn't rollback old descriptor:\n" << msg;
	    }
	    _load->failed(ex);
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
ServerI::updateImpl(const ServerInfo& info)
{
    assert(_load);

    //
    // Remember if the server was just released by a session, this
    // will be used later to not update the configuration on the disk
    // (as an optimization and to allow users to review the
    // configuration file after allocating a server -- that's useful
    // if the server configuration is bogus and the session server 
    // can't start).
    //
    bool serverSessionReleased = _info.descriptor && _info.descriptor->activation == "session" && 
	_info.revision == info.revision && !_info.sessionId.empty() && info.sessionId.empty();

    _info = info;
    _waitForReplication = true;

    if(!_info.descriptor)
    {
	return;
    }

    //
    // Create the object adapter objects if necessary.
    //
    _processRegistered = false;
    ServerAdapterDict oldAdapters;
    oldAdapters.swap(_adapters);
    _serverLifetimeAdapters.clear();
    AdapterDescriptorSeq::const_iterator r;
    for(r = _info.descriptor->adapters.begin(); r != _info.descriptor->adapters.end(); ++r)
    {
	assert(!r->id.empty());
	oldAdapters.erase(addAdapter(*r, _info.descriptor));
	_processRegistered |= r->registerProcess;
    }
    IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(_info.descriptor);
    if(iceBox)
    {
	ServiceInstanceDescriptorSeq::const_iterator s;
	for(s = iceBox->services.begin(); s != iceBox->services.end(); ++s)
	{
	    CommunicatorDescriptorPtr svc = s->descriptor;
	    for(r = svc->adapters.begin(); r != svc->adapters.end(); ++r)
	    {
		assert(!r->id.empty());
		oldAdapters.erase(addAdapter(*r, svc));
		_processRegistered |= r->registerProcess;
	    }
	}
    }

    //
    // Remove old object adapters.
    //
    for(ServerAdapterDict::const_iterator t = oldAdapters.begin(); t != oldAdapters.end(); ++t)
    {
	try
	{
	    t->second->destroy();
	}
	catch(const Ice::ObjectAdapterDeactivatedException&)
	{
	    // IGNORE
	}
	catch(const Ice::LocalException& ex)
	{
	    Ice::Error out(_node->getTraceLevels()->logger);
	    out << "couldn't destroy adapter `" << t->first << "':\n" << ex;
	}
    }

    //
    // If the server was disabled because it failed (or because it's
    // the first time it's being updated). Set the activation mode
    // based on the descriptor activation. Otherwise, if the server is
    // disabled and failure time isn't set, we don't change the
    // activation since the user explicitely disabled the server.
    //
    if(_activation != Disabled || _failureTime != IceUtil::Time())
    {
	_activation = toServerActivation(_info.descriptor->activation);
	_failureTime = IceUtil::Time();
    }

    if(_timer)
    {
	_node->getWaitQueue()->remove(_timer);
	_timer = 0;
    }	

    //
    // Don't change the user if the server has the session activation
    // mode and if it's not currently owned by a session.
    //
    string user;
    if(_info.descriptor->activation != "session" || !_info.sessionId.empty())
    {
	user = _info.descriptor->user;
#ifndef _WIN32
	//
	// Check if the node is running as root, if that's the case we
	// make sure that a user is set for the process.
	//
	if(getuid() == 0 && user.empty())
	{
	    //
	    // If no user is configured and if this server is owned by
	    // a session we set the user to the session id, otherwise
	    // we set it to "nobody".
	    //
	    user = !_info.sessionId.empty() ? _info.sessionId : "nobody";
	}
#endif
    }

#ifndef _WIN32
    bool newUser = false;
#endif

    if(!user.empty())
    {
	UserAccountMapperPrx mapper = _node->getUserAccountMapper();
	if(mapper)
	{
	    try
	    {
		user = mapper->getUserAccount(user);
	    }
	    catch(const UserAccountNotFoundException&)
	    {
		throw "couldn't find user account for user `" + user + "'";
	    }
	    catch(const Ice::LocalException& ex)
	    {
		ostringstream os;
		os << "unexpected exception while trying to find user account for user `" << user << "':\n" << ex;
		throw os.str();
	    }
	}

#ifdef _WIN32
	//
	// Windows doesn't support running processes under another
	// account (at least not easily, see the CreateProcessAsUser
	// documentation). So if a user is specified, we just check
	// that the node is running under the same user account as the
	// one which is specified.
	//	
	vector<char> buf(256);
	buf.resize(256);
	DWORD size = static_cast<DWORD>(buf.size());
	bool success = GetUserName(&buf[0], &size);
	if(!success && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
	    buf.resize(size);
	    success = GetUserName(&buf[0], &size);
	}
	if(!success)
	{
	    Ice::SyscallException ex(__FILE__, __LINE__);
	    ex.error = getSystemErrno();
	    throw ex;
	}
	if(user != string(&buf[0]))
	{
	    throw "couldn't load server under user account `" + user + "': feature not supported on Windows";
	}
#else
	//
	// Get the uid/gid associated with the given user.
	//
	struct passwd* pw = getpwnam(user.c_str());
	if(!pw)
	{
	    throw "unknown user account `" + user + "'";
	}

	//
	// If the node isn't running as root and if the uid of the
	// configured user is different from the uid of the userr
	// running the node we throw, a regular user can't run a
	// process as another user.
	//
	uid_t uid = getuid();
	if(uid != 0 && pw->pw_uid != uid)
	{
	    throw "node has insufficient privileges to load server under user account `" + user + "'";
	}

	if(pw->pw_uid == 0) // Don't allow running proccesses as "root"
	{
	    throw "running server as `root' is not allowed";
	}

	newUser = _uid != pw->pw_uid || _gid != pw->pw_gid;
	_uid = pw->pw_uid;
	_gid = pw->pw_gid;
#endif
    }
#ifndef _WIN32
    else
    {	 
	//
	// If no user is specified, we'll run the process as the
	// current user.
	//
	uid_t uid = getuid();
	uid_t gid = getgid();
	newUser = _uid != uid || _gid != gid;
	_uid = uid;
	_gid = gid;
    }
#endif

    istringstream at(_info.descriptor->activationTimeout);
    if(!(at >> _activationTimeout) || !at.eof() || _activationTimeout == 0)
    {
	_activationTimeout = _waitTime;
    }
    istringstream dt(_info.descriptor->deactivationTimeout);
    if(!(dt >> _deactivationTimeout) || !dt.eof() || _deactivationTimeout == 0)
    {
	_deactivationTimeout = _waitTime;
    }

    //
    // Cache the path of each log file.
    //
    _logs.clear();
    LogDescriptorSeq::const_iterator l;
    for(l = _info.descriptor->logs.begin(); l != _info.descriptor->logs.end(); ++l)
    {
	_logs.insert(IcePatch2::simplify(l->path));
    }
    if(iceBox)
    {
	ServiceInstanceDescriptorSeq::const_iterator s;
	for(s = iceBox->services.begin(); s != iceBox->services.end(); ++s)
	{
	    CommunicatorDescriptorPtr svc = s->descriptor;
	    for(l = svc->logs.begin(); l != svc->logs.end(); ++l)
	    {
		_logs.insert(IcePatch2::simplify(l->path));
	    }
	}
    }

    //
    // Cache the path of the stderr/stdout file.
    //
    string outputDir = _node->getOutputDir();
    _stdOutFile = outputDir.empty() ? string() : (outputDir + "/" + _id + ".out");
    string suffix = _node->getRedirectErrToOut() ? ".out" : ".err";
    _stdErrFile = outputDir.empty() ? string() : (outputDir + "/" + _id + suffix);
    PropertyDescriptorSeq::const_iterator s;
    for(s = _info.descriptor->propertySet.properties.begin(); s != _info.descriptor->propertySet.properties.end(); ++s)
    {
	if(s->name == "Ice.StdErr")
	{
	    _stdErrFile = s->value;
	}
	else if(s->name == "Ice.StdOut")
	{
	    _stdOutFile = s->value;	    
	}
    }

    //
    // If the server is a session server and it wasn't udpated but
    // just released by a session, we don't update the configuration,
    // it will be done when the server is re-allocated.
    //
    if(serverSessionReleased)
    {
	return;
    }

    //
    // Update the revision file.
    //
    updateRevisionFile();

    //
    // Create or update the server directories exists.
    //
    createOrUpdateDirectory(_serverDir);
    createOrUpdateDirectory(_serverDir + "/config");
    createOrUpdateDirectory(_serverDir + "/dbs");
    createOrUpdateDirectory(_serverDir + "/distrib");

    //
    // Update the configuration file(s) of the server if necessary.
    //
    Ice::StringSeq knownFiles;
    updateConfigFile(_serverDir, _info.descriptor);
    knownFiles.push_back("config");
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
    // Update the database environments if necessary.
    //
    Ice::StringSeq knownDbEnvs;
    for(DbEnvDescriptorSeq::const_iterator q = _info.descriptor->dbEnvs.begin(); 
	q != _info.descriptor->dbEnvs.end(); ++q)
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
    // Remove old configuration files.
    //
    Ice::StringSeq files = IcePatch2::readDirectory(_serverDir + "/config");
    Ice::StringSeq toDel;
    set_difference(files.begin(), files.end(), knownFiles.begin(), knownFiles.end(), back_inserter(toDel));
    Ice::StringSeq::const_iterator p;
    for(p = toDel.begin(); p != toDel.end(); ++p)
    {
	if(p->find("config_") == 0)
	{
	    try
	    {
		IcePatch2::remove(_serverDir + "/config/" + *p);
	    }
	    catch(const string& msg)
	    {
		Ice::Warning out(_node->getTraceLevels()->logger);
		out << "couldn't remove file `" + _serverDir + "/config/" + *p + "':\n" + msg;
	    }
	}
    }

    //
    // Remove old database environments.
    //
    Ice::StringSeq dbEnvs = IcePatch2::readDirectory(_serverDir + "/dbs");
    toDel.clear();
    set_difference(dbEnvs.begin(), dbEnvs.end(), knownDbEnvs.begin(), knownDbEnvs.end(), back_inserter(toDel));
    for(p = toDel.begin(); p != toDel.end(); ++p)
    {
	try
	{
	    IcePatch2::removeRecursive(_serverDir + "/dbs/" + *p);
	}
	catch(const string& msg)
	{
	    Ice::Warning out(_node->getTraceLevels()->logger);
	    out << "couldn't remove directory `" + _serverDir + "/dbs/" + *p + "':\n" + msg;
	}
    }

#ifndef _WIN32
    if(newUser)
    {
	chownRecursive(_serverDir + "/config", _uid, _gid);
	chownRecursive(_serverDir + "/dbs", _uid, _gid);
	chownRecursive(_serverDir + "/distrib", _uid, _gid);
    }
#endif
}

void
ServerI::updateRevisionFile()
{
    string idFilePath = _serverDir + "/revision";
    ofstream os(idFilePath.c_str());
    if(os.good())
    {
	os << "#" << endl;
	os << "# This server belongs to the application `" << _info.application << "'" << endl;
	os << "#" << endl;
	os << "uuid: " << _info.uuid << endl;
	os << "revision: " << _info.revision << endl;
    }
}

void
ServerI::checkActivation()
{
    //assert(locked());
    if(_state == ServerI::WaitForActivation || _state == ServerI::ActivationTimeout)
    {
	if(includes(_activatedAdapters.begin(), _activatedAdapters.end(),
		    _serverLifetimeAdapters.begin(), _serverLifetimeAdapters.end()))
	{
	    setStateNoSync(ServerI::Active);
	}
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
    if(_stop && _stop->canExecute(_state))
    {
	command = _stop;
    }
    else if(_destroy && _destroy->canExecute(_state))
    {
	command = _destroy;
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
	//assert(_stop && _stop->canExecute(_state));
	break;
    case DeactivatingWaitForProcess:
	assert(_state == Deactivating);
	break;
    case Destroying:
	assert(_state == Inactive && _destroy && _destroy->canExecute(_state));
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
	    _start->failed(reason.empty() ? string("The server is being deactivated.") : reason);
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
	if(_stop)
	{
	    _stop->failed("The server is being destroyed.");
	    _stop = 0;
	}
	break;
    case Destroyed:
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
	//
	// If the server is destroyed and there's no load command, we
	// remove the servant from the ASM.
	//
	try
	{
	    _node->getAdapter()->remove(_this->ice_getIdentity());
	}
	catch(const Ice::ObjectAdapterDeactivatedException&)
	{
	    // IGNORE
	}
	_info = ServerInfo();
    }
    else if(_state == Inactive)
    {
	if(_activation == Always)
	{
	    _timer = new DelayedStart(this, _node->getTraceLevels());
	    _node->getWaitQueue()->add(_timer, IceUtil::Time::milliSeconds(500));
	}
	else if(_activation == Disabled && _disableOnFailure > 0 && _failureTime != IceUtil::Time())
	{
	    //
	    // If the server was disabled because it failed, we
	    // schedule a callback to re-enable it. We add 500ms to
	    // the disable on failure duration to make sure that the
	    // server will be ready to be reactivated when the
	    // callback is executed.  
	    //
	    _timer = new DelayedStart(this, _node->getTraceLevels());
	    _node->getWaitQueue()->add(_timer, 
				       IceUtil::Time::seconds(_disableOnFailure) + IceUtil::Time::milliSeconds(500));
	}
    }

    //
    // Don't send the server update if the state didn't change or if
    // the server couldn't be forked.
    //
    if(toServerState(previous) != toServerState(_state) && !(previous == Inactive && _state == Deactivating))
    {
	_node->observerUpdateServer(getDynamicInfo());
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
    id.name = _id + "-" + desc.id;
    try
    {
	AdapterPrx proxy = AdapterPrx::uncheckedCast(_node->getAdapter()->createProxy(id));
	ServerAdapterIPtr servant = ServerAdapterIPtr::dynamicCast(_node->getAdapter()->find(id));
	if(!servant)
	{
	    servant = new ServerAdapterI(_node, this, _id, proxy, desc.id);
	    _node->getAdapter()->add(servant, id);
	}
 	_adapters.insert(make_pair(desc.id, servant));
	if(desc.serverLifetime)
	{
	    _serverLifetimeAdapters.insert(desc.id);
	}
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
	// IGNORE
    }
    catch(const Ice::LocalException& ex)
    {
	Ice::Error out(_node->getTraceLevels()->logger);
	out << "couldn't add adapter `" << desc.id << "':\n" << ex;
    }

    return desc.id;
}

void
ServerI::updateConfigFile(const string& serverDir, const CommunicatorDescriptorPtr& descriptor)
{
    string configFilePath;

    PropertyDescriptorSeq props;
    if(ServerDescriptorPtr::dynamicCast(descriptor))
    {
	ServerDescriptorPtr svrDesc = ServerDescriptorPtr::dynamicCast(descriptor);

	configFilePath = serverDir + "/config/config";

	//
	// Add server properties.
	//
	props.push_back(createProperty("# Server configuration"));
	props.push_back(createProperty("Ice.ServerId", _id));
	props.push_back(createProperty("Ice.ProgramName", _id));
	props.push_back(createProperty("Ice.Default.Locator",
				       _node->getCommunicator()->getProperties()->getProperty("Ice.Default.Locator")));

	//
	// Add Ice.StdOut, Ice.StdErr if necessary.
	//
	string outputDir = _node->getOutputDir();
	if(!outputDir.empty())
	{
	    props.push_back(createProperty("Ice.StdOut", outputDir + "/" + _id + ".out"));
	    string suffix = _node->getRedirectErrToOut() ? ".out" : ".err";
	    props.push_back(createProperty("Ice.StdErr", outputDir + "/" + _id + suffix));
	}

	//
	// Add server descriptor properties.
	//
	copy(svrDesc->propertySet.properties.begin(), svrDesc->propertySet.properties.end(), back_inserter(props));

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
	ServiceDescriptorPtr svcDesc = ServiceDescriptorPtr::dynamicCast(descriptor);
	assert(svcDesc);
	configFilePath = serverDir + "/config/config_" + svcDesc->name;
	props.push_back(createProperty("# Service configuration"));
	copy(svcDesc->propertySet.properties.begin(), svcDesc->propertySet.properties.end(), back_inserter(props));
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
	    //
	    // DEPREACTED PROPERTIES: Removed extra code in future release
	    //
	    props.push_back(createProperty("# Object adapter " + q->name));
	    if(!q->id.empty())
	    {
		props.push_back(createProperty("Ice.OA." + q->name + ".AdapterId", q->id));
		props.push_back(createProperty(q->name + ".AdapterId", q->id));
		if(!q->replicaGroupId.empty())
		{
		    props.push_back(createProperty("Ice.OA." + q->name + ".ReplicaGroupId", q->replicaGroupId));
		    props.push_back(createProperty(q->name + ".ReplicaGroupId", q->replicaGroupId));
		}
	    }
	    if(q->registerProcess)
	    {
		props.push_back(createProperty("Ice.OA." + q->name + ".RegisterProcess", "1"));
		props.push_back(createProperty(q->name + ".RegisterProcess", "1"));
	    }
	}

	ObjectDescriptorSeq::const_iterator o;
	for(o = q->objects.begin(); o != q->objects.end(); ++o)
	{
	    if(!o->property.empty())
	    {
		props.push_back(createProperty(o->property, _node->getCommunicator()->identityToString(o->id)));
	    }
	}
	for(o = q->allocatables.begin(); o != q->allocatables.end(); ++o)
	{
	    if(!o->property.empty())
	    {
		props.push_back(createProperty(o->property, _node->getCommunicator()->identityToString(o->id)));
	    }
	}
    }

    //
    // Add log properties.
    //
    if(!descriptor->logs.empty())
    {
	props.push_back(createProperty("# Log paths"));
	for(LogDescriptorSeq::const_iterator l = descriptor->logs.begin(); l != descriptor->logs.end(); ++l)
	{
	    if(!l->property.empty())
	    {
		props.push_back(createProperty(l->property, l->path));
	    }
	}
    }

    ofstream configfile(configFilePath.c_str());
    if(!configfile.good())
    {
	throw "couldn't create configuration file: " + configFilePath;
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
	createOrUpdateDirectory(dbEnvHome);
    }

    if(!dbEnv.properties.empty())
    {
	string file = dbEnvHome + "/DB_CONFIG";
	ofstream configfile(file.c_str());
	if(!configfile.good())
	{
	    throw "couldn't create configuration file `" + file + "'";
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
}

PropertyDescriptor
ServerI::createProperty(const string& name, const string& value)
{
    PropertyDescriptor prop;
    prop.name = name;
    prop.value = value;
    return prop;
}

void
ServerI::createOrUpdateDirectory(const string& dir)
{
    try
    {
	IcePatch2::createDirectory(dir);
    }
    catch(const string&)
    {
    }
#ifndef _WIN32
    if(chown(dir.c_str(), _uid, _gid) != 0)
    {
	throw "can't set permissions on directory `" + dir + "'";
    }    
#endif
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
	return IceGrid::Activating;
    case ServerI::ActivationTimeout:
	return IceGrid::ActivationTimedOut;
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

ServerI::ServerActivation
ServerI::toServerActivation(const string& activation) const
{
    if(activation == "on-demand")
    {
	return OnDemand;
    }
    else if(activation == "session")
    {
	return Session;
    }
    else if(activation == "always")
    {
	return Always;
    }
    else if(activation == "manual" || activation.empty())
    {
	return Manual;
    }
    else
    {
	Ice::Warning out(_node->getTraceLevels()->logger);
	out << "unknown activation mode `" << activation << "' for server `" << _id << "'";
	return Manual;
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
    info.enabled = _activation != Disabled;
    return info;
}

string
ServerI::getFilePath(const string& filename) const
{
    if(filename == "stderr")
    {
	if(_stdErrFile.empty())
	{
	    throw FileNotAvailableException("Ice.StdErr configuration property is not set");
	}
	return _stdErrFile;
    }
    else if(filename == "stdout")
    {
	if(_stdOutFile.empty())
	{
	    throw FileNotAvailableException("Ice.StdOut configuration property is not set");
	}
	return _stdOutFile;
    }
    else if(!filename.empty() && filename[0] == '#')
    {
	string path = IcePatch2::simplify(filename.substr(1));
	if(_logs.find(path) == _logs.end())
	{
	    throw FileNotAvailableException("unknown log file `" + path + "'");
	}
	return path;
    }
    else
    {
	throw FileNotAvailableException("unknown file");
    }
}

