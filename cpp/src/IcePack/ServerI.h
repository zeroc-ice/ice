// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PACK_SERVER_I_H
#define ICE_PACK_SERVER_I_H

#include <IceUtil/Mutex.h>
#include <Freeze/EvictorF.h>
#include <IcePack/Activator.h>
#include <IceUtil/AbstractMutex.h>

#include <IcePack/Internal.h>

namespace IcePack
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ServerFactory;
typedef IceUtil::Handle<ServerFactory> ServerFactoryPtr;

class ServerI : public Server, public IceUtil::AbstractMutexI<IceUtil::Monitor<IceUtil::Mutex> >
{
public:

    ServerI(const ServerFactoryPtr&, const TraceLevelsPtr&, const ActivatorPtr&, Ice::Int waitTime,const std::string&);
    virtual ~ServerI();
    
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

    void setExePath(const std::string&, const ::Ice::Current&);
    void setPwd(const std::string&, const ::Ice::Current&);
    void setEnvs(const Ice::StringSeq&, const ::Ice::Current&);
    void setOptions(const Ice::StringSeq&, const ::Ice::Current&);
    void addAdapter(const ServerAdapterPrx&, bool, const ::Ice::Current&);
    void removeAdapter(const ServerAdapterPrx&, const ::Ice::Current&);
    std::string addConfigFile(const std::string&, const PropertyDescriptorSeq&, const ::Ice::Current&);
    void removeConfigFile(const std::string&, const ::Ice::Current&);
    std::string addDbEnv(const DbEnvDescriptor&, const std::string&, const ::Ice::Current&);
    void removeDbEnv(const DbEnvDescriptor&, const std::string&, const ::Ice::Current&);

private:

    void stopInternal(const Ice::Current&);
    void setState(ServerState, const Ice::Current&);

    ServerFactoryPtr _factory;
    TraceLevelsPtr _traceLevels;
    ActivatorPtr _activator;
    ::Ice::Int _waitTime;
    std::string _serversDir;

    ServerState _state;

    ::Ice::ProcessPrx _process;
};

}

#endif
