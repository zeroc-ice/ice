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

    void nodeAdded(const NodePrx&);
    void nodeRemoved(const NodePrx&);

    Ice::ObjectPrx getClientProxy(const Ice::ObjectPrx&) const;
    Ice::ObjectPrx getServerProxy(const Ice::ObjectPrx&) const;    

private:

    const Ice::CommunicatorPtr _communicator;
    const IceStorm::TopicPrx _topic;
    const NodePrx _nodes;
};

};

#endif
