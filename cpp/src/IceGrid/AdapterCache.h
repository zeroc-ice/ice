// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
#include <IceGrid/Registry.h>
#include <IceGrid/Internal.h>

#include <set>

namespace IceGrid
{

class AdapterCache;

class SynchronizationCallback;
typedef IceUtil::Handle<SynchronizationCallback> SynchronizationCallbackPtr;

class ServerEntry;
typedef IceUtil::Handle<ServerEntry> ServerEntryPtr;
typedef std::vector<ServerEntryPtr> ServerEntrySeq;

class AdapterEntry;
typedef IceUtil::Handle<AdapterEntry> AdapterEntryPtr;

struct LocatorAdapterInfo
{
    std::string id;
    AdapterPrx proxy;
    int activationTimeout;
    int deactivationTimeout;
};
typedef std::vector<LocatorAdapterInfo> LocatorAdapterInfoSeq;

class AdapterEntry : public virtual IceUtil::Shared
{
public:
    
    AdapterEntry(AdapterCache&, const std::string&, const std::string&);

    virtual bool addSyncCallback(const SynchronizationCallbackPtr&, const std::set<std::string>&) = 0;

    virtual void getLocatorAdapterInfo(LocatorAdapterInfoSeq&, int&, bool&, bool&, std::string&, 
                                       const std::set<std::string>&) = 0;
    virtual float getLeastLoadedNodeLoad(LoadSample) const = 0;
    virtual AdapterInfoSeq getAdapterInfo() const = 0;
    virtual AdapterPrx getProxy(const std::string&, bool) const = 0;

    virtual bool canRemove();

    std::string getId() const;
    std::string getApplication() const;
    
protected:

    AdapterCache& _cache;
    const std::string _id;
    std::string _application;
};
typedef IceUtil::Handle<AdapterEntry> AdapterEntryPtr;

class ServerAdapterEntry : public AdapterEntry
{
public:

    ServerAdapterEntry(AdapterCache&, const std::string&, const std::string&, const std::string&, int, 
                       const ServerEntryPtr&);

    virtual bool addSyncCallback(const SynchronizationCallbackPtr&, const std::set<std::string>&);

    virtual void getLocatorAdapterInfo(LocatorAdapterInfoSeq&, int&, bool&, bool&, std::string&,
                                       const std::set<std::string>&);

    virtual float getLeastLoadedNodeLoad(LoadSample) const;
    virtual AdapterInfoSeq getAdapterInfo() const;
    virtual AdapterPrx getProxy(const std::string&, bool) const;

    void getLocatorAdapterInfo(LocatorAdapterInfoSeq&) const;
    const std::string& getReplicaGroupId() const { return _replicaGroupId; }
    int getPriority() const;

    std::string getServerId() const;
    std::string getNodeName() const;

private:

    const std::string _replicaGroupId;
    const int _priority;
    const ServerEntryPtr _server;
};
typedef IceUtil::Handle<ServerAdapterEntry> ServerAdapterEntryPtr;

class ReplicaGroupEntry : public AdapterEntry, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    ReplicaGroupEntry(AdapterCache&, const std::string&, const std::string&, const LoadBalancingPolicyPtr&, 
                      const std::string&);

    virtual bool addSyncCallback(const SynchronizationCallbackPtr&, const std::set<std::string>&);

    virtual void getLocatorAdapterInfo(LocatorAdapterInfoSeq&, int&, bool&, bool&, std::string&,
                                       const std::set<std::string>&);
    virtual float getLeastLoadedNodeLoad(LoadSample) const;
    virtual AdapterInfoSeq getAdapterInfo() const;
    virtual AdapterPrx getProxy(const std::string&, bool) const { return 0; }

    void addReplica(const std::string&, const ServerAdapterEntryPtr&);
    bool removeReplica(const std::string&);

    void update(const std::string&, const LoadBalancingPolicyPtr&, const std::string&);
    bool hasAdaptersFromOtherApplications() const;

    const std::string& getFilter() const { return _filter; }

private:

    LoadBalancingPolicyPtr _loadBalancing;
    int _loadBalancingNReplicas;
    LoadSample _loadSample;
    std::string _filter;
    std::vector<ServerAdapterEntryPtr> _replicas;
    int _lastReplica;
    bool _requestInProgress;
};
typedef IceUtil::Handle<ReplicaGroupEntry> ReplicaGroupEntryPtr;

class AdapterCache : public CacheByString<AdapterEntry>
{
public:

    AdapterCache(const Ice::CommunicatorPtr&);

    void addServerAdapter(const AdapterDescriptor&, const ServerEntryPtr&, const std::string&);
    void addReplicaGroup(const ReplicaGroupDescriptor&, const std::string&);

    AdapterEntryPtr get(const std::string&) const;
    
    void removeServerAdapter(const std::string&);
    void removeReplicaGroup(const std::string&);

protected:
    
    virtual AdapterEntryPtr addImpl(const std::string&, const AdapterEntryPtr&);
    virtual void removeImpl(const std::string&);

private:

    const Ice::CommunicatorPtr _communicator;
};

};

#endif
