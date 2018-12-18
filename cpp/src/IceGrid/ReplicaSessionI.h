// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEGRID_REPLICA_SESSION_H
#define ICEGRID_REPLICA_SESSION_H

#include <IceGrid/Registry.h>
#include <IceGrid/Internal.h>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class WellKnownObjectsManager;
typedef IceUtil::Handle<WellKnownObjectsManager> WellKnownObjectsManagerPtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ReplicaSessionI : public ReplicaSession, public IceUtil::Mutex
{
public:

    ReplicaSessionI(const DatabasePtr&, const WellKnownObjectsManagerPtr&, const InternalReplicaInfoPtr&,
                    const InternalRegistryPrx&, int);

    virtual void keepAlive(const Ice::Current&);
    virtual int getTimeout(const Ice::Current&) const;
    virtual void setDatabaseObserver(const DatabaseObserverPrx&, const IceUtil::Optional<StringLongDict>&,
                                     const Ice::Current&);
    virtual void setEndpoints(const StringObjectProxyDict&, const Ice::Current&);
    virtual void registerWellKnownObjects(const ObjectInfoSeq&, const Ice::Current&);
    virtual void setAdapterDirectProxy(const std::string&, const std::string&, const Ice::ObjectPrx&,
                                       const Ice::Current&);
    virtual void receivedUpdate(TopicName, int, const std::string&, const Ice::Current&);
    virtual void destroy(const Ice::Current&);

    virtual IceUtil::Time timestamp() const;
    virtual void shutdown();

    const InternalRegistryPrx& getInternalRegistry() const;
    const InternalReplicaInfoPtr& getInfo() const;
    ReplicaSessionPrx getProxy() const;

    Ice::ObjectPrx getEndpoint(const std::string&);
    bool isDestroyed() const;

private:

    void destroyImpl(bool);

    const DatabasePtr _database;
    const WellKnownObjectsManagerPtr _wellKnownObjects;
    const TraceLevelsPtr _traceLevels;
    const InternalRegistryPrx _internalRegistry;
    const InternalReplicaInfoPtr _info;
    const int _timeout;
    ReplicaSessionPrx _proxy;
    DatabaseObserverPrx _observer;
    ObjectInfoSeq _replicaWellKnownObjects;
    StringObjectProxyDict _replicaEndpoints;
    IceUtil::Time _timestamp;
    bool _destroy;
};
typedef IceUtil::Handle<ReplicaSessionI> ReplicaSessionIPtr;

};

#endif
