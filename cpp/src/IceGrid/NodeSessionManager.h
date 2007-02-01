// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
#include <set>

namespace IceGrid
{

class NodeI;
typedef IceUtil::Handle<NodeI> NodeIPtr;

class NodeSessionKeepAliveThread : public SessionKeepAliveThread<NodeSessionPrx>
{
public:

    NodeSessionKeepAliveThread(const InternalRegistryPrx&, const NodeIPtr&, const std::vector<QueryPrx>&);

    virtual NodeSessionPrx createSession(InternalRegistryPrx&, IceUtil::Time&);
    virtual void destroySession(const NodeSessionPrx&);
    virtual bool keepAlive(const NodeSessionPrx&);

    std::string getName() const { return _name; }

protected:

    virtual NodeSessionPrx createSessionImpl(const InternalRegistryPrx&, IceUtil::Time&);

    const NodeIPtr _node;
    const std::string _name;
    const std::vector<QueryPrx> _queryObjects;
};
typedef IceUtil::Handle<NodeSessionKeepAliveThread> NodeSessionKeepAliveThreadPtr;

class NodeSessionManager : public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    NodeSessionManager();
    
    void create(const NodeIPtr&);
    void create(const InternalRegistryPrx&);
    void activate();
    bool waitForCreate();
    void terminate();
    void destroy();

    void replicaInit(const InternalRegistryPrxSeq&);
    void replicaAdded(const InternalRegistryPrx&);
    void replicaRemoved(const InternalRegistryPrx&);

    NodeSessionPrx getMasterNodeSession() const { return _thread->getSession(); }

private:

    NodeSessionKeepAliveThreadPtr addReplicaSession(const InternalRegistryPrx&);

    void reapReplicas();

    void syncServers(const NodeSessionPrx&);

    class Thread : public NodeSessionKeepAliveThread
    {
    public:

        Thread(NodeSessionManager& manager) : 
            NodeSessionKeepAliveThread(manager._master, manager._node, manager._queryObjects),
            _manager(manager)
        {
        }

        virtual NodeSessionPrx 
        createSession(InternalRegistryPrx& master, IceUtil::Time& timeout)
        {
            NodeSessionPrx session = NodeSessionKeepAliveThread::createSession(master, timeout);
            _manager.createdSession(session);
            _manager.reapReplicas();
            return session;
        }

        virtual void 
        destroySession(const NodeSessionPrx& session)
        {
            NodeSessionKeepAliveThread::destroySession(session);
            _manager.reapReplicas();
        }

        virtual bool 
        keepAlive(const NodeSessionPrx& session)
        {
            bool alive = NodeSessionKeepAliveThread::keepAlive(session);
            _manager.reapReplicas();
            return alive;
        }

    private:
        
        NodeSessionManager& _manager;
    };
    typedef IceUtil::Handle<Thread> ThreadPtr;
    friend class Thread;

    void createdSession(const NodeSessionPrx&);

    const NodeIPtr _node;
    ThreadPtr _thread;
    std::vector<QueryPrx> _queryObjects;
    InternalRegistryPrx _master;
    unsigned long _serial;
    bool _destroyed;
    bool _activated;

    typedef std::map<Ice::Identity, NodeSessionKeepAliveThreadPtr> NodeSessionMap;
    NodeSessionMap _sessions;
    std::set<Ice::Identity> _replicas;
};

}

#endif
