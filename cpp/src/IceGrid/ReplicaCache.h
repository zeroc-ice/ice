//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_REPLICACACHE_H
#define ICE_GRID_REPLICACACHE_H

#include <IceGrid/Cache.h>
#include <IceGrid/Internal.h>
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
    InternalRegistryPrxPtr getProxy() const;

    Ice::ObjectPrxPtr getAdminProxy() const;

private:

    const std::string _name;
    const std::shared_ptr<ReplicaSessionI> _session;
};

class ReplicaCache final : public CacheByString<ReplicaEntry>
{
public:

    ReplicaCache(const std::shared_ptr<Ice::Communicator>&, const IceStorm::TopicManagerPrxPtr&);

    std::shared_ptr<ReplicaEntry> add(const std::string&, const std::shared_ptr<ReplicaSessionI>&);
    std::shared_ptr<ReplicaEntry> remove(const std::string&, bool);
    std::shared_ptr<ReplicaEntry> get(const std::string&) const;

    void subscribe(const ReplicaObserverPrxPtr&);
    void unsubscribe(const ReplicaObserverPrxPtr&);

    Ice::ObjectPrxPtr getEndpoints(const std::string&, const Ice::ObjectPrxPtr&) const;

    void setInternalRegistry(const InternalRegistryPrxPtr&);
    InternalRegistryPrxPtr getInternalRegistry() const;

private:

    const std::shared_ptr<Ice::Communicator> _communicator;
    const IceStorm::TopicPrxPtr _topic;
    const ReplicaObserverPrxPtr _observers;
    InternalRegistryPrxPtr _self; // This replica internal registry proxy.
};

};

#endif
