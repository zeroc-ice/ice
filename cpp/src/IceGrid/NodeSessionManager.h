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

#include <IceGrid/SessionManager.h>
#include <IceGrid/Query.h>
#include <IceGrid/Internal.h>

namespace IceGrid
{

class NodeI;
typedef IceUtil::Handle<NodeI> NodeIPtr;

class NodeSessionKeepAliveThread : public SessionKeepAliveThread<NodeSessionPrx>
{
public:

    NodeSessionKeepAliveThread(const InternalRegistryPrx&, const NodeIPtr&, const IceGrid::QueryPrx&);

    virtual NodeSessionPrx createSession(const InternalRegistryPrx&, IceUtil::Time&);
    virtual void destroySession(const NodeSessionPrx&);
    virtual bool keepAlive(const NodeSessionPrx&);

protected:

    virtual NodeSessionPrx createSessionImpl(const InternalRegistryPrx&, IceUtil::Time&);

    const NodeIPtr _node;
    const std::string _name;
    const IceGrid::QueryPrx _query;
};
typedef IceUtil::Handle<NodeSessionKeepAliveThread> NodeSessionKeepAliveThreadPtr;

class NodeSessionManager : public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    NodeSessionManager();
    
    void create(const NodeIPtr&);
    void create(const InternalRegistryPrx&);
    void waitForCreate();
    void destroy();

    NodeSessionKeepAliveThreadPtr replicaAdded(const InternalRegistryPrx&);
    void replicaRemoved(const InternalRegistryPrx&);

    NodeSessionPrx getMasterNodeSession() const { return _thread->getSession(); }

private:

    void syncReplicas(const InternalRegistryPrxSeq&);

    class Thread : public NodeSessionKeepAliveThread
    {
    public:

	Thread(NodeSessionManager& manager) : 
	    NodeSessionKeepAliveThread(manager._master, manager._node, manager._query),
	    _manager(manager)
        {
	}

	virtual NodeSessionPrx 
	createSession(const InternalRegistryPrx& master, IceUtil::Time& timeout)
        {
	    NodeSessionPrx session = NodeSessionKeepAliveThread::createSession(master, timeout);
	    _manager.createdSession(session);
	    return session;
	}

    private:
	
	NodeSessionManager& _manager;
    };
    typedef IceUtil::Handle<Thread> ThreadPtr;
    friend class Thread;

    void createdSession(const NodeSessionPrx&);

    const NodeIPtr _node;
    ThreadPtr _thread;
    QueryPrx _query;
    InternalRegistryPrx _master;
    unsigned long _serial;
    bool _destroyed;

    typedef std::map<Ice::Identity, NodeSessionKeepAliveThreadPtr> NodeSessionMap;
    NodeSessionMap _sessions;
};

}

#endif
