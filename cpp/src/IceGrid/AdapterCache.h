// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

class AdapterEntry : public IceUtil::Shared, public IceUtil::Mutex
{
public:
    
    AdapterEntry(Cache<std::string, AdapterEntry>&, const std::string&);

    virtual std::vector<std::pair<std::string, AdapterPrx> > getProxies(bool, int&) { 
	return std::vector<std::pair<std::string, AdapterPrx> >(); }
    virtual float getLeastLoadedNodeLoad(LoadSample) const { return 0.0f; }
    virtual std::string getApplication() const { return ""; }
    virtual bool canRemove();
    
protected:
    
    AdapterCache& _cache;
    const std::string _id;
};
typedef IceUtil::Handle<AdapterEntry> AdapterEntryPtr;

class ServerAdapterEntry : public AdapterEntry
{
public:

    ServerAdapterEntry(Cache<std::string, AdapterEntry>&, const std::string&);

    virtual std::vector<std::pair<std::string, AdapterPrx> > getProxies(bool, int&);
    virtual float getLeastLoadedNodeLoad(LoadSample) const;
    virtual std::string getApplication() const;

    void set(const ServerEntryPtr&, const std::string&);
    void destroy();

    AdapterPrx getProxy(const std::string& = std::string()) const;

private:
    
    ServerEntryPtr getServer() const;

    ServerEntryPtr _server;
    std::string _replicaGroupId;
};
typedef IceUtil::Handle<ServerAdapterEntry> ServerAdapterEntryPtr;

class ReplicaGroupEntry : public AdapterEntry
{
public:

    ReplicaGroupEntry(Cache<std::string, AdapterEntry>&, const std::string&);

    virtual std::vector<std::pair<std::string, AdapterPrx> > getProxies(bool, int&);
    virtual float getLeastLoadedNodeLoad(LoadSample) const;
    virtual std::string getApplication() const;

    void set(const std::string&, const LoadBalancingPolicyPtr&);
    void addReplica(const std::string&, const ServerAdapterEntryPtr&);
    void removeReplica(const std::string&);

private:

    LoadBalancingPolicyPtr _loadBalancing;
    int _loadBalancingNReplicas;
    LoadSample _loadSample;
    std::string _application;
    typedef std::vector<std::pair<std::string, ServerAdapterEntryPtr> > ReplicaSeq;
    ReplicaSeq _replicas;
    int _lastReplica;
};
typedef IceUtil::Handle<ReplicaGroupEntry> ReplicaGroupEntryPtr;

class AdapterCache : public CacheByString<AdapterEntry>
{
public:

    AdapterEntryPtr get(const std::string&) const;
    ServerAdapterEntryPtr getServerAdapter(const std::string&, bool = false) const;
    ReplicaGroupEntryPtr getReplicaGroup(const std::string&, bool = false) const;
    
protected:
    
    virtual AdapterEntryPtr addImpl(const std::string&, const AdapterEntryPtr&);
    virtual AdapterEntryPtr removeImpl(const std::string&);

};

};

#endif
