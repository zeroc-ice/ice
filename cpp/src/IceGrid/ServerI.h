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

#include <IceGrid/Internal.h>

namespace IceGrid
{

class NodeI;
typedef IceUtil::Handle<NodeI> NodeIPtr;    

class ServerAdapterI;
typedef IceUtil::Handle<ServerAdapterI> ServerAdapterIPtr;    

class ServerI : public Server, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    enum InternalServerState
    {
	Inactive,
	Activating,
	WaitForActivation,
	WaitForActivationTimeout,
	Active,
	Deactivating,
	Destroying,
	Destroyed,
	Updating
    };

    enum ServerActivation
    {
	OnDemand,
	Manual,
	Disabled
    };

    ServerI(const NodeIPtr&, const ServerPrx&, const std::string&, const std::string&, int);
    virtual ~ServerI();

    virtual void update(const std::string&, const ServerDescriptorPtr&, bool, AdapterPrxDict&, int&, int&, 
			const Ice::Current&);
    virtual void start_async(const AMD_Server_startPtr&, const ::Ice::Current&);
    virtual void stop(const ::Ice::Current& = Ice::Current());
    virtual void sendSignal(const std::string&, const ::Ice::Current&);
    virtual void writeMessage(const std::string&, Ice::Int, const ::Ice::Current&);
    virtual void destroy(const ::Ice::Current&);
    virtual void terminated(const ::Ice::Current&);

    virtual ServerState getState(const ::Ice::Current& = Ice::Current()) const;
    virtual Ice::Int getPid(const ::Ice::Current& = Ice::Current()) const;

    virtual void setEnabled(bool, const ::Ice::Current&);
    virtual bool isEnabled(const ::Ice::Current& = Ice::Current()) const;
    virtual void setProcess(const ::Ice::ProcessPrx&, const ::Ice::Current&);

    ServerDescriptorPtr getDescriptor() const;
    std::string getApplication() const;
    ServerActivation getActivationMode() const;

    bool startInternal(ServerActivation, const AMD_Server_startPtr& = AMD_Server_startPtr());
    void adapterActivated(const std::string&);
    void adapterDeactivated(const std::string&);
    void activationFailed(bool);
    void addDynamicInfo(ServerDynamicInfoSeq&, AdapterDynamicInfoSeq&) const;

    bool startUpdating(bool);
    void finishUpdating();
    const std::string& getId() const;

private:
    
    void checkActivation();
    void stopInternal(bool);
    void setState(InternalServerState, const std::string& = std::string());
    void setStateNoSync(InternalServerState, const std::string& = std::string());
    
    void updateImpl(const std::string&, const ServerDescriptorPtr&, bool, const Ice::Current&);
    std::string addAdapter(const AdapterDescriptor&, const CommunicatorDescriptorPtr&, const Ice::Current&);
    void updateConfigFile(const std::string&, const CommunicatorDescriptorPtr&, bool);
    void updateDbEnv(const std::string&, const DbEnvDescriptor&);
    void getAdaptersAndTimeouts(AdapterPrxDict&, int&, int&) const;
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
    std::vector<AMD_Server_startPtr> _startCB;
    int _pid;
};
typedef IceUtil::Handle<ServerI> ServerIPtr;

}

#endif
