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
#include <IceGrid/Allocatable.h>

namespace IceGrid
{

class AdapterCache;

class ServerEntry;
typedef IceUtil::Handle<ServerEntry> ServerEntryPtr;
typedef std::vector<ServerEntryPtr> ServerEntrySeq;

class AdapterEntry;
typedef IceUtil::Handle<AdapterEntry> AdapterEntryPtr;

class AdapterEntry : virtual public IceUtil::Shared, public IceUtil::Mutex
{
public:
    
    AdapterEntry(AdapterCache&, const std::string&);

    virtual std::vector<std::pair<std::string, AdapterPrx> > getProxies(int&, const SessionIPtr&) = 0;
    virtual float getLeastLoadedNodeLoad(LoadSample) const = 0;
    virtual std::string getApplication() const = 0;
    virtual AdapterInfoSeq getAdapterInfo() const = 0;

    virtual bool canRemove();
    
protected:

    AdapterCache& _cache;
    const std::string _id;
};
typedef IceUtil::Handle<AdapterEntry> AdapterEntryPtr;

class ServerAdapterEntry : public AdapterEntry, public Allocatable
{
public:

    ServerAdapterEntry(AdapterCache&, const std::string&, const std::string&, bool, const ServerEntryPtr&);

    virtual std::vector<std::pair<std::string, AdapterPrx> > getProxies(int&, const SessionIPtr&);
    virtual float getLeastLoadedNodeLoad(LoadSample) const;
    virtual std::string getApplication() const;
    virtual AdapterInfoSeq getAdapterInfo() const;
    virtual const std::string& getReplicaGroupId() const { return _replicaGroupId; }

    AdapterPrx getProxy(const std::string& = std::string()) const;

    virtual bool allocated(const SessionIPtr&);
    virtual void released(const SessionIPtr&);

private:
    
    ServerEntryPtr getServer() const;

    const std::string _replicaGroupId;
    const ServerEntryPtr _server;
};
typedef IceUtil::Handle<ServerAdapterEntry> ServerAdapterEntryPtr;

class ReplicaGroupEntry : public AdapterEntry
{
public:

    ReplicaGroupEntry(AdapterCache&, const std::string&, const std::string&, const LoadBalancingPolicyPtr&);

    virtual std::vector<std::pair<std::string, AdapterPrx> > getProxies(int&, const SessionIPtr&);
    virtual float getLeastLoadedNodeLoad(LoadSample) const;
    virtual std::string getApplication() const;
    virtual AdapterInfoSeq getAdapterInfo() const;

    void addReplica(const std::string&, const ServerAdapterEntryPtr&);
    void removeReplica(const std::string&);

    void update(const LoadBalancingPolicyPtr&);

    typedef std::vector<std::pair<std::string, ServerAdapterEntryPtr> > ReplicaSeq;

private:

    const std::string _application;

    LoadBalancingPolicyPtr _loadBalancing;
    int _loadBalancingNReplicas;
    LoadSample _loadSample;
    ReplicaSeq _replicas;
    int _lastReplica;

    static std::pointer_to_unary_function<int, int> _rand;
};
typedef IceUtil::Handle<ReplicaGroupEntry> ReplicaGroupEntryPtr;

class AdapterCache : public CacheByString<AdapterEntry>
{
public:

    ServerAdapterEntryPtr addServerAdapter(const std::string&, const std::string&, bool, const ServerEntryPtr&);
    ReplicaGroupEntryPtr addReplicaGroup(const std::string&, const std::string&, const LoadBalancingPolicyPtr&);

    AdapterEntryPtr get(const std::string&) const;
    ServerAdapterEntryPtr getServerAdapter(const std::string&) const;
    ReplicaGroupEntryPtr getReplicaGroup(const std::string&) const;
    
    void removeServerAdapter(const std::string&);
    void removeReplicaGroup(const std::string&);

protected:
    
    virtual AdapterEntryPtr addImpl(const std::string&, const AdapterEntryPtr&);
    virtual AdapterEntryPtr removeImpl(const std::string&);

};

};

#endif
