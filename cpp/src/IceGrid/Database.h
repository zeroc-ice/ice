// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <IceUtil/Mutex.h>
#include <IceUtil/Shared.h>
#include <Ice/CommunicatorF.h>
#include <IceGrid/Admin.h>
#include <IceGrid/Internal.h>
#include <IceGrid/ServerCache.h>
#include <IceGrid/NodeCache.h>
#include <IceGrid/ReplicaCache.h>
#include <IceGrid/ObjectCache.h>
#include <IceGrid/AllocatableObjectCache.h>
#include <IceGrid/AdapterCache.h>
#include <IceGrid/Topics.h>
#include <IceGrid/DB.h>

namespace IceGrid
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class NodeSessionI;
typedef IceUtil::Handle<NodeSessionI> NodeSessionIPtr;

class ReplicaSessionI;
typedef IceUtil::Handle<ReplicaSessionI> ReplicaSessionIPtr;

class AdminSessionI;

class ServerEntry;
typedef IceUtil::Handle<ServerEntry> ServerEntryPtr;

class ApplicationHelper;

class Database : public IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

#ifdef __SUNPRO_CC
    using IceUtil::Monitor<IceUtil::Mutex>::lock;
    using IceUtil::Monitor<IceUtil::Mutex>::unlock;
#endif


    Database(const Ice::ObjectAdapterPtr&, const IceStorm::TopicManagerPrx&, const std::string&, const TraceLevelsPtr&,
             const RegistryInfo&, const DatabasePluginPtr&, bool);
    virtual ~Database();
    
    std::string getInstanceName() const;
    bool isReadOnly() const { return _readonly; }
    const TraceLevelsPtr& getTraceLevels() const { return _traceLevels; }
    const Ice::CommunicatorPtr& getCommunicator() const { return _communicator; }
    const Ice::ObjectAdapterPtr& getInternalAdapter() { return _internalAdapter; }

    void destroyTopics();
    ObserverTopicPtr getObserverTopic(TopicName) const;

    int lock(AdminSessionI*, const std::string&);
    void unlock(AdminSessionI*);

    void syncApplications(const ApplicationInfoSeq&);
    void syncAdapters(const AdapterInfoSeq&);
    void syncObjects(const ObjectInfoSeq&);

    void addApplication(const ApplicationInfo&, AdminSessionI* = 0);
    void updateApplication(const ApplicationUpdateInfo&, bool, AdminSessionI* = 0);
    void syncApplicationDescriptor(const ApplicationDescriptor&, bool, AdminSessionI* = 0);
    void instantiateServer(const std::string&, const std::string&, const ServerInstanceDescriptor&, AdminSessionI* =0);
    void removeApplication(const std::string&, AdminSessionI* = 0);
    ApplicationInfo getApplicationInfo(const std::string&);
    Ice::StringSeq getAllApplications(const std::string& = std::string());
    void waitForApplicationUpdate(const AMD_NodeSession_waitForApplicationUpdatePtr&, const std::string&, int);

    NodeCache& getNodeCache();
    NodeEntryPtr getNode(const std::string&, bool = false) const;

    ReplicaCache& getReplicaCache();
    ReplicaEntryPtr getReplica(const std::string&) const;
    
    ServerCache& getServerCache();
    ServerEntryPtr getServer(const std::string&) const;

    AllocatableObjectCache& getAllocatableObjectCache();
    AllocatableObjectEntryPtr getAllocatableObject(const Ice::Identity&) const;

    void setAdapterDirectProxy(const std::string&, const std::string&, const Ice::ObjectPrx&);
    Ice::ObjectPrx getAdapterDirectProxy(const std::string&);

    void removeAdapter(const std::string&);
    AdapterPrx getAdapterProxy(const std::string&, const std::string&, bool);
    void getLocatorAdapterInfo(const std::string&, LocatorAdapterInfoSeq&, int&, bool&, bool&,
                               const std::set<std::string>& = std::set<std::string>());
    bool addAdapterSyncCallback(const std::string&, const SynchronizationCallbackPtr&, 
                                const std::set<std::string>& = std::set<std::string>());

    std::vector<std::pair<std::string, AdapterPrx> > getAdapters(const std::string&, int&, bool&);
    AdapterInfoSeq getAdapterInfo(const std::string&);
    Ice::StringSeq getAllAdapters(const std::string& = std::string());

    void addObject(const ObjectInfo&);
    void addOrUpdateObject(const ObjectInfo&);
    void removeObject(const Ice::Identity&);
    void updateObject(const Ice::ObjectPrx&);
    int addOrUpdateObjectsInDatabase(const ObjectInfoSeq&);
    void removeObjectsInDatabase(const ObjectInfoSeq&);

    Ice::ObjectPrx getObjectProxy(const Ice::Identity&);
    Ice::ObjectPrx getObjectByType(const std::string&);
    Ice::ObjectPrx getObjectByTypeOnLeastLoadedNode(const std::string&, LoadSample);
    Ice::ObjectProxySeq getObjectsByType(const std::string&);
    ObjectInfo getObjectInfo(const Ice::Identity&);
    ObjectInfoSeq getObjectInfosByType(const std::string&);
    ObjectInfoSeq getAllObjectInfos(const std::string& = std::string());

    void addInternalObject(const ObjectInfo&, bool = false);
    void removeInternalObject(const Ice::Identity&);
    Ice::ObjectProxySeq getInternalObjectsByType(const std::string&);

private:

    void checkForAddition(const ApplicationHelper&, const IceDB::DatabaseConnectionPtr&);
    void checkForUpdate(const ApplicationHelper&, const ApplicationHelper&, const IceDB::DatabaseConnectionPtr&);
    void checkForRemove(const ApplicationHelper&);

    void checkServerForAddition(const std::string&);
    void checkAdapterForAddition(const std::string&, const AdaptersWrapperPtr&);
    void checkObjectForAddition(const Ice::Identity&, const ObjectsWrapperPtr&);
    void checkReplicaGroupExists(const std::string&);
    void checkReplicaGroupForRemove(const std::string&);

    void load(const ApplicationHelper&, ServerEntrySeq&, const std::string&, int);
    void unload(const ApplicationHelper&, ServerEntrySeq&);
    void reload(const ApplicationHelper&, const ApplicationHelper&, ServerEntrySeq&, const std::string&, int, bool);

    void checkUpdate(const ApplicationHelper&, const ApplicationHelper&, const std::string&, int, bool);

    void saveApplication(const ApplicationInfo&, const IceDB::DatabaseConnectionPtr&);
    void removeApplication(const std::string&, const IceDB::DatabaseConnectionPtr&);

    void finishApplicationUpdate(const ApplicationUpdateInfo&, const ApplicationInfo&, const ApplicationHelper&,
                                 const ApplicationHelper&, AdminSessionI*, bool);

    void checkSessionLock(AdminSessionI*);

    void waitForUpdate(const std::string&);
    void startUpdating(const std::string&, const std::string&, int);
    void finishUpdating(const std::string&);
    
    friend struct AddComponent;

    static const std::string _applicationDbName;
    static const std::string _objectDbName;
    static const std::string _internalObjectDbName;
    static const std::string _adapterDbName;
    static const std::string _replicaGroupDbName;
  
    const Ice::CommunicatorPtr _communicator;
    const Ice::ObjectAdapterPtr _internalAdapter;
    const IceStorm::TopicManagerPrx _topicManager;
    const std::string _instanceName;
    const TraceLevelsPtr _traceLevels;
    const bool _master;
    const bool _readonly;

    ReplicaCache _replicaCache;
    NodeCache _nodeCache;
    AdapterCache _adapterCache;
    ObjectCache _objectCache;
    AllocatableObjectCache _allocatableObjectCache;
    ServerCache _serverCache;

    RegistryObserverTopicPtr _registryObserverTopic;
    NodeObserverTopicPtr _nodeObserverTopic;
    ApplicationObserverTopicPtr _applicationObserverTopic;
    AdapterObserverTopicPtr _adapterObserverTopic;
    ObjectObserverTopicPtr _objectObserverTopic;

    ConnectionPoolPtr _connectionPool;
    DatabasePluginPtr _databasePlugin;
    
    AdminSessionI* _lock;
    std::string _lockUserId;
    int _applicationSerial;
    int _replicaApplicationSerial;
    int _adapterSerial;
    int _objectSerial;

    struct UpdateInfo
    {
        std::string name;
        std::string uuid;
        int revision;
        std::vector<AMD_NodeSession_waitForApplicationUpdatePtr> cbs;
        bool updated;

        UpdateInfo(const std::string& n, const std::string& u, int r) :
            name(n), uuid(u), revision(r), updated(false)
        {
        }

        bool operator==(const std::string& n)
        {
            return name == n;
        }
        bool operator==(const std::pair<std::string, int>& p)
        {
            return uuid == p.first && revision == p.second;
        }

        void markUpdated()
        {
            updated = true;
            std::vector<AMD_NodeSession_waitForApplicationUpdatePtr>::const_iterator q;
            for(q = cbs.begin(); q != cbs.end(); ++q)
            {
                (*q)->ice_response();
            }
            cbs.clear();
        }

        void unmarkUpdated()
        {
            updated = false;
        }
    };
    std::vector<UpdateInfo> _updating;
};
typedef IceUtil::Handle<Database> DatabasePtr;

};
