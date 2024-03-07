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
                const InternalRegistryPrxPtr& master,
                const std::shared_ptr<Ice::Logger>& logger)
                : SessionKeepAliveThread<ReplicaSessionPrx>(master, logger),
                  _manager(manager)
            {
            }

            ReplicaSessionPrxPtr createSession(InternalRegistryPrxPtr& master, std::chrono::seconds& timeout) override
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
            const InternalRegistryPrxPtr&);
        void create(const InternalRegistryPrxPtr&);

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

        ReplicaSessionPrxPtr createSession(InternalRegistryPrxPtr&, std::chrono::seconds&);
        ReplicaSessionPrxPtr createSessionImpl(const InternalRegistryPrxPtr&, std::chrono::seconds&);
        void destroySession(const ReplicaSessionPrxPtr&);
        bool keepAlive(const ReplicaSessionPrxPtr&);

        std::shared_ptr<Thread> _thread;
        std::string _name;
        std::shared_ptr<InternalReplicaInfo> _info;
        RegistryPrxPtr _registry;
        InternalRegistryPrxPtr _internalRegistry;
        DatabaseObserverPrxPtr _observer;
        std::shared_ptr<Database> _database;
        std::shared_ptr<WellKnownObjectsManager> _wellKnownObjects;
        std::shared_ptr<TraceLevels> _traceLevels;
    };
}

#endif
