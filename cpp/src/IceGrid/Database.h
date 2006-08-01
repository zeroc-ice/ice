// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_DATABASE_H
#define ICE_GRID_DATABASE_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Shared.h>
#include <Freeze/ConnectionF.h>
#include <Ice/CommunicatorF.h>
#include <IceGrid/Admin.h>
#include <IceGrid/Internal.h>
#include <IceGrid/StringApplicationInfoDict.h>
#include <IceGrid/IdentityObjectInfoDict.h>
#include <IceGrid/StringAdapterInfoDict.h>
#include <IceGrid/ServerCache.h>
#include <IceGrid/NodeCache.h>
#include <IceGrid/ReplicaCache.h>
#include <IceGrid/ObjectCache.h>
#include <IceGrid/AllocatableObjectCache.h>
#include <IceGrid/AdapterCache.h>

namespace IceGrid
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class NodeSessionI;
typedef IceUtil::Handle<NodeSessionI> NodeSessionIPtr;

class RegistryObserverTopic;
typedef IceUtil::Handle<RegistryObserverTopic> RegistryObserverTopicPtr;

class NodeObserverTopic;
typedef IceUtil::Handle<NodeObserverTopic> NodeObserverTopicPtr;

class ReplicaSessionI;
typedef IceUtil::Handle<ReplicaSessionI> ReplicaSessionIPtr;

class AdminSessionI;

class ServerEntry;
typedef IceUtil::Handle<ServerEntry> ServerEntryPtr;

class ApplicationHelper;

class Database : public IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
public:
    
    Database(const Ice::ObjectAdapterPtr&, const IceStorm::TopicManagerPrx&, const std::string&, int, 
	     const TraceLevelsPtr&);
    virtual ~Database();
    
    void destroy();

    std::string getInstanceName() const;
    const TraceLevelsPtr& getTraceLevels() const { return _traceLevels; }
    const Ice::CommunicatorPtr& getCommunicator() const { return _communicator; }
    const Ice::ObjectAdapterPtr& getInternalAdapter() { return _internalAdapter; }

    void clearTopics();
    RegistryObserverTopicPtr getRegistryObserverTopic() const;
    NodeObserverTopicPtr getNodeObserverTopic() const;
    int getSessionTimeout() const { return _sessionTimeout; }

    int lock(AdminSessionI*, const std::string&);
    void unlock(AdminSessionI*);

    void init(int serial);
    void initMaster();
    void initReplica(int, const ApplicationInfoSeq&, const AdapterInfoSeq&, const ObjectInfoSeq&);

    void setClientProxy(const Ice::ObjectPrx&);
    void setServerProxy(const Ice::ObjectPrx&);
    Ice::ObjectPrx getClientProxy() const;
    Ice::ObjectPrx getServerProxy() const;
    void updateReplicatedWellKnownObjects();

    void addApplicationDescriptor(AdminSessionI*, const ApplicationDescriptor&, int = -1);
    void updateApplicationDescriptor(AdminSessionI*, const ApplicationUpdateDescriptor&, int = -1);
    void syncApplicationDescriptor(AdminSessionI*, const ApplicationDescriptor&);
    void instantiateServer(AdminSessionI*, const std::string&, const std::string&, const ServerInstanceDescriptor&);
    void removeApplicationDescriptor(AdminSessionI*, const std::string&, int = -1);

    ApplicationDescriptor getApplicationDescriptor(const std::string&);
    Ice::StringSeq getAllApplications(const std::string& = std::string());

    void addNode(const std::string&, const NodeSessionIPtr&);
    NodePrx getNode(const std::string&) const;
    NodeInfo getNodeInfo(const std::string&) const;
    void removeNode(const std::string&, const NodeSessionIPtr&, bool);
    Ice::StringSeq getAllNodes(const std::string& = std::string());

    void addReplica(const std::string&, const ReplicaSessionIPtr&);
    void removeReplica(const std::string&, const ReplicaSessionIPtr&, bool);

    ServerInfo getServerInfo(const std::string&, bool = false);
    ServerPrx getServer(const std::string&, bool = true);
    ServerPrx getServerWithTimeouts(const std::string&, int&, int&, std::string&, bool = true);
    Ice::StringSeq getAllServers(const std::string& = std::string());
    Ice::StringSeq getAllNodeServers(const std::string&);

    bool setAdapterDirectProxy(const std::string&, const std::string&, const Ice::ObjectPrx&, int = -1);
    Ice::ObjectPrx getAdapterDirectProxy(const std::string&);
    void removeAdapter(const std::string&);
    AdapterPrx getAdapter(const std::string&, const std::string&, bool = true);
    std::vector<std::pair<std::string, AdapterPrx> > getAdapters(const std::string&, int&, bool&);
    AdapterInfoSeq getAdapterInfo(const std::string&);
    Ice::StringSeq getAllAdapters(const std::string& = std::string());

    void addObject(const ObjectInfo&, bool = false, int = -1);
    void removeObject(const Ice::Identity&, int = -1);
    void updateObject(const Ice::ObjectPrx&, int = -1);

    void allocateObject(const Ice::Identity&, const ObjectAllocationRequestPtr&);
    void allocateObjectByType(const std::string&, const ObjectAllocationRequestPtr&);
    void releaseObject(const Ice::Identity&, const SessionIPtr&);

    Ice::ObjectPrx getObjectProxy(const Ice::Identity&);
    Ice::ObjectPrx getObjectByType(const std::string&);
    Ice::ObjectPrx getObjectByTypeOnLeastLoadedNode(const std::string&, LoadSample);
    Ice::ObjectProxySeq getObjectsByType(const std::string&);
    ObjectInfo getObjectInfo(const Ice::Identity&);
    ObjectInfoSeq getObjectInfosByType(const std::string&);
    ObjectInfoSeq getAllObjectInfos(const std::string& = std::string());

private:

    void checkForAddition(const ApplicationHelper&);
    void checkForUpdate(const ApplicationHelper&, const ApplicationHelper&);

    void checkServerForAddition(const std::string&);
    void checkAdapterForAddition(const std::string&);
    void checkObjectForAddition(const Ice::Identity&);

    void load(const ApplicationHelper&, ServerEntrySeq&, int);
    void unload(const ApplicationHelper&, ServerEntrySeq&);
    void reload(const ApplicationHelper&, const ApplicationHelper&, ServerEntrySeq&, int);
    void finishUpdate(ServerEntrySeq&, const ApplicationUpdateDescriptor&, const ApplicationInfo&, 
		      const ApplicationDescriptor&);

    void checkSessionLock(AdminSessionI*);

    friend struct AddComponent;

    static const std::string _applicationDbName;
    static const std::string _objectDbName;
    static const std::string _adapterDbName;
    static const std::string _replicaGroupDbName;
  
    const Ice::CommunicatorPtr _communicator;
    const Ice::ObjectAdapterPtr _internalAdapter;
    const IceStorm::TopicManagerPrx _topicManager;
    const std::string _envName;
    const std::string _instanceName;
    const TraceLevelsPtr _traceLevels;
    const int _sessionTimeout;

    ReplicaCache _replicaCache;
    NodeCache _nodeCache;
    AdapterCache _adapterCache;
    ObjectCache _objectCache;
    AllocatableObjectCache _allocatableObjectCache;
    ServerCache _serverCache;

    RegistryObserverTopicPtr _registryObserverTopic;
    NodeObserverTopicPtr _nodeObserverTopic;

    Ice::ObjectPrx _clientProxy;
    Ice::ObjectPrx _serverProxy;

    Freeze::ConnectionPtr _connection;
    StringApplicationInfoDict _applications;
    IdentityObjectInfoDict _objects;
    StringAdapterInfoDict _adapters;
    
    AdminSessionI* _lock;
    std::string _lockUserId;
    int _serial;
    std::set<std::string> _updating;
};
typedef IceUtil::Handle<Database> DatabasePtr;

};

#endif
