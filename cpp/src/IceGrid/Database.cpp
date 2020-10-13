//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/StringUtil.h>
#include <IceUtil/Random.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Communicator.h>
#include <Ice/ObjectAdapter.h>
#include <IceGrid/Database.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/Util.h>
#include <IceGrid/DescriptorHelper.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/ReplicaSessionI.h>
#include <IceGrid/Session.h>
#include <IceGrid/Topics.h>
#include <IceGrid/IceGrid.h>

#include <algorithm>
#include <functional>
#include <iterator>

using namespace std;
using namespace IceGrid;

using ApplicationMapRWCursor = IceDB::ReadWriteCursor<string, ApplicationInfo, IceDB::IceContext, Ice::OutputStream>;
using AdapterMapROCursor = IceDB::ReadOnlyCursor<string, AdapterInfo, IceDB::IceContext, Ice::OutputStream>;
using AdaptersByGroupMapCursor = IceDB::Cursor<string, string, IceDB::IceContext, Ice::OutputStream>;
using ObjectsByTypeMapROCursor = IceDB::ReadOnlyCursor<string, Ice::Identity, IceDB::IceContext, Ice::OutputStream>;
using ObjectsMapROCursor = IceDB::ReadOnlyCursor<Ice::Identity, ObjectInfo, IceDB::IceContext, Ice::OutputStream>;

namespace
{

const string applicationsDbName = "applications";
const string adaptersDbName = "adapters";
const string adaptersByReplicaGroupIdDbName = "adaptersByReplicaGroupId";
const string objectsDbName = "objects";
const string objectsByTypeDbName = "objectsByType";
const string internalObjectsDbName = "internal-objects";
const string internalObjectsByTypeDbName = "internal-objectsByType";
const string serialsDbName = "serials";

template<typename K, typename V, typename C, typename H> vector<V>
toVector(const IceDB::ReadOnlyTxn& txn, const IceDB::Dbi<K, V, C, H>& m)
{
    vector<V> v;
    IceDB::ReadOnlyCursor<K, V, C, H> cursor(m, txn);
    K key;
    V value;
    while(cursor.get(key, value, MDB_NEXT))
    {
        v.push_back(value);
    }
    return v;
}

template<typename K, typename V, typename C, typename H> map<K, V>
toMap(const IceDB::Txn& txn, const IceDB::Dbi<K, V, C, H>& d)
{
    std::map<K, V> m;
    IceDB::Cursor<K, V, C, H> cursor(d, txn);
    K key;
    V value;
    while(cursor.get(key, value, MDB_NEXT))
    {
        typename std::map<K, V>::value_type v(key, value);
        m.insert(v);
    }
    cursor.close();
    return m;
}

void
logError(const shared_ptr<Ice::Communicator>& com, const IceDB::LMDBException& ex)
{
    Ice::Error error(com->getLogger());
    error << "LMDB error: " << ex;
}

void
filterAdapterInfos(const string& filter,
                   const string& replicaGroupId,
                   const shared_ptr<RegistryPluginFacadeI>& pluginFacade,
                   const shared_ptr<Ice::Connection>& con,
                   const Ice::Context& ctx,
                   AdapterInfoSeq& infos)
{
    if(infos.empty() || !pluginFacade->hasReplicaGroupFilters())
    {
        return;
    }

    auto filters = pluginFacade->getReplicaGroupFilters(filter);
    if(filters.empty())
    {
        return;
    }

    Ice::StringSeq adapterIds;
    adapterIds.reserve(infos.size());
    for(const auto& info : infos)
    {
        adapterIds.push_back(info.id);
    }

    for(const auto& f : filters)
    {
        adapterIds = f->filter(replicaGroupId, adapterIds, con, ctx);
    }

    vector<AdapterInfo> filteredAdpts;
    filteredAdpts.reserve(infos.size());
    for(const auto& id : adapterIds)
    {
        for(const auto& info : infos)
        {
            if(id == info.id)
            {
                filteredAdpts.push_back(info);
                break;
            }
        }
    }
    infos.swap(filteredAdpts);
}

vector<AdapterInfo>
findByReplicaGroupId(const IceDB::Txn& txn,
                     const StringAdapterInfoMap& adapters,
                     const StringStringMap& adaptersByGroupId,
                     const string& name)
{
    vector<AdapterInfo> result;
    AdaptersByGroupMapCursor cursor(adaptersByGroupId, txn);
    string id;
    if(cursor.find(name, id))
    {
        AdapterInfo info;
        adapters.get(txn, id, info);
        result.push_back(info);

        string n;
        while(cursor.get(n, id, MDB_NEXT) && n == name)
        {
            adapters.get(txn, id, info);
            result.push_back(info);
        }
    }
    return result;
}

vector<ObjectInfo>
findByType(const IceDB::ReadOnlyTxn& txn,
           const IdentityObjectInfoMap& objects,
           const StringIdentityMap& objectsByType,
           const string& type)
{
    vector<ObjectInfo> result;
    ObjectsByTypeMapROCursor cursor(objectsByType, txn);
    Ice::Identity id;
    if(cursor.find(type, id))
    {
        ObjectInfo info;
        objects.get(txn, id, info);
        result.push_back(info);

        string t;
        while(cursor.get(t, id, MDB_NEXT) && t == type)
        {
            objects.get(txn, id, info);
            result.push_back(info);
        }
    }
    return result;
}

}

shared_ptr<Database>
Database::create(const shared_ptr<Ice::ObjectAdapter>& registryAdapter,
                 const shared_ptr<IceStorm::TopicManagerPrx>& topicManager,
                 const string& instanceName,
                 const shared_ptr<TraceLevels>& traceLevels,
                 const RegistryInfo& info,
                 bool readonly)
{
    shared_ptr<Database> db(new Database(registryAdapter, topicManager, instanceName, traceLevels, info, readonly));

    db->_pluginFacade->setDatabase(db);

    return db;
}

Database::Database(const shared_ptr<Ice::ObjectAdapter>& registryAdapter,
                   const shared_ptr<IceStorm::TopicManagerPrx>& topicManager,
                   const string& instanceName,
                   const shared_ptr<TraceLevels>& traceLevels,
                   const RegistryInfo& info,
                   bool readonly) :
    _communicator(registryAdapter->getCommunicator()),
    _internalAdapter(registryAdapter),
    _topicManager(topicManager),
    _instanceName(instanceName),
    _traceLevels(traceLevels),
    _master(info.name == "Master"),
    _readonly(readonly || !_master),
    _replicaCache(_communicator, topicManager),
    _nodeCache(_communicator, _replicaCache, _readonly && _master ? string("Master (read-only)") : info.name),
    _adapterCache(_communicator),
    _objectCache(_communicator),
    _allocatableObjectCache(_communicator),
    _serverCache(_communicator, _instanceName, _nodeCache, _adapterCache, _objectCache, _allocatableObjectCache),
    _dbLock(_communicator->getProperties()->getProperty("IceGrid.Registry.LMDB.Path") + "/icedb.lock"),
    _env(_communicator->getProperties()->getProperty("IceGrid.Registry.LMDB.Path"), 8,
         IceDB::getMapSize(_communicator->getProperties()->getPropertyAsInt("IceGrid.Registry.LMDB.MapSize"))),
    _pluginFacade(dynamic_pointer_cast<RegistryPluginFacadeI>(getRegistryPluginFacade())),
    _lock(nullptr)
{
    IceDB::ReadWriteTxn txn(_env);

    IceDB::IceContext context;
    context.communicator = _communicator;
    context.encoding.major = 1;
    context.encoding.minor = 1;

    _applications = StringApplicationInfoMap(txn, applicationsDbName, context, MDB_CREATE);

    _adapters = StringAdapterInfoMap(txn, adaptersDbName, context, MDB_CREATE);
    _adaptersByGroupId = StringStringMap(txn, adaptersByReplicaGroupIdDbName, context, MDB_CREATE|MDB_DUPSORT);

    _objects = IdentityObjectInfoMap(txn, objectsDbName, context, MDB_CREATE);
    _objectsByType = StringIdentityMap(txn, objectsByTypeDbName, context, MDB_CREATE|MDB_DUPSORT);

    _internalObjects = IdentityObjectInfoMap(txn, internalObjectsDbName, context, MDB_CREATE);
    _internalObjectsByType = StringIdentityMap(txn, internalObjectsByTypeDbName, context, MDB_CREATE|MDB_DUPSORT);

    _serials = StringLongMap(txn, serialsDbName, context, MDB_CREATE);

    ServerEntrySeq entries;

    string k;
    ApplicationInfo v;
    ApplicationMapRWCursor cursor(_applications, txn);
    while(cursor.get(k, v, MDB_NEXT))
    {
        try
        {
            load(ApplicationHelper(_communicator, v.descriptor), entries, v.uuid, v.revision);
        }
        catch(const DeploymentException& ex)
        {
            Ice::Error err(_traceLevels->logger);
            err << "invalid application `" << k << "':\n" << ex.reason;
        }
    }

    _serverCache.setTraceLevels(_traceLevels);
    _nodeCache.setTraceLevels(_traceLevels);
    _replicaCache.setTraceLevels(_traceLevels);
    _adapterCache.setTraceLevels(_traceLevels);
    _objectCache.setTraceLevels(_traceLevels);
    _allocatableObjectCache.setTraceLevels(_traceLevels);

    _nodeObserverTopic = NodeObserverTopic::create(_topicManager, _internalAdapter);
    _registryObserverTopic = make_shared<RegistryObserverTopic>(_topicManager);

    _serverCache.setNodeObserverTopic(_nodeObserverTopic);

    // Set all serials to 1 if they have not yet been set.
    long long serial;
    if(!_serials.get(txn, applicationsDbName, serial))
    {
        _serials.put(txn, applicationsDbName, 1);
    }
    if(!_serials.get(txn, adaptersDbName, serial))
    {
        _serials.put(txn, adaptersDbName, 1);
    }
    if(!_serials.get(txn, objectsDbName, serial))
    {
        _serials.put(txn, objectsDbName, 1);
    }

    _applicationObserverTopic = make_shared<ApplicationObserverTopic>(_topicManager, toMap(txn, _applications),
                                                                      getSerial(txn, applicationsDbName));
    _adapterObserverTopic = make_shared<AdapterObserverTopic>(_topicManager, toMap(txn, _adapters),
                                                              getSerial(txn, adaptersDbName));
    _objectObserverTopic = make_shared<ObjectObserverTopic>(_topicManager, toMap(txn, _objects),
                                                            getSerial(txn, objectsDbName));

    txn.commit();

    _registryObserverTopic->registryUp(info);
}

std::string
Database::getInstanceName() const
{
    return _instanceName;
}

void
Database::destroy()
{
    _pluginFacade->setDatabase(nullptr);

    _registryObserverTopic->destroy();
    _nodeObserverTopic->destroy();
    _applicationObserverTopic->destroy();
    _adapterObserverTopic->destroy();
    _objectObserverTopic->destroy();
}

shared_ptr<ObserverTopic>
Database::getObserverTopic(TopicName name) const
{
    switch(name)
    {
    case TopicName::RegistryObserver:
        return _registryObserverTopic;
    case TopicName::NodeObserver:
        return _nodeObserverTopic;
    case TopicName::ApplicationObserver:
        return _applicationObserverTopic;
    case TopicName::AdapterObserver:
        return _adapterObserverTopic;
    case TopicName::ObjectObserver:
        return _objectObserverTopic;
    default:
        break;
    }
    return nullptr;
}

void
Database::checkSessionLock(AdminSessionI* session)
{
    if(_lock != nullptr && session != _lock)
    {
        throw AccessDeniedException(_lockUserId); // Lock held by another session.
    }
}

int
Database::lock(AdminSessionI* session, const string& userId)
{
    lock_guard lock(_mutex);

    if(_lock != nullptr && session != _lock)
    {
        throw AccessDeniedException(_lockUserId); // Lock held by another session.
    }
    assert(_lock == nullptr || _lock == session);

    _lock = session;
    _lockUserId = userId;

    return _applicationObserverTopic->getSerial();
}

void
Database::unlock(AdminSessionI* session)
{
    lock_guard lock(_mutex);

    if(_lock != session)
    {
        throw AccessDeniedException();
    }

    _lock = nullptr;
    _lockUserId.clear();
}

void
Database::syncApplications(const ApplicationInfoSeq& newApplications, long long dbSerial)
{
    assert(dbSerial != 0);
    int serial = 0;
    {
        lock_guard lock(_mutex);

        map<string, ApplicationInfo> oldApplications;
        try
        {
            IceDB::ReadWriteTxn txn(_env);

            oldApplications = toMap(txn, _applications);
            _applications.clear(txn);
            for(ApplicationInfoSeq::const_iterator p = newApplications.begin(); p != newApplications.end(); ++p)
            {
                _applications.put(txn, p->descriptor.name, *p);
            }
            dbSerial = updateSerial(txn, applicationsDbName, dbSerial);

            txn.commit();
        }
        catch(const IceDB::LMDBException& ex)
        {
            logError(_communicator, ex);
            throw;
        }

        ServerEntrySeq entries;
        set<string> names;

        for(ApplicationInfoSeq::const_iterator p = newApplications.begin(); p != newApplications.end(); ++p)
        {
            try
            {
                map<string, ApplicationInfo>::const_iterator q = oldApplications.find(p->descriptor.name);
                if(q != oldApplications.end())
                {
                    ApplicationHelper previous(_communicator, q->second.descriptor);
                    ApplicationHelper helper(_communicator, p->descriptor);
                    reload(previous, helper, entries, p->uuid, p->revision, false);
                }
                else
                {
                    load(ApplicationHelper(_communicator, p->descriptor), entries, p->uuid, p->revision);
                }
            }
            catch(const DeploymentException& ex)
            {
                Ice::Warning warn(_traceLevels->logger);
                warn << "invalid application `" << p->descriptor.name << "':\n" << ex.reason;
            }
            names.insert(p->descriptor.name);
        }

        for(map<string, ApplicationInfo>::iterator s = oldApplications.begin(); s != oldApplications.end(); ++s)
        {
            if(names.find(s->first) == names.end())
            {
                unload(ApplicationHelper(_communicator, s->second.descriptor), entries);
            }
        }

        for(const auto& entry : entries)
        {
            entry->sync();
        }

        if(_traceLevels->application > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->applicationCat);
            out << "synchronized applications (serial = `" << dbSerial << "')";
        }

        serial = _applicationObserverTopic->applicationInit(dbSerial, newApplications);
    }
    _applicationObserverTopic->waitForSyncedSubscribers(serial);
}

void
Database::syncAdapters(const AdapterInfoSeq& adapters, long long dbSerial)
{
    assert(dbSerial != 0);
    int serial = 0;
    {
        lock_guard lock(_mutex);
        try
        {
            IceDB::ReadWriteTxn txn(_env);

            _adapters.clear(txn);
            _adaptersByGroupId.clear(txn);
            for(const auto& adapter : adapters)
            {
                addAdapter(txn, adapter);
            }
            dbSerial = updateSerial(txn, adaptersDbName, dbSerial);

            txn.commit();
        }
        catch(const IceDB::KeyTooLongException&)
        {
            throw;
        }
        catch(const IceDB::LMDBException& ex)
        {
            logError(_communicator, ex);
            throw;
        }

        if(_traceLevels->adapter > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
            out << "synchronized adapters (serial = `" << dbSerial << "')";
        }

        serial = _adapterObserverTopic->adapterInit(dbSerial, adapters);
    }
    _adapterObserverTopic->waitForSyncedSubscribers(serial);
}

void
Database::syncObjects(const ObjectInfoSeq& objects, long long dbSerial)
{
    assert(dbSerial != 0);
    int serial = 0;
    {
        lock_guard lock(_mutex);
        try
        {
            IceDB::ReadWriteTxn txn(_env);

            _objects.clear(txn);
            _objectsByType.clear(txn);
            for(const auto& obj : objects)
            {
                addObject(txn, obj, false);
            }
            dbSerial = updateSerial(txn, objectsDbName, dbSerial);

            txn.commit();
        }
        catch(const IceDB::LMDBException& ex)
        {
            logError(_communicator, ex);
            throw;
        }

        if(_traceLevels->object > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
            out << "synchronized objects (serial = `" << dbSerial << "')";
        }

        serial = _objectObserverTopic->objectInit(dbSerial, objects);
    }
    _objectObserverTopic->waitForSyncedSubscribers(serial);
}

ApplicationInfoSeq
Database::getApplications(long long& serial)
{
    try
    {
        IceDB::ReadOnlyTxn txn(_env);

        serial = getSerial(txn, applicationsDbName);
        return toVector(txn, _applications);
    }
    catch(const IceDB::LMDBException& ex)
    {
        logError(_communicator, ex);
        throw;
    }
}

AdapterInfoSeq
Database::getAdapters(long long& serial)
{
    try
    {
        IceDB::ReadOnlyTxn txn(_env);

        serial = getSerial(txn, adaptersDbName);
        return toVector(txn, _adapters);
    }
    catch(const IceDB::LMDBException& ex)
    {
        logError(_communicator, ex);
        throw;
    }
}

ObjectInfoSeq
Database::getObjects(long long& serial)
{
    try
    {
        IceDB::ReadOnlyTxn txn(_env);

        serial = getSerial(txn, objectsDbName);
        return toVector(txn, _objects);
    }
    catch(const IceDB::LMDBException& ex)
    {
        logError(_communicator, ex);
        throw;
    }
}

StringLongDict
Database::getSerials() const
{
    IceDB::ReadOnlyTxn txn(_env);
    return toMap(txn, _serials);
}

void
Database::addApplication(const ApplicationInfo& info, AdminSessionI* session, long long dbSerial)
{
    assert(dbSerial != 0 || _master);

    int serial = 0; // Initialize to prevent warning.
    ServerEntrySeq entries;
    try
    {
        unique_lock lock(_mutex);
        checkSessionLock(session);

        waitForUpdate(lock, info.descriptor.name);

        IceDB::ReadWriteTxn txn(_env);

        if(_applications.find(txn, info.descriptor.name))
        {
            throw DeploymentException("application `" + info.descriptor.name + "' already exists");
        }

        ApplicationHelper helper(_communicator, info.descriptor, true);
        checkForAddition(helper, txn);
        dbSerial = saveApplication(info, txn, dbSerial);

        txn.commit();

        load(helper, entries, info.uuid, info.revision);
        startUpdating(info.descriptor.name, info.uuid, info.revision);

        for(const auto& entry : entries)
        {
            entry->sync();
        }

        serial = _applicationObserverTopic->applicationAdded(dbSerial, info);
    }
    catch(const IceDB::KeyTooLongException& ex)
    {
        throw DeploymentException("application name `" + info.descriptor.name + "' is too long: " + ex.what());
    }
    catch(const IceDB::LMDBException& ex)
    {
        logError(_communicator, ex);
        throw;
    }

    _applicationObserverTopic->waitForSyncedSubscribers(serial); // Wait for replicas to be updated.

    //
    // Mark the application as updated. All the replicas received the update so it's now safe
    // for the nodes to start the servers.
    //
    {
        lock_guard lock(_mutex);

        vector<UpdateInfo>::iterator p = find(_updating.begin(), _updating.end(), info.descriptor.name);
        assert(p != _updating.end());
        p->markUpdated();
    }

    if(_master)
    {
        try
        {
            for(ServerEntrySeq::const_iterator p = entries.begin(); p != entries.end(); ++p)
            {
                try
                {
                    (*p)->waitForSync();
                }
                catch(const NodeUnreachableException&)
                {
                    // Ignore.
                }
            }
        }
        catch(const DeploymentException&)
        {
            try
            {
                lock_guard lock(_mutex);
                entries.clear();
                unload(ApplicationHelper(_communicator, info.descriptor), entries);

                IceDB::ReadWriteTxn txn(_env);
                dbSerial = removeApplication(info.descriptor.name, txn);
                txn.commit();

                for(const auto& entry : entries)
                {
                    entry->sync();
                }

                serial = _applicationObserverTopic->applicationRemoved(dbSerial, info.descriptor.name);
            }
            catch(const DeploymentException& ex)
            {
                Ice::Error err(_traceLevels->logger);
                err << "failed to rollback previous application `" << info.descriptor.name << "':\n" << ex.reason;
            }
            catch(const IceDB::LMDBException& ex)
            {
                logError(_communicator, ex);
            }

            _applicationObserverTopic->waitForSyncedSubscribers(serial);
            for(const auto& entry : entries)
            {
                entry->waitForSyncNoThrow();
            }
            finishUpdating(info.descriptor.name);
            throw;
        }
    }

    if(_traceLevels->application > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->applicationCat);
        out << "added application `" << info.descriptor.name << "' (serial = `" << dbSerial << "')";
    }
    finishUpdating(info.descriptor.name);
}

void
Database::updateApplication(const ApplicationUpdateInfo& updt, bool noRestart, AdminSessionI* session,
                            Ice::Long dbSerial)
{
    assert(dbSerial != 0 || _master);

    ApplicationInfo oldApp;
    ApplicationUpdateInfo update = updt;
    unique_ptr<ApplicationHelper> previous;
    unique_ptr<ApplicationHelper> helper;
    try
    {
        unique_lock lock(_mutex);
        checkSessionLock(session);

        waitForUpdate(lock, update.descriptor.name);

        IceDB::ReadOnlyTxn txn(_env);

        if(!_applications.get(txn, update.descriptor.name, oldApp))
        {
            throw ApplicationNotExistException(update.descriptor.name);
        }

        if(update.revision < 0)
        {
            update.revision = oldApp.revision + 1;
        }

        previous = make_unique<ApplicationHelper>(_communicator, oldApp.descriptor);
        helper = make_unique<ApplicationHelper>(_communicator, previous->update(update.descriptor), true);

        startUpdating(update.descriptor.name, oldApp.uuid, oldApp.revision + 1);
    }
    catch(const IceDB::LMDBException& ex)
    {
        logError(_communicator, ex);
        throw;
    }

    finishApplicationUpdate(update, oldApp, *previous.get(), *helper.get(), session, noRestart, dbSerial);
}

void
Database::syncApplicationDescriptor(const ApplicationDescriptor& newDesc, bool noRestart, AdminSessionI* session)
{
    assert(_master);

    ApplicationUpdateInfo update;
    ApplicationInfo oldApp;
    unique_ptr<ApplicationHelper> previous;
    unique_ptr<ApplicationHelper> helper;
    try
    {
        unique_lock lock(_mutex);
        checkSessionLock(session);

        waitForUpdate(lock, newDesc.name);

        IceDB::ReadOnlyTxn txn(_env);

        if(!_applications.get(txn, newDesc.name, oldApp))
        {
            throw ApplicationNotExistException(newDesc.name);
        }

        previous = make_unique<ApplicationHelper>(_communicator, oldApp.descriptor);
        helper = make_unique<ApplicationHelper>(_communicator, newDesc, true);

        update.updateTime = IceUtil::Time::now().toMilliSeconds();
        update.updateUser = _lockUserId;
        update.revision = oldApp.revision + 1;
        update.descriptor = helper->diff(*previous);

        startUpdating(update.descriptor.name, oldApp.uuid, oldApp.revision + 1);
    }
    catch(const IceDB::LMDBException& ex)
    {
        logError(_communicator, ex);
        throw;
    }

    finishApplicationUpdate(update, oldApp, *previous.get(), *helper.get(), session, noRestart);
}

void
Database::instantiateServer(const string& application,
                            const string& node,
                            const ServerInstanceDescriptor& instance,
                            AdminSessionI* session)
{
    assert(_master);

    ApplicationUpdateInfo update;
    ApplicationInfo oldApp;
    unique_ptr<ApplicationHelper> previous;
    unique_ptr<ApplicationHelper> helper;

    try
    {
        unique_lock lock(_mutex);
        checkSessionLock(session);

        waitForUpdate(lock, application);

        IceDB::ReadOnlyTxn txn(_env);

        if(!_applications.get(txn, application, oldApp))
        {
            throw ApplicationNotExistException(application);
        }

        previous = make_unique<ApplicationHelper>(_communicator, oldApp.descriptor);
        helper = make_unique<ApplicationHelper>(_communicator, previous->instantiateServer(node, instance), true);

        update.updateTime = IceUtil::Time::now().toMilliSeconds();
        update.updateUser = _lockUserId;
        update.revision = oldApp.revision + 1;
        update.descriptor = helper->diff(*previous);

        startUpdating(update.descriptor.name, oldApp.uuid, oldApp.revision + 1);
    }
    catch(const IceDB::LMDBException& ex)
    {
        logError(_communicator, ex);
        throw;
    }

    finishApplicationUpdate(update, oldApp, *previous.get(), *helper.get(), session, true);
}

void
Database::removeApplication(const string& name, AdminSessionI* session, Ice::Long dbSerial)
{
    assert(dbSerial != 0 || _master);
    ServerEntrySeq entries;

    int serial = 0; // Initialize to prevent warning.
    try
    {
        unique_lock lock(_mutex);
        checkSessionLock(session);

        waitForUpdate(lock, name);

        ApplicationInfo appInfo;

        IceDB::ReadWriteTxn txn(_env);

        if(!_applications.get(txn, name, appInfo))
        {
            throw ApplicationNotExistException(name);
        }

        bool init = false;
        try
        {
            ApplicationHelper helper(_communicator, appInfo.descriptor);
            init = true;
            checkForRemove(helper);
            unload(helper, entries);
        }
        catch(const DeploymentException&)
        {
            if(init)
            {
                throw;
            }
        }
        dbSerial = removeApplication(name, txn, dbSerial);

        txn.commit();

        startUpdating(name, appInfo.uuid, appInfo.revision);

        for(const auto& entry : entries)
        {
            entry->sync();
        }

        serial = _applicationObserverTopic->applicationRemoved(dbSerial, name);
    }
    catch(const IceDB::LMDBException& ex)
    {
        logError(_communicator, ex);
        throw;
    }

    _applicationObserverTopic->waitForSyncedSubscribers(serial);

    if(_master)
    {
        for(const auto& entry : entries)
        {
            entry->waitForSyncNoThrow();
        }
    }

    if(_traceLevels->application > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->applicationCat);
        out << "removed application `" << name << "' (serial = `" << dbSerial << "')";
    }

    finishUpdating(name);
}

ApplicationInfo
Database::getApplicationInfo(const std::string& name)
{
    IceDB::ReadOnlyTxn txn(_env);

    ApplicationInfo info;
    if(!_applications.get(txn, name, info))
    {
        throw ApplicationNotExistException(name);
    }
    return info;
}

Ice::StringSeq
Database::getAllApplications(const string& expression)
{
    IceDB::ReadOnlyTxn txn(_env);
    return getMatchingKeys<map<string, ApplicationInfo> >(toMap(txn, _applications), expression);
}

void
Database::waitForApplicationUpdate(const string& uuid,
                                   int revision,
                                   function<void()> response, function<void(exception_ptr)> exception)
{
    lock_guard lock(_mutex);

    vector<UpdateInfo>::iterator p = find(_updating.begin(), _updating.end(), make_pair(uuid, revision));
    if(p != _updating.end() && !p->updated)
    {
        p->cbs.push_back({ response, exception });
    }
    else
    {
        response();
    }
}

NodeCache&
Database::getNodeCache()
{
    return _nodeCache;
}

shared_ptr<NodeEntry>
Database::getNode(const string& name, bool create) const
{
    return _nodeCache.get(name, create);
}

ReplicaCache&
Database::getReplicaCache()
{
    return _replicaCache;
}

shared_ptr<ReplicaEntry>
Database::getReplica(const string& name) const
{
    return _replicaCache.get(name);
}

ServerCache&
Database::getServerCache()
{
    return _serverCache;
}

shared_ptr<ServerEntry>
Database::getServer(const string& id) const
{
    return _serverCache.get(id);
}

AllocatableObjectCache&
Database::getAllocatableObjectCache()
{
    return _allocatableObjectCache;
}

shared_ptr<AllocatableObjectEntry>
Database::getAllocatableObject(const Ice::Identity& id) const
{
    return _allocatableObjectCache.get(id);
}

void
Database::setAdapterDirectProxy(const string& adapterId, const string& replicaGroupId,
                                const shared_ptr<Ice::ObjectPrx>& proxy, long long dbSerial)
{
    assert(dbSerial != 0 || _master);

    int serial = 0; // Initialize to prevent warning.
    {
        lock_guard lock(_mutex);
        if(_adapterCache.has(adapterId))
        {
            throw AdapterExistsException(adapterId);
        }
        if(_adapterCache.has(replicaGroupId))
        {
            throw DeploymentException("registering adapter `" + adapterId + "' with the replica group `" +
                                      replicaGroupId + "' is not allowed:\nthe replica group was added with an "
                                      "application descriptor and only adapters specified in an application descriptor "
                                      "can be member of this replica group");
        }

        AdapterInfo info = { adapterId, proxy, replicaGroupId };

        bool updated = false;
        try
        {
            IceDB::ReadWriteTxn txn(_env);

            AdapterInfo oldInfo;
            bool found = _adapters.get(txn, adapterId, oldInfo);
            if(proxy)
            {
                updated = found;

                if(replicaGroupId != oldInfo.replicaGroupId)
                {
                    _adaptersByGroupId.del(txn, oldInfo.replicaGroupId, adapterId);
                }
                addAdapter(txn, info);
            }
            else
            {
                if(!found)
                {
                    return;
                }
                deleteAdapter(txn, oldInfo);
            }
            dbSerial = updateSerial(txn, adaptersDbName, dbSerial);

            txn.commit();
        }
        catch(const IceDB::KeyTooLongException&)
        {
            throw;
        }
        catch(const IceDB::LMDBException& ex)
        {
            logError(_communicator, ex);
            throw;
        }

        if(_traceLevels->adapter > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
            out << (proxy ? (updated ? "updated" : "added") : "removed") << " adapter `" << adapterId << "'";
            if(!replicaGroupId.empty())
            {
                out << " with replica group `" << replicaGroupId << "'";
            }
            out << " (serial = `" << dbSerial << "')";
        }

        if(proxy)
        {
            if(updated)
            {
                serial = _adapterObserverTopic->adapterUpdated(dbSerial, info);
            }
            else
            {
                serial = _adapterObserverTopic->adapterAdded(dbSerial, info);
            }
        }
        else
        {
            serial = _adapterObserverTopic->adapterRemoved(dbSerial, adapterId);
        }
    }
    _adapterObserverTopic->waitForSyncedSubscribers(serial);
}

shared_ptr<Ice::ObjectPrx>
Database::getAdapterDirectProxy(const string& id, const Ice::EncodingVersion& encoding,
                                const shared_ptr<Ice::Connection>& con,
                                const Ice::Context& ctx)
{
    IceDB::ReadOnlyTxn txn(_env);

    AdapterInfo info;
    if(_adapters.get(txn, id, info))
    {
        return info.proxy;
    }

    Ice::EndpointSeq endpoints;
    vector<AdapterInfo> infos = findByReplicaGroupId(txn, _adapters, _adaptersByGroupId, id);
    if(infos.empty())
    {
        throw AdapterNotExistException(id);
    }

    filterAdapterInfos("", id, _pluginFacade, con, ctx, infos);
    for(unsigned int i = 0; i < infos.size(); ++i)
    {
        if(IceInternal::isSupported(encoding, infos[i].proxy->ice_getEncodingVersion()))
        {
            Ice::EndpointSeq edpts = infos[i].proxy->ice_getEndpoints();
            endpoints.insert(endpoints.end(), edpts.begin(), edpts.end());
        }
    }
    if(!endpoints.empty())
    {
        return _communicator->stringToProxy("dummy:default")->ice_endpoints(endpoints);
    }
    return 0;
}

void
Database::removeAdapter(const string& adapterId)
{
    assert(_master);

    int serial = 0; // Initialize to prevent warning.
    {
        lock_guard lock(_mutex);
        if(_adapterCache.has(adapterId))
        {
            auto adpt = _adapterCache.get(adapterId);
            throw DeploymentException("removing adapter `" + adapterId + "' is not allowed:\n" +
                                      "the adapter was added with the application descriptor `" +
                                      adpt->getApplication() + "'");
        }

        AdapterInfoSeq infos;
        Ice::Long dbSerial = 0;
        try
        {
            IceDB::ReadWriteTxn txn(_env);

            AdapterInfo info;
            if(_adapters.get(txn, adapterId, info))
            {
                deleteAdapter(txn, info);
            }
            else
            {
                infos = findByReplicaGroupId(txn, _adapters, _adaptersByGroupId, adapterId);
                if(infos.empty())
                {
                    throw AdapterNotExistException(adapterId);
                }
                for(AdapterInfoSeq::iterator p = infos.begin(); p != infos.end(); ++p)
                {
                    _adaptersByGroupId.del(txn, p->replicaGroupId, p->id);
                    p->replicaGroupId.clear();
                    addAdapter(txn, *p);
                }
            }
            dbSerial = updateSerial(txn, adaptersDbName);

            txn.commit();
        }
        catch(const IceDB::KeyTooLongException&)
        {
            throw;
        }
        catch(const IceDB::LMDBException& ex)
        {
            logError(_communicator, ex);
            throw;
        }

        if(_traceLevels->adapter > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
            out << "removed " << (infos.empty() ? "adapter" : "replica group") << " `" << adapterId << "' (serial = `" << dbSerial << "')";
        }

        if(infos.empty())
        {
            serial = _adapterObserverTopic->adapterRemoved(dbSerial, adapterId);
        }
        else
        {
            for(AdapterInfoSeq::const_iterator p = infos.begin(); p != infos.end(); ++p)
            {
                serial = _adapterObserverTopic->adapterUpdated(dbSerial, *p);
            }
        }
    }
    _adapterObserverTopic->waitForSyncedSubscribers(serial);
}

shared_ptr<AdapterPrx>
Database::getAdapterProxy(const string& adapterId, const string& replicaGroupId, bool upToDate)
{
    lock_guard lock(_mutex); // Make sure this isn't call during an update.
    return _adapterCache.get(adapterId)->getProxy(replicaGroupId, upToDate);
}

void
Database::getLocatorAdapterInfo(const string& id,
                                const shared_ptr<Ice::Connection>& connection,
                                const Ice::Context& context,
                                LocatorAdapterInfoSeq& adpts,
                                int& count,
                                bool& replicaGroup,
                                bool& roundRobin,
                                const set<string>& excludes)
{
    string filter;
    {
        lock_guard lock(_mutex); // Make sure this isn't called during an update.
        _adapterCache.get(id)->getLocatorAdapterInfo(adpts, count, replicaGroup, roundRobin, filter, excludes);
    }

    if(_pluginFacade->hasReplicaGroupFilters() && !adpts.empty())
    {
        auto filters = _pluginFacade->getReplicaGroupFilters(filter);
        if(!filters.empty())
        {
            Ice::StringSeq adapterIds;
            for(LocatorAdapterInfoSeq::const_iterator q = adpts.begin(); q != adpts.end(); ++q)
            {
                adapterIds.push_back(q->id);
            }

            for(const auto& f : filters)
            {
                adapterIds = f->filter(id, adapterIds, connection, context);
            }

            LocatorAdapterInfoSeq filteredAdpts;
            filteredAdpts.reserve(adpts.size());
            for(Ice::StringSeq::const_iterator q = adapterIds.begin(); q != adapterIds.end(); ++q)
            {
                for(LocatorAdapterInfoSeq::const_iterator r = adpts.begin(); r != adpts.end(); ++r)
                {
                    if(*q == r->id)
                    {
                        filteredAdpts.push_back(*r);
                        break;
                    }
                }
            }
            adpts.swap(filteredAdpts);
        }
    }
}

bool
Database::addAdapterSyncCallback(const string& id,
                                 const shared_ptr<SynchronizationCallback>& callback,
                                 const std::set<std::string>& excludes)
{
    lock_guard lock(_mutex); // Make sure this isn't call during an update.
    return _adapterCache.get(id)->addSyncCallback(callback, excludes);
}

AdapterInfoSeq
Database::getAdapterInfo(const string& id)
{
    //
    // First we check if the given adapter id is associated to a
    // server, if that's the case we get the adapter proxy from the
    // server.
    //
    shared_ptr<GetAdapterInfoResult> result;
    try
    {
        lock_guard lock(_mutex); // Make sure this isn't call during an update.
        result = _adapterCache.get(id)->getAdapterInfoAsync();
    }
    catch(const AdapterNotExistException&)
    {
    }
    if(result)
    {
        return result->get(); // Don't hold the database lock while waiting for the endpoints
    }

    //
    // Otherwise, we check the adapter endpoint table -- if there's an
    // entry the adapter is managed by the registry itself.
    //
    IceDB::ReadOnlyTxn txn(_env);

    AdapterInfo info;
    AdapterInfoSeq infos;
    if(_adapters.get(txn, id, info))
    {
        infos.push_back(info);
    }
    else
    {
        //
        // If it's not a regular object adapter, perhaps it's a replica
        // group...
        //
        infos = findByReplicaGroupId(txn, _adapters, _adaptersByGroupId, id);
        if(infos.empty())
        {
            throw AdapterNotExistException(id);
        }
    }
    return infos;
}

AdapterInfoSeq
Database::getFilteredAdapterInfo(const string& id, const shared_ptr<Ice::Connection>& con, const Ice::Context& ctx)
{
    //
    // First we check if the given adapter id is associated to a
    // server, if that's the case we get the adapter proxy from the
    // server.
    //
    try
    {
        AdapterInfoSeq infos;
        shared_ptr<ReplicaGroupEntry> replicaGroup;
        {
            lock_guard lock(_mutex); // Make sure this isn't call during an update.

            auto entry = _adapterCache.get(id);
            infos = entry->getAdapterInfoNoEndpoints();
            replicaGroup = dynamic_pointer_cast<ReplicaGroupEntry>(entry);
        }
        if(replicaGroup)
        {
            filterAdapterInfos(replicaGroup->getFilter(), id, _pluginFacade, con, ctx, infos);
        }
        return infos;
    }
    catch(const AdapterNotExistException&)
    {
    }

    //
    // Otherwise, we check the adapter endpoint table -- if there's an
    // entry the adapter is managed by the registry itself.
    //
    IceDB::ReadOnlyTxn txn(_env);

    AdapterInfo info;
    AdapterInfoSeq infos;
    if(_adapters.get(txn, id, info))
    {
        infos.push_back(info);
    }
    else
    {
        //
        // If it's not a regular object adapter, perhaps it's a replica
        // group...
        //
        infos = findByReplicaGroupId(txn, _adapters, _adaptersByGroupId, id);
        if(infos.empty())
        {
            throw AdapterNotExistException(id);
        }
        filterAdapterInfos("", id, _pluginFacade, con, ctx, infos);
    }
    return infos;
}

string
Database::getAdapterServer(const string& id) const
{
    try
    {
        lock_guard lock(_mutex); // Make sure this isn't call during an update.
        auto adapter = dynamic_pointer_cast<ServerAdapterEntry>(_adapterCache.get(id));
        if(adapter)
        {
            return adapter->getServerId();
        }
    }
    catch(const AdapterNotExistException&)
    {
    }
    return "";
}

string
Database::getAdapterApplication(const string& id) const
{
    try
    {
        lock_guard lock(_mutex); // Make sure this isn't call during an update.
        return _adapterCache.get(id)->getApplication();
    }
    catch(const AdapterNotExistException&)
    {
    }
    return "";
}

string
Database::getAdapterNode(const string& id) const
{
    try
    {
        lock_guard lock(_mutex); // Make sure this isn't call during an update.
        auto adapter = dynamic_pointer_cast<ServerAdapterEntry>(_adapterCache.get(id));
        if(adapter)
        {
            return adapter->getNodeName();
        }
    }
    catch(const AdapterNotExistException&)
    {
    }
    return "";
}

Ice::StringSeq
Database::getAllAdapters(const string& expression)
{
    lock_guard lock(_mutex);
    vector<string> result;
    vector<string> ids = _adapterCache.getAll(expression);
    result.swap(ids);
    set<string> groups;

    IceDB::ReadOnlyTxn txn(_env);

    string name;
    AdapterInfo info;
    AdapterMapROCursor cursor(_adapters, txn);
    while(cursor.get(name, info, MDB_NEXT))
    {
        if(expression.empty() || IceUtilInternal::match(name, expression, true))
        {
            result.push_back(name);
        }
        string replicaGroupId = info.replicaGroupId;
        if(!replicaGroupId.empty() && (expression.empty() || IceUtilInternal::match(replicaGroupId, expression, true)))
        {
            groups.insert(replicaGroupId);
        }
    }
    cursor.close();

    //
    // COMPILERFIX: We're not using result.insert() here, this doesn't compile on Sun.
    //
    //result.insert(result.end(), groups.begin(), groups.end())
    for(set<string>::const_iterator q = groups.begin(); q != groups.end(); ++q)
    {
        result.push_back(*q);
    }
    return result;
}

void
Database::addObject(const ObjectInfo& info)
{
    assert(_master);

    int serial = 0;
    {
        lock_guard lock(_mutex);
        const Ice::Identity id = info.proxy->ice_getIdentity();

        if(_objectCache.has(id))
        {
            throw ObjectExistsException(id);
        }

        long long dbSerial = 0;
        try
        {
            IceDB::ReadWriteTxn txn(_env);

            if(_objects.find(txn, id))
            {
                throw ObjectExistsException(id);
            }
            addObject(txn, info, false);
            dbSerial = updateSerial(txn, objectsDbName);

            txn.commit();
        }
        catch(const IceDB::LMDBException& ex)
        {
            logError(_communicator, ex);
            throw;
        }

        serial = _objectObserverTopic->objectAdded(dbSerial, info);

        if(_traceLevels->object > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
            out << "added object `" << _communicator->identityToString(id) << "' (serial = `" << dbSerial << "')";
        }
    }
    _objectObserverTopic->waitForSyncedSubscribers(serial);
}

void
Database::addOrUpdateObject(const ObjectInfo& info, long long dbSerial)
{
    assert(dbSerial != 0 || _master);

    int serial = 0; // Initialize to prevent warning.
    {
        lock_guard lock(_mutex);
        const Ice::Identity id = info.proxy->ice_getIdentity();

        if(_objectCache.has(id))
        {
            throw ObjectExistsException(id);
        }

        bool update = false;
        try
        {
            IceDB::ReadWriteTxn txn(_env);

            Ice::Identity k;
            ObjectInfo v;
            update = _objects.get(txn, k, v);
            if(update)
            {
                _objectsByType.del(txn, v.type, v.proxy->ice_getIdentity());
            }
            addObject(txn, info, false);
            dbSerial = updateSerial(txn, objectsDbName, dbSerial);

            txn.commit();
        }
        catch(const IceDB::LMDBException& ex)
        {
            logError(_communicator, ex);
            throw;
        }

        if(update)
        {
            serial = _objectObserverTopic->objectUpdated(dbSerial, info);
        }
        else
        {
            serial = _objectObserverTopic->objectAdded(dbSerial, info);
        }

        if(_traceLevels->object > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
            out << (!update ? "added" : "updated") << " object `" << _communicator->identityToString(id) << "' (serial = `" << dbSerial << "')";
        }
    }
    _objectObserverTopic->waitForSyncedSubscribers(serial);
}

void
Database::removeObject(const Ice::Identity& id, long long dbSerial)
{
    assert(dbSerial != 0 || _master);

    int serial = 0; // Initialize to prevent warning.
    {
        lock_guard lock(_mutex);
        if(_objectCache.has(id))
        {
            throw DeploymentException("removing object `" + _communicator->identityToString(id) + "' is not allowed:\n"
                                      + "the object was added with the application descriptor `" +
                                      _objectCache.get(id)->getApplication());
        }

        try
        {
            IceDB::ReadWriteTxn txn(_env);

            ObjectInfo info;
            if(!_objects.get(txn, id, info))
            {
                throw ObjectNotRegisteredException(id);
            }
            deleteObject(txn, info, false);
            dbSerial = updateSerial(txn, objectsDbName, dbSerial);

            txn.commit();
        }
        catch(const IceDB::LMDBException& ex)
        {
            logError(_communicator, ex);
            throw;
        }

        serial = _objectObserverTopic->objectRemoved(dbSerial, id);

        if(_traceLevels->object > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
            out << "removed object `" << _communicator->identityToString(id) << "' (serial = `" << dbSerial << "')";
        }
    }
    _objectObserverTopic->waitForSyncedSubscribers(serial);
}

void
Database::updateObject(const shared_ptr<Ice::ObjectPrx>& proxy)
{
    assert(_master);

    int serial = 0;
    {
        lock_guard lock(_mutex);

        const Ice::Identity id = proxy->ice_getIdentity();
        if(_objectCache.has(id))
        {
            throw DeploymentException("updating object `" + _communicator->identityToString(id) + "' is not allowed:\n"
                                      + "the object was added with the application descriptor `" +
                                      _objectCache.get(id)->getApplication() + "'");
        }

        ObjectInfo info;
        Ice::Long dbSerial = 0;
        try
        {
            IceDB::ReadWriteTxn txn(_env);

            if(!_objects.get(txn, id, info))
            {
                throw ObjectNotRegisteredException(id);
            }
            info.proxy = proxy;
            addObject(txn, info, false);
            dbSerial = updateSerial(txn, objectsDbName);

            txn.commit();
        }
        catch(const IceDB::LMDBException& ex)
        {
            logError(_communicator, ex);
            throw;
        }

        serial = _objectObserverTopic->objectUpdated(dbSerial, info);
        if(_traceLevels->object > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
            out << "updated object `" << _communicator->identityToString(id) << "' (serial = `" << dbSerial << "')";
        }
    }
    _objectObserverTopic->waitForSyncedSubscribers(serial);
}

int
Database::addOrUpdateRegistryWellKnownObjects(const ObjectInfoSeq& objects)
{
    lock_guard lock(_mutex);
    try
    {
        IceDB::ReadWriteTxn txn(_env);
        for(const auto& obj : objects)
        {
            Ice::Identity id = obj.proxy->ice_getIdentity();
            ObjectInfo info;
            if(_objects.get(txn, id, info))
            {
                _objectsByType.del(txn, info.type, id);
            }
            addObject(txn, obj, false);
        }
        txn.commit();
    }
    catch(const IceDB::LMDBException& ex)
    {
        logError(_communicator, ex);
        throw;
    }

    return _objectObserverTopic->wellKnownObjectsAddedOrUpdated(objects);
}

int
Database::removeRegistryWellKnownObjects(const ObjectInfoSeq& objects)
{
    lock_guard lock(_mutex);
    try
    {
        IceDB::ReadWriteTxn txn(_env);
        for(const auto& obj : objects)
        {
            Ice::Identity id = obj.proxy->ice_getIdentity();
            ObjectInfo info;
            if(_objects.get(txn, id, info))
            {
                deleteObject(txn, info, false);
            }
        }
        txn.commit();
    }
    catch(const IceDB::LMDBException& ex)
    {
        logError(_communicator, ex);
        throw;
    }

    return _objectObserverTopic->wellKnownObjectsRemoved(objects);
}

shared_ptr<Ice::ObjectPrx>
Database::getObjectProxy(const Ice::Identity& id)
{
    try
    {
        //
        // Only return proxies for non allocatable objects.
        //
        return _objectCache.get(id)->getProxy();
    }
    catch(const ObjectNotRegisteredException&)
    {
    }

    IceDB::ReadOnlyTxn txn(_env);
    ObjectInfo info;
    if(!_objects.get(txn, id, info))
    {
        throw ObjectNotRegisteredException(id);
    }
    return info.proxy;
}

shared_ptr<Ice::ObjectPrx>
Database::getObjectByType(const string& type, const shared_ptr<Ice::Connection>& con, const Ice::Context& ctx)
{
    Ice::ObjectProxySeq objs = getObjectsByType(type, con, ctx);
    if(objs.empty())
    {
        return 0;
    }
    return objs[IceUtilInternal::random(static_cast<int>(objs.size()))];
}

shared_ptr<Ice::ObjectPrx>
Database::getObjectByTypeOnLeastLoadedNode(const string& type, LoadSample sample,
                                           const shared_ptr<Ice::Connection>& con, const Ice::Context& ctx)
{
    Ice::ObjectProxySeq objs = getObjectsByType(type, con, ctx);
    if(objs.empty())
    {
        return 0;
    }

    IceUtilInternal::shuffle(objs.begin(), objs.end());
    vector<pair<shared_ptr<Ice::ObjectPrx>, float>> objectsWithLoad;
    objectsWithLoad.reserve(objs.size());
    for(const auto& obj : objs)
    {
        float load = 1.0f;
        if(!obj->ice_getAdapterId().empty())
        {
            try
            {
                load = _adapterCache.get(obj->ice_getAdapterId())->getLeastLoadedNodeLoad(sample);
            }
            catch(const AdapterNotExistException&)
            {
            }
        }
        objectsWithLoad.push_back(make_pair(obj, load));
    }
    return min_element(objectsWithLoad.begin(), objectsWithLoad.end(), [](const auto& lhs, const auto& rhs)
           {
               return lhs.second < rhs.second;
           })->first;
}

Ice::ObjectProxySeq
Database::getObjectsByType(const string& type, const shared_ptr<Ice::Connection>& con, const Ice::Context& ctx)
{
    Ice::ObjectProxySeq proxies;

    auto objects = _objectCache.getObjectsByType(type);

    for(const auto& obj : objects)
    {
        if(_nodeObserverTopic->isServerEnabled(obj->getServer())) // Only return proxies from enabled servers.
        {
            proxies.push_back(obj->getProxy());
        }
    }

    IceDB::ReadOnlyTxn txn(_env);
    vector<ObjectInfo> infos = findByType(txn, _objects, _objectsByType, type);
    for(const auto& info : infos)
    {
        proxies.push_back(info.proxy);
    }

    if(con && !proxies.empty() && _pluginFacade->hasTypeFilters())
    {
        auto filters = _pluginFacade->getTypeFilters(type);
        if(!filters.empty())
        {
            for(const auto& filter: filters)
            {
                proxies = filter->filter(type, proxies, con, ctx);
            }
        }
    }
    return proxies;
}

ObjectInfo
Database::getObjectInfo(const Ice::Identity& id)
{
    try
    {
        return _objectCache.get(id)->getObjectInfo();
    }
    catch(const ObjectNotRegisteredException&)
    {
    }

    IceDB::ReadOnlyTxn txn(_env);
    ObjectInfo info;
    if(!_objects.get(txn, id, info))
    {
        throw ObjectNotRegisteredException(id);
    }
    return info;
}

ObjectInfoSeq
Database::getAllObjectInfos(const string& expression)
{
    ObjectInfoSeq infos = _objectCache.getAll(expression);

    IceDB::ReadOnlyTxn txn(_env);

    Ice::Identity id;
    ObjectInfo info;
    ObjectsMapROCursor cursor(_objects, txn);
    while(cursor.get(id, info, MDB_NEXT))
    {
        if(expression.empty() || IceUtilInternal::match(_communicator->identityToString(id), expression, true))
        {
            infos.push_back(info);
        }
    }
    return infos;
}

ObjectInfoSeq
Database::getObjectInfosByType(const string& type)
{
    ObjectInfoSeq infos = _objectCache.getAllByType(type);

    IceDB::ReadOnlyTxn txn(_env);
    ObjectInfoSeq dbInfos = findByType(txn, _objects, _objectsByType, type);
    for(unsigned int i = 0; i < dbInfos.size(); ++i)
    {
        infos.push_back(dbInfos[i]);
    }
    return infos;
}

void
Database::addInternalObject(const ObjectInfo& info, bool replace)
{
    lock_guard lock(_mutex);
    const Ice::Identity id = info.proxy->ice_getIdentity();

    try
    {
        IceDB::ReadWriteTxn txn(_env);

        ObjectInfo oldInfo;
        if(_internalObjects.get(txn, id, oldInfo))
        {
            if(!replace)
            {
                throw ObjectExistsException(id);
            }
            _internalObjectsByType.del(txn, oldInfo.type, id);
        }
        addObject(txn, info, true);

        txn.commit();
    }
    catch(const IceDB::LMDBException& ex)
    {
        logError(_communicator, ex);
        throw;
    }
}

void
Database::removeInternalObject(const Ice::Identity& id)
{
    lock_guard lock(_mutex);

    try
    {
        IceDB::ReadWriteTxn txn(_env);

        ObjectInfo info;
        if(!_internalObjects.get(txn, id, info))
        {
            throw ObjectNotRegisteredException(id);
        }
        deleteObject(txn, info, true);

        txn.commit();
    }
    catch(const IceDB::LMDBException& ex)
    {
        logError(_communicator, ex);
        throw;
    }
}

Ice::ObjectProxySeq
Database::getInternalObjectsByType(const string& type)
{
    Ice::ObjectProxySeq proxies;

    IceDB::ReadOnlyTxn txn(_env);
    vector<ObjectInfo> infos = findByType(txn, _internalObjects, _internalObjectsByType, type);
    for(unsigned int i = 0; i < infos.size(); ++i)
    {
        proxies.push_back(infos[i].proxy);
    }
    return proxies;
}

void
Database::checkForAddition(const ApplicationHelper& app, const IceDB::ReadWriteTxn& txn)
{
    set<string> serverIds;
    set<string> adapterIds;
    set<Ice::Identity> objectIds;

    app.getIds(serverIds, adapterIds, objectIds);

    for(const auto& serverId : serverIds)
    {
        checkServerForAddition(serverId);
    }

    if(!adapterIds.empty())
    {
        for(set<string>::const_iterator p = adapterIds.begin(); p != adapterIds.end(); ++p)
        {
            checkAdapterForAddition(*p, txn);
        }
    }
    if(!objectIds.empty())
    {
        for(set<Ice::Identity>::const_iterator p = objectIds.begin(); p != objectIds.end(); ++p)
        {
            checkObjectForAddition(*p, txn);
        }
    }

    set<string> repGrps;
    set<string> adptRepGrps;
    app.getReplicaGroups(repGrps, adptRepGrps);
    for(const auto& repGrp : adptRepGrps)
    {
        checkReplicaGroupExists(repGrp);
    }
}

void
Database::checkForUpdate(const ApplicationHelper& origApp,
                         const ApplicationHelper& newApp,
                         const IceDB::ReadWriteTxn& txn)
{
    set<string> oldSvrs, newSvrs;
    set<string> oldAdpts, newAdpts;
    set<Ice::Identity> oldObjs, newObjs;

    origApp.getIds(oldSvrs, oldAdpts, oldObjs);
    newApp.getIds(newSvrs, newAdpts, newObjs);

    Ice::StringSeq addedSvrs;
    set_difference(newSvrs.begin(), newSvrs.end(), oldSvrs.begin(), oldSvrs.end(), back_inserter(addedSvrs));
    for(const auto& svr : addedSvrs)
    {
        checkServerForAddition(svr);
    }

    Ice::StringSeq addedAdpts;
    set_difference(newAdpts.begin(), newAdpts.end(), oldAdpts.begin(), oldAdpts.end(), back_inserter(addedAdpts));
    if(!addedAdpts.empty())
    {
        for(const auto& adpt : addedAdpts)
        {
            checkAdapterForAddition(adpt, txn);
        }
    }

    vector<Ice::Identity> addedObjs;
    set_difference(newObjs.begin(), newObjs.end(), oldObjs.begin(), oldObjs.end(), back_inserter(addedObjs));
    if(!addedObjs.empty())
    {
        for(const auto& obj : addedObjs)
        {
            checkObjectForAddition(obj, txn);
        }
    }

    set<string> oldRepGrps, newRepGrps;
    set<string> oldAdptRepGrps, newAdptRepGrps;
    origApp.getReplicaGroups(oldRepGrps, oldAdptRepGrps);
    newApp.getReplicaGroups(newRepGrps, newAdptRepGrps);

    set<string> rmRepGrps;
    set_difference(oldRepGrps.begin(), oldRepGrps.end(), newRepGrps.begin(),newRepGrps.end(), set_inserter(rmRepGrps));
    for(const auto& repGrp : rmRepGrps)
    {
        checkReplicaGroupForRemove(repGrp);
    }

    set<string> addedAdptRepGrps;
    set_difference(newAdptRepGrps.begin(),newAdptRepGrps.end(), oldAdptRepGrps.begin(), oldAdptRepGrps.end(),
                   set_inserter(addedAdptRepGrps));
    for(const auto& repGrp : addedAdptRepGrps)
    {
        checkReplicaGroupExists(repGrp);
    }

    vector<string> invalidAdptRepGrps;
    set_intersection(rmRepGrps.begin(), rmRepGrps.end(), newAdptRepGrps.begin(), newAdptRepGrps.end(),
                     back_inserter(invalidAdptRepGrps));
    if(!invalidAdptRepGrps.empty())
    {
        throw DeploymentException("couldn't find replica group `" + invalidAdptRepGrps.front() + "'");
    }
}

void
Database::checkForRemove(const ApplicationHelper& app)
{
    set<string> replicaGroups;
    set<string> adapterReplicaGroups;
    app.getReplicaGroups(replicaGroups, adapterReplicaGroups);

    for(const auto& replicaGroup : replicaGroups)
    {
        checkReplicaGroupForRemove(replicaGroup);
    }
}

void
Database::checkServerForAddition(const string& id)
{
    if(_serverCache.has(id))
    {
        throw DeploymentException("server `" + id + "' is already registered");
    }
}

void
Database::checkAdapterForAddition(const string& id, const IceDB::ReadWriteTxn& txn)
{
    bool found = false;
    if(_adapterCache.has(id))
    {
        found = true;
    }
    else
    {
        if(_adapters.find(txn, id))
        {
            found = true;
        }
        else
        {
            if(!findByReplicaGroupId(txn, _adapters, _adaptersByGroupId, id).empty())
            {
                found = true;
            }
        }
    }

    if(found)
    {
        throw DeploymentException("adapter `" + id + "' is already registered");
    }
}

void
Database::checkObjectForAddition(const Ice::Identity& objectId,
                                 const IceDB::ReadWriteTxn& txn)
{
    bool found = false;
    if(_objectCache.has(objectId) || _allocatableObjectCache.has(objectId))
    {
        found = true;
    }
    else
    {
        if(_objects.find(txn, objectId))
        {
            found = true;
        }
    }

    if(found)
    {
        throw DeploymentException("object `" + _communicator->identityToString(objectId) + "' is already registered");
    }
}

void
Database::checkReplicaGroupExists(const string& replicaGroup)
{
    shared_ptr<ReplicaGroupEntry> entry;
    try
    {
        entry = dynamic_pointer_cast<ReplicaGroupEntry>(_adapterCache.get(replicaGroup));
    }
    catch(const AdapterNotExistException&)
    {
    }

    if(!entry)
    {
        throw DeploymentException("couldn't find replica group `" + replicaGroup + "'");
    }
}

void
Database::checkReplicaGroupForRemove(const string& replicaGroup)
{
    shared_ptr<ReplicaGroupEntry> entry;
    try
    {
        entry = dynamic_pointer_cast<ReplicaGroupEntry>(_adapterCache.get(replicaGroup));
    }
    catch(const AdapterNotExistException&)
    {
    }

    if(!entry)
    {
        //
        // This would indicate an inconsistency with the cache and
        // database. We don't print an error, it will be printed
        // when the application is actually removed.
        //
        return;
    }

    if(entry->hasAdaptersFromOtherApplications())
    {
        throw DeploymentException("couldn't remove application because the replica group `" + replicaGroup +
                                  "' is used by object adapters from other applications.");
    }
}

void
Database::load(const ApplicationHelper& app, ServerEntrySeq& entries, const string& uuid, int revision)
{
    const NodeDescriptorDict& nodes = app.getInstance().nodes;
    const string application = app.getInstance().name;
    for(const auto& node : nodes)
    {
        _nodeCache.get(node.first, true)->addDescriptor(application, node.second);
    }

    const ReplicaGroupDescriptorSeq& adpts = app.getInstance().replicaGroups;
    for(const auto& adpt : adpts)
    {
        assert(!adpt.id.empty());
        _adapterCache.addReplicaGroup(adpt, application);
        for(const auto& obj : adpt.objects)
        {
            _objectCache.add(toObjectInfo(_communicator, obj, adpt.id), application, "");
        }
    }

    for(const auto& server : app.getServerInfos(uuid, revision))
    {
        entries.push_back(_serverCache.add(server.second));
    }
}

void
Database::unload(const ApplicationHelper& app, ServerEntrySeq& entries)
{
    for(const auto& server : app.getServerInfos("", 0))
    {
        entries.push_back(_serverCache.remove(server.first, false));
    }

    for(const auto& adpt : app.getInstance().replicaGroups)
    {
        for(ObjectDescriptorSeq::const_iterator o = adpt.objects.begin(); o != adpt.objects.end(); ++o)
        {
            _objectCache.remove(o->id);
        }
        _adapterCache.removeReplicaGroup(adpt.id);
    }

    const NodeDescriptorDict& nodes = app.getInstance().nodes;
    const string application = app.getInstance().name;
    for(const auto& node : nodes)
    {
        _nodeCache.get(node.first)->removeDescriptor(application);
    }
}

void
Database::reload(const ApplicationHelper& oldApp,
                 const ApplicationHelper& newApp,
                 ServerEntrySeq& entries,
                 const string& uuid,
                 int revision,
                 bool noRestart)
{
    const string application = oldApp.getInstance().name;

    //
    // Remove destroyed servers.
    //
    auto oldServers = oldApp.getServerInfos(uuid, revision);
    auto newServers = newApp.getServerInfos(uuid, revision);
    vector<pair<bool, ServerInfo> > load;
    for(map<string, ServerInfo>::const_iterator p = newServers.begin(); p != newServers.end(); ++p)
    {
        map<string, ServerInfo>::const_iterator q = oldServers.find(p->first);
        if(q == oldServers.end())
        {
            load.push_back(make_pair(false, p->second));
        }
        else if(isServerUpdated(p->second, q->second))
        {
            _serverCache.preUpdate(p->second, noRestart);
            load.push_back(make_pair(true, p->second));
        }
        else
        {
            auto server = _serverCache.get(p->first);
            server->update(q->second, noRestart); // Just update the server revision on the node.
            entries.push_back(server);
        }
    }
    for(map<string, ServerInfo>::const_iterator p = oldServers.begin(); p != oldServers.end(); ++p)
    {
        map<string, ServerInfo>::const_iterator q = newServers.find(p->first);
        if(q == newServers.end())
        {
            entries.push_back(_serverCache.remove(p->first, noRestart));
        }
    }

    //
    // Remove destroyed replica groups.
    //
    const ReplicaGroupDescriptorSeq& oldAdpts = oldApp.getInstance().replicaGroups;
    const ReplicaGroupDescriptorSeq& newAdpts = newApp.getInstance().replicaGroups;
    for(ReplicaGroupDescriptorSeq::const_iterator r = oldAdpts.begin(); r != oldAdpts.end(); ++r)
    {
        ReplicaGroupDescriptorSeq::const_iterator t;
        for(t = newAdpts.begin(); t != newAdpts.end(); ++t)
        {
            if(t->id == r->id)
            {
                break;
            }
        }
        for(ObjectDescriptorSeq::const_iterator o = r->objects.begin(); o != r->objects.end(); ++o)
        {
            _objectCache.remove(o->id);
        }
        if(t == newAdpts.end())
        {
            _adapterCache.removeReplicaGroup(r->id);
        }
    }

    //
    // Remove all the node descriptors.
    //
    const NodeDescriptorDict& oldNodes = oldApp.getInstance().nodes;
    for(NodeDescriptorDict::const_iterator n = oldNodes.begin(); n != oldNodes.end(); ++n)
    {
        _nodeCache.get(n->first)->removeDescriptor(application);
    }

    //
    // Add back node descriptors.
    //
    const NodeDescriptorDict& newNodes = newApp.getInstance().nodes;
    for(NodeDescriptorDict::const_iterator n = newNodes.begin(); n != newNodes.end(); ++n)
    {
        _nodeCache.get(n->first, true)->addDescriptor(application, n->second);
    }

    //
    // Add back replica groups.
    //
    for(ReplicaGroupDescriptorSeq::const_iterator r = newAdpts.begin(); r != newAdpts.end(); ++r)
    {
        try
        {
            auto entry = dynamic_pointer_cast<ReplicaGroupEntry>(_adapterCache.get(r->id));
            assert(entry);
            entry->update(application, r->loadBalancing, r->filter);
        }
        catch(const AdapterNotExistException&)
        {
            _adapterCache.addReplicaGroup(*r, application);
        }

        for(ObjectDescriptorSeq::const_iterator o = r->objects.begin(); o != r->objects.end(); ++o)
        {
            _objectCache.add(toObjectInfo(_communicator, *o, r->id), application, "");
        }
    }

    //
    // Add back servers.
    //
    for(vector<pair<bool, ServerInfo> >::const_iterator q = load.begin(); q != load.end(); ++q)
    {
        if(q->first) // Update
        {
            entries.push_back(_serverCache.postUpdate(q->second, noRestart));
        }
        else
        {
            entries.push_back(_serverCache.add(q->second));
        }
    }
}

long long
Database::saveApplication(const ApplicationInfo& info, const IceDB::ReadWriteTxn& txn, long long dbSerial)
{
    assert(dbSerial != 0 || _master);
    _applications.put(txn, info.descriptor.name, info);
    return updateSerial(txn, applicationsDbName, dbSerial);
}

long long
Database::removeApplication(const string& name, const IceDB::ReadWriteTxn& txn, long long dbSerial)
{
    assert(dbSerial != 0 || _master);
    _applications.del(txn, name);
    return updateSerial(txn, applicationsDbName, dbSerial);
}

void
Database::checkUpdate(const ApplicationHelper& oldApp,
                      const ApplicationHelper& newApp,
                      const string& uuid,
                      int revision,
                      bool noRestart)
{
    const string application = oldApp.getInstance().name;

    map<string, ServerInfo> oldServers = oldApp.getServerInfos(uuid, revision);
    map<string, ServerInfo> newServers = newApp.getServerInfos(uuid, revision + 1);

    map<string, ServerInfo>::const_iterator p;
    vector<string> servers;
    vector<string> reasons;
    vector<shared_ptr<CheckUpdateResult>> results;
    set<string> unreachableNodes;

    if(noRestart)
    {
        for(p = oldServers.begin(); p != oldServers.end(); ++p)
        {
            auto q = newServers.find(p->first);
            if(q == newServers.end())
            {
                try
                {
                    auto info = p->second;
                    info.descriptor = 0; // Clear the descriptor to indicate removal.
                    auto result = _serverCache.get(p->first)->checkUpdate(info, true);
                    if(result)
                    {
                        results.push_back(result);
                    }
                }
                catch(const NodeUnreachableException& ex)
                {
                    unreachableNodes.insert(ex.name);
                }
                catch(const DeploymentException& ex)
                {
                    servers.push_back(p->first);
                    reasons.push_back(ex.reason);
                }
            }
        }
    }

    for(p = newServers.begin(); p != newServers.end(); ++p)
    {
        auto q = oldServers.find(p->first);
        if(q != oldServers.end() && isServerUpdated(p->second, q->second))
        {
            if(noRestart &&
               p->second.node == q->second.node &&
               isServerUpdated(p->second, q->second, true)) // Ignore properties
            {
                //
                // The updates are not only property updates and noRestart is required, no
                // need to check the server update on the node, we know already it requires
                // a restart.
                //
                servers.push_back(p->first);
                reasons.push_back("update requires the server `" + p->first + "' to be stopped");
            }
            else
            {
                //
                // Ask the node to check the server update.
                //
                try
                {
                    auto result = _serverCache.get(p->first)->checkUpdate(p->second, noRestart);
                    if(result)
                    {
                        results.push_back(result);
                    }
                }
                catch(const NodeUnreachableException& ex)
                {
                    unreachableNodes.insert(ex.name);
                }
                catch(const DeploymentException& ex)
                {
                    servers.push_back(p->first);
                    reasons.push_back(ex.reason);
                }
            }
        }
    }

    for(const auto& result : results)
    {
        try
        {
            result->getResult();
        }
        catch(const NodeUnreachableException& ex)
        {
            unreachableNodes.insert(ex.name);
        }
        catch(const DeploymentException& ex)
        {
            servers.push_back(result->getServer());
            reasons.push_back(ex.reason);
        }
    }

    if(noRestart)
    {
        if(!servers.empty() || !unreachableNodes.empty())
        {
            if(_traceLevels->application > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->applicationCat);
                out << "check for application `" << application << "' update failed:";
                if(!unreachableNodes.empty())
                {

                    Ice::StringSeq nodes(unreachableNodes.begin(), unreachableNodes.end());

                    if(nodes.size() == 1)
                    {
                        out << "\nthe node `" << nodes[0] << "' is down";
                    }
                    else
                    {
                        out << "\nthe nodes `" << toString(nodes, ", ") << "' are down";
                    }
                }
                if(!reasons.empty())
                {
                    for(vector<string>::const_iterator r = reasons.begin(); r != reasons.end(); ++r)
                    {
                        out << "\n" << *r;
                    }
                }
            }

            ostringstream os;
            os << "check for application `" << application << "' update failed:";
            if(!servers.empty())
            {
                if(servers.size() == 1)
                {
                    os << "\nthe server `" << servers[0] << "' would need to be stopped";
                }
                else
                {
                    os << "\nthe servers `" << toString(servers, ", ") << "' would need to be stopped";
                }
            }
            if(!unreachableNodes.empty())
            {
                Ice::StringSeq nodes(unreachableNodes.begin(), unreachableNodes.end());
                if(nodes.size() == 1)
                {
                    os << "\nthe node `" << nodes[0] << "' is down";
                }
                else
                {
                    os << "\nthe nodes `" << toString(nodes, ", ") << "' are down";
                }
            }
            throw DeploymentException(os.str());
        }
    }
    else if(!reasons.empty())
    {
        ostringstream os;
        os << "check for application `" << application << "' update failed:";
        for(vector<string>::const_iterator r = reasons.begin(); r != reasons.end(); ++r)
        {
            os << "\n" << *r;
        }
        throw DeploymentException(os.str());
    }
}

void
Database::finishApplicationUpdate(const ApplicationUpdateInfo& update,
                                  const ApplicationInfo& oldApp,
                                  const ApplicationHelper& previousAppHelper,
                                  const ApplicationHelper& appHelper,
                                  AdminSessionI* /*session*/,
                                  bool noRestart,
                                  Ice::Long dbSerial)
{
    const ApplicationDescriptor& newDesc = appHelper.getDefinition();

    ServerEntrySeq entries;
    int serial = 0;
    try
    {
        if(_master)
        {
            checkUpdate(previousAppHelper, appHelper, oldApp.uuid, oldApp.revision, noRestart);
        }

        lock_guard lock(_mutex);

        IceDB::ReadWriteTxn txn(_env);

        checkForUpdate(previousAppHelper, appHelper, txn);
        reload(previousAppHelper, appHelper, entries, oldApp.uuid, oldApp.revision + 1, noRestart);

        for(const auto& entry : entries)
        {
            entry->sync();
        }

        ApplicationInfo info = oldApp;
        info.updateTime = update.updateTime;
        info.updateUser = update.updateUser;
        info.revision = update.revision;
        info.descriptor = newDesc;
        dbSerial = saveApplication(info, txn, dbSerial);

        txn.commit();

        serial = _applicationObserverTopic->applicationUpdated(dbSerial, update);
    }
    catch(const DeploymentException&)
    {
        finishUpdating(update.descriptor.name);
        throw;
    }
    catch(const IceDB::LMDBException& ex)
    {
        logError(_communicator, ex);
        throw;
    }

    _applicationObserverTopic->waitForSyncedSubscribers(serial); // Wait for replicas to be updated.

    //
    // Mark the application as updated. All the replicas received the update so it's now safe
    // for the nodes to start servers.
    //
    {
        lock_guard lock(_mutex);
        auto p = find(_updating.begin(), _updating.end(), update.descriptor.name);
        assert(p != _updating.end());
        p->markUpdated();
    }

    if(_master)
    {
        try
        {
            for(ServerEntrySeq::const_iterator p = entries.begin(); p != entries.end(); ++p)
            {
                try
                {
                    (*p)->waitForSync();
                }
                catch(const NodeUnreachableException&)
                {
                    // Ignore.
                }
            }
        }
        catch(const DeploymentException&)
        {
            ApplicationUpdateInfo newUpdate;
            {
                lock_guard lock(_mutex);
                entries.clear();
                ApplicationHelper previous(_communicator, newDesc);
                ApplicationHelper helper(_communicator, oldApp.descriptor);

                ApplicationInfo info = oldApp;
                info.revision = update.revision + 1;

                try
                {
                    IceDB::ReadWriteTxn txn(_env);
                    dbSerial = saveApplication(info, txn);
                    txn.commit();
                }
                catch(const IceDB::LMDBException& ex)
                {
                    logError(_communicator, ex);
                }

                reload(previous, helper, entries, info.uuid, info.revision, noRestart);

                newUpdate.updateTime = IceUtil::Time::now().toMilliSeconds();
                newUpdate.updateUser = _lockUserId;
                newUpdate.revision = info.revision;
                newUpdate.descriptor = helper.diff(previous);

                vector<UpdateInfo>::iterator p = find(_updating.begin(), _updating.end(), update.descriptor.name);
                assert(p != _updating.end());
                p->unmarkUpdated();

                for(const auto& entry : entries)
                {
                    entry->sync();
                }

                serial = _applicationObserverTopic->applicationUpdated(dbSerial, newUpdate);
            }
            _applicationObserverTopic->waitForSyncedSubscribers(serial); // Wait for subscriber to be updated.

            for(const auto& entry : entries)
            {
                entry->waitForSyncNoThrow();
            }

            finishUpdating(newDesc.name);
            throw;
        }
    }

    if(_traceLevels->application > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->applicationCat);
        out << "updated application `" << update.descriptor.name << "' (serial = `" << dbSerial << "')";
    }
    finishUpdating(update.descriptor.name);
}

void
Database::waitForUpdate(unique_lock<mutex>& lock, const string& name)
{
    _condVar.wait(lock, [this, &name] { return find(_updating.begin(), _updating.end(), name) == _updating.end(); });
}

void
Database::startUpdating(const string& name, const string& uuid, int revision)
{
    // Must be called within the synchronization.
    assert(find(_updating.begin(), _updating.end(), name) == _updating.end());
    _updating.push_back(UpdateInfo(name, uuid, revision));
}

void
Database::finishUpdating(const string& name)
{
    lock_guard lock(_mutex);

    vector<UpdateInfo>::iterator p = find(_updating.begin(), _updating.end(), name);
    assert(p != _updating.end());
    p->markUpdated();
    _updating.erase(p);
    _condVar.notify_all();
}

long long
Database::getSerial(const IceDB::Txn& txn, const string& dbName)
{
    long long serial = 1;
    _serials.get(txn, dbName, serial);
    return serial;
}

long long
Database::updateSerial(const IceDB::ReadWriteTxn& txn, const string& dbName, long long serial)
{
    if(serial == -1) // The master we are talking to doesn't support serials (old IceGrid versions)
    {
        return -1;
    }

    //
    // If a serial number is set, just update the serial number from the database,
    // otherwise if the serial is 0, we increment the serial from the database.
    //
    if(serial > 0)
    {
        _serials.put(txn, dbName, serial);
        return serial;
    }
    else
    {
        Ice::Long dbSerial = getSerial(txn, dbName) + 1;
        _serials.put(txn, dbName, dbSerial);
        return dbSerial;
    }
}

void
Database::addAdapter(const IceDB::ReadWriteTxn& txn, const AdapterInfo& info)
{
    _adapters.put(txn, info.id, info);
    _adaptersByGroupId.put(txn, info.replicaGroupId, info.id);
}

void
Database::deleteAdapter(const IceDB::ReadWriteTxn& txn, const AdapterInfo& info)
{

    _adapters.del(txn, info.id);
    _adaptersByGroupId.del(txn, info.replicaGroupId, info.id);
}

void
Database::addObject(const IceDB::ReadWriteTxn& txn, const ObjectInfo& info, bool internal)
{
    if(internal)
    {
        _internalObjects.put(txn, info.proxy->ice_getIdentity(), info);
        _internalObjectsByType.put(txn, info.type, info.proxy->ice_getIdentity());
    }
    else
    {
        try
        {
            _objects.put(txn, info.proxy->ice_getIdentity(), info);
        }
        catch(const IceDB::KeyTooLongException& ex)
        {
            throw DeploymentException("object identity `" +
                                      _communicator->identityToString(info.proxy->ice_getIdentity())
                                      + "' is too long: " + ex.what());
        }
        try
        {
            _objectsByType.put(txn, info.type, info.proxy->ice_getIdentity());
        }
        catch(const IceDB::KeyTooLongException& ex)
        {
            throw DeploymentException("object type `" + info.type + "' is too long: " + ex.what());
        }
    }
}

void
Database::deleteObject(const IceDB::ReadWriteTxn& txn, const ObjectInfo& info, bool internal)
{
    if(internal)
    {
        _internalObjects.del(txn, info.proxy->ice_getIdentity());
        _internalObjectsByType.del(txn, info.type, info.proxy->ice_getIdentity());
    }
    else
    {
        _objects.del(txn, info.proxy->ice_getIdentity());
        _objectsByType.del(txn, info.type, info.proxy->ice_getIdentity());
    }
}
