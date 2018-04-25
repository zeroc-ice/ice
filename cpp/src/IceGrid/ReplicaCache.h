// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_REPLICACACHE_H
#define ICE_GRID_REPLICACACHE_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Shared.h>
#include <IceGrid/Cache.h>
#include <IceGrid/Internal.h>
#include <IceStorm/IceStorm.h>

namespace IceGrid
{

class ReplicaCache;

class ReplicaSessionI;
typedef IceUtil::Handle<ReplicaSessionI> ReplicaSessionIPtr;

class ReplicaEntry : public IceUtil::Shared
{
public:

    ReplicaEntry(const std::string&, const ReplicaSessionIPtr&);
    virtual ~ReplicaEntry();

    bool canRemove() const { return true; }
    const ReplicaSessionIPtr& getSession() const;
    InternalReplicaInfoPtr getInfo() const;
    InternalRegistryPrx getProxy() const;

    Ice::ObjectPrx getAdminProxy() const;

private:

    const std::string _name;
    const ReplicaSessionIPtr _session;
};
typedef IceUtil::Handle<ReplicaEntry> ReplicaEntryPtr;

class ReplicaCache : public CacheByString<ReplicaEntry>
{
public:

#ifdef __SUNPRO_CC
    using CacheByString<ReplicaEntry>::remove;
#endif

    ReplicaCache(const Ice::CommunicatorPtr&, const IceStorm::TopicManagerPrx&);

    ReplicaEntryPtr add(const std::string&, const ReplicaSessionIPtr&);
    ReplicaEntryPtr remove(const std::string&, bool);
    ReplicaEntryPtr get(const std::string&) const;

    void subscribe(const ReplicaObserverPrx&);
    void unsubscribe(const ReplicaObserverPrx&);

    Ice::ObjectPrx getEndpoints(const std::string&, const Ice::ObjectPrx&) const;

    void setInternalRegistry(const InternalRegistryPrx&);
    InternalRegistryPrx getInternalRegistry() const;

private:

    const Ice::CommunicatorPtr _communicator;
    const IceStorm::TopicPrx _topic;
    const ReplicaObserverPrx _observers;
    InternalRegistryPrx _self; // This replica internal registry proxy.
};

};

#endif
