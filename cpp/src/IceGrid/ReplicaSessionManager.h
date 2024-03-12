//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_REPLICA_SESSION_MANAGER_H
#define ICE_GRID_REPLICA_SESSION_MANAGER_H

#include <IceGrid/SessionManager.h>
#include <IceGrid/Internal.h>

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
                const std::shared_ptr<Ice::Logger>& logger)
                : SessionKeepAliveThread<ReplicaSessionPrx>(master, logger),
                  _manager(manager)
            {
            }

            std::optional<ReplicaSessionPrx>
            createSession(InternalRegistryPrx& master, std::chrono::seconds& timeout) override
            {
                return _manager.createSession(master, timeout);
            }

            void destroySession(const ReplicaSessionPrxPtr& session) override { _manager.destroySession(session); }

            bool keepAlive(const ReplicaSessionPrxPtr& session) override { return _manager.keepAlive(session); }

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

        void create(InternalRegistryPrx);

        NodePrxSeq getNodes(const NodePrxSeq&) const;
        void destroy();

        void registerAllWellKnownObjects();
        ReplicaSessionPrxPtr getSession() const { return _thread ? _thread->getSession() : std::nullopt; }

        InternalRegistryPrxPtr findInternalRegistryForReplica(const Ice::Identity&);

    private:
        friend class Thread;

        bool isDestroyed()
        {
            std::lock_guard<std::mutex> lock(_mutex);
            return !_communicator;
        }

        std::optional<ReplicaSessionPrx> createSession(InternalRegistryPrx&, std::chrono::seconds&);
        ReplicaSessionPrx createSessionImpl(InternalRegistryPrx, std::chrono::seconds&);
        void destroySession(const ReplicaSessionPrxPtr&);
        bool keepAlive(const ReplicaSessionPrxPtr&);

        std::shared_ptr<Thread> _thread;
        std::string _name;
        std::shared_ptr<InternalReplicaInfo> _info;
        RegistryPrxPtr _registry;
        InternalRegistryPrxPtr _internalRegistry;
        std::optional<DatabaseObserverPrx> _observer;
        std::shared_ptr<Database> _database;
        std::shared_ptr<WellKnownObjectsManager> _wellKnownObjects;
        std::shared_ptr<TraceLevels> _traceLevels;
    };
}

#endif
