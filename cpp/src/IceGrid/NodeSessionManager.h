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

class NodeSessionKeepAliveThread : public SessionKeepAliveThread<NodeSessionPrx, InternalRegistryPrx>
{
public:

    NodeSessionKeepAliveThread(const InternalRegistryPrx&, const NodeIPtr&);

    virtual NodeSessionPrx createSession(const InternalRegistryPrx&, IceUtil::Time&) const;
    virtual void destroySession(const NodeSessionPrx&) const;
    virtual bool keepAlive(const NodeSessionPrx&) const;

private:

    const NodeIPtr _node;
    const std::string _name;
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

private:

    void syncReplicas(const InternalRegistryPrxSeq&);

    class Thread : public SessionKeepAliveThread<NodeSessionPrx, InternalRegistryPrx>
    {
    public:

	Thread(NodeSessionManager& manager, const InternalRegistryPrx& master) : 
	    SessionKeepAliveThread<NodeSessionPrx, InternalRegistryPrx>(master),
	    _manager(manager)
        {
	}

	virtual NodeSessionPrx 
	createSession(const InternalRegistryPrx& master, IceUtil::Time& timeout) const
        {
	    return _manager.createSession(master, timeout);
	}

	virtual void 
	destroySession(const NodeSessionPrx& session) const
        {
	    _manager.destroySession(session);
	}

	virtual bool 
	keepAlive(const NodeSessionPrx& session) const
        {
	    return _manager.keepAlive(session);
	}

    private:
	
	NodeSessionManager& _manager;
    };
    typedef IceUtil::Handle<Thread> ThreadPtr;

    NodeSessionPrx createSession(const InternalRegistryPrx&, IceUtil::Time&) const;
    void destroySession(const NodeSessionPrx&) const;
    bool keepAlive(const NodeSessionPrx&) const;

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
