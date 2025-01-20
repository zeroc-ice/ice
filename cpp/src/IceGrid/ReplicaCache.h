// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_REPLICACACHE_H
#define ICEGRID_REPLICACACHE_H

#include "Cache.h"
#include "IceStorm/IceStorm.h"
#include "Internal.h"

namespace IceGrid
{
    class ReplicaCache;
    class ReplicaSessionI;

    class ReplicaEntry final
    {
    public:
        ReplicaEntry(std::string, const std::shared_ptr<ReplicaSessionI>&);

        [[nodiscard]] bool canRemove() const { return true; }
        [[nodiscard]] const std::shared_ptr<ReplicaSessionI>& getSession() const;
        [[nodiscard]] std::shared_ptr<InternalReplicaInfo> getInfo() const;
        [[nodiscard]] InternalRegistryPrx getProxy() const;

        [[nodiscard]] Ice::ObjectPrx getAdminProxy() const;

    private:
        const std::string _name;
        const std::shared_ptr<ReplicaSessionI> _session;
    };

    class ReplicaCache final : public CacheByString<ReplicaEntry>
    {
    public:
        ReplicaCache(const Ice::CommunicatorPtr&, const IceStorm::TopicManagerPrx&);

        std::shared_ptr<ReplicaEntry> add(const std::string&, const std::shared_ptr<ReplicaSessionI>&);
        std::shared_ptr<ReplicaEntry> remove(const std::string&, bool);
        [[nodiscard]] std::shared_ptr<ReplicaEntry> get(const std::string&) const;

        void subscribe(const ReplicaObserverPrx&);
        void unsubscribe(const ReplicaObserverPrx&);

        [[nodiscard]] Ice::ObjectPrx getEndpoints(const std::string&, const std::optional<Ice::ObjectPrx>&) const;

        void setInternalRegistry(InternalRegistryPrx);
        [[nodiscard]] InternalRegistryPrx getInternalRegistry() const;

    private:
        const Ice::CommunicatorPtr _communicator;
        const IceStorm::TopicPrx _topic;
        const ReplicaObserverPrx _observers;
        std::optional<InternalRegistryPrx> _self; // This replica internal registry proxy.
    };

};

#endif
