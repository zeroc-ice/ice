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
        NodeSessionKeepAliveThread(const InternalRegistryPrxPtr&, const std::shared_ptr<NodeI>&, NodeSessionManager&);

        virtual NodeSessionPrxPtr createSession(InternalRegistryPrxPtr&, std::chrono::seconds&) override;
        virtual void destroySession(const NodeSessionPrxPtr&) override;
        virtual bool keepAlive(const NodeSessionPrxPtr&) override;

        std::string getName() const { return "IceGrid session keepalive thread"; }

    protected:
        NodeSessionPrxPtr createSessionImpl(const InternalRegistryPrxPtr&, std::chrono::seconds&);

        const std::shared_ptr<NodeI> _node;
        const std::string _replicaName;
        NodeSessionManager& _manager;
    };

    class NodeSessionManager : public SessionManager
    {
    public:
        NodeSessionManager(const std::shared_ptr<Ice::Communicator>&, const std::string&);

        void create(const std::shared_ptr<NodeI>&);
        void create(const InternalRegistryPrxPtr&);
        void activate();
        bool isWaitingForCreate();
        bool waitForCreate();
        void terminate();
        void destroy();

        void replicaInit(const InternalRegistryPrxSeq&);
        void replicaAdded(const InternalRegistryPrxPtr&);
        void replicaRemoved(const InternalRegistryPrxPtr&);

        NodeSessionPrxPtr getMasterNodeSession() const { return _thread->getSession(); }
        std::vector<IceGrid::QueryPrxPtr> getQueryObjects() { return findAllQueryObjects(true); }

    private:
        std::shared_ptr<NodeSessionKeepAliveThread> addReplicaSession(const InternalRegistryPrxPtr&);

        void reapReplicas();

        void syncServers(const NodeSessionPrxPtr&);

        bool isDestroyed()
        {
            std::lock_guard<std::mutex> lock(_mutex);
            return _destroyed;
        }

        class Thread final : public NodeSessionKeepAliveThread
        {
        public:
            Thread(NodeSessionManager& manager) : NodeSessionKeepAliveThread(manager._master, manager._node, manager) {}

            NodeSessionPrxPtr createSession(InternalRegistryPrxPtr& master, std::chrono::seconds& timeout) override
            {
                auto session = NodeSessionKeepAliveThread::createSession(master, timeout);
                _manager.createdSession(session);
                _manager.reapReplicas();
                return session;
            }

            void destroySession(const NodeSessionPrxPtr& session) override
            {
                NodeSessionKeepAliveThread::destroySession(session);
                _manager.reapReplicas();
            }

            bool keepAlive(const NodeSessionPrxPtr& session) override
            {
                bool alive = NodeSessionKeepAliveThread::keepAlive(session);
                _manager.reapReplicas();
                return alive;
            }
        };
        friend class Thread;

        void createdSession(const NodeSessionPrxPtr&);

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
