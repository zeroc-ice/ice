// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

class NodeSessionI;
typedef IceUtil::Handle<NodeSessionI> NodeSessionIPtr;

class ServerEntry;
typedef IceUtil::Handle<ServerEntry> ServerEntryPtr;

class NodeEntry : public IceUtil::Shared, public IceUtil::Mutex
{
public:
    
    NodeEntry(Cache<std::string, NodeEntry>&, const std::string&);

    void addDescriptor(const std::string&, const NodeDescriptor&);
    void removeDescriptor(const std::string&);

    void addServer(const ServerEntryPtr&);
    void removeServer(const ServerEntryPtr&);
    void setSession(const NodeSessionIPtr&);

    NodePrx getProxy() const;
    Ice::StringSeq getServers() const;
    LoadInfo getLoadInfoAndLoadFactor(const std::string&, float&) const;

    bool canRemove();
    
private:
    
    Cache<std::string, NodeEntry>& _cache;
    const std::string _name;
    NodeSessionIPtr _session;
    std::map<std::string, ServerEntryPtr> _servers;
    std::map<std::string, NodeDescriptor> _descriptors;
};
typedef IceUtil::Handle<NodeEntry> NodeEntryPtr;

class NodeCache : public CacheByString<NodeEntry>
{
public:

    NodeCache(int, const TraceLevelsPtr&);

    NodeEntryPtr get(const std::string&, bool = false) const;

    int getSessionTimeout() { return _sessionTimeout; }

private:
    
    const int _sessionTimeout;
};

};

#endif
