// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

    bool canRemove() const { return true; }
    const ReplicaSessionIPtr& getSession() const;
    RegistryInfo getInfo() const;
    InternalRegistryPrx getProxy() const;
    
private:
    
    const std::string _name;
    const ReplicaSessionIPtr _session;
};
typedef IceUtil::Handle<ReplicaEntry> ReplicaEntryPtr;

class ReplicaCache : public CacheByString<ReplicaEntry>
{
public:

    ReplicaCache(const Ice::CommunicatorPtr&, const IceStorm::TopicManagerPrx&);
    void destroy();

    ReplicaEntryPtr add(const std::string&, const ReplicaSessionIPtr&);
    ReplicaEntryPtr remove(const std::string&);
    ReplicaEntryPtr get(const std::string&) const;

    void nodeAdded(const NodePrx&);
    void nodeRemoved(const NodePrx&);

    Ice::ObjectPrx getEndpoints(const std::string&, const Ice::ObjectPrx&) const;

    void waitForUpdateReplication(const std::string&, int);
    void replicaReceivedUpdate(const std::string&, const std::string&, int, const std::string&);
    
    void startApplicationReplication(const std::string&, int);
    void finishApplicationReplication(const std::string&, int);
    void waitForApplicationReplication(const std::string&, int);

private:

    void removeReplicaUpdates(const std::string&);

    const Ice::CommunicatorPtr _communicator;
    const IceStorm::TopicPrx _topic;
    const NodePrx _nodes;

    std::map<std::string, std::set<std::string> > _waitForUpdates;
    std::map<std::string, std::map<std::string, std::string> > _updateFailures;
    std::set<std::string> _applicationReplication;
};

};

#endif
