// **********************************************************************
//
// Copyright (c) 2001
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

namespace IcePack
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ServerFactory;
typedef IceUtil::Handle<ServerFactory> ServerFactoryPtr;

class ServerAdapterI : public ServerAdapter, public ::IceUtil::Monitor< ::IceUtil::Mutex>
{
public:

    ServerAdapterI(const ServerFactoryPtr&, const TraceLevelsPtr&, Ice::Int waitTime);
    virtual ~ServerAdapterI();

    virtual ::Ice::ObjectPrx getDirectProxy(bool, const ::Ice::Current&);
    virtual void setDirectProxy(const ::Ice::ObjectPrx&, const ::Ice::Current&);
    virtual void destroy(const ::Ice::Current&);

private:

    ServerFactoryPtr _factory;
    TraceLevelsPtr _traceLevels;
    Ice::Int _waitTime;

    Ice::ObjectPrx _proxy;
    bool _notified;
};

}

#endif
