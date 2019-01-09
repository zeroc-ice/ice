// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_GRID_SERVERCACHE_H
#define ICE_GRID_SERVERCACHE_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Shared.h>
#include <Ice/UniquePtr.h>
#include <IceGrid/Descriptor.h>
#include <IceGrid/Internal.h>
#include <IceGrid/Registry.h>
#include <IceGrid/Allocatable.h>
#include <IceGrid/Cache.h>

namespace IceGrid
{

class ServerCache;
class ObjectCache;
class AdapterCache;
class AllocatableObjectCache;
class NodeCache;

class NodeEntry;
typedef IceUtil::Handle<NodeEntry> NodeEntryPtr;

class CheckServerResult;
typedef IceUtil::Handle<CheckServerResult> CheckServerResultPtr;

class NodeObserverTopic;
typedef IceUtil::Handle<NodeObserverTopic> NodeObserverTopicPtr;

class CheckUpdateResult : public IceUtil::Shared
{
public:

    CheckUpdateResult(const std::string&, const std::string&, bool, bool, const Ice::AsyncResultPtr&);

    bool getResult();

    const std::string& getServer() { return _server; }

private:

    const std::string _server;
    const std::string _node;
    const bool _remove;
    const bool _noRestart;
    const Ice::AsyncResultPtr _result;
};
typedef IceUtil::Handle<CheckUpdateResult> CheckUpdateResultPtr;

class ServerEntry : public Allocatable
{
public:

    ServerEntry(ServerCache&, const std::string&);

    void sync();
    void waitForSync(int);
    void waitForSync()
    {
        waitForSync(-1);
    }
    void waitForSyncNoThrow(int);
    void waitForSyncNoThrow()
    {
        waitForSyncNoThrow(-1);
    }
    void unsync();

    bool addSyncCallback(const SynchronizationCallbackPtr&);

    void update(const ServerInfo&, bool);

    void destroy(bool);

    ServerInfo getInfo(bool = false) const;
    std::string getId() const;

    ServerPrx getProxy(int&, int&, std::string&, bool = true, int = 0);
    ServerPrx getProxy(bool = true, int = 0);
    Ice::ObjectPrx getAdminProxy();

    AdapterPrx getAdapter(const std::string&, bool);
    AdapterPrx getAdapter(int&, int&, const std::string&, bool);
    float getLoad(LoadSample) const;

    bool canRemove();
    CheckUpdateResultPtr checkUpdate(const ServerInfo&, bool);
    bool isDestroyed();

    void loadCallback(const ServerPrx&, const AdapterPrxDict&, int, int);
    void destroyCallback();
    void exception(const Ice::Exception&);

    virtual bool isEnabled() const;
    virtual void allocated(const SessionIPtr&);
    virtual void allocatedNoSync(const SessionIPtr&);
    virtual void released(const SessionIPtr&);
    virtual void releasedNoSync(const SessionIPtr&);

private:

    void syncImpl();
    void waitImpl(int);
    void synchronized();
    void synchronized(const Ice::Exception&);

    ServerCache& _cache;
    const std::string _id;
    IceInternal::UniquePtr<ServerInfo> _loaded;
    IceInternal::UniquePtr<ServerInfo> _load;
    IceInternal::UniquePtr<ServerInfo> _destroy;

    ServerPrx _proxy;
    AdapterPrxDict _adapters;
    int _activationTimeout;
    int _deactivationTimeout;

    bool _synchronizing;
    bool _updated;
    IceInternal::UniquePtr<Ice::Exception> _exception;
    bool _noRestart;
    std::vector<SynchronizationCallbackPtr> _callbacks;

    SessionIPtr _allocationSession;
};
typedef IceUtil::Handle<ServerEntry> ServerEntryPtr;
typedef std::vector<ServerEntryPtr> ServerEntrySeq;

class ServerCache : public CacheByString<ServerEntry>
{
public:

#ifdef __SUNPRO_CC
    using CacheByString<ServerEntry>::remove;
#endif

    ServerCache(const Ice::CommunicatorPtr&, const std::string&, NodeCache&, AdapterCache&, ObjectCache&,
                AllocatableObjectCache&);

    ServerEntryPtr add(const ServerInfo&);
    ServerEntryPtr get(const std::string&) const;
    bool has(const std::string&) const;
    ServerEntryPtr remove(const std::string&, bool);

    void preUpdate(const ServerInfo&, bool);
    ServerEntryPtr postUpdate(const ServerInfo&, bool);

    void clear(const std::string&);

    NodeCache& getNodeCache() const { return _nodeCache; }
    Ice::CommunicatorPtr getCommunicator() const { return _communicator; }
    const std::string& getInstanceName() const { return _instanceName; }

    const NodeObserverTopicPtr& getNodeObserverTopic() const { return _nodeObserverTopic; }
    void setNodeObserverTopic(const NodeObserverTopicPtr&);

private:

    void addCommunicator(const CommunicatorDescriptorPtr&, const CommunicatorDescriptorPtr&, const ServerEntryPtr&,
                         const std::string&);
    void removeCommunicator(const CommunicatorDescriptorPtr&, const CommunicatorDescriptorPtr&, const ServerEntryPtr&);

    friend struct AddCommunicator;
    friend struct RemoveCommunicator;

    const Ice::CommunicatorPtr _communicator;
    const std::string _instanceName;
    NodeCache& _nodeCache;
    AdapterCache& _adapterCache;
    ObjectCache& _objectCache;
    AllocatableObjectCache& _allocatableObjectCache;
    NodeObserverTopicPtr _nodeObserverTopic;
};

};

#endif
