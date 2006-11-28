// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_ADAPTERCACHE_H
#define ICE_GRID_ADAPTERCACHE_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Shared.h>
#include <IceGrid/Cache.h>
#include <IceGrid/Query.h>
#include <IceGrid/Internal.h>

namespace IceGrid
{

class AdapterCache;

class ServerEntry;
typedef IceUtil::Handle<ServerEntry> ServerEntryPtr;
typedef std::vector<ServerEntryPtr> ServerEntrySeq;

class AdapterEntry;
typedef IceUtil::Handle<AdapterEntry> AdapterEntryPtr;

class AdapterEntry : virtual public IceUtil::Shared
{
public:
    
    AdapterEntry(AdapterCache&, const std::string&, const std::string&);

    virtual std::vector<std::pair<std::string, AdapterPrx> > getProxies(int&, bool&) = 0;
    virtual float getLeastLoadedNodeLoad(LoadSample) const = 0;
    virtual AdapterInfoSeq getAdapterInfo() const = 0;

    virtual bool canRemove();

    std::string getId() const;
    std::string getApplication() const;
    
protected:

    AdapterCache& _cache;
    const std::string _id;
    const std::string _application;
};
typedef IceUtil::Handle<AdapterEntry> AdapterEntryPtr;

class ServerAdapterEntry : public AdapterEntry
{
public:

    ServerAdapterEntry(AdapterCache&, const std::string&, const std::string&, const std::string&, int, 
		       const ServerEntryPtr&);

    virtual std::vector<std::pair<std::string, AdapterPrx> > getProxies(int&, bool&);
    virtual float getLeastLoadedNodeLoad(LoadSample) const;
    virtual AdapterInfoSeq getAdapterInfo() const;
    virtual const std::string& getReplicaGroupId() const { return _replicaGroupId; }

    AdapterPrx getProxy(const std::string& = std::string(), bool = true) const;
    int getPriority() const;
    
private:

    const std::string _replicaGroupId;
    const int _priority;
    const ServerEntryPtr _server;
};
typedef IceUtil::Handle<ServerAdapterEntry> ServerAdapterEntryPtr;

class ReplicaGroupEntry : public AdapterEntry, public IceUtil::Mutex
{
public:

    ReplicaGroupEntry(AdapterCache&, const std::string&, const std::string&, const LoadBalancingPolicyPtr&);

    virtual std::vector<std::pair<std::string, AdapterPrx> > getProxies(int&, bool&);
    virtual float getLeastLoadedNodeLoad(LoadSample) const;
    virtual AdapterInfoSeq getAdapterInfo() const;

    void addReplica(const std::string&, const ServerAdapterEntryPtr&);
    void removeReplica(const std::string&);

    void update(const LoadBalancingPolicyPtr&);

private:

    LoadBalancingPolicyPtr _loadBalancing;
    int _loadBalancingNReplicas;
    LoadSample _loadSample;
    std::vector<ServerAdapterEntryPtr> _replicas;
    int _lastReplica;
};
typedef IceUtil::Handle<ReplicaGroupEntry> ReplicaGroupEntryPtr;

class AdapterCache : public CacheByString<AdapterEntry>
{
public:

    ServerAdapterEntryPtr addServerAdapter(const AdapterDescriptor&, const ServerEntryPtr&, const std::string&);
    ReplicaGroupEntryPtr addReplicaGroup(const ReplicaGroupDescriptor&, const std::string&);

    AdapterEntryPtr get(const std::string&) const;
    
    void removeServerAdapter(const std::string&);
    void removeReplicaGroup(const std::string&);

protected:
    
    virtual AdapterEntryPtr addImpl(const std::string&, const AdapterEntryPtr&);
    virtual void removeImpl(const std::string&);

};

};

#endif
