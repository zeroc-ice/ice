//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/FileUtil.h>
#include <Ice/Ice.h>
#include <IceGrid/ServerI.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/Activator.h>
#include <IceGrid/NodeI.h>
#include <IceGrid/Util.h>
#include <IceGrid/ServerAdapterI.h>
#include <IceGrid/DescriptorHelper.h>

#include <IcePatch2Lib/Util.h>
#include <IceUtil/FileUtil.h>

#include <sys/types.h>
#include <fstream>

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

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

#ifndef _WIN32
void
chownRecursive(const string& path, uid_t uid, gid_t gid)
{
    vector<vector<Ice::Byte> > namelist;
    DIR* d;
    if((d = opendir(path.c_str())) == 0)
    {
        throw runtime_error("cannot read directory `" + path + "':\n" + IceUtilInternal::lastErrorToString());
    }

    struct dirent* entry;
    while((entry = readdir(d)) != 0)
    {
        size_t index = namelist.size();
        namelist.resize(index + 1);

        size_t entrysize = sizeof(struct dirent) - sizeof(entry->d_name) + strlen(entry->d_name) + 1;
        namelist[index].resize(entrysize);

        memcpy(&namelist[index][0], entry, entrysize);
    }

    if(closedir(d))
    {
        throw runtime_error("cannot read directory `" + path + "':\n" + IceUtilInternal::lastErrorToString());
    }

    for(size_t i = 0; i < namelist.size(); ++i)
    {
        string name = reinterpret_cast<struct dirent*>(&namelist[i][0])->d_name;
        assert(!name.empty());

        if(name == ".")
        {
            if(chown(path.c_str(), uid, gid) != 0)
            {
                throw runtime_error("can't change permissions on `" + name + "':\n" + IceUtilInternal::lastErrorToString());
            }
        }
        else if(name != "..")
        {
            name = path + "/" + name;

            IceUtilInternal::structstat buf;
            if(IceUtilInternal::stat(name, &buf) == -1)
            {
                throw runtime_error("cannot stat `" + name + "':\n" + IceUtilInternal::lastErrorToString());
            }

            if(S_ISDIR(buf.st_mode))
            {
                chownRecursive(name, uid, gid);
            }
            else
            {
                if(chown(name.c_str(), uid, gid) != 0)
                {
                    throw runtime_error("can't change permissions on `" + name + "':\n" + IceUtilInternal::lastErrorToString());
                }
            }
        }
    }
}
#endif

static bool
descriptorUpdated(const InternalServerDescriptorPtr& lhs, const InternalServerDescriptorPtr& rhs, bool noProps = false)
{
    if(lhs->uuid == rhs->uuid && lhs->revision == rhs->revision)
    {
        return false;
    }

    if(lhs->id != rhs->id ||
       lhs->application != rhs->application ||
       lhs->uuid != rhs->uuid ||
       lhs->sessionId != rhs->sessionId ||
       lhs->exe != rhs->exe ||
       lhs->pwd != rhs->pwd ||
       lhs->user != rhs->user ||
       lhs->activation != rhs->activation ||
       lhs->activationTimeout != rhs->activationTimeout ||
       lhs->deactivationTimeout != rhs->deactivationTimeout ||
       lhs->applicationDistrib != rhs->applicationDistrib ||
       lhs->processRegistered != rhs->processRegistered ||
       lhs->options != rhs->options ||
       lhs->envs != rhs->envs ||
       lhs->logs != rhs->logs)
    {
        return true;
    }

    if((!lhs->distrib && rhs->distrib) || (lhs->distrib && !rhs->distrib))
    {
        return true;
    }
    else if(lhs->distrib && rhs->distrib)
    {
        if(lhs->distrib->icepatch != rhs->distrib->icepatch ||
           lhs->distrib->directories != rhs->distrib->directories)
        {
            return true;
        }
    }

    if(lhs->adapters.size() != rhs->adapters.size())
    {
        return true;
    }
    else
    {
        InternalAdapterDescriptorSeq::const_iterator q = rhs->adapters.begin();
        for(InternalAdapterDescriptorSeq::const_iterator p = lhs->adapters.begin(); p != lhs->adapters.end(); ++p, ++q)
        {
            if((*p)->id != (*q)->id || (*p)->serverLifetime != (*q)->serverLifetime)
            {
                return true;
            }
        }
    }

    if(lhs->dbEnvs.size() != rhs->dbEnvs.size())
    {
        return true;
    }
    else
    {
        InternalDbEnvDescriptorSeq::const_iterator q = rhs->dbEnvs.begin();
        for(InternalDbEnvDescriptorSeq::const_iterator p = lhs->dbEnvs.begin(); p != lhs->dbEnvs.end(); ++p, ++q)
        {
            if((*p)->name != (*q)->name || (*p)->properties != (*q)->properties)
            {
                return true;
            }
        }
    }

    if(!noProps && lhs->properties != rhs->properties)
    {
        return true;
    }

    return false;
}

Ice::PropertyDict
toPropertyDict(const PropertyDescriptorSeq& seq)
{
    Ice::PropertyDict props;
    for(PropertyDescriptorSeq::const_iterator q = seq.begin(); q != seq.end(); ++q)
    {
        if(q->value.empty() && q->name.find('#') == 0)
        {
            continue; // Ignore comments.
        }

        if(q->value.empty())
        {
            props.erase(q->name);
        }
        else
        {
            props[q->name] = q->value;
        }
    }
    return props;
}

class CommandTimeoutTimerTask : public IceUtil::TimerTask
{
public:

    CommandTimeoutTimerTask(const TimedServerCommandPtr& command) : _command(command)
    {
    }

    virtual void runTimerTask()
    {
        _command->timeout();
    }

private:

    const TimedServerCommandPtr _command;
};

class DelayedStart : public IceUtil::TimerTask
{
public:

    DelayedStart(const ServerIPtr& server, const TraceLevelsPtr& traceLevels) :
        _server(server),
        _traceLevels(traceLevels)
    {
    }

    virtual void runTimerTask()
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

private:

    const ServerIPtr _server;
    const TraceLevelsPtr _traceLevels;
};

class ResetPropertiesCB : public IceUtil::Shared
{
public:

    ResetPropertiesCB(const ServerIPtr& server,
                      const Ice::ObjectPrx admin,
                      const InternalServerDescriptorPtr& desc,
                      const InternalServerDescriptorPtr& old,
                      const TraceLevelsPtr& traceLevels) :
        _server(server),
        _admin(admin),
        _desc(desc),
        _traceLevels(traceLevels),
        _properties(server->getProperties(desc)),
        _oldProperties(server->getProperties(old)),
        _p(_properties.begin())
    {
    }

    void
    execute()
    {
        assert(_p != _properties.end());
        next();
    }

private:

    void
    next()
    {
        while(_p != _properties.end() && _p->second == _oldProperties[_p->first])
        {
            ++_p;
        }
        if(_p == _properties.end())
        {
            _server->updateRuntimePropertiesCallback(_desc);
            return;
        }

        Ice::PropertyDict oldProps = toPropertyDict(_oldProperties[_p->first]);
        Ice::PropertyDict props = toPropertyDict(_p->second);
        for(Ice::PropertyDict::const_iterator q = oldProps.begin(); q != oldProps.end(); ++q)
        {
            if(props.find(q->first) == props.end())
            {
                props[q->first] = "";
            }
        }

        string facet;
        if(_p->first == "config")
        {
            facet = "Properties";
            if(_traceLevels->server > 1)
            {
                const string id = _server->getId();
                Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
                out << "updating runtime properties for server `" << id << "'";
            }
        }
        else
        {
            assert(_p->first.find("config_") == 0);
            const string service = _p->first.substr(7);
            if(getPropertyAsInt(_properties["config"], "IceBox.UseSharedCommunicator." + service) > 0)
            {
                facet = "IceBox.SharedCommunicator.Properties";
            }
            else
            {
                facet = "IceBox.Service." + service + ".Properties";
            }
            if(_traceLevels->server > 1)
            {
                const string id = _server->getId();
                Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
                out << "updating runtime properties for service `" << service << "' from server `" + id + "'";
            }
        }

        //
        // Increment the iterator *before* invoking setProperties_async to avoid a
        // race condition.
        //
        ++_p;

        Ice::PropertiesAdminPrx p = Ice::PropertiesAdminPrx::uncheckedCast(_admin, facet);
        p->begin_setProperties(props, Ice::newCallback_PropertiesAdmin_setProperties(this,
                                                                                     &ResetPropertiesCB::next,
                                                                                     &ResetPropertiesCB::exception));
    }

    void
    exception(const Ice::Exception& ex)
    {
        _server->updateRuntimePropertiesCallback(ex, _desc);
    }

    const ServerIPtr _server;
    const Ice::ObjectPrx _admin;
    const InternalServerDescriptorPtr _desc;
    const TraceLevelsPtr _traceLevels;
    PropertyDescriptorSeqDict _properties;
    PropertyDescriptorSeqDict _oldProperties;
    PropertyDescriptorSeqDict::const_iterator _p;
};
typedef IceUtil::Handle<ResetPropertiesCB> ResetPropertiesCBPtr;

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
        vector<wchar_t> buf;
        buf.resize(32767);
        while((beg = v.find("%", beg)) != string::npos && beg < v.size() - 1)
        {
            end = v.find("%", beg + 1);
            if(end == string::npos)
            {
                break;
            }
            string variable = v.substr(beg + 1, end - beg - 1);
            DWORD ret = GetEnvironmentVariableW(Ice::stringToWstring(variable).c_str(), &buf[0],
                                                static_cast<DWORD>(buf.size()));
            string valstr = (ret > 0 && ret < buf.size()) ? Ice::wstringToString(&buf[0]) : string("");
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
                while((isalnum(static_cast<unsigned char>(v[end])) || v[end] == '_')  && end < v.size())
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

TimedServerCommand::TimedServerCommand(const ServerIPtr& server, const IceUtil::TimerPtr& timer, int timeout) :
    ServerCommand(server), _timer(timer), _timeout(timeout)
{
}

void
TimedServerCommand::startTimer()
{
    _timerTask = new CommandTimeoutTimerTask(this);
    try
    {
        _timer->schedule(_timerTask, IceUtil::Time::seconds(_timeout));
    }
    catch(const IceUtil::Exception&)
    {
        // Ignore, timer is destroyed because node is shutting down.
    }
}

void
TimedServerCommand::stopTimer()
{
    if(_timerTask)
    {
        _timer->cancel(_timerTask);
        _timerTask = 0;
    }
}

LoadCommand::LoadCommand(const ServerIPtr& server,
                         const InternalServerDescriptorPtr& runtime,
                         const TraceLevelsPtr& traceLevels) :
    ServerCommand(server), _runtime(runtime), _updating(false), _traceLevels(traceLevels)
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
LoadCommand::setUpdate(const InternalServerDescriptorPtr& descriptor, bool clearDir)
{
    _clearDir = clearDir;
    _desc = descriptor;
}

InternalServerDescriptorPtr
LoadCommand::getInternalServerDescriptor() const
{
    return _desc;
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
LoadCommand::startRuntimePropertiesUpdate(const Ice::ObjectPrx& process)
{
    if(_updating)
    {
        return;
    }
    assert(_desc != _runtime);
    _updating = true;

    ResetPropertiesCBPtr cb = new ResetPropertiesCB(_server, process, _desc, _runtime, _traceLevels);
    cb->execute();
}

bool
LoadCommand::finishRuntimePropertiesUpdate(const InternalServerDescriptorPtr& runtime, const Ice::ObjectPrx& process)
{
    _updating = false;
    _runtime = runtime; // The new runtime server descriptor.

    if(_traceLevels->server > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
        out << "updated runtime properties for server `" << _server->getId() << "'";
    }

    if(_desc != _runtime)
    {
        startRuntimePropertiesUpdate(process);
        return false;
    }
    else
    {
        return true;
    }
}

void
LoadCommand::failed(const Ice::Exception& ex)
{
    for(vector<AMD_Node_loadServerPtr>::const_iterator p = _loadCB.begin(); p != _loadCB.end(); ++p)
    {
        (*p)->ice_exception(ex);
    }
    _loadCB.clear();
}

void
LoadCommand::finished(const ServerPrx& proxy, const AdapterPrxDict& adapters, int at, int dt)
{
    for(vector<AMD_Node_loadServerPtr>::const_iterator p = _loadCB.begin(); p != _loadCB.end(); ++p)
    {
        (*p)->ice_response(proxy, adapters, at, dt);
    }
    _loadCB.clear();
}

DestroyCommand::DestroyCommand(const ServerIPtr& server, bool loadFailure, bool clearDir) :
    ServerCommand(server),
    _loadFailure(loadFailure),
    _clearDir(clearDir)
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
    _server->destroy();
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

bool
DestroyCommand::loadFailure() const
{
    return _loadFailure;
}

bool
DestroyCommand::clearDir() const
{
    return _clearDir;
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

StartCommand::StartCommand(const ServerIPtr& server, const IceUtil::TimerPtr& timer, int timeout) :
    TimedServerCommand(server, timer, timeout)
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
StartCommand::timeout()
{
    _server->activationTimedOut();
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

StopCommand::StopCommand(const ServerIPtr& server, const IceUtil::TimerPtr& timer, int timeout, bool deactivate)
    : TimedServerCommand(server, timer, timeout), _deactivate(deactivate)
{
}

bool
StopCommand::isStopped(ServerI::InternalServerState state)
{
    return state == ServerI::Inactive || state == ServerI::Patching || state == ServerI::Loading ||
        state >= ServerI::Destroying;
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
    if(_deactivate)
    {
        _server->deactivate();
    }
}

void
StopCommand::timeout()
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
    _failureTime(IceUtil::Time::now(IceUtil::Time::Monotonic)), // Ensure that _activation is init. in updateImpl().
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
ServerI::waitForApplicationUpdateCompleted(const Ice::AsyncResultPtr&)
{
    activate();
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
            _stop = new StopCommand(this, _node->getTimer(), _deactivationTimeout);
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
ServerI::sendSignal(const string& signal, const Ice::Current&)
{
    _node->getActivator()->sendSignal(_id, signal);
}

void
ServerI::writeMessage(const string& message, Ice::Int fd, const Ice::Current&)
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

Ice::ObjectPrx
ServerI::getProcess() const
{
    Lock sync(*this);

    if(_process == 0 || _state <= Inactive || _state >= Deactivating)
    {
        return 0;
    }
    else
    {
        return _process;
    }
}

void
ServerI::setEnabled(bool enabled, const ::Ice::Current&)
{
    bool activate = false;
    ServerAdapterDict adpts;
    {
        Lock sync(*this);
        checkDestroyed();
        assert(_desc);

        if(enabled && _activation == Disabled)
        {
            _failureTime = IceUtil::Time();
            _activation = toServerActivation(_desc->activation);
            activate = _state == Inactive && _activation == Always;
        }
        else if(!enabled && (_activation != Disabled || _failureTime != IceUtil::Time()))
        {
            _failureTime = IceUtil::Time();
            _activation = Disabled;
            if(_timerTask)
            {
                _node->getTimer()->cancel(_timerTask);
                _timerTask = 0;
            }
        }
        else
        {
            return; // Nothing to change!
        }

        adpts = _adapters;
        _node->observerUpdateServer(getDynamicInfo());
    }

    for(ServerAdapterDict::iterator r = adpts.begin(); r != adpts.end(); ++r)
    {
        r->second->updateEnabled();
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
    ServerAdapterDict adpts;
    ServerCommandPtr command;
    {
        Lock sync(*this);
        checkDestroyed();
        _process = process;
        if(_state == DeactivatingWaitForProcess)
        {
            deact = true;
        }
        else
        {
            if(checkActivation())
            {
                adpts = _adapters;
            }
            command = nextCommand();
        }
    }
    amdCB->ice_response();

    for(ServerAdapterDict::iterator r = adpts.begin(); r != adpts.end(); ++r)
    {
        try
        {
            r->second->activationCompleted();
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
    }

    if(deact)
    {
        deactivate();
    }

    if(command)
    {
        command->execute();
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
    if(!_desc || _activation == Disabled)
    {
        return false;
    }

    if(_desc->activation == "manual" ||
       (_desc->activation == "session" && _desc->sessionId.empty()))
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
        return _node->getActivator()->isActive(); // The server is being deactivated and the
                                                  // node isn't shutting down yet.
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

InternalDistributionDescriptorPtr
ServerI::getDistribution() const
{
    Lock sync(*this);
    return _desc ? _desc->distrib : InternalDistributionDescriptorPtr();
}

bool
ServerI::dependsOnApplicationDistrib() const
{
    Lock sync(*this);
    return _desc ? _desc->applicationDistrib : false;
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
               (_failureTime + IceUtil::Time::seconds(_disableOnFailure) <
                IceUtil::Time::now(IceUtil::Time::Monotonic)))
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
        else if(_activation == Session && _desc->sessionId.empty())
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

        if(_timerTask)
        {
            _node->getTimer()->cancel(_timerTask);
            _timerTask = 0;
        }

        if(!_start)
        {
            _start = new StartCommand(this, _node->getTimer(), _activationTimeout);
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
ServerI::load(const AMD_Node_loadServerPtr& amdCB, const InternalServerDescriptorPtr& desc, const string& replicaName,
              bool noRestart)
{
    Lock sync(*this);
    checkDestroyed();
    checkRevision(replicaName, desc->uuid, desc->revision);

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
    InternalServerDescriptorPtr d = _load ? _load->getInternalServerDescriptor() : _desc;
    if(d && (replicaName != "Master" || d->sessionId == desc->sessionId) && !descriptorUpdated(d, desc))
    {
        if(d->revision != desc->revision)
        {
            updateRevision(desc->uuid, desc->revision);
        }

        if(!_desc || (_load && descriptorUpdated(_load->getInternalServerDescriptor(), _desc)))
        {
            //
            // If the server initial loading didn't complete yet or there's a new updated descriptor
            // waiting to be loaded, we wait for the loading to complete before replying.
            //
            _load->addCallback(amdCB);
            return 0;
        }
        else if(amdCB)
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

    if(!StopCommand::isStopped(_state) && !_stop) // Server is running and no stop is scheduled
    {
        assert(_desc);
        if(noRestart)
        {
            //
            // If the server is not inactive, we have to make sure the update doesn't require
            // a restart. If it requires a restart, we throw. Otherwise we update its properties
            // now.
            //
            checkNoRestart(desc);
        }
        else
        {
            _stop = new StopCommand(this, _node->getTimer(), _deactivationTimeout);
        }
    }

    if(!_load)
    {
        _load = new LoadCommand(this, _desc, _node->getTraceLevels());
    }
    _load->setUpdate(desc, _destroy);
    if(_destroy && _state != Destroying)
    {
        _destroy->finished();
        _destroy = 0;
    }

    if(_stop || StopCommand::isStopped(_state))
    {
        _load->addCallback(amdCB); // Load will return once the server is loaded.
    }
    else
    {
        if(_state >= ServerI::Activating && _state < ServerI::Active)
        {
            //
            // If the server is being activated, return the response
            // now. We can't wait for runtime properties to be updated
            // as this could cause a deadlock if the registry needs
            // the server proxy to register the server process proxy
            // with the node.
            //
            AdapterPrxDict adapters;
            for(ServerAdapterDict::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
            {
                adapters.insert(make_pair(p->first, p->second->getProxy()));
            }
            amdCB->ice_response(_this, adapters, _activationTimeout, _deactivationTimeout);
        }
        else if(_state == ServerI::Active)
        {
            _load->addCallback(amdCB); // Must be called before startRuntimePropertiesUpdate!
            updateRevision(desc->uuid, desc->revision);
            _load->startRuntimePropertiesUpdate(_process);
        }
        else
        {
            _load->addCallback(amdCB);
        }
    }
    return nextCommand();
}

bool
ServerI::checkUpdate(const InternalServerDescriptorPtr& desc, bool noRestart, const Ice::Current&)
{
    Lock sync(*this);
    checkDestroyed();

    if(!_desc)
    {
        throw DeploymentException("server not loaded");
    }

    if(!desc)
    {
        if(noRestart && !StopCommand::isStopped(_state) && !_stop)
        {
            throw DeploymentException("the server is running");
        }
        return true;
    }

    InternalServerDescriptorPtr d = _load ? _load->getInternalServerDescriptor() : _desc;
    if(!descriptorUpdated(d, desc))
    {
        return StopCommand::isStopped(_state);
    }

    if(noRestart)
    {
        checkNoRestart(desc);
    }

    try
    {
        checkAndUpdateUser(desc, false); // false = don't update the user, just check.
    }
    catch(const exception& ex)
    {
        throw DeploymentException(ex.what());
    }

    return StopCommand::isStopped(_state);
}

void
ServerI::checkRemove(bool noRestart, const Ice::Current&)
{
    Lock sync(*this);
    checkDestroyed();

    if(noRestart && !StopCommand::isStopped(_state) && !_stop)
    {
        throw DeploymentException("the server is running");
    }
}

ServerCommandPtr
ServerI::destroy(const AMD_Node_destroyServerPtr& amdCB, const string& uuid, int revision, const string& replicaName,
                 bool noRestart)
{
    Lock sync(*this);
    checkDestroyed();
    checkRevision(replicaName, uuid, revision);

    if(!_desc)
    {
        amdCB->ice_response();
        return 0; // Server is already destroyed.
    }

    if(!StopCommand::isStopped(_state) && !_stop)
    {
        if(noRestart)
        {
            throw DeploymentException("removal requires server to be stopped");
        }
        _stop = new StopCommand(this, _node->getTimer(), _deactivationTimeout);
    }

    if(!_destroy)
    {
        //
        // If uuid is empty, the destroy call comes from the consistency check. In
        // this case, we clear the server directory only if it contains non-user data.
        //
        _destroy = new DestroyCommand(this, false, !uuid.empty());
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
                _stop = new StopCommand(this, _node->getTimer(), _deactivationTimeout);
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
        catch(const exception& ex)
        {
            Ice::Warning out(_node->getTraceLevels()->logger);
            out << ex.what();
        }
    }
#endif
    setState(ServerI::Inactive);
}

void
ServerI::adapterActivated(const string& id)
{
    ServerCommandPtr command;
    ServerAdapterDict adpts;
    {
        Lock sync(*this);
        if(_state != ServerI::Activating &&
           _state != ServerI::WaitForActivation &&
           _state != ServerI::ActivationTimeout)
        {
            return;
        }
        _activatedAdapters.insert(id);
        if(checkActivation())
        {
            adpts = _adapters;
        }
        command = nextCommand();
    }
    for(ServerAdapterDict::iterator r = adpts.begin(); r != adpts.end(); ++r)
    {
        if(r->first != id)
        {
            try
            {
                r->second->activationCompleted();
            }
            catch(const Ice::ObjectNotExistException&)
            {
            }
        }
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
        while(_state == ServerI::Activating)
        {
            wait(); // Wait for activate() to set the state to WaitForActivation
        }

        if((_state == Active || _state == WaitForActivation) &&
           _serverLifetimeAdapters.find(id) != _serverLifetimeAdapters.end())
        {
            _stop = new StopCommand(this, _node->getTimer(), _deactivationTimeout, false);
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
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, _this->ice_getIdentity(), "", "");
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
    if(_disableOnFailure != 0 || (_activation == Always && (_state == Activating || _state == WaitForActivation)))
    {
        _previousActivation = _activation;
        _activation = Disabled;
        _failureTime = IceUtil::Time::now(IceUtil::Time::Monotonic);
    }
}

void
ServerI::activationTimedOut()
{
    ServerCommandPtr command;
    ServerAdapterDict adapters;
    {
        Lock sync(*this);
        if(_state != ServerI::WaitForActivation)
        {
            return;
        }

        setStateNoSync(ServerI::ActivationTimeout, "The server activation timed out.");

        if(_node->getTraceLevels()->server > 1)
        {
            Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
            out << "server `" << _id << "' activation timed out";
        }
        adapters = _adapters;
        command = nextCommand();
    }

    for(ServerAdapterDict::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
        try
        {
            p->second->activationFailed("server activation timed out");
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
    InternalServerDescriptorPtr desc;
    ServerAdapterDict adpts;
    bool waitForReplication;

#ifndef _WIN32
    uid_t uid;
    gid_t gid;
#endif
    string failure;
    try
    {
        {
            Lock sync(*this);
            assert(_state == Activating && _desc);
            desc = _desc;
            adpts = _adapters;

            if(_activation == Disabled)
            {
                throw runtime_error("The server is disabled.");
            }

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
        // time the server is updated or initialy loaded on the node.
        //
        if(waitForReplication)
        {
            NodeSessionPrx session = _node->getMasterNodeSession();
            if(session)
            {
                _node->getMasterNodeSession()->begin_waitForApplicationUpdate(
                    desc->uuid, desc->revision, ::Ice::newCallback(this, &ServerI::waitForApplicationUpdateCompleted));
                return;
            }
        }

        //
        // Compute the server command line options.
        //
        Ice::StringSeq options;
        for(Ice::StringSeq::const_iterator p = desc->options.begin(); p != desc->options.end(); ++p)
        {
            if(!p->empty())
            {
                options.push_back(*p);
            }
        }
        options.push_back("--Ice.Config=" + escapeProperty(_serverDir + "/config/config"));

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

#ifndef _WIN32
        int pid = _node->getActivator()->activate(desc->id, desc->exe, desc->pwd, uid, gid, options, envs, this);
#else
        int pid = _node->getActivator()->activate(desc->id, desc->exe, desc->pwd, options, envs, this);
#endif
        ServerCommandPtr command;
        bool active = false;
        {
            Lock sync(*this);
            assert(_state == Activating);
            _pid = pid;
            setStateNoSync(ServerI::WaitForActivation);
            active = checkActivation();
            command = nextCommand();
            notifyAll(); // Terminated might be waiting for the state change.
        }
        if(active)
        {
            for(ServerAdapterDict::iterator r = adpts.begin(); r != adpts.end(); ++r)
            {
                try
                {
                    r->second->activationCompleted();
                }
                catch(const Ice::ObjectNotExistException&)
                {
                }
            }
        }
        if(command)
        {
            command->execute();
        }
        return;
    }
    catch(const Ice::Exception& ex)
    {
        Ice::Warning out(_node->getTraceLevels()->logger);
        out << "activation failed for server `" << _id << "':\n";
        out << ex;

        failure = ex.what();
    }
    catch(const std::exception& ex)
    {
        failure = ex.what();
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

        assert(_desc);

        //
        // If a process object is supposed to be registered and it's
        // not set yet, we wait for the server to set this process
        // object before attempting to deactivate the server again.
        //
        if(_desc->processRegistered && !_process)
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
        assert(_desc);
        assert(_state == Destroying);
        adpts = _adapters;
    }

    _node->removeServer(this, _desc->application);

    //
    // Remove the server directory only if the clear dir flag is set (user
    // explicitly destroyed the server) or if the server directory doesn't
    // contain user data).
    //
    if(_destroy->clearDir() || _node->canRemoveServerDirectory(_id))
    {
        try
        {
            IcePatch2Internal::removeRecursive(_serverDir);
        }
        catch(const exception& ex)
        {
            if(!_destroy->loadFailure())
            {
                Ice::Warning out(_node->getTraceLevels()->logger);
                out << "removing server directory `" << _serverDir << "' failed:\n" << ex.what();
            }
        }
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
            p->second->setDirectProxy(0, Ice::emptyCurrent);
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
ServerI::shutdown()
{
    Lock sync(*this);
    assert(_state == ServerI::Inactive);
    assert(!_destroy);
    assert(!_stop);
    assert(!_load);
    assert(!_patch);
    assert(!_start);
    _timerTask = 0;
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

        InternalServerDescriptorPtr oldDescriptor = _desc;
        bool disabled = oldDescriptor && _activation == Disabled;
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
                    IcePatch2Internal::removeRecursive(_serverDir);
                }
                catch(const exception&)
                {
                }
            }

            try
            {
                updateImpl(_load->getInternalServerDescriptor());
            }
            catch(const exception& ex)
            {
                throw DeploymentException(ex.what());
            }

            if(oldDescriptor)
            {
                if(oldDescriptor->application != _desc->application)
                {
                    _node->removeServer(this, oldDescriptor->application);
                    _node->addServer(this, _desc->application);
                }

                if(_node->getTraceLevels()->server > 0)
                {
                    Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
                    out << "updated configuration for server `" << _id << "'";
                }
            }
            else
            {
                _node->addServer(this, _desc->application);
            }

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
            if(oldDescriptor)
            {
                try
                {
                    updateImpl(oldDescriptor);
                }
                catch(const Ice::Exception& e)
                {
                    Ice::Warning out(_node->getTraceLevels()->logger);
                    out << "update failed:\n" << ex.reason << "\nand couldn't rollback old descriptor:\n" << e;
                }
                catch(const exception& e)
                {
                    Ice::Warning out(_node->getTraceLevels()->logger);
                    out << "update failed:\n" << ex.reason << "\nand couldn't rollback old descriptor:\n" << e.what();
                }
            }
            else if(!_destroy)
            {
                _destroy = new DestroyCommand(this, true, true);
            }

            _load->failed(ex);
        }

        if(oldDescriptor && disabled != (_activation == Disabled))
        {
            _node->observerUpdateServer(getDynamicInfo());
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
ServerI::updateImpl(const InternalServerDescriptorPtr& descriptor)
{
    assert(_load && descriptor);

    _desc = descriptor;
    _waitForReplication = true;

    //
    // Remember if the server was just released by a session, this
    // will be used later to not update the configuration on the disk
    // (as an optimization and to allow users to review the
    // configuration file after allocating a server -- that's useful
    // if the server configuration is bogus and the session server
    // can't start).
    //
    bool serverSessionReleased = _desc && _desc->activation == "session" &&
        _desc->revision == descriptor->revision && !_desc->sessionId.empty() && descriptor->sessionId.empty();

    //
    // Go through the adapters and create the object adapter Ice
    // objects if necessary, also remove the old ones.
    //
    {
        ServerAdapterDict oldAdapters;
        oldAdapters.swap(_adapters);
        _serverLifetimeAdapters.clear();
        Ice::ObjectAdapterPtr adapter = _node->getAdapter();
        for(InternalAdapterDescriptorSeq::const_iterator r = _desc->adapters.begin(); r != _desc->adapters.end(); ++r)
        {
            try
            {
                Ice::Identity id;
                id.category = _this->ice_getIdentity().category + "Adapter";
                id.name = _id + "-" + (*r)->id;
                ServerAdapterIPtr servant = ServerAdapterIPtr::dynamicCast(adapter->find(id));
                if(!servant)
                {
                    AdapterPrx proxy = AdapterPrx::uncheckedCast(adapter->createProxy(id));
                    servant = new ServerAdapterI(_node, this, _id, proxy, (*r)->id, _activation != Disabled ||
                                                 _failureTime != IceUtil::Time());
                    adapter->add(servant, id);
                }
                _adapters.insert(make_pair((*r)->id, servant));

                if((*r)->serverLifetime)
                {
                    _serverLifetimeAdapters.insert((*r)->id);
                }
            }
            catch(const Ice::ObjectAdapterDeactivatedException&)
            {
                // IGNORE
            }
            catch(const Ice::LocalException& ex)
            {
                Ice::Error out(_node->getTraceLevels()->logger);
                out << "couldn't add adapter `" << (*r)->id << "':\n" << ex;
            }
            oldAdapters.erase((*r)->id);
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
        _activation = toServerActivation(_desc->activation);
        _failureTime = IceUtil::Time();
    }

    if(_timerTask)
    {
        _node->getTimer()->cancel(_timerTask);
        _timerTask = 0;
    }

    checkAndUpdateUser(_desc, true); // we pass true to update _uid/_gid.

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
    // Simplify the log paths and transform relative paths into
    // absolute paths, also make sure the logs are sorted.
    //
    for(Ice::StringSeq::const_iterator p = _desc->logs.begin(); p != _desc->logs.end(); ++p)
    {
        string path = IcePatch2Internal::simplify(*p);
        if(IceUtilInternal::isAbsolutePath(path))
        {
            _logs.push_back(path);
        }
        else
        {
            _logs.push_back(_node->getPlatformInfo().getCwd() + '/' + path);
        }
    }
    sort(_logs.begin(), _logs.begin());

    PropertyDescriptorSeqDict properties = getProperties(_desc);
    PropertyDescriptorSeq& props = properties["config"];
    _stdErrFile = getProperty(props, "Ice.StdErr");
    _stdOutFile = getProperty(props, "Ice.StdOut");

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
    updateRevision(_desc->uuid, _desc->revision);

    //
    // Create or update the server directories exists.
    //
    IcePatch2Internal::createDirectory(_serverDir);
    IcePatch2Internal::createDirectory(_serverDir + "/config");
    IcePatch2Internal::createDirectory(_serverDir + "/dbs");
    IcePatch2Internal::createDirectory(_serverDir + "/distrib");
    IcePatch2Internal::createDirectory(_serverDir + "/data");

    //
    // Create the configuration files, remove the old ones.
    //
    {
        //
        // We do not want to escape the properties if the Ice version is
        // previous to Ice 3.3.
        //
        Ice::StringSeq knownFiles;
        for(PropertyDescriptorSeqDict::const_iterator p = properties.begin(); p != properties.end(); ++p)
        {
            knownFiles.push_back(p->first);

            const string configFilePath = _serverDir + "/config/" + p->first;
            ofstream configfile(IceUtilInternal::streamFilename(configFilePath).c_str()); // configFilePath is a UTF-8 string
            if(!configfile.good())
            {
                throw runtime_error("couldn't create configuration file: " + configFilePath);
            }
            configfile << "# Configuration file (" << IceUtil::Time::now().toDateTime() << ")" << endl << endl;
            for(PropertyDescriptorSeq::const_iterator r = p->second.begin(); r != p->second.end(); ++r)
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
        sort(knownFiles.begin(), knownFiles.end());

        //
        // Remove old configuration files.
        //
        Ice::StringSeq files = IcePatch2Internal::readDirectory(_serverDir + "/config");
        Ice::StringSeq toDel;
        set_difference(files.begin(), files.end(), knownFiles.begin(), knownFiles.end(), back_inserter(toDel));
        for(Ice::StringSeq::const_iterator q = toDel.begin(); q != toDel.end(); ++q)
        {
            if(q->find("config_") == 0)
            {
                try
                {
                    IcePatch2Internal::remove(_serverDir + "/config/" + *q);
                }
                catch(const exception& ex)
                {
                    Ice::Warning out(_node->getTraceLevels()->logger);
                    out << "couldn't remove file `" << _serverDir << "/config/" << *q << "':\n" << ex.what();
                }
            }
        }
    }

    //
    // Update the service data directories if necessary and remove the old ones.
    //
    if(_desc->services)
    {
        Ice::StringSeq knownDirs;
        for(Ice::StringSeq::const_iterator q = _desc->services->begin(); q != _desc->services->end(); ++q)
        {
            knownDirs.push_back("data_" + *q);
            IcePatch2Internal::createDirectory(_serverDir + "/data_" + *q);
        }
        sort(knownDirs.begin(), knownDirs.end());

        //
        // Remove old directories
        //
        Ice::StringSeq dirs = IcePatch2Internal::readDirectory(_serverDir);
        Ice::StringSeq svcDirs;
        for(Ice::StringSeq::const_iterator p = dirs.begin(); p != dirs.end(); ++p)
        {
            if(p->find("data_") == 0)
            {
                svcDirs.push_back(*p);
            }
        }
        Ice::StringSeq toDel;
        set_difference(svcDirs.begin(), svcDirs.end(), knownDirs.begin(), knownDirs.end(), back_inserter(toDel));
        for(Ice::StringSeq::const_iterator p = toDel.begin(); p != toDel.end(); ++p)
        {
            try
            {
                IcePatch2Internal::removeRecursive(_serverDir + "/" + *p);
            }
            catch(const exception& ex)
            {
                Ice::Warning out(_node->getTraceLevels()->logger);
                out << "couldn't remove directory `" << _serverDir << "/" << *p << "':\n" << ex.what();
            }
        }
    }

    //
    // Update the database environments if necessary and remove the
    // old ones.
    //
    {
        Ice::StringSeq knownDbEnvs;
        for(InternalDbEnvDescriptorSeq::const_iterator q = _desc->dbEnvs.begin(); q != _desc->dbEnvs.end(); ++q)
        {
            knownDbEnvs.push_back((*q)->name);

            string dbEnvHome = _serverDir + "/dbs/" + (*q)->name;
            IcePatch2Internal::createDirectory(dbEnvHome);

            if(!(*q)->properties.empty())
            {
                string file = dbEnvHome + "/DB_CONFIG";

                ofstream configfile(IceUtilInternal::streamFilename(file).c_str()); // file is a UTF-8 string
                if(!configfile.good())
                {
                    throw runtime_error("couldn't create configuration file `" + file + "'");
                }

                for(PropertyDescriptorSeq::const_iterator p = (*q)->properties.begin();
                    p != (*q)->properties.end(); ++p)
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
        sort(knownDbEnvs.begin(), knownDbEnvs.end());

        //
        // Remove old database environments.
        //
        Ice::StringSeq dbEnvs = IcePatch2Internal::readDirectory(_serverDir + "/dbs");
        Ice::StringSeq toDel;
        set_difference(dbEnvs.begin(), dbEnvs.end(), knownDbEnvs.begin(), knownDbEnvs.end(), back_inserter(toDel));
        for(Ice::StringSeq::const_iterator p = toDel.begin(); p != toDel.end(); ++p)
        {
            try
            {
                IcePatch2Internal::removeRecursive(_serverDir + "/dbs/" + *p);
            }
            catch(const exception& ex)
            {
                Ice::Warning out(_node->getTraceLevels()->logger);
                out << "couldn't remove directory `" << _serverDir << "/dbs/" << *p << "':\n" << ex.what();
            }
        }
    }

#ifndef _WIN32
    chownRecursive(_serverDir, _uid, _gid);
#endif
}

void
ServerI::checkRevision(const string& replicaName, const string& uuid, int revision) const
{
    if(replicaName == "Master")
    {
        return;
    }

    string descUUID;
    int descRevision;
    if(_load)
    {
        descUUID = _load->getInternalServerDescriptor()->uuid;
        descRevision = _load->getInternalServerDescriptor()->revision;
    }
    else if(_desc)
    {
        descUUID = _desc->uuid;
        descRevision = _desc->revision;
    }
    else
    {
        string idFilePath = _serverDir + "/revision";
        ifstream is(IceUtilInternal::streamFilename(idFilePath).c_str()); // idFilePath is a UTF-8 string
        if(!is.good())
        {
            return;
        }

        char line[1024];
        is.getline(line, 1024); // Ignore comments
        is.getline(line, 1024);
        is.getline(line, 1024);
        string ignored;
        is >> ignored >> descUUID;
        is >> ignored >> descRevision;
    }

    if(uuid != descUUID)
    {
        throw DeploymentException("server from replica `" + replicaName + "' is from another application (`" + uuid +
                                  "')");
    }
    else if(revision != descRevision)
    {
        ostringstream os;
        os << "server from replica `" << replicaName << "' has a different version:\n"
           << "current revision: " << descRevision << "\nreplica revision: " << revision;
        throw DeploymentException(os.str());
    }
}

void
ServerI::checkNoRestart(const InternalServerDescriptorPtr& desc)
{
    assert(_desc);

    if(_desc->sessionId != desc->sessionId)
    {
        throw DeploymentException("server allocated by another session");
    }

    if(descriptorUpdated(_desc, desc, true)) // true = ignore properties
    {
        throw DeploymentException("update requires server to be stopped");
    }
}

#ifndef _WIN32
void
ServerI::checkAndUpdateUser(const InternalServerDescriptorPtr& desc, bool update)
#else
void
ServerI::checkAndUpdateUser(const InternalServerDescriptorPtr& desc, bool /*update*/)
#endif
{
#ifndef _WIN32
    uid_t uid = getuid();
    uid_t gid = getgid();
#endif

    //
    // Don't change the user if the server has the session activation
    // mode and if it's not currently owned by a session.
    //
    string user;
    if(desc->activation != "session" || !desc->sessionId.empty())
    {
        user = desc->user;
#ifndef _WIN32
        //
        // Check if the node is running as root, if that's the case we
        // make sure that a user is set for the process.
        //
        if(uid == 0 && user.empty())
        {
            //
            // If no user is configured and if this server is owned by
            // a session we set the user to the session id, otherwise
            // we set it to "nobody".
            //
            user = !desc->sessionId.empty() ? desc->sessionId : "nobody";
        }
#endif
    }

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
                throw runtime_error("couldn't find user account for user `" + user + "'");
            }
            catch(const Ice::LocalException& ex)
            {
                ostringstream os;
                os << "unexpected exception while trying to find user account for user `" << user << "':\n" << ex;
                throw runtime_error(os.str());
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
            throw Ice::SyscallException(__FILE__, __LINE__, IceInternal::getSystemErrno());
        }
        if(user != string(&buf[0]))
        {
            throw runtime_error("couldn't load server under user account `" + user + "': feature not supported on Windows");
        }
    }
#else
        //
        // Get the uid/gid associated with the given user.
        //
        struct passwd pwbuf;
        int sz = sysconf(_SC_GETPW_R_SIZE_MAX);
        if(sz == -1)
        {
            sz = 4096;
        }
        vector<char> buffer(sz);
        struct passwd *pw;
        int err = getpwnam_r(user.c_str(), &pwbuf, &buffer[0], buffer.size(), &pw);
        while(err == ERANGE && buffer.size() < 1024 * 1024) // Limit buffer to 1MB
        {
            buffer.resize(buffer.size() * 2);
            err = getpwnam_r(user.c_str(), &pwbuf, &buffer[0], buffer.size(), &pw);
        }

        if(err != 0)
        {
            throw Ice::SyscallException(__FILE__, __LINE__, err);
        }
        else if(pw == 0)
        {
            throw runtime_error("unknown user account `" + user + "'");
        }

        //
        // If the node isn't running as root and if the uid of the
        // configured user is different from the uid of the userr
        // running the node we throw, a regular user can't run a
        // process as another user.
        //
        if(uid != 0 && pw->pw_uid != uid)
        {
            throw runtime_error("node has insufficient privileges to load server under user account `" + user + "'");
        }

        if(pw->pw_uid == 0 &&
           _node->getCommunicator()->getProperties()->getPropertyAsInt("IceGrid.Node.AllowRunningServersAsRoot") <= 0)
        {
            throw runtime_error("running server as `root' is not allowed");
        }

        if(update)
        {
            _uid = pw->pw_uid;
            _gid = pw->pw_gid;
        }
    }
    else if(update)
    {
        _uid = uid;
        _gid = gid;
    }
#endif
}

void
ServerI::updateRevision(const string& uuid, int revision)
{
    _desc->uuid = uuid;
    _desc->revision = revision;

    if(_load)
    {
        _load->getInternalServerDescriptor()->uuid = uuid;
        _load->getInternalServerDescriptor()->revision = revision;
    }

    string idFilePath = _serverDir + "/revision";
    ofstream os(IceUtilInternal::streamFilename(idFilePath).c_str()); // idFilePath is a UTF-8 string
    if(os.good())
    {
        os << "#" << endl;
        os << "# This server belongs to the application `" << _desc->application << "'" << endl;
        os << "#" << endl;
        os << "uuid: " << _desc->uuid << endl;
        os << "revision: " << _desc->revision << endl;
    }
}

void
ServerI::updateRuntimePropertiesCallback(const InternalServerDescriptorPtr& desc)
{
    Lock sync(*this);
    if(_state != Active || !_load)
    {
        return;
    }

    if(_load->finishRuntimePropertiesUpdate(desc, _process))
    {
        AdapterPrxDict adapters;
        for(ServerAdapterDict::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
        {
            adapters.insert(make_pair(p->first, p->second->getProxy()));
        }
        _load->finished(_this, adapters, _activationTimeout, _deactivationTimeout);
    }
}

void
ServerI::updateRuntimePropertiesCallback(const Ice::Exception& ex, const InternalServerDescriptorPtr& desc)
{
    Lock sync(*this);
    if(_state != Active || !_load)
    {
        return;
    }

    if(_load->finishRuntimePropertiesUpdate(desc, _process))
    {
        _load->failed(ex);
    }
}

bool
ServerI::checkActivation()
{
    //assert(locked());
    if(_state == ServerI::WaitForActivation || _state == ServerI::ActivationTimeout)
    {
        //
        // Mark the server as active if the server process proxy is registered (or it's not expecting
        // one to be registered) and if all the server lifetime adapters have been activated.
        //
        if((!_desc->processRegistered || _process) &&
           includes(_activatedAdapters.begin(), _activatedAdapters.end(),
                    _serverLifetimeAdapters.begin(), _serverLifetimeAdapters.end()))
        {
            setStateNoSync(ServerI::Active);
            return true;
        }
    }
    return false;
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
    bool loadFailure = false;
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
        loadFailure = _destroy->loadFailure();
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
            loadFailure = _destroy->loadFailure();
            _destroy->finished();
            _destroy = 0;
        }
        notifyAll(); // for getProperties()
        break;
    default:
        break;
    }

    if(_timerTask)
    {
        _node->getTimer()->cancel(_timerTask);
        _timerTask = 0;
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
        _desc = 0;
    }
    else if(_state == Inactive)
    {
        if(_activation == Always)
        {
            assert(!_timerTask);
            _timerTask = new DelayedStart(this, _node->getTraceLevels());
            try
            {
                _node->getTimer()->schedule(_timerTask, IceUtil::Time::milliSeconds(500));
            }
            catch(const IceUtil::Exception&)
            {
                // Ignore, timer is destroyed because node is shutting down.
            }
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
            assert(!_timerTask);
            _timerTask = new DelayedStart(this, _node->getTraceLevels());
            try
            {
                IceUtil::Time now = IceUtil::Time::now(IceUtil::Time::Monotonic);
                if(now - _failureTime < IceUtil::Time::seconds(_disableOnFailure))
                {
                    _node->getTimer()->schedule(_timerTask,
                                                IceUtil::Time::seconds(_disableOnFailure) - now + _failureTime +
                                                IceUtil::Time::milliSeconds(500));
                }
                else
                {
                    _node->getTimer()->schedule(_timerTask, IceUtil::Time::milliSeconds(500));
                }
            }
            catch(const IceUtil::Exception&)
            {
                // Ignore, timer is destroyed because node is shutting down.
            }
        }
    }
    else if(_state == Active && _load)
    {
        //
        // If there's a pending load command, it's time to update the
        // runtime properties of the server now that it's active.
        //
        _load->startRuntimePropertiesUpdate(_process);
    }

    //
    // Don't send the server update if the state didn't change or if
    // the server couldn't be forked. If the server is destroyed, we
    // also check if it's the result of a load failure. If that's the
    // case we don't send an update because the server was never
    // actually loaded.
    //
    if(toServerState(previous) != toServerState(_state) &&
       !(previous == Inactive && _state == Deactivating) &&
       !loadFailure)
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
            if(_node->getTraceLevels()->server != 2 && !(previous == ServerI::Loading || previous == ServerI::Patching))
            {
                Ice::Trace out(_node->getTraceLevels()->logger, _node->getTraceLevels()->serverCat);
                out << "changed server `" << _id << "' state to `Inactive'";
            }
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
    ServerDynamicInfo descriptor;
    descriptor.id = _id;
    descriptor.state = toServerState(_state);

    //
    // NOTE: this must be done only for the active state. Otherwise, we could get a
    // deadlock since getPid() will lock the activator and since this method might
    // be called from the activator locked.
    //
    descriptor.pid = _pid;
    descriptor.enabled = _activation != Disabled;
    return descriptor;
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
        string path = IcePatch2Internal::simplify(filename.substr(1));
        if(!IceUtilInternal::isAbsolutePath(path))
        {
            path = _node->getPlatformInfo().getCwd() + "/" + path;
        }
        if(find(_logs.begin(), _logs.end(), path) == _logs.end())
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

PropertyDescriptorSeqDict
ServerI::getProperties(const InternalServerDescriptorPtr& desc)
{
    //
    // Copy the descriptor properties.
    //
    PropertyDescriptorSeqDict propDict = desc->properties;
    PropertyDescriptorSeq& props = propDict["config"];

    //
    // Cache the path of the stderr/stdout file, first check if the
    // node OutputDir property is set and then we check the server
    // configuration file for the Ice.StdErr and Ice.StdOut
    // properties.
    //
    string stdErrFile = getProperty(props, "Ice.StdErr");
    string stdOutFile = getProperty(props, "Ice.StdOut");
    string outputDir = _node->getOutputDir();
    if(!outputDir.empty())
    {
        if(stdErrFile.empty())
        {
            stdErrFile = outputDir + "/" + _id + (_node->getRedirectErrToOut() ? ".out" : ".err");
            props.push_back(createProperty("Ice.StdErr", stdErrFile));
        }
        if(stdOutFile.empty())
        {
            stdOutFile = outputDir + "/" + _id + ".out";
            props.push_back(createProperty("Ice.StdOut", stdOutFile));
        }
    }

    //
    // Add the locator proxy property and the node properties override
    //
    {
        const PropertyDescriptorSeq& overrides = _node->getPropertiesOverride();
        for(PropertyDescriptorSeqDict::iterator p = propDict.begin(); p != propDict.end(); ++p)
        {
            if(getProperty(p->second, "Ice.Default.Locator").empty())
            {
                Ice::PropertiesPtr properties = _node->getCommunicator()->getProperties();

                string locator = properties->getProperty("Ice.Default.Locator");
                if(!locator.empty())
                {
                    p->second.push_back(createProperty("Ice.Default.Locator", locator));
                }

                string discoveryPlugin = properties->getProperty("Ice.Plugin.IceLocatorDiscovery");
                if(!discoveryPlugin.empty())
                {
                    p->second.push_back(createProperty("Ice.Plugin.IceLocatorDiscovery", discoveryPlugin));
                    p->second.push_back(createProperty("IceLocatorDiscovery.InstanceName", _node->getInstanceName()));
                }
            }

            if(!overrides.empty())
            {
                p->second.push_back(createProperty("# Node properties override"));
                p->second.insert(p->second.end(), overrides.begin(), overrides.end());
            }
        }
    }

    return propDict;
}
