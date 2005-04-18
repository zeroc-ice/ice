// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_SERVER_ADAPTER_I_H
#define ICE_GRID_SERVER_ADAPTER_I_H

#include <IceUtil/Mutex.h>
#include <IceGrid/Internal.h>
#include <IceUtil/AbstractMutex.h>

namespace IceGrid
{

class NodeI;
typedef IceUtil::Handle<NodeI> NodeIPtr;

class ServerAdapterI : public Adapter, public IceUtil::AbstractMutexI<IceUtil::Mutex>
{
public:

    ServerAdapterI(const NodeIPtr&, const ServerPrx&, const std::string&, Ice::Int);
    virtual ~ServerAdapterI();

    virtual std::string getId(const Ice::Current&);

    virtual void activate_async(const AMD_Adapter_activatePtr& cb, const Ice::Current&);
    virtual Ice::ObjectPrx getDirectProxy(const Ice::Current&) const;
    virtual void setDirectProxy(const ::Ice::ObjectPrx&, const ::Ice::Current&);
    virtual void destroy(const ::Ice::Current&);

private:

    const NodeIPtr _node;
    const std::string _id;
    const ServerPrx _server;
    const IceUtil::Time _waitTime;

    Ice::ObjectPrx _proxy;
};

}

#endif
