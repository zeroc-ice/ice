// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_PACK_SERVER_ADAPTER_I_H
#define ICE_PACK_SERVER_ADAPTER_I_H

#include <IceUtil/Mutex.h>
#include <IcePack/Internal.h>
#include <IceUtil/AbstractMutex.h>

namespace IcePack
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ServerFactory;
typedef IceUtil::Handle<ServerFactory> ServerFactoryPtr;

class ServerAdapterI : public ServerAdapter, public IceUtil::AbstractMutexI<IceUtil::Mutex>
{
public:

    ServerAdapterI(const ServerFactoryPtr&, const TraceLevelsPtr&, Ice::Int waitTime);
    virtual ~ServerAdapterI();

    virtual void getDirectProxy_async(const AMD_Adapter_getDirectProxyPtr& cb, bool, const Ice::Current&);
    virtual void setDirectProxy(const ::Ice::ObjectPrx&, const ::Ice::Current&);
    virtual void destroy(const ::Ice::Current&);

private:

    ServerFactoryPtr _factory;
    TraceLevelsPtr _traceLevels;
    IceUtil::Time _waitTime;

    Ice::ObjectPrx _proxy;
    bool _notified;
};

}

#endif
