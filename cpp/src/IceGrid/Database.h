//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_DATABASE_H
#define ICE_GRID_DATABASE_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Shared.h>
#include <IceUtil/FileUtil.h>
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
#include <IceGrid/PluginFacadeI.h>

#include <IceDB/IceDB.h>

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

typedef IceDB::Dbi<std::string, IceGrid::ApplicationInfo, IceDB::IceContext, Ice::OutputStream>
    StringApplicationInfoMap;

typedef IceDB::Dbi<Ice::Identity, IceGrid::ObjectInfo, IceDB::IceContext, Ice::OutputStream> IdentityObjectInfoMap;
typedef IceDB::Dbi<std::string, Ice::Identity, IceDB::IceContext, Ice::OutputStream> StringIdentityMap;

typedef IceDB::Dbi<std::string, IceGrid::AdapterInfo, IceDB::IceContext, Ice::OutputStream> StringAdapterInfoMap;
typedef IceDB::Dbi<std::string, std::string, IceDB::IceContext, Ice::OutputStream> StringStringMap;

typedef IceDB::Dbi<std::string, Ice::Long, IceDB::IceContext, Ice::OutputStream> StringLongMap;

class Database : public IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

#ifdef __SUNPRO_CC
    using IceUtil::Monitor<IceUtil::Mutex>::lock;
    using IceUtil::Monitor<IceUtil::Mutex>::unlock;
#endif

    Database(const Ice::ObjectAdapterPtr&, const IceStorm::TopicManagerPrx&, const std::string&, const TraceLevelsPtr&,
             const RegistryInfo&, bool);

    std::string getInstanceName() const;
    bool isReadOnly() const { return _readonly; }
    const TraceLevelsPtr& getTraceLevels() const { return _traceLevels; }
    const Ice::CommunicatorPtr& getCommunicator() const { return _communicator; }
    const Ice::ObjectAdapterPtr& getInternalAdapter() { return _internalAdapter; }

    void destroy();

    ObserverTopicPtr getObserverTopic(TopicName) const;

    int lock(AdminSessionI*, const std::string&);
    void unlock(AdminSessionI*);

    void syncApplications(const ApplicationInfoSeq&, Ice::Long);
    void syncAdapters(const AdapterInfoSeq&, Ice::Long);
    void syncObjects(const ObjectInfoSeq&, Ice::Long);

    ApplicationInfoSeq getApplications(Ice::Long&);
    AdapterInfoSeq getAdapters(Ice::Long&);
    ObjectInfoSeq getObjects(Ice::Long&);

    StringLongDict getSerials() const;

    void addApplication(const ApplicationInfo&, AdminSessionI*, Ice::Long = 0);
    void updateApplication(const ApplicationUpdateInfo&, bool, AdminSessionI*, Ice::Long = 0);
    void syncApplicationDescriptor(const ApplicationDescriptor&, bool, AdminSessionI*);
    void instantiateServer(const std::string&, const std::string&, const ServerInstanceDescriptor&, AdminSessionI*);
    void removeApplication(const std::string&, AdminSessionI*, Ice::Long = 0);
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

    void setAdapterDirectProxy(const std::string&, const std::string&, const Ice::ObjectPrx&, Ice::Long = 0);
    Ice::ObjectPrx getAdapterDirectProxy(const std::string&, const Ice::EncodingVersion&, const Ice::ConnectionPtr&,
                                         const Ice::Context&);

    void removeAdapter(const std::string&);
    AdapterPrx getAdapterProxy(const std::string&, const std::string&, bool);
    void getLocatorAdapterInfo(const std::string&, const Ice::ConnectionPtr&, const Ice::Context&,
                               LocatorAdapterInfoSeq&, int&, bool&, bool&,
                               const std::set<std::string>& = std::set<std::string>());

    bool addAdapterSyncCallback(const std::string&, const SynchronizationCallbackPtr&,
                                const std::set<std::string>& = std::set<std::string>());

    std::vector<std::pair<std::string, AdapterPrx> > getAdapters(const std::string&, int&, bool&);
    AdapterInfoSeq getAdapterInfo(const std::string&);
    AdapterInfoSeq getFilteredAdapterInfo(const std::string&, const Ice::ConnectionPtr&, const Ice::Context&);
    std::string getAdapterServer(const std::string&) const;
    std::string getAdapterApplication(const std::string&) const;
    std::string getAdapterNode(const std::string&) const;
    Ice::StringSeq getAllAdapters(const std::string& = std::string());

    void addObject(const ObjectInfo&);
    void addOrUpdateObject(const ObjectInfo&, Ice::Long = 0);
    void removeObject(const Ice::Identity&, Ice::Long = 0);
    void updateObject(const Ice::ObjectPrx&);
    int addOrUpdateRegistryWellKnownObjects(const ObjectInfoSeq&);
    int removeRegistryWellKnownObjects(const ObjectInfoSeq&);

    Ice::ObjectPrx getObjectProxy(const Ice::Identity&);
    Ice::ObjectPrx getObjectByType(const std::string&,
                                   const Ice::ConnectionPtr& = Ice::ConnectionPtr(),
                                   const Ice::Context& = Ice::Context());
    Ice::ObjectPrx getObjectByTypeOnLeastLoadedNode(const std::string&, LoadSample,
                                                    const Ice::ConnectionPtr& = Ice::ConnectionPtr(),
                                                    const Ice::Context& = Ice::Context());
    Ice::ObjectProxySeq getObjectsByType(const std::string&,
                                         const Ice::ConnectionPtr& = Ice::ConnectionPtr(),
                                         const Ice::Context& = Ice::Context());
    ObjectInfo getObjectInfo(const Ice::Identity&);
    ObjectInfoSeq getObjectInfosByType(const std::string&);
    ObjectInfoSeq getAllObjectInfos(const std::string& = std::string());

    void addInternalObject(const ObjectInfo&, bool = false);
    void removeInternalObject(const Ice::Identity&);
    Ice::ObjectProxySeq getInternalObjectsByType(const std::string&);

private:

    void checkForAddition(const ApplicationHelper&, const IceDB::ReadWriteTxn&);
    void checkForUpdate(const ApplicationHelper&, const ApplicationHelper&, const IceDB::ReadWriteTxn&);
    void checkForRemove(const ApplicationHelper&);

    void checkServerForAddition(const std::string&);
    void checkAdapterForAddition(const std::string&, const IceDB::ReadWriteTxn&);
    void checkObjectForAddition(const Ice::Identity&, const IceDB::ReadWriteTxn&);
    void checkReplicaGroupExists(const std::string&);
    void checkReplicaGroupForRemove(const std::string&);

    void load(const ApplicationHelper&, ServerEntrySeq&, const std::string&, int);
    void unload(const ApplicationHelper&, ServerEntrySeq&);
    void reload(const ApplicationHelper&, const ApplicationHelper&, ServerEntrySeq&, const std::string&, int, bool);

    void checkUpdate(const ApplicationHelper&, const ApplicationHelper&, const std::string&, int, bool);

    Ice::Long saveApplication(const ApplicationInfo&, const IceDB::ReadWriteTxn&, Ice::Long = 0);
    Ice::Long removeApplication(const std::string&, const IceDB::ReadWriteTxn&, Ice::Long = 0);

    void finishApplicationUpdate(const ApplicationUpdateInfo&, const ApplicationInfo&, const ApplicationHelper&,
                                 const ApplicationHelper&, AdminSessionI*, bool, Ice::Long = 0);

    void checkSessionLock(AdminSessionI*);

    void waitForUpdate(const std::string&);
    void startUpdating(const std::string&, const std::string&, int);
    void finishUpdating(const std::string&);

    Ice::Long getSerial(const IceDB::Txn&, const std::string&);
    Ice::Long updateSerial(const IceDB::ReadWriteTxn&, const std::string&, Ice::Long = 0);

    void addAdapter(const IceDB::ReadWriteTxn&, const AdapterInfo&);
    void deleteAdapter(const IceDB::ReadWriteTxn&, const AdapterInfo&);

    void addObject(const IceDB::ReadWriteTxn&, const ObjectInfo&, bool);
    void deleteObject(const IceDB::ReadWriteTxn&, const ObjectInfo&, bool);

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

    IceUtilInternal::FileLock _dbLock;
    IceDB::Env _env;

    StringApplicationInfoMap _applications;

    StringAdapterInfoMap _adapters;
    StringStringMap _adaptersByGroupId;

    IdentityObjectInfoMap _objects;
    StringIdentityMap _objectsByType;

    IdentityObjectInfoMap _internalObjects;
    StringIdentityMap _internalObjectsByType;

    StringLongMap _serials;

    RegistryPluginFacadeIPtr _pluginFacade;

    AdminSessionI* _lock;
    std::string _lockUserId;

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

#endif
