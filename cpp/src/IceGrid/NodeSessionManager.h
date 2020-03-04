//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_NODE_SESSION_MANAGER_H
#define ICE_GRID_NODE_SESSION_MANAGER_H

#include <IceGrid/SessionManager.h>
#include <IceGrid/Registry.h>
#include <IceGrid/Internal.h>
#include <set>

namespace IceGrid
{

class NodeI;
class NodeSessionManager;

class NodeSessionKeepAliveThread : public SessionKeepAliveThread<NodeSessionPrx>
{
public:

    NodeSessionKeepAliveThread(const std::shared_ptr<InternalRegistryPrx>&, const std::shared_ptr<NodeI>&,
                               NodeSessionManager&);

    virtual std::shared_ptr<NodeSessionPrx> createSession(std::shared_ptr<InternalRegistryPrx>&,
                                                          std::chrono::seconds&) override;
    virtual void destroySession(const std::shared_ptr<NodeSessionPrx>&) override;
    virtual bool keepAlive(const std::shared_ptr<NodeSessionPrx>&) override;

    std::string getName() const { return "IceGrid session keepalive thread"; }

protected:

    std::shared_ptr<NodeSessionPrx> createSessionImpl(const std::shared_ptr<InternalRegistryPrx>&,
                                                      std::chrono::seconds&);

    const std::shared_ptr<NodeI> _node;
    const std::string _replicaName;
    NodeSessionManager& _manager;
};

class NodeSessionManager : public SessionManager
{
public:

    NodeSessionManager(const std::shared_ptr<Ice::Communicator>&, const std::string&);

    void create(const std::shared_ptr<NodeI>&);
    void create(const std::shared_ptr<InternalRegistryPrx>&);
    void activate();
    bool isWaitingForCreate();
    bool waitForCreate();
    void terminate();
    void destroy();

    void replicaInit(const InternalRegistryPrxSeq&);
    void replicaAdded(const std::shared_ptr<InternalRegistryPrx>&);
    void replicaRemoved(const std::shared_ptr<InternalRegistryPrx>&);

    std::shared_ptr<NodeSessionPrx> getMasterNodeSession() const { return _thread->getSession(); }
    std::vector<std::shared_ptr<IceGrid::QueryPrx>> getQueryObjects() { return findAllQueryObjects(true); }

private:

    std::shared_ptr<NodeSessionKeepAliveThread> addReplicaSession(const std::shared_ptr<InternalRegistryPrx>&);

    void reapReplicas();

    void syncServers(const std::shared_ptr<NodeSessionPrx>&);

    bool isDestroyed()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _destroyed;
    }

    class Thread final : public NodeSessionKeepAliveThread
    {
    public:

        Thread(NodeSessionManager& manager) : NodeSessionKeepAliveThread(manager._master, manager._node, manager)
        {
        }

        std::shared_ptr<NodeSessionPrx>
        createSession(std::shared_ptr<InternalRegistryPrx>& master, std::chrono::seconds& timeout) override
        {
            auto session = NodeSessionKeepAliveThread::createSession(master, timeout);
            _manager.createdSession(session);
            _manager.reapReplicas();
            return session;
        }

        void
        destroySession(const std::shared_ptr<NodeSessionPrx>& session) override
        {
            NodeSessionKeepAliveThread::destroySession(session);
            _manager.reapReplicas();
        }

        bool
        keepAlive(const std::shared_ptr<NodeSessionPrx>& session) override
        {
            bool alive = NodeSessionKeepAliveThread::keepAlive(session);
            _manager.reapReplicas();
            return alive;
        }
    };
    friend class Thread;

    void createdSession(const std::shared_ptr<NodeSessionPrx>&);

    const std::shared_ptr<NodeI> _node;
    std::shared_ptr<Thread> _thread;
    bool _destroyed;
    bool _activated;

    using NodeSessionMap = std::map<Ice::Identity, std::shared_ptr<NodeSessionKeepAliveThread>>;
    NodeSessionMap _sessions;
    std::set<Ice::Identity> _replicas;
};

}

#endif
