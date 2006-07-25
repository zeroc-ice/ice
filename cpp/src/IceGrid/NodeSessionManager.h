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

#include <IceGrid/Query.h>
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

    bool waitForCreate();
    void terminate();

private:

    const InternalRegistryPrx _registry;
    const NodeIPtr _node;
    const std::string _name;
    NodeSessionPrx _session;
    bool _shutdown;
};
typedef IceUtil::Handle<NodeSessionKeepAliveThread> NodeSessionKeepAliveThreadPtr;

class NodeSessionManager : public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    NodeSessionManager();
    
    void create(const NodeIPtr&);
    void waitForCreate();
    void destroy();

    void run();

    void replicaAdded(const InternalRegistryPrx&);
    void replicaRemoved(const InternalRegistryPrx&);

private:

    void syncReplicas(const InternalRegistryPrxSeq&);

    class Thread : public IceUtil::Thread
    {
    public:

	Thread(NodeSessionManager& manager) : _manager(manager)
        {
	}

	virtual void
	run()
        {
	    _manager.run();
	}

    private:
	
	NodeSessionManager& _manager;
    };


    const NodeIPtr _node;
    IceUtil::ThreadPtr _thread;
    QueryPrx _query;
    InternalRegistryPrx _master;
    NodeSessionPrx _masterSession;
    unsigned long _serial;
    bool _destroyed;
    IceUtil::Time _timeout;

    typedef std::map<Ice::Identity, NodeSessionKeepAliveThreadPtr> NodeSessionMap;
    NodeSessionMap _sessions;
};

}

#endif
