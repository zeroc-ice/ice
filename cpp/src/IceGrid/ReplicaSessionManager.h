// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_REPLICA_SESSION_MANAGER_H
#define ICEGRID_REPLICA_SESSION_MANAGER_H

#include "Internal.h"
#include "SessionManager.h"

namespace IceGrid
{
    class Database;
    class TraceLevels;
    class WellKnownObjectsManager;

    class ReplicaSessionManager : public SessionManager
    {
    public:
        class Thread : public SessionKeepAliveThread<ReplicaSessionPrx>
        {
        public:
            Thread(
                ReplicaSessionManager& manager,
                const std::optional<InternalRegistryPrx>& master,
                const Ice::LoggerPtr& logger)
                : SessionKeepAliveThread<ReplicaSessionPrx>(master, logger),
                  _manager(manager)
            {
            }

            std::optional<ReplicaSessionPrx>
            createSession(InternalRegistryPrx& master, std::chrono::seconds& timeout) override
            {
                return _manager.createSession(master, timeout);
            }

            void destroySession(const ReplicaSessionPrx& session) override { _manager.destroySession(session); }

            bool keepAlive(const ReplicaSessionPrx& session) override { return _manager.keepAlive(session); }

            void registerAllWellKnownObjects();

        private:
            ReplicaSessionManager& _manager;
        };

        using SessionManager::SessionManager;

        void create(
            const std::string&,
            const std::shared_ptr<InternalReplicaInfo>&,
            const std::shared_ptr<Database>&,
            const std::shared_ptr<WellKnownObjectsManager>&,
            InternalRegistryPrx);

        void create(const InternalRegistryPrx&);

        [[nodiscard]] NodePrxSeq getNodes(const NodePrxSeq&) const;
        void destroy();

        void registerAllWellKnownObjects();
        [[nodiscard]] std::optional<ReplicaSessionPrx> getSession() const
        {
            return _thread ? _thread->getSession() : std::nullopt;
        }

        std::optional<InternalRegistryPrx> findInternalRegistryForReplica(const Ice::Identity&);

    private:
        friend class Thread;

        bool isDestroyed() override
        {
            std::lock_guard<std::mutex> lock(_mutex);
            return !_communicator;
        }

        std::optional<ReplicaSessionPrx> createSession(InternalRegistryPrx&, std::chrono::seconds&);
        ReplicaSessionPrx createSessionImpl(const InternalRegistryPrx&, std::chrono::seconds&);
        void destroySession(const ReplicaSessionPrx&);
        bool keepAlive(const ReplicaSessionPrx&);

        std::shared_ptr<Thread> _thread;
        std::string _name;
        std::shared_ptr<InternalReplicaInfo> _info;
        std::optional<InternalRegistryPrx> _internalRegistry;
        std::optional<DatabaseObserverPrx> _observer;
        std::shared_ptr<Database> _database;
        std::shared_ptr<WellKnownObjectsManager> _wellKnownObjects;
        std::shared_ptr<TraceLevels> _traceLevels;
    };
}

#endif
