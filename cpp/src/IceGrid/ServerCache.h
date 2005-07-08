// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_SERVERCACHE_H
#define ICE_GRID_SERVERCACHE_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Shared.h>
#include <IceGrid/Descriptor.h>
#include <IceGrid/Internal.h>
#include <IceGrid/Cache.h>
#include <IceGrid/NodeCache.h>

namespace IceGrid
{

class Database;

class ServerCache;
class AdapterCache;
class ObjectCache;

class ServerEntry : public IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
public:
    
    ServerEntry(Cache<std::string, ServerEntry>&, const std::string&);
    
    void sync();
    bool needsSync() const;
    void update(const ServerInstanceDescriptor&, const std::string& = std::string());
    void destroy();

    ServerInstanceDescriptor getDescriptor() const;
    std::string getApplication() const;
    std::string getName() const;

    ServerPrx getProxy(int&, int&);
    AdapterPrx getAdapter(const std::string&);

    bool canRemove();
    bool isDestroyed();
    
private:
    
    ServerPrx sync(StringAdapterPrxDict&, int&, int&);
    
    ServerCache& _cache;
    const std::string _name;
    std::auto_ptr<ServerInstanceDescriptor> _loaded;
    std::auto_ptr<ServerInstanceDescriptor> _load;
    std::auto_ptr<ServerInstanceDescriptor> _destroy;
    std::string _application;

    ServerPrx _proxy;
    std::map<std::string, AdapterPrx> _adapters;
    int _activationTimeout;
    int _deactivationTimeout;

    bool _synchronizing;
    bool _failed;
};
typedef IceUtil::Handle<ServerEntry> ServerEntryPtr;
typedef std::vector<ServerEntryPtr> ServerEntrySeq;

class ServerCache : public CacheByString<ServerEntry>
{
public:

    ServerCache(Database&, NodeCache&, AdapterCache&, ObjectCache&);

    ServerEntryPtr add(const std::string&, const ServerInstanceDescriptor&, const std::string&);
    ServerEntryPtr get(const std::string&);
    ServerEntryPtr update(const ServerInstanceDescriptor&);
    ServerEntryPtr remove(const std::string&);

    void clear(const std::string&);
    
    Database& getDatabase() const;

private:
    
    void addComponent(const ComponentDescriptorPtr&, const ServerEntryPtr&);
    void removeComponent(const ComponentDescriptorPtr&, const ServerEntryPtr&);

    friend struct AddComponent;
    friend struct RemoveComponent;

    Database& _database;
    NodeCache& _nodeCache;
    AdapterCache& _adapterCache;
    ObjectCache& _objectCache;
};

};

#endif
