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
                                                   const InternalRegistryPrxPtr&,
                                                   std::chrono::seconds);

    void keepAlive(const Ice::Current&) override;
    int getTimeout(const Ice::Current&) const override;
    void setDatabaseObserver(DatabaseObserverPrxPtr, std::optional<StringLongDict>,
                             const Ice::Current&) override;
    void setEndpoints(StringObjectProxyDict, const Ice::Current&) override;
    void registerWellKnownObjects(ObjectInfoSeq, const Ice::Current&) override;
    void setAdapterDirectProxy(std::string, std::string, Ice::ObjectPrxPtr, const Ice::Current&) override;
    void receivedUpdate(TopicName, int, std::string, const Ice::Current&) override;
    void destroy(const Ice::Current&) override;

    std::chrono::steady_clock::time_point timestamp() const;
    void shutdown();

    const InternalRegistryPrxPtr& getInternalRegistry() const;
    const std::shared_ptr<InternalReplicaInfo>& getInfo() const;
    ReplicaSessionPrxPtr getProxy() const;

    Ice::ObjectPrxPtr getEndpoint(const std::string&);
    bool isDestroyed() const;

private:

    ReplicaSessionI(const std::shared_ptr<Database>&, const std::shared_ptr<WellKnownObjectsManager>&,
                    const std::shared_ptr<InternalReplicaInfo>&, const InternalRegistryPrxPtr&,
                    std::chrono::seconds);

    void destroyImpl(bool);

    const std::shared_ptr<Database> _database;
    const std::shared_ptr<WellKnownObjectsManager> _wellKnownObjects;
    const std::shared_ptr<TraceLevels> _traceLevels;
    const InternalRegistryPrxPtr _internalRegistry;
    const std::shared_ptr<InternalReplicaInfo> _info;
    const std::chrono::seconds _timeout;
    ReplicaSessionPrxPtr _proxy;
    DatabaseObserverPrxPtr _observer;
    ObjectInfoSeq _replicaWellKnownObjects;
    StringObjectProxyDict _replicaEndpoints;
    std::chrono::steady_clock::time_point _timestamp;
    bool _destroy;

    mutable std::mutex _mutex;
};

};

#endif
