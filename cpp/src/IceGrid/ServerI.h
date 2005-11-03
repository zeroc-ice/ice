// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_SERVER_I_H
#define ICE_GRID_SERVER_I_H

#include <IceUtil/Mutex.h>
#include <Freeze/EvictorF.h>
#include <IceGrid/Activator.h>
#include <IceGrid/WaitQueue.h>
#include <IceGrid/Internal.h>

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
	Inactive,
	Activating,
	WaitForActivation,
	ActivationTimeout,
	Active,
	Deactivating,
	DeactivatingWaitForProcess,
	Destroying,
	Destroyed,
	Loading,
	Patching
    };

    enum ServerActivation
    {
	OnDemand,
	Manual,
	Disabled
    };

    ServerI(const NodeIPtr&, const ServerPrx&, const std::string&, const std::string&, int);
    virtual ~ServerI();

    virtual void start_async(const AMD_Server_startPtr&, const ::Ice::Current& = Ice::Current());
    virtual void stop_async(const AMD_Server_stopPtr&, const ::Ice::Current& = Ice::Current());
    virtual void sendSignal(const std::string&, const ::Ice::Current&);
    virtual void writeMessage(const std::string&, Ice::Int, const ::Ice::Current&);

    virtual ServerState getState(const ::Ice::Current& = Ice::Current()) const;
    virtual Ice::Int getPid(const ::Ice::Current& = Ice::Current()) const;

    virtual void setEnabled(bool, const ::Ice::Current&);
    virtual bool isEnabled(const ::Ice::Current& = Ice::Current()) const;
    virtual void setProcess_async(const AMD_Server_setProcessPtr&, const ::Ice::ProcessPrx&, const ::Ice::Current&);

    ServerDescriptorPtr getDescriptor() const;
    std::string getApplication() const;
    ServerActivation getActivationMode() const;
    const std::string& getId() const;

    void load(const AMD_Node_loadServerPtr&, const std::string&, const ServerDescriptorPtr&);
    bool startPatch(bool);
    bool waitForPatch();
    void finishPatch();
    void destroy(const AMD_Node_destroyServerPtr&);

    void adapterActivated(const std::string&);
    void adapterDeactivated(const std::string&);
    void activationFailed(bool);
    void deactivationFailed();
    void addDynamicInfo(ServerDynamicInfoSeq&, AdapterDynamicInfoSeq&) const;

    void activate();
    void kill();
    void deactivate();
    void update();
    void destroy();
    void terminated();

private:
    
    void updateImpl();
    void checkActivation();
    void checkDestroyed();

    void setState(InternalServerState, const std::string& = std::string());
    ServerCommandPtr nextCommand();
    void setStateNoSync(InternalServerState, const std::string& = std::string());
    
    std::string addAdapter(const AdapterDescriptor&, const CommunicatorDescriptorPtr&);
    void updateConfigFile(const std::string&, const CommunicatorDescriptorPtr&);
    void updateDbEnv(const std::string&, const DbEnvDescriptor&);
    PropertyDescriptor createProperty(const std::string&, const std::string& = std::string());
    ServerState toServerState(InternalServerState) const;
    ServerDynamicInfo getDynamicInfo() const;

    const NodeIPtr _node;
    const ServerPrx _this;
    const std::string _id;
    const Ice::Int _waitTime;
    const std::string _serversDir;

    std::string _serverDir;
    std::string _application;
    ServerDescriptorPtr _desc;
    InternalServerState _state;
    ServerActivation _activation;
    int _activationTimeout;
    int _deactivationTimeout;
    typedef std::map<std::string, ServerAdapterIPtr> ServerAdapterDict;
    ServerAdapterDict _adapters;
    bool _processRegistered;
    Ice::ProcessPrx _process;
    std::set<std::string> _activeAdapters;

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
    virtual void execute() = 0;
    virtual ServerI::InternalServerState nextState() = 0;

protected:

    const ServerIPtr _server;
};
typedef IceUtil::Handle<ServerCommand> ServerCommandPtr;

class TimedServerCommand : public ServerCommand
{
public:

    TimedServerCommand(const ServerIPtr&, const WaitQueuePtr&, int);
    virtual void timeout(bool) = 0;

    void startTimer();
    void stopTimer();

private:

    WaitQueuePtr _waitQueue;
    WaitItemPtr _timer;
    int _timeout;
};
typedef IceUtil::Handle<TimedServerCommand> TimedServerCommandPtr;

}

#endif
