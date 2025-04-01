// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_REPLICA_SESSION_H
#define ICEGRID_REPLICA_SESSION_H

#include "IceGrid/Registry.h"
#include "Internal.h"

namespace IceGrid
{
    class Database;
    class TraceLevels;
    class WellKnownObjectsManager;

    class ReplicaSessionI final : public ReplicaSession
    {
    public:
        static std::shared_ptr<ReplicaSessionI> create(
            const std::shared_ptr<Database>&,
            const std::shared_ptr<WellKnownObjectsManager>&,
            const std::shared_ptr<InternalReplicaInfo>&,
            InternalRegistryPrx,
            std::chrono::seconds);

        void keepAlive(const Ice::Current&) override;
        [[nodiscard]] int getTimeout(const Ice::Current&) const override;
        void setDatabaseObserver(std::optional<DatabaseObserverPrx>, std::optional<StringLongDict>, const Ice::Current&)
            final;
        void setEndpoints(StringObjectProxyDict, const Ice::Current&) override;
        void registerWellKnownObjects(ObjectInfoSeq, const Ice::Current&) override;
        void
        setAdapterDirectProxy(std::string, std::string, std::optional<Ice::ObjectPrx>, const Ice::Current&) override;
        void receivedUpdate(TopicName, int, std::string, const Ice::Current&) override;

        void destroy();
        void destroy(const Ice::Current&) override;

        [[nodiscard]] std::optional<std::chrono::steady_clock::time_point> timestamp() const noexcept;
        void shutdown();

        [[nodiscard]] const InternalRegistryPrx& getInternalRegistry() const;
        [[nodiscard]] const std::shared_ptr<InternalReplicaInfo>& getInfo() const;
        [[nodiscard]] ReplicaSessionPrx getProxy() const;

        std::optional<Ice::ObjectPrx> getEndpoint(const std::string&);
        [[nodiscard]] bool isDestroyed() const;

    private:
        ReplicaSessionI(
            const std::shared_ptr<Database>&,
            const std::shared_ptr<WellKnownObjectsManager>&,
            const std::shared_ptr<InternalReplicaInfo>&,
            InternalRegistryPrx,
            std::chrono::seconds,
            ReplicaSessionPrx);

        void destroyImpl(bool);

        const std::shared_ptr<Database> _database;
        const std::shared_ptr<WellKnownObjectsManager> _wellKnownObjects;
        const std::shared_ptr<TraceLevels> _traceLevels;
        const InternalRegistryPrx _internalRegistry;
        const std::shared_ptr<InternalReplicaInfo> _info;
        const std::chrono::seconds _timeout;
        const ReplicaSessionPrx _proxy;
        std::optional<DatabaseObserverPrx> _observer;
        ObjectInfoSeq _replicaWellKnownObjects;
        StringObjectProxyDict _replicaEndpoints;
        std::chrono::steady_clock::time_point _timestamp;
        bool _destroy{false};

        mutable std::mutex _mutex;
    };

};

#endif
