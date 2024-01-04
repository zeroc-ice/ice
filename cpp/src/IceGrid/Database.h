//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_DATABASE_H
#define ICE_GRID_DATABASE_H

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

class AdminSessionI;
class ApplicationHelper;
class NodeSessionI;
class ReplicaSessionI;
class ServerEntry;
class TraceLevels;

using StringApplicationInfoMap = IceDB::Dbi<std::string,
                                            IceGrid::ApplicationInfo,
                                            IceDB::IceContext, Ice::OutputStream>;
using IdentityObjectInfoMap = IceDB::Dbi<Ice::Identity,
                                         IceGrid::ObjectInfo,
                                         IceDB::IceContext,
                                         Ice::OutputStream>;
using StringIdentityMap = IceDB::Dbi<std::string, Ice::Identity, IceDB::IceContext, Ice::OutputStream>;
using StringAdapterInfoMap = IceDB::Dbi<std::string, IceGrid::AdapterInfo, IceDB::IceContext, Ice::OutputStream>;
using StringStringMap = IceDB::Dbi<std::string, std::string, IceDB::IceContext, Ice::OutputStream>;
using StringLongMap = IceDB::Dbi<std::string, Ice::Long, IceDB::IceContext, Ice::OutputStream>;

class Database final
{
public:

    static std::shared_ptr<Database>
    create(const std::shared_ptr<Ice::ObjectAdapter>&, const std::shared_ptr<IceStorm::TopicManagerPrx>&,
           const std::string&, const std::shared_ptr<TraceLevels>&, const RegistryInfo&, bool);

    std::string getInstanceName() const;
    bool isReadOnly() const { return _readonly; }
    const std::shared_ptr<TraceLevels>& getTraceLevels() const { return _traceLevels; }
    const std::shared_ptr<Ice::Communicator>& getCommunicator() const { return _communicator; }
    const std::shared_ptr<Ice::ObjectAdapter>& getInternalAdapter() { return _internalAdapter; }

    void destroy();

    std::shared_ptr<ObserverTopic> getObserverTopic(TopicName) const;

    int lock(AdminSessionI*, const std::string&);
    void unlock(AdminSessionI*);

    void syncApplications(const ApplicationInfoSeq&, long long);
    void syncAdapters(const AdapterInfoSeq&, long long);
    void syncObjects(const ObjectInfoSeq&, long long);

    ApplicationInfoSeq getApplications(long long&);
    AdapterInfoSeq getAdapters(long long&);
    ObjectInfoSeq getObjects(long long&);

    StringLongDict getSerials() const;

    void addApplication(const ApplicationInfo&, AdminSessionI*, long long = 0);
    void updateApplication(const ApplicationUpdateInfo&, bool, AdminSessionI*, long long = 0);
    void syncApplicationDescriptor(const ApplicationDescriptor&, bool, AdminSessionI*);
    void instantiateServer(const std::string&, const std::string&, const ServerInstanceDescriptor&, AdminSessionI*);
    void removeApplication(const std::string&, AdminSessionI*, long long = 0);
    ApplicationInfo getApplicationInfo(const std::string&);
    Ice::StringSeq getAllApplications(const std::string& = std::string());
    void waitForApplicationUpdate(const std::string&, int, std::function<void()>,
                                  std::function<void(std::exception_ptr)>);

    NodeCache& getNodeCache();
    std::shared_ptr<NodeEntry> getNode(const std::string&, bool = false) const;

    ReplicaCache& getReplicaCache();
    std::shared_ptr<ReplicaEntry> getReplica(const std::string&) const;

    ServerCache& getServerCache();
    std::shared_ptr<ServerEntry> getServer(const std::string&) const;

    AllocatableObjectCache& getAllocatableObjectCache();
    std::shared_ptr<AllocatableObjectEntry> getAllocatableObject(const Ice::Identity&) const;

    void setAdapterDirectProxy(const std::string&, const std::string&, const std::shared_ptr<Ice::ObjectPrx>&,
                               long long = 0);
    std::shared_ptr<Ice::ObjectPrx> getAdapterDirectProxy(const std::string&, const Ice::EncodingVersion&,
                                                          const std::shared_ptr<Ice::Connection>&,
                                                          const Ice::Context&);

    void removeAdapter(const std::string&);
    std::shared_ptr<AdapterPrx> getAdapterProxy(const std::string&, const std::string&, bool);
    void getLocatorAdapterInfo(const std::string&, const std::shared_ptr<Ice::Connection>&, const Ice::Context&,
                               LocatorAdapterInfoSeq&, int&, bool&, bool&,
                               const std::set<std::string>& = std::set<std::string>());

    bool addAdapterSyncCallback(const std::string&, const std::shared_ptr<SynchronizationCallback>&,
                                const std::set<std::string>& = std::set<std::string>());

    std::vector<std::pair<std::string, std::shared_ptr<AdapterPrx>>> getAdapters(const std::string&, int&, bool&);
    AdapterInfoSeq getAdapterInfo(const std::string&);
    AdapterInfoSeq getFilteredAdapterInfo(const std::string&, const std::shared_ptr<Ice::Connection>&, const Ice::Context&);
    std::string getAdapterServer(const std::string&) const;
    std::string getAdapterApplication(const std::string&) const;
    std::string getAdapterNode(const std::string&) const;
    Ice::StringSeq getAllAdapters(const std::string& = std::string());

    void addObject(const ObjectInfo&);
    void addOrUpdateObject(const ObjectInfo&, long long = 0);
    void removeObject(const Ice::Identity&, long long = 0);
    void updateObject(const std::shared_ptr<Ice::ObjectPrx>&);
    int addOrUpdateRegistryWellKnownObjects(const ObjectInfoSeq&);
    int removeRegistryWellKnownObjects(const ObjectInfoSeq&);

    std::shared_ptr<Ice::ObjectPrx> getObjectProxy(const Ice::Identity&);
    std::shared_ptr<Ice::ObjectPrx> getObjectByType(const std::string&,
                                                    const std::shared_ptr<Ice::Connection>& = nullptr,
                                                    const Ice::Context& = Ice::Context());
    std::shared_ptr<Ice::ObjectPrx> getObjectByTypeOnLeastLoadedNode(const std::string&, LoadSample,
                                                                     const std::shared_ptr<Ice::Connection>& = nullptr,
                                                                     const Ice::Context& = Ice::Context());
    Ice::ObjectProxySeq getObjectsByType(const std::string&,
                                         const std::shared_ptr<Ice::Connection>& = nullptr,
                                         const Ice::Context& = Ice::Context());
    ObjectInfo getObjectInfo(const Ice::Identity&);
    ObjectInfoSeq getObjectInfosByType(const std::string&);
    ObjectInfoSeq getAllObjectInfos(const std::string& = std::string());

    void addInternalObject(const ObjectInfo&, bool = false);
    void removeInternalObject(const Ice::Identity&);
    Ice::ObjectProxySeq getInternalObjectsByType(const std::string&);

private:

    Database(const std::shared_ptr<Ice::ObjectAdapter>&, const std::shared_ptr<IceStorm::TopicManagerPrx>&,
             const std::string&, const std::shared_ptr<TraceLevels>&, const RegistryInfo&, bool);

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

    long long saveApplication(const ApplicationInfo&, const IceDB::ReadWriteTxn&, long long = 0);
    long long removeApplication(const std::string&, const IceDB::ReadWriteTxn&, long long = 0);

    void finishApplicationUpdate(const ApplicationUpdateInfo&, const ApplicationInfo&, const ApplicationHelper&,
                                 const ApplicationHelper&, AdminSessionI*, bool, long long = 0);

    void checkSessionLock(AdminSessionI*);

    void waitForUpdate(std::unique_lock<std::mutex>&, const std::string&);
    void startUpdating(const std::string&, const std::string&, int);
    void finishUpdating(const std::string&);

    long long getSerial(const IceDB::Txn&, const std::string&);
    long long updateSerial(const IceDB::ReadWriteTxn&, const std::string&, long long = 0);

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

    const std::shared_ptr<Ice::Communicator> _communicator;
    const std::shared_ptr<Ice::ObjectAdapter> _internalAdapter;
    const std::shared_ptr<IceStorm::TopicManagerPrx> _topicManager;
    const std::string _instanceName;
    const std::shared_ptr<TraceLevels> _traceLevels;
    const bool _master;
    const bool _readonly;

    ReplicaCache _replicaCache;
    NodeCache _nodeCache;
    AdapterCache _adapterCache;
    ObjectCache _objectCache;
    AllocatableObjectCache _allocatableObjectCache;
    ServerCache _serverCache;

    std::shared_ptr<RegistryObserverTopic> _registryObserverTopic;
    std::shared_ptr<NodeObserverTopic> _nodeObserverTopic;
    std::shared_ptr<ApplicationObserverTopic> _applicationObserverTopic;
    std::shared_ptr<AdapterObserverTopic> _adapterObserverTopic;
    std::shared_ptr<ObjectObserverTopic> _objectObserverTopic;

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

    std::shared_ptr<RegistryPluginFacadeI> _pluginFacade;

    AdminSessionI* _lock;
    std::string _lockUserId;

    struct UpdateInfo
    {
        std::string name;
        std::string uuid;
        int revision;
        std::vector<std::pair<std::function<void()>, std::function<void(std::exception_ptr)>>> cbs;
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
            for(const auto& cb: cbs)
            {
                cb.first();
            }
            cbs.clear();
        }

        void unmarkUpdated()
        {
            updated = false;
        }
    };
    std::vector<UpdateInfo> _updating;

    mutable std::mutex _mutex;
    std::condition_variable _condVar;
};

}

#endif
