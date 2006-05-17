// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_NODECACHE_H
#define ICE_GRID_NODECACHE_H

#include <IceUtil/Mutex.h>
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

class NodeEntry : public IceUtil::Shared, public IceUtil::Mutex
{
public:
    
    NodeEntry(NodeCache&, const std::string&);

    void addDescriptor(const std::string&, const NodeDescriptor&);
    void removeDescriptor(const std::string&);

    void addServer(const ServerEntryPtr&);
    void removeServer(const ServerEntryPtr&);
    void setSession(const NodeSessionIPtr&);

    NodePrx getProxy() const;
    NodeInfo getInfo() const;
    Ice::StringSeq getServers() const;
    LoadInfo getLoadInfoAndLoadFactor(const std::string&, float&) const;

    bool canRemove();
    
    void loadServer(const ServerEntryPtr&, const ServerInfo&, const SessionIPtr&);
    void destroyServer(const ServerEntryPtr&, const std::string&);
    ServerInfo getServerInfo(const ServerInfo&, const SessionIPtr&);

private:
    
    ServerDescriptorPtr getServerDescriptor(const ServerInfo&, const SessionIPtr&);

    NodeCache& _cache;
    const std::string _name;
    NodeSessionIPtr _session;
    std::map<std::string, ServerEntryPtr> _servers;
    std::map<std::string, NodeDescriptor> _descriptors;
};
typedef IceUtil::Handle<NodeEntry> NodeEntryPtr;

class NodeCache : public CacheByString<NodeEntry>
{
public:

    NodeCache(const Ice::CommunicatorPtr&, int);

    void destroy();

    NodeEntryPtr get(const std::string&, bool = false) const;

    int getSessionTimeout() { return _sessionTimeout; }

    const Ice::CommunicatorPtr& getCommunicator() const { return _communicator; }

private:
    
    Ice::CommunicatorPtr _communicator;
    const int _sessionTimeout;
};

};

#endif
