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
#include <IceUtil/AbstractMutex.h>

#include <IceGrid/Internal.h>

namespace IceGrid
{

class NodeI;
typedef IceUtil::Handle<NodeI> NodeIPtr;    

class ServerI : public Server, public IceUtil::AbstractMutexI<IceUtil::Monitor<IceUtil::Mutex> >
{
public:

    ServerI(const NodeIPtr&, const std::string&, const std::string&);
    virtual ~ServerI();

    virtual void load(const ServerDescriptorPtr&, StringAdapterPrxDict&, const Ice::Current&);
    virtual bool start(ServerActivation, const ::Ice::Current&);
    virtual void stop(const ::Ice::Current&);
    virtual void sendSignal(const std::string&, const ::Ice::Current&);
    virtual void writeMessage(const std::string&, Ice::Int, const ::Ice::Current&);
    virtual void destroy(const ::Ice::Current&);
    virtual void terminated(const ::Ice::Current&);

    virtual ServerState getState(const ::Ice::Current&);
    virtual Ice::Int getPid(const ::Ice::Current&);

    virtual void setActivationMode(ServerActivation, const ::Ice::Current&);
    virtual ServerActivation getActivationMode(const ::Ice::Current&);
    virtual ServerDescriptorPtr getDescriptor(const ::Ice::Current&);
    virtual void setProcess(const ::Ice::ProcessPrx&, const ::Ice::Current&);
    virtual StringAdapterPrxDict getAdapters(const Ice::Current&);

private:

    void stopInternal(bool, const Ice::Current&);
    void setState(ServerState, const Ice::Current&);
    void setStateNoSync(ServerState, const Ice::Current&);

    void update(const ServerDescriptorPtr&, StringAdapterPrxDict&, const Ice::Current&);
    void addAdapter(const AdapterDescriptor&, const ServerPrx&, const Ice::Current&);
    void updateConfigFile(const std::string&, const ComponentDescriptorPtr&);
    void updateDbEnv(const std::string&, const DbEnvDescriptor&);
    PropertyDescriptor createProperty(const std::string&, const std::string& = std::string());

    const NodeIPtr _node;
    const std::string _name;
    const Ice::Int _waitTime;
    const std::string _serversDir;

    std::string _serverDir;
    ServerDescriptorPtr _desc;
    ServerState _state;
    ServerActivation _activation;
    StringAdapterPrxDict _adapters;
    bool _processRegistered;
    ::Ice::ProcessPrx _process;
};

}

#endif
