//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_REPLICA_SESSION_MANAGER_H
#define ICE_GRID_REPLICA_SESSION_MANAGER_H

#include <IceUtil/Handle.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>

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

        Thread(ReplicaSessionManager& manager, const std::shared_ptr<InternalRegistryPrx>& master,
               const std::shared_ptr<Ice::Logger>& logger) :
            SessionKeepAliveThread<ReplicaSessionPrx>(master, logger),
            _manager(manager)
        {
        }

        std::shared_ptr<ReplicaSessionPrx>
        createSession(std::shared_ptr<InternalRegistryPrx>& master, std::chrono::seconds& timeout) override
        {
            return _manager.createSession(master, timeout);
        }

        void
        destroySession(const std::shared_ptr<ReplicaSessionPrx>& session) override
        {
            _manager.destroySession(session);
        }

        bool
        keepAlive(const std::shared_ptr<ReplicaSessionPrx>& session) override
        {
            return _manager.keepAlive(session);
        }

        void registerAllWellKnownObjects();

    private:

        ReplicaSessionManager& _manager;
    };

    using SessionManager::SessionManager;

    void create(const std::string&, const std::shared_ptr<InternalReplicaInfo>&, const std::shared_ptr<Database>&,
                const std::shared_ptr<WellKnownObjectsManager>&, const std::shared_ptr<InternalRegistryPrx>&);
    void create(const std::shared_ptr<InternalRegistryPrx>&);

    NodePrxSeq getNodes(const NodePrxSeq&) const;
    void destroy();

    void registerAllWellKnownObjects();
    std::shared_ptr<ReplicaSessionPrx> getSession() const { return _thread ? _thread->getSession() : nullptr; }

    std::shared_ptr<InternalRegistryPrx> findInternalRegistryForReplica(const Ice::Identity&);

private:

    friend class Thread;

    bool isDestroyed()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return !_communicator;
    }

    std::shared_ptr<ReplicaSessionPrx> createSession(std::shared_ptr<InternalRegistryPrx>&, std::chrono::seconds&);
    std::shared_ptr<ReplicaSessionPrx> createSessionImpl(const std::shared_ptr<InternalRegistryPrx>&, std::chrono::seconds&);
    void destroySession(const std::shared_ptr<ReplicaSessionPrx>&);
    bool keepAlive(const std::shared_ptr<ReplicaSessionPrx>&);

    std::shared_ptr<Thread> _thread;
    std::string _name;
    std::shared_ptr<InternalReplicaInfo> _info;
    std::shared_ptr<RegistryPrx> _registry;
    std::shared_ptr<InternalRegistryPrx> _internalRegistry;
    std::shared_ptr<DatabaseObserverPrx> _observer;
    std::shared_ptr<Database> _database;
    std::shared_ptr<WellKnownObjectsManager> _wellKnownObjects;
    std::shared_ptr<TraceLevels> _traceLevels;
};

}

#endif
