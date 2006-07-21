// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_NODE_SESSION_MANAGER_H
#define ICE_GRID_NODE_SESSION_MANAGER_H

#include <IceUtil/Handle.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Thread.h>

#include <IceGrid/Internal.h>

namespace IceGrid
{

class NodeI;
typedef IceUtil::Handle<NodeI> NodeIPtr;

class NodeSessionKeepAliveThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    NodeSessionKeepAliveThread(const InternalRegistryPrx&, const NodeIPtr&);

    virtual void run();

    void waitForCreate();
    void terminate();

private:

    void keepAlive(const NodeSessionPrx&);

    const InternalRegistryPrx _registry;
    const NodeIPtr _node;
    IceUtil::Time _timeout;
    NodeSessionPrx _session;
    bool _shutdown;
};
typedef IceUtil::Handle<NodeSessionKeepAliveThread> NodeSessionKeepAliveThreadPtr;

class NodeSessionManager : public IceUtil::Mutex
{
public:

    NodeSessionManager();
    
    void create(const NodeIPtr&);
    void waitForCreate();
    void destroy();

    void replicaAdded(const InternalRegistryPrx&);
    void replicaRemoved(const InternalRegistryPrx&);

private:

    const NodeIPtr _node;
    typedef std::map<Ice::Identity, NodeSessionKeepAliveThreadPtr> NodeSessionMap;
    NodeSessionMap _sessions;
};

}

#endif
