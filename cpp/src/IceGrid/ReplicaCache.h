//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_REPLICACACHE_H
#define ICE_GRID_REPLICACACHE_H

#include <IceGrid/Cache.h>
#include "Internal.h"
#include <IceStorm/IceStorm.h>

namespace IceGrid
{
    class ReplicaCache;
    class ReplicaSessionI;

    class ReplicaEntry final
    {
    public:
        ReplicaEntry(const std::string&, const std::shared_ptr<ReplicaSessionI>&);

        bool canRemove() const { return true; }
        const std::shared_ptr<ReplicaSessionI>& getSession() const;
        std::shared_ptr<InternalReplicaInfo> getInfo() const;
        InternalRegistryPrx getProxy() const;

        Ice::ObjectPrx getAdminProxy() const;

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
        std::shared_ptr<ReplicaEntry> get(const std::string&) const;

        void subscribe(const ReplicaObserverPrx&);
        void unsubscribe(const ReplicaObserverPrx&);

        Ice::ObjectPrx getEndpoints(const std::string&, const std::optional<Ice::ObjectPrx>&) const;

        void setInternalRegistry(InternalRegistryPrx);
        InternalRegistryPrx getInternalRegistry() const;

    private:
        const Ice::CommunicatorPtr _communicator;
        const IceStorm::TopicPrx _topic;
        const ReplicaObserverPrx _observers;
        std::optional<InternalRegistryPrx> _self; // This replica internal registry proxy.
    };

};

#endif
