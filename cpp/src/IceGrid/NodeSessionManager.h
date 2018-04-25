// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
#include <IceGrid/Registry.h>
#include <IceGrid/Internal.h>
#include <set>

namespace IceGrid
{

class NodeI;
typedef IceUtil::Handle<NodeI> NodeIPtr;

class NodeSessionManager;

class NodeSessionKeepAliveThread : public SessionKeepAliveThread<NodeSessionPrx>
{
public:

    NodeSessionKeepAliveThread(const InternalRegistryPrx&, const NodeIPtr&, NodeSessionManager&);

    virtual NodeSessionPrx createSession(InternalRegistryPrx&, IceUtil::Time&);
    virtual void destroySession(const NodeSessionPrx&);
    virtual bool keepAlive(const NodeSessionPrx&);

    std::string getName() const { return _name; }

protected:

    virtual NodeSessionPrx createSessionImpl(const InternalRegistryPrx&, IceUtil::Time&);

    const NodeIPtr _node;
    const std::string _name;
    NodeSessionManager& _manager;
};
typedef IceUtil::Handle<NodeSessionKeepAliveThread> NodeSessionKeepAliveThreadPtr;

class NodeSessionManager : public SessionManager
{
public:

    NodeSessionManager(const Ice::CommunicatorPtr&, const std::string&);

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
    std::vector<IceGrid::QueryPrx> getQueryObjects() { return findAllQueryObjects(true); }

private:

    NodeSessionKeepAliveThreadPtr addReplicaSession(const InternalRegistryPrx&);

    void reapReplicas();

    void syncServers(const NodeSessionPrx&);

    bool isDestroyed()
    {
        Lock sync(*this);
        return _destroyed;
    }

    class Thread : public NodeSessionKeepAliveThread
    {
    public:

        Thread(NodeSessionManager& manager) : NodeSessionKeepAliveThread(manager._master, manager._node, manager)
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
    };
    typedef IceUtil::Handle<Thread> ThreadPtr;
    friend class Thread;

    void createdSession(const NodeSessionPrx&);

    const NodeIPtr _node;
    ThreadPtr _thread;
    bool _destroyed;
    bool _activated;

    typedef std::map<Ice::Identity, NodeSessionKeepAliveThreadPtr> NodeSessionMap;
    NodeSessionMap _sessions;
    std::set<Ice::Identity> _replicas;
};

}

#endif
