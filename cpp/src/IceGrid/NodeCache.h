// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_NODECACHE_H
#define ICE_GRID_NODECACHE_H

#include <IceUtil/RecMutex.h>
#include <IceUtil/Shared.h>
#include <IceGrid/Cache.h>
#include <IceGrid/Internal.h>

namespace IceGrid
{

class NodeCache;

class SessionI;
typedef IceUtil::Handle<SessionI> SessionIPtr;

class NodeSessionI;
typedef IceUtil::Handle<NodeSessionI> NodeSessionIPtr;

class ServerEntry;
typedef IceUtil::Handle<ServerEntry> ServerEntryPtr;
typedef std::vector<ServerEntryPtr> ServerEntrySeq;

class ReplicaCache;

class NodeEntry : private IceUtil::Monitor<IceUtil::RecMutex>
{
public:

    NodeEntry(NodeCache&, const std::string&);
    virtual ~NodeEntry();

    void addDescriptor(const std::string&, const NodeDescriptor&);
    void removeDescriptor(const std::string&);

    void addServer(const ServerEntryPtr&);
    void removeServer(const ServerEntryPtr&);
    void setSession(const NodeSessionIPtr&);

    NodePrx getProxy() const;
    InternalNodeInfoPtr getInfo() const;
    ServerEntrySeq getServers() const;
    LoadInfo getLoadInfoAndLoadFactor(const std::string&, float&) const;
    NodeSessionIPtr getSession() const;

    Ice::ObjectPrx getAdminProxy() const;

    bool canRemove();

    void loadServer(const ServerEntryPtr&, const ServerInfo&, const SessionIPtr&, int, bool);
    void destroyServer(const ServerEntryPtr&, const ServerInfo&, int, bool);

    ServerInfo getServerInfo(const ServerInfo&, const SessionIPtr&);
    InternalServerDescriptorPtr getInternalServerDescriptor(const ServerInfo&, const SessionIPtr&);

    void __incRef();
    void __decRef();

    void checkSession() const;
    void setProxy(const NodePrx&);
    void finishedRegistration();
    void finishedRegistration(const Ice::Exception&);

private:

    ServerDescriptorPtr getServerDescriptor(const ServerInfo&, const SessionIPtr&);
    InternalServerDescriptorPtr getInternalServerDescriptor(const ServerInfo&) const;

    NodeCache& _cache;
    IceUtil::Mutex _refMutex;
    int _ref;
    const std::string _name;
    NodeSessionIPtr _session;
    std::map<std::string, ServerEntryPtr> _servers;
    std::map<std::string, NodeDescriptor> _descriptors;

    mutable bool _registering;
    mutable NodePrx _proxy;
};
typedef IceUtil::Handle<NodeEntry> NodeEntryPtr;

class NodeCache : public CacheByString<NodeEntry>
{
public:

    NodeCache(const Ice::CommunicatorPtr&, ReplicaCache&, const std::string&);

    NodeEntryPtr get(const std::string&, bool = false) const;

    const Ice::CommunicatorPtr& getCommunicator() const { return _communicator; }
    const std::string& getReplicaName() const { return _replicaName; }
    ReplicaCache& getReplicaCache() const { return _replicaCache; }

private:

    const Ice::CommunicatorPtr _communicator;
    const std::string _replicaName;
    ReplicaCache& _replicaCache;
};

};

#endif
