//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEGRID_REPLICA_SESSION_H
#define ICEGRID_REPLICA_SESSION_H

#include <IceGrid/Registry.h>
#include <IceGrid/Internal.h>

namespace IceGrid
{

class Database;
class TraceLevels;
class WellKnownObjectsManager;

class ReplicaSessionI final : public ReplicaSession
{
public:

    static std::shared_ptr<ReplicaSessionI> create(const std::shared_ptr<Database>&,
                                                   const std::shared_ptr<WellKnownObjectsManager>&,
                                                   const std::shared_ptr<InternalReplicaInfo>&,
                                                   const std::shared_ptr<InternalRegistryPrx>&,
                                                   std::chrono::seconds);

    void keepAlive(const Ice::Current&) override;
    int getTimeout(const Ice::Current&) const override;
    void setDatabaseObserver(std::shared_ptr<DatabaseObserverPrx>, IceUtil::Optional<StringLongDict>,
                             const Ice::Current&) override;
    void setEndpoints(StringObjectProxyDict, const Ice::Current&) override;
    void registerWellKnownObjects(ObjectInfoSeq, const Ice::Current&) override;
    void setAdapterDirectProxy(std::string, std::string, std::shared_ptr<Ice::ObjectPrx>, const Ice::Current&) override;
    void receivedUpdate(TopicName, int, std::string, const Ice::Current&) override;
    void destroy(const Ice::Current&) override;

    std::chrono::steady_clock::time_point timestamp() const;
    void shutdown();

    const std::shared_ptr<InternalRegistryPrx>& getInternalRegistry() const;
    const std::shared_ptr<InternalReplicaInfo>& getInfo() const;
    std::shared_ptr<ReplicaSessionPrx> getProxy() const;

    std::shared_ptr<Ice::ObjectPrx> getEndpoint(const std::string&);
    bool isDestroyed() const;

private:

    ReplicaSessionI(const std::shared_ptr<Database>&, const std::shared_ptr<WellKnownObjectsManager>&,
                    const std::shared_ptr<InternalReplicaInfo>&, const std::shared_ptr<InternalRegistryPrx>&,
                    std::chrono::seconds);

    void destroyImpl(bool);

    const std::shared_ptr<Database> _database;
    const std::shared_ptr<WellKnownObjectsManager> _wellKnownObjects;
    const std::shared_ptr<TraceLevels> _traceLevels;
    const std::shared_ptr<InternalRegistryPrx> _internalRegistry;
    const std::shared_ptr<InternalReplicaInfo> _info;
    const std::chrono::seconds _timeout;
    std::shared_ptr<ReplicaSessionPrx> _proxy;
    std::shared_ptr<DatabaseObserverPrx> _observer;
    ObjectInfoSeq _replicaWellKnownObjects;
    StringObjectProxyDict _replicaEndpoints;
    std::chrono::steady_clock::time_point _timestamp;
    bool _destroy;

    mutable std::mutex _mutex;
};

};

#endif
