// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_NODECACHE_H
#define ICEGRID_NODECACHE_H

#include "Cache.h"
#include "Internal.h"

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
        NodeEntry(NodeCache&, std::string);

        void addDescriptor(const std::string&, const NodeDescriptor&);
        void removeDescriptor(const std::string&);

        void addServer(const std::shared_ptr<ServerEntry>&);
        void removeServer(const std::shared_ptr<ServerEntry>&);
        void setSession(const std::shared_ptr<NodeSessionI>&);

        NodePrx getProxy() const;
        std::shared_ptr<InternalNodeInfo> getInfo() const;
        ServerEntrySeq getServers() const;
        LoadInfo getLoadInfoAndLoadFactor(const std::string&, float&) const;
        std::shared_ptr<NodeSessionI> getSession() const;

        Ice::ObjectPrx getAdminProxy() const;

        bool canRemove();

        void loadServer(
            const std::shared_ptr<ServerEntry>&,
            const ServerInfo&,
            const std::shared_ptr<SessionI>&,
            std::chrono::seconds,
            bool);
        void destroyServer(const std::shared_ptr<ServerEntry>&, const ServerInfo&, std::chrono::seconds, bool);

        ServerInfo getServerInfo(const ServerInfo&, const std::shared_ptr<SessionI>&);
        std::shared_ptr<InternalServerDescriptor>
        getInternalServerDescriptor(const ServerInfo&, const std::shared_ptr<SessionI>&);

        void checkSession(std::unique_lock<std::mutex>&) const;
        void setProxy(NodePrx);
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

        mutable bool _registering{false};
        mutable std::optional<NodePrx> _proxy;

        mutable std::mutex _mutex;
        mutable std::condition_variable _condVar;

        // A self removing shared_ptr of 'this' which removes itself from the NodeCache upon destruction
        std::weak_ptr<NodeEntry> _selfRemovingPtr;

        // The number of self removing shared_ptr deleters left to run.
        // Always accessed with the cache mutex locked
        int _selfRemovingRefCount{0};

        friend NodeCache;
    };

    class NodeCache : public CacheByString<NodeEntry>
    {
    public:
        using ValueType = NodeEntry*;

        NodeCache(const Ice::CommunicatorPtr&, ReplicaCache&, std::string);

        [[nodiscard]] std::shared_ptr<NodeEntry> get(const std::string&, bool = false) const;

        [[nodiscard]] const Ice::CommunicatorPtr& getCommunicator() const { return _communicator; }
        [[nodiscard]] const std::string& getReplicaName() const { return _replicaName; }
        [[nodiscard]] ReplicaCache& getReplicaCache() const { return _replicaCache; }

    private:
        const Ice::CommunicatorPtr _communicator;
        const std::string _replicaName;
        ReplicaCache& _replicaCache;
    };

};

#endif
