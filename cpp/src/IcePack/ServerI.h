// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_SERVER_I_H
#define ICE_PACK_SERVER_I_H

#include <IceUtil/Mutex.h>
#include <Freeze/EvictorF.h>
#include <IcePack/Activator.h>

#include <IcePack/Internal.h>

namespace IcePack
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ServerFactory;
typedef IceUtil::Handle<ServerFactory> ServerFactoryPtr;

class ServerI : public Server, public ::IceUtil::Monitor< ::IceUtil::Mutex>
{
public:

    ServerI(const ServerFactoryPtr&, const TraceLevelsPtr&, const ActivatorPtr&, Ice::Int waitTime);
    virtual ~ServerI();
    
    virtual ServerDescription getServerDescription(const ::Ice::Current&);

    virtual bool start(ServerActivation, const ::Ice::Current&);
    virtual void stop(const ::Ice::Current&);
    virtual void destroy(const ::Ice::Current&);
    virtual void terminated(const ::Ice::Current&);

    virtual ServerState getState(const ::Ice::Current&);
    virtual Ice::Int getPid(const ::Ice::Current&);

    virtual void setActivationMode(ServerActivation, const ::Ice::Current&);
    virtual ServerActivation getActivationMode(const ::Ice::Current&);

private:

    void stopInternal();
    void setState(ServerState);

    ServerFactoryPtr _factory;
    TraceLevelsPtr _traceLevels;
    ActivatorPtr _activator;
    ::Ice::Int _waitTime;

    ServerState _state;
};

};

#endif
