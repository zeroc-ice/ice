// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_SERVER_I_H
#define ICE_GRID_SERVER_I_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Timer.h>
#include <IceGrid/Activator.h>
#include <IceGrid/Internal.h>
#include <set>

#ifndef _WIN32
#   include <sys/types.h> // for uid_t, gid_t
#endif

namespace IceGrid
{

class NodeI;
typedef IceUtil::Handle<NodeI> NodeIPtr;
class ServerAdapterI;
typedef IceUtil::Handle<ServerAdapterI> ServerAdapterIPtr;
class ServerCommand;
typedef IceUtil::Handle<ServerCommand> ServerCommandPtr;
class DestroyCommand;
typedef IceUtil::Handle<DestroyCommand> DestroyCommandPtr;
class StopCommand;
typedef IceUtil::Handle<StopCommand> StopCommandPtr;
class StartCommand;
typedef IceUtil::Handle<StartCommand> StartCommandPtr;
class PatchCommand;
typedef IceUtil::Handle<PatchCommand> PatchCommandPtr;
class LoadCommand;
typedef IceUtil::Handle<LoadCommand> LoadCommandPtr;

class ServerI : public Server, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    enum InternalServerState
    {
        Loading,
        Patching,
        Inactive,
        Activating,
        WaitForActivation,
        ActivationTimeout,
        Active,
        Deactivating,
        DeactivatingWaitForProcess,
        Destroying,
        Destroyed
    };

    enum ServerActivation
    {
        Always,
        Session,
        OnDemand,
        Manual,
        Disabled
    };

    ServerI(const NodeIPtr&, const ServerPrx&, const std::string&, const std::string&, int);
    virtual ~ServerI();

    void waitForApplicationUpdateCompleted(const Ice::AsyncResultPtr&);

    virtual void start_async(const AMD_Server_startPtr&, const ::Ice::Current& = Ice::noExplicitCurrent);
    virtual void stop_async(const AMD_Server_stopPtr&, const ::Ice::Current& = Ice::noExplicitCurrent);
    virtual void sendSignal(const std::string&, const ::Ice::Current&);
    virtual void writeMessage(const std::string&, Ice::Int, const ::Ice::Current&);

    virtual ServerState getState(const ::Ice::Current& = Ice::noExplicitCurrent) const;
    virtual Ice::Int getPid(const ::Ice::Current& = Ice::noExplicitCurrent) const;

    virtual void setEnabled(bool, const ::Ice::Current&);
    virtual bool isEnabled(const ::Ice::Current& = Ice::noExplicitCurrent) const;
    virtual void setProcess_async(const AMD_Server_setProcessPtr&, const ::Ice::ProcessPrx&, const ::Ice::Current&);

    virtual Ice::Long getOffsetFromEnd(const std::string&, int, const Ice::Current&) const;
    virtual bool read(const std::string&, Ice::Long, int, Ice::Long&, Ice::StringSeq&, const Ice::Current&) const;

    bool isAdapterActivatable(const std::string&) const;
    const std::string& getId() const;
    InternalDistributionDescriptorPtr getDistribution() const;
    bool dependsOnApplicationDistrib() const;

    void start(ServerActivation, const AMD_Server_startPtr& = AMD_Server_startPtr());
    ServerCommandPtr load(const AMD_Node_loadServerPtr&, const InternalServerDescriptorPtr&, const std::string&, bool);
    bool checkUpdate(const InternalServerDescriptorPtr&, bool, const Ice::Current&);
    void checkRemove(bool, const Ice::Current&);
    ServerCommandPtr destroy(const AMD_Node_destroyServerPtr&, const std::string&, int, const std::string&, bool);
    bool startPatch(bool);
    bool waitForPatch();
    void finishPatch();

    void adapterActivated(const std::string&);
    void adapterDeactivated(const std::string&);
    void activationTimedOut();

    void activate();
    void kill();
    void deactivate();
    void update();
    void destroy();
    void terminated(const std::string&, int);
    void shutdown();

    //
    // A proxy to the Process facet of the real Admin object; called by the AdminFacade servant implementation
    //
    Ice::ObjectPrx getProcess() const;

    PropertyDescriptorSeqDict getProperties(const InternalServerDescriptorPtr&);

    void updateRuntimePropertiesCallback(const InternalServerDescriptorPtr&);
    void updateRuntimePropertiesCallback(const Ice::Exception&, const InternalServerDescriptorPtr&);

private:

    void updateImpl(const InternalServerDescriptorPtr&);
    void checkRevision(const std::string&, const std::string&, int) const;
    void checkNoRestart(const InternalServerDescriptorPtr&);
    void checkAndUpdateUser(const InternalServerDescriptorPtr&, bool);
    void updateRevision(const std::string&, int);
    bool checkActivation();
    void checkDestroyed() const;
    void disableOnFailure();

    void setState(InternalServerState, const std::string& = std::string());
    ServerCommandPtr nextCommand();
    void setStateNoSync(InternalServerState, const std::string& = std::string());

    ServerState toServerState(InternalServerState) const;
    ServerActivation toServerActivation(const std::string&) const;
    ServerDynamicInfo getDynamicInfo() const;
    std::string getFilePath(const std::string&) const;

    const NodeIPtr _node;
    const ServerPrx _this;
    const std::string _id;
    const Ice::Int _waitTime;
    const std::string _serverDir;
    const int _disableOnFailure;

    InternalServerDescriptorPtr _desc;
#ifndef _WIN32
    uid_t _uid;
    gid_t _gid;
#endif
    InternalServerState _state;
    ServerActivation _activation;
    int _activationTimeout;
    int _deactivationTimeout;
    typedef std::map<std::string, ServerAdapterIPtr> ServerAdapterDict;
    ServerAdapterDict _adapters;
    std::set<std::string> _serverLifetimeAdapters;
    Ice::ProcessPrx _process;
    std::set<std::string> _activatedAdapters;
    IceUtil::Time _failureTime;
    ServerActivation _previousActivation;
    IceUtil::TimerTaskPtr _timerTask;
    bool _waitForReplication;
    std::string _stdErrFile;
    std::string _stdOutFile;
    Ice::StringSeq _logs;
    PropertyDescriptorSeq _properties;

    DestroyCommandPtr _destroy;
    StopCommandPtr _stop;
    LoadCommandPtr _load;
    PatchCommandPtr _patch;
    StartCommandPtr _start;

    int _pid;
};
typedef IceUtil::Handle<ServerI> ServerIPtr;

class ServerCommand : public IceUtil::SimpleShared
{
public:

    ServerCommand(const ServerIPtr&);
    virtual ~ServerCommand();

    virtual void execute() = 0;
    virtual ServerI::InternalServerState nextState() = 0;

protected:

    const ServerIPtr _server;
};
typedef IceUtil::Handle<ServerCommand> ServerCommandPtr;

class TimedServerCommand : public ServerCommand
{
public:

    TimedServerCommand(const ServerIPtr&, const IceUtil::TimerPtr&, int);
    virtual void timeout() = 0;

    void startTimer();
    void stopTimer();

private:

    IceUtil::TimerPtr _timer;
    IceUtil::TimerTaskPtr _timerTask;
    int _timeout;
};
typedef IceUtil::Handle<TimedServerCommand> TimedServerCommandPtr;

class DestroyCommand : public ServerCommand
{
public:

    DestroyCommand(const ServerIPtr&, bool, bool);

    bool canExecute(ServerI::InternalServerState);
    ServerI::InternalServerState nextState();
    void execute();

    void addCallback(const AMD_Node_destroyServerPtr&);
    void finished();
    bool loadFailure() const;
    bool clearDir() const;

private:

    const bool _loadFailure;
    const bool _clearDir;
    std::vector<AMD_Node_destroyServerPtr> _destroyCB;
};

class StopCommand : public TimedServerCommand
{
public:

    StopCommand(const ServerIPtr&, const IceUtil::TimerPtr&, int, bool = true);

    static bool isStopped(ServerI::InternalServerState);

    bool canExecute(ServerI::InternalServerState);
    ServerI::InternalServerState nextState();
    void execute();
    void timeout();

    void addCallback(const AMD_Server_stopPtr&);
    void failed(const std::string& reason);
    void finished();

private:

    std::vector<AMD_Server_stopPtr> _stopCB;
    bool _deactivate;
};

class StartCommand : public TimedServerCommand
{
public:

    StartCommand(const ServerIPtr&, const IceUtil::TimerPtr&, int);

    bool canExecute(ServerI::InternalServerState);
    ServerI::InternalServerState nextState();
    void execute();
    void timeout();

    void addCallback(const AMD_Server_startPtr&);
    void failed(const std::string& reason);
    void finished();

private:

    std::vector<AMD_Server_startPtr> _startCB;
};

class PatchCommand : public ServerCommand, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    PatchCommand(const ServerIPtr&);

    bool canExecute(ServerI::InternalServerState);
    ServerI::InternalServerState nextState();
    void execute();

    bool waitForPatch();
    void destroyed();
    void finished();

private:

    bool _notified;
    bool _destroyed;
};

class LoadCommand : public ServerCommand
{
public:

    LoadCommand(const ServerIPtr&, const InternalServerDescriptorPtr&, const TraceLevelsPtr&);

    bool canExecute(ServerI::InternalServerState);
    ServerI::InternalServerState nextState();
    void execute();

    void setUpdate(const InternalServerDescriptorPtr&, bool);
    bool clearDir() const;
    InternalServerDescriptorPtr getInternalServerDescriptor() const;
    void addCallback(const AMD_Node_loadServerPtr&);
    void startRuntimePropertiesUpdate(const Ice::ObjectPrx&);
    bool finishRuntimePropertiesUpdate(const InternalServerDescriptorPtr&, const Ice::ObjectPrx&);
    void failed(const Ice::Exception&);
    void finished(const ServerPrx&, const AdapterPrxDict&, int, int);

private:

    std::vector<AMD_Node_loadServerPtr> _loadCB;
    bool _clearDir;
    InternalServerDescriptorPtr _desc;
    IceInternal::UniquePtr<DeploymentException> _exception;
    InternalServerDescriptorPtr _runtime;
    bool _updating;
    TraceLevelsPtr _traceLevels;
};

}

#endif
