// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    virtual std::string getId(const Ice::Current&);

    virtual void activate_async(const AMD_Adapter_activatePtr& cb, const Ice::Current&);
    virtual Ice::ObjectPrx getDirectProxy(const Ice::Current&) const;
    virtual void setDirectProxy(const ::Ice::ObjectPrx&, const ::Ice::Current&);
    virtual void destroy(const ::Ice::Current&);

private:

    ServerFactoryPtr _factory;
    TraceLevelsPtr _traceLevels;
    IceUtil::Time _waitTime;

    Ice::ObjectPrx _proxy;
};

}

#endif
