// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_NODE_SESSION_MANAGER_H
#define ICEGRID_NODE_SESSION_MANAGER_H

#include "IceGrid/Registry.h"
#include "Internal.h"
#include "SessionManager.h"
#include <set>

namespace IceGrid
{
    class NodeI;
    class NodeSessionManager;

    class NodeSessionKeepAliveThread : public SessionKeepAliveThread<NodeSessionPrx>
    {
    public:
        NodeSessionKeepAliveThread(InternalRegistryPrx, const std::shared_ptr<NodeI>&, NodeSessionManager&);

        std::optional<NodeSessionPrx> createSession(InternalRegistryPrx&, std::chrono::seconds&) override;
        void destroySession(const NodeSessionPrx&) override;
        bool keepAlive(const NodeSessionPrx&) override;

        [[nodiscard]] std::string getName() const { return "IceGrid session keepalive thread"; }

    protected:
        NodeSessionPrx createSessionImpl(const InternalRegistryPrx&, std::chrono::seconds&);

        const std::shared_ptr<NodeI> _node;
        const std::string _replicaName;
        NodeSessionManager& _manager;
    };

    class NodeSessionManager : public SessionManager
    {
    public:
        NodeSessionManager(const Ice::CommunicatorPtr&, const std::string&);

        void create(const std::shared_ptr<NodeI>&);
        void create(InternalRegistryPrx);
        void activate();
        bool isWaitingForCreate();
        bool waitForCreate();
        void terminate();
        void destroy();

        void replicaInit(const InternalRegistryPrxSeq&, const Ice::Current&);
        void replicaAdded(InternalRegistryPrx);
        void replicaRemoved(const InternalRegistryPrx&);

        [[nodiscard]] std::optional<NodeSessionPrx> getMasterNodeSession() const { return _thread->getSession(); }
        std::vector<IceGrid::QueryPrx> getQueryObjects() { return findAllQueryObjects(true); }

    private:
        std::shared_ptr<NodeSessionKeepAliveThread> addReplicaSession(InternalRegistryPrx);

        void reapReplicas();

        void syncServers(const NodeSessionPrx&);

        bool isDestroyed() override
        {
            std::lock_guard<std::mutex> lock(_mutex);
            return _destroyed;
        }

        class Thread final : public NodeSessionKeepAliveThread
        {
        public:
            Thread(NodeSessionManager& manager) : NodeSessionKeepAliveThread(*manager._master, manager._node, manager)
            {
            }

            std::optional<NodeSessionPrx>
            createSession(InternalRegistryPrx& master, std::chrono::seconds& timeout) final
            {
                auto session = NodeSessionKeepAliveThread::createSession(master, timeout);
                _manager.createdSession(session);
                _manager.reapReplicas();
                return session;
            }

            void destroySession(const NodeSessionPrx& session) override
            {
                NodeSessionKeepAliveThread::destroySession(session);
                _manager.reapReplicas();
            }

            bool keepAlive(const NodeSessionPrx& session) override
            {
                bool alive = NodeSessionKeepAliveThread::keepAlive(session);
                _manager.reapReplicas();
                return alive;
            }
        };
        friend class Thread;

        void createdSession(const std::optional<NodeSessionPrx>&);

        const std::shared_ptr<NodeI> _node;
        std::shared_ptr<Thread> _thread;
        bool _destroyed{false};
        bool _activated{false};

        using NodeSessionMap = std::map<Ice::Identity, std::shared_ptr<NodeSessionKeepAliveThread>>;
        NodeSessionMap _sessions;
        std::set<Ice::Identity> _replicas;
    };
}

#endif
