//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_NODECACHE_H
#define ICE_GRID_NODECACHE_H

#include <IceGrid/Cache.h>
#include <IceGrid/Internal.h>

namespace IceGrid
{

class NodeCache;
class NodeSessionI;
class ReplicaCache;
class ServerEntry;
class SessionI;

using ServerEntrySeq = std::vector<std::shared_ptr<ServerEntry>>;

class NodeEntry final
{
public:

    NodeEntry(NodeCache&, const std::string&);

    void addDescriptor(const std::string&, const NodeDescriptor&);
    void removeDescriptor(const std::string&);

    void addServer(const std::shared_ptr<ServerEntry>&);
    void removeServer(const std::shared_ptr<ServerEntry>&);
    void setSession(const std::shared_ptr<NodeSessionI>&);

    std::shared_ptr<NodePrx> getProxy() const;
    std::shared_ptr<InternalNodeInfo> getInfo() const;
    ServerEntrySeq getServers() const;
    LoadInfo getLoadInfoAndLoadFactor(const std::string&, float&) const;
    std::shared_ptr<NodeSessionI> getSession() const;

    std::shared_ptr<Ice::ObjectPrx> getAdminProxy() const;

    bool canRemove();

    void loadServer(const std::shared_ptr<ServerEntry>&, const ServerInfo&, const std::shared_ptr<SessionI>&,
                    std::chrono::seconds, bool);
    void destroyServer(const std::shared_ptr<ServerEntry>&, const ServerInfo&, std::chrono::seconds, bool);

    ServerInfo getServerInfo(const ServerInfo&, const std::shared_ptr<SessionI>&);
    std::shared_ptr<InternalServerDescriptor> getInternalServerDescriptor(const ServerInfo&, const std::shared_ptr<SessionI>&);

    void checkSession(std::unique_lock<std::mutex>&) const;
    void setProxy(const std::shared_ptr<NodePrx>&);
    void finishedRegistration();
    void finishedRegistration(std::exception_ptr);

private:

    std::shared_ptr<NodeEntry> selfRemovingPtr() const;

    std::shared_ptr<ServerDescriptor> getServerDescriptor(const ServerInfo&, const std::shared_ptr<SessionI>&);
    std::shared_ptr<InternalServerDescriptor> getInternalServerDescriptor(const ServerInfo&) const;

    NodeCache& _cache;
    const std::string _name;
    std::shared_ptr<NodeSessionI> _session;
    std::map<std::string, std::shared_ptr<ServerEntry>> _servers;
    std::map<std::string, NodeDescriptor> _descriptors;

    mutable bool _registering;
    mutable std::shared_ptr<NodePrx> _proxy;

    mutable std::mutex _mutex;
    mutable std::condition_variable _condVar;

    // A self removing shared_ptr of 'this' which removes itself from the NodeCache upon destruction
    std::weak_ptr<NodeEntry> _selfRemovingPtr;

    // The number of self removing shared_ptr deleters left to run.
    // Always accessed with the cache mutex locked
    int _selfRemovingRefCount;

    friend NodeCache;
};

class NodeCache : public CacheByString<NodeEntry>
{
public:

    using ValueType = NodeEntry*;

    NodeCache(const std::shared_ptr<Ice::Communicator>&, ReplicaCache&, const std::string&);

    std::shared_ptr<NodeEntry> get(const std::string&, bool = false) const;

    const std::shared_ptr<Ice::Communicator>& getCommunicator() const { return _communicator; }
    const std::string& getReplicaName() const { return _replicaName; }
    ReplicaCache& getReplicaCache() const { return _replicaCache; }

private:

    const std::shared_ptr<Ice::Communicator> _communicator;
    const std::string _replicaName;
    ReplicaCache& _replicaCache;
};

};

#endif
