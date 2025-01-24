// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_DATABASE_H
#define ICEGRID_DATABASE_H

#include "../Ice/FileUtil.h"
#include "../IceDB/IceDB.h"
#include "AdapterCache.h"
#include "AllocatableObjectCache.h"
#include "Ice/CommunicatorF.h"
#include "IceGrid/Admin.h"
#include "Internal.h"
#include "NodeCache.h"
#include "ObjectCache.h"
#include "PluginFacadeI.h"
#include "ReplicaCache.h"
#include "ServerCache.h"
#include "Topics.h"

namespace IceGrid
{
    class AdminSessionI;
    class ApplicationHelper;
    class NodeSessionI;
    class ReplicaSessionI;
    class ServerEntry;
    class TraceLevels;

    using StringApplicationInfoMap =
        IceDB::Dbi<std::string, IceGrid::ApplicationInfo, IceDB::IceContext, Ice::OutputStream>;
    using IdentityObjectInfoMap = IceDB::Dbi<Ice::Identity, IceGrid::ObjectInfo, IceDB::IceContext, Ice::OutputStream>;
    using StringIdentityMap = IceDB::Dbi<std::string, Ice::Identity, IceDB::IceContext, Ice::OutputStream>;
    using StringAdapterInfoMap = IceDB::Dbi<std::string, IceGrid::AdapterInfo, IceDB::IceContext, Ice::OutputStream>;
    using StringStringMap = IceDB::Dbi<std::string, std::string, IceDB::IceContext, Ice::OutputStream>;
    using StringLongMap = IceDB::Dbi<std::string, std::int64_t, IceDB::IceContext, Ice::OutputStream>;

    class Database final
    {
    public:
        static std::shared_ptr<Database> create(
            const Ice::ObjectAdapterPtr&,
            IceStorm::TopicManagerPrx,
            const std::string&,
            const std::shared_ptr<TraceLevels>&,
            const RegistryInfo&,
            bool);

        [[nodiscard]] std::string getInstanceName() const;
        [[nodiscard]] bool isReadOnly() const { return _readonly; }
        [[nodiscard]] const std::shared_ptr<TraceLevels>& getTraceLevels() const { return _traceLevels; }
        [[nodiscard]] const Ice::CommunicatorPtr& getCommunicator() const { return _communicator; }
        const Ice::ObjectAdapterPtr& getInternalAdapter() { return _internalAdapter; }

        void destroy();

        [[nodiscard]] std::shared_ptr<ObserverTopic> getObserverTopic(TopicName) const;

        int lock(AdminSessionI*, const std::string&);
        void unlock(AdminSessionI*);

        void syncApplications(const ApplicationInfoSeq&, std::int64_t);
        void syncAdapters(const AdapterInfoSeq&, std::int64_t);
        void syncObjects(const ObjectInfoSeq&, std::int64_t);

        ApplicationInfoSeq getApplications(std::int64_t&);
        AdapterInfoSeq getAdapters(std::int64_t&);
        ObjectInfoSeq getObjects(std::int64_t&);

        [[nodiscard]] StringLongDict getSerials() const;

        void addApplication(const ApplicationInfo&, AdminSessionI*, std::int64_t = 0);
        void updateApplication(const ApplicationUpdateInfo&, bool, AdminSessionI*, std::int64_t = 0);
        void syncApplicationDescriptor(const ApplicationDescriptor&, bool, AdminSessionI*);
        void instantiateServer(const std::string&, const std::string&, const ServerInstanceDescriptor&, AdminSessionI*);
        void removeApplication(const std::string&, AdminSessionI*, std::int64_t = 0);
        ApplicationInfo getApplicationInfo(const std::string&);
        Ice::StringSeq getAllApplications(const std::string& = std::string());
        void waitForApplicationUpdate(
            const std::string&,
            int,
            std::function<void()>,
            std::function<void(std::exception_ptr)>);

        NodeCache& getNodeCache();
        [[nodiscard]] std::shared_ptr<NodeEntry> getNode(const std::string&, bool = false) const;

        ReplicaCache& getReplicaCache();
        [[nodiscard]] std::shared_ptr<ReplicaEntry> getReplica(const std::string&) const;

        ServerCache& getServerCache();
        [[nodiscard]] std::shared_ptr<ServerEntry> getServer(const std::string&) const;

        AllocatableObjectCache& getAllocatableObjectCache();
        [[nodiscard]] std::shared_ptr<AllocatableObjectEntry> getAllocatableObject(const Ice::Identity&) const;

        void setAdapterDirectProxy(
            const std::string&,
            const std::string&,
            const std::optional<Ice::ObjectPrx>&,
            std::int64_t = 0);

        std::optional<Ice::ObjectPrx> getAdapterDirectProxy(
            const std::string&,
            const Ice::EncodingVersion&,
            const Ice::ConnectionPtr&,
            const Ice::Context&);

        void removeAdapter(const std::string&);

        std::optional<AdapterPrx> getAdapterProxy(const std::string&, const std::string&, bool);

        void getLocatorAdapterInfo(
            const std::string&,
            const Ice::ConnectionPtr&,
            const Ice::Context&,
            LocatorAdapterInfoSeq&,
            int&,
            bool&,
            bool&,
            const std::set<std::string>& = std::set<std::string>());

        bool addAdapterSyncCallback(
            const std::string&,
            const std::shared_ptr<SynchronizationCallback>&,
            const std::set<std::string>& = std::set<std::string>());

        std::vector<std::pair<std::string, std::optional<AdapterPrx>>> getAdapters(const std::string&, int&, bool&);

        AdapterInfoSeq getAdapterInfo(const std::string&);

        AdapterInfoSeq getFilteredAdapterInfo(const std::string&, const Ice::ConnectionPtr&, const Ice::Context&);

        [[nodiscard]] std::string getAdapterServer(const std::string&) const;
        [[nodiscard]] std::string getAdapterApplication(const std::string&) const;
        [[nodiscard]] std::string getAdapterNode(const std::string&) const;
        Ice::StringSeq getAllAdapters(const std::string& = std::string());

        void addObject(const ObjectInfo&);
        void addOrUpdateObject(const ObjectInfo&, std::int64_t = 0);
        void removeObject(const Ice::Identity&, std::int64_t = 0);
        void updateObject(Ice::ObjectPrx);
        int addOrUpdateRegistryWellKnownObjects(const ObjectInfoSeq&);
        int removeRegistryWellKnownObjects(const ObjectInfoSeq&);

        Ice::ObjectPrx getObjectProxy(const Ice::Identity&);

        std::optional<Ice::ObjectPrx>
        getObjectByType(const std::string&, const Ice::ConnectionPtr& = nullptr, const Ice::Context& = Ice::Context());

        std::optional<Ice::ObjectPrx> getObjectByTypeOnLeastLoadedNode(
            const std::string&,
            LoadSample,
            const Ice::ConnectionPtr& = nullptr,
            const Ice::Context& = Ice::Context());

        Ice::ObjectProxySeq
        getObjectsByType(const std::string&, const Ice::ConnectionPtr& = nullptr, const Ice::Context& = Ice::Context());
        ObjectInfo getObjectInfo(const Ice::Identity&);
        ObjectInfoSeq getObjectInfosByType(const std::string&);
        ObjectInfoSeq getAllObjectInfos(const std::string& = std::string());

        void addInternalObject(const ObjectInfo&, bool = false);
        void removeInternalObject(const Ice::Identity&);
        Ice::ObjectProxySeq getInternalObjectsByType(const std::string&);

    private:
        Database(
            const Ice::ObjectAdapterPtr&,
            IceStorm::TopicManagerPrx,
            std::string,
            const std::shared_ptr<TraceLevels>&,
            const RegistryInfo&,
            bool);

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

        std::int64_t saveApplication(const ApplicationInfo&, const IceDB::ReadWriteTxn&, std::int64_t = 0);
        std::int64_t removeApplication(const std::string&, const IceDB::ReadWriteTxn&, std::int64_t = 0);

        void finishApplicationUpdate(
            const ApplicationUpdateInfo&,
            const ApplicationInfo&,
            const ApplicationHelper&,
            const ApplicationHelper&,
            AdminSessionI*,
            bool,
            std::int64_t = 0);

        void checkSessionLock(AdminSessionI*);

        void waitForUpdate(std::unique_lock<std::mutex>&, const std::string&);
        void startUpdating(const std::string&, const std::string&, int);
        void finishUpdating(const std::string&);

        std::int64_t getSerial(const IceDB::Txn&, const std::string&);
        std::int64_t updateSerial(const IceDB::ReadWriteTxn&, const std::string&, std::int64_t = 0);

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

        IceInternal::FileLock _dbLock;
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

        AdminSessionI* _lock{nullptr};
        std::string _lockUserId;

        struct UpdateInfo
        {
            std::string name;
            std::string uuid;
            int revision;
            std::vector<std::pair<std::function<void()>, std::function<void(std::exception_ptr)>>> cbs;
            bool updated{false};

            UpdateInfo(std::string n, std::string u, int r) : name(std::move(n)), uuid(std::move(u)), revision(r) {}

            bool operator==(const std::string& n) { return name == n; }
            bool operator==(const std::pair<std::string, int>& p) { return uuid == p.first && revision == p.second; }

            void markUpdated()
            {
                updated = true;
                for (const auto& cb : cbs)
                {
                    cb.first();
                }
                cbs.clear();
            }

            void unmarkUpdated() { updated = false; }
        };
        std::vector<UpdateInfo> _updating;

        mutable std::mutex _mutex;
        std::condition_variable _condVar;
    };
}

#endif
