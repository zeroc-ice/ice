// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/StringUtil.h>
#include <IceUtil/Random.h>
#include <IceUtil/Functional.h>
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
#include <IceGrid/DB.h>

#include <algorithm>
#include <functional>
#include <iterator>

using namespace std;
using namespace IceGrid;

using namespace IceDB;

namespace
{

struct ObjectLoadCI : binary_function<pair<Ice::ObjectPrx, float>&, pair<Ice::ObjectPrx, float>&, bool>
{
    bool operator()(const pair<Ice::ObjectPrx, float>& lhs, const pair<Ice::ObjectPrx, float>& rhs)
    {
        return lhs.second < rhs.second;
    }
};

template<typename K, typename V> vector<V>
toVector(const map<K,V>& m)
{
    vector<V> v;
    for(typename map<K,V>::const_iterator p = m.begin(); p != m.end(); ++p)
    {
        v.push_back(p->second);
    }
    return v;
}

void
halt(const Ice::CommunicatorPtr& com, const DatabaseException& ex)
{
    {
        Ice::Error error(com->getLogger());
        error << "fatal exception: " << ex << "\n*** Aborting application ***";
    }

    abort();
}

}

Database::Database(const Ice::ObjectAdapterPtr& registryAdapter,
                   const IceStorm::TopicManagerPrx& topicManager,
                   const string& instanceName,
                   const TraceLevelsPtr& traceLevels,
                   const RegistryInfo& info,
                   const DatabasePluginPtr& plugin,
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
    _connectionPool(plugin->getConnectionPool()),
    _databasePlugin(plugin),
    _lock(0)
{
    ServerEntrySeq entries;

    DatabaseConnectionPtr connection = _connectionPool->getConnection();
    ApplicationsWrapperPtr applicationsWrapper = _connectionPool->getApplications(connection);
    map<string, ApplicationInfo> applications = applicationsWrapper->getMap();
    for(map<string, ApplicationInfo>::iterator p = applications.begin(); p != applications.end(); ++p)
    {
        try
        {
            load(ApplicationHelper(_communicator, p->second.descriptor), entries, p->second.uuid, p->second.revision);
        }
        catch(const DeploymentException& ex)
        {
            Ice::Error err(_traceLevels->logger);
            err << "invalid application `" << p->first << "':\n" << ex.reason;
        }
    }

    _serverCache.setTraceLevels(_traceLevels);
    _nodeCache.setTraceLevels(_traceLevels);
    _replicaCache.setTraceLevels(_traceLevels);
    _adapterCache.setTraceLevels(_traceLevels);
    _objectCache.setTraceLevels(_traceLevels);
    _allocatableObjectCache.setTraceLevels(_traceLevels);

    _nodeObserverTopic = new NodeObserverTopic(_topicManager, _internalAdapter);
    _registryObserverTopic = new RegistryObserverTopic(_topicManager);
    _applicationObserverTopic = new ApplicationObserverTopic(_topicManager, applicationsWrapper);
    _adapterObserverTopic = new AdapterObserverTopic(_topicManager, _connectionPool->getAdapters(connection));
    _objectObserverTopic = new ObjectObserverTopic(_topicManager, _connectionPool->getObjects(connection));

    _registryObserverTopic->registryUp(info);
}

Database::~Database()
{
    //
    // Release first the cache and then the plugin. This must be done in this order
    // to make sure the plugin is destroyed after the database cache.
    //
    _connectionPool = 0;
    _databasePlugin = 0;
}

std::string
Database::getInstanceName() const
{
    return _instanceName;
}

void
Database::destroyTopics()
{
    _registryObserverTopic->destroy();
    _nodeObserverTopic->destroy();
    _applicationObserverTopic->destroy();
    _adapterObserverTopic->destroy();
    _objectObserverTopic->destroy();
}

ObserverTopicPtr
Database::getObserverTopic(TopicName name) const
{
    switch(name)
    {
    case RegistryObserverTopicName:
        return _registryObserverTopic;
    case NodeObserverTopicName:
        return _nodeObserverTopic;
    case ApplicationObserverTopicName:
        return _applicationObserverTopic;
    case AdapterObserverTopicName:
        return _adapterObserverTopic;
    case ObjectObserverTopicName:
        return _objectObserverTopic;
    default:
        break;
    }
    return 0;
}

void
Database::checkSessionLock(AdminSessionI* session)
{
    if(_lock != 0 && session != _lock)
    {
        throw AccessDeniedException(_lockUserId); // Lock held by another session.
    }
}

int
Database::lock(AdminSessionI* session, const string& userId)
{
    Lock sync(*this);

    if(_lock != 0 && session != _lock)
    {
        throw AccessDeniedException(_lockUserId); // Lock held by another session.
    }
    assert(_lock == 0 || _lock == session);

    _lock = session;
    _lockUserId = userId;

    return _applicationObserverTopic->getSerial();
}

void
Database::unlock(AdminSessionI* session)
{
    Lock sync(*this);
    if(_lock != session)
    {
        throw AccessDeniedException();
    }

    _lock = 0;
    _lockUserId.clear();
}

void
Database::syncApplications(const ApplicationInfoSeq& newApplications, Ice::Long dbSerial)
{
    assert(dbSerial != 0);
    int serial = 0;
    {
        Lock sync(*this);

        map<string, ApplicationInfo> oldApplications;
        for(;;)
        {
            DatabaseConnectionPtr connection = _connectionPool->getConnection();
            try
            {
                TransactionHolder txHolder(connection);
                ApplicationsWrapperPtr applicationsWrapper = _connectionPool->getApplications(connection);
                oldApplications = applicationsWrapper->getMap();
                applicationsWrapper->clear();
                for(ApplicationInfoSeq::const_iterator p = newApplications.begin(); p != newApplications.end(); ++p)
                {
                    applicationsWrapper->put(p->descriptor.name, *p);
                }
                dbSerial = applicationsWrapper->updateSerial(dbSerial);
                txHolder.commit();
                break;
            }
            catch(const DeadlockException&)
            {
                continue;
            }
            catch(const DatabaseException& ex)
            {
                halt(_communicator, ex);
            }
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

        for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::sync));

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
Database::syncAdapters(const AdapterInfoSeq& adapters, Ice::Long dbSerial)
{
    assert(dbSerial != 0);
    int serial = 0;
    {
        Lock sync(*this);
        for(;;)
        {
            DatabaseConnectionPtr connection = _connectionPool->getConnection();
            try
            {
                TransactionHolder txHolder(connection);
                AdaptersWrapperPtr adaptersWrapper = _connectionPool->getAdapters(connection);
                adaptersWrapper->clear();
                for(AdapterInfoSeq::const_iterator r = adapters.begin(); r != adapters.end(); ++r)
                {
                    adaptersWrapper->put(r->id, *r);
                }
                dbSerial = adaptersWrapper->updateSerial(dbSerial);
                txHolder.commit();
                break;
            }
            catch(const DeadlockException&)
            {
                continue;
            }
            catch(const DatabaseException& ex)
            {
                halt(_communicator, ex);
            }
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
Database::syncObjects(const ObjectInfoSeq& objects, Ice::Long dbSerial)
{
    assert(dbSerial != 0);
    int serial = 0;
    {
        Lock sync(*this);
        for(;;)
        {
            DatabaseConnectionPtr connection = _connectionPool->getConnection();
            try
            {
                TransactionHolder txHolder(connection);
                ObjectsWrapperPtr objectsWrapper = _connectionPool->getObjects(connection);
                objectsWrapper->clear();
                for(ObjectInfoSeq::const_iterator q = objects.begin(); q != objects.end(); ++q)
                {
                    objectsWrapper->put(q->proxy->ice_getIdentity(), *q);
                }
                dbSerial = objectsWrapper->updateSerial(dbSerial);
                txHolder.commit();
                break;
            }
            catch(const DeadlockException&)
            {
                continue;
            }
            catch(const DatabaseException& ex)
            {
                halt(_communicator, ex);
            }
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
Database::getApplications(Ice::Long& serial) const
{
    for(;;)
    {
        try
        {
            DatabaseConnectionPtr connection = _connectionPool->newConnection();
            TransactionHolder txHolder(connection);
            ApplicationsWrapperPtr applicationsWrapper = _connectionPool->getApplications(connection);
            serial = applicationsWrapper->getSerial();
            return toVector(applicationsWrapper->getMap());
        }
        catch(const DeadlockException&)
        {
            continue;
        }
        catch(const DatabaseException& ex)
        {
            halt(_communicator, ex);
        }
    }
}

AdapterInfoSeq
Database::getAdapters(Ice::Long& serial) const
{
    for(;;)
    {
        try
        {
            DatabaseConnectionPtr connection = _connectionPool->newConnection();
            TransactionHolder txHolder(connection);
            AdaptersWrapperPtr adaptersWrapper = _connectionPool->getAdapters(connection);
            serial = adaptersWrapper->getSerial();
            return toVector(adaptersWrapper->getMap());
        }
        catch(const DeadlockException&)
        {
            continue;
        }
        catch(const DatabaseException& ex)
        {
            halt(_communicator, ex);
        }
    }
}

ObjectInfoSeq
Database::getObjects(Ice::Long& serial) const
{
    for(;;)
    {
        try
        {
            DatabaseConnectionPtr connection = _connectionPool->newConnection();
            TransactionHolder txHolder(connection);
            ObjectsWrapperPtr objectsWrapper = _connectionPool->getObjects(connection);
            serial = objectsWrapper->getSerial();
            return toVector(objectsWrapper->getMap());
        }
        catch(const DeadlockException&)
        {
            continue;
        }
        catch(const DatabaseException& ex)
        {
            halt(_communicator, ex);
        }
    }
}

StringLongDict
Database::getSerials() const
{
    return _connectionPool->getSerials();
}

void
Database::addApplication(const ApplicationInfo& info, AdminSessionI* session, Ice::Long dbSerial)
{
    assert(dbSerial != 0 || _master);

    int serial = 0; // Initialize to prevent warning.
    ServerEntrySeq entries;
    try
    {
        Lock sync(*this);
        checkSessionLock(session);

        waitForUpdate(info.descriptor.name);

        DatabaseConnectionPtr connection = _connectionPool->getConnection();
        try
        {
            ApplicationsWrapperPtr applicationsWrapper = _connectionPool->getApplications(connection);
            applicationsWrapper->find(info.descriptor.name);
            throw DeploymentException("application `" + info.descriptor.name + "' already exists");
        }
        catch(const NotFoundException&)
        {
        }

        ApplicationHelper helper(_communicator, info.descriptor, true);
        checkForAddition(helper, connection);
        dbSerial = saveApplication(info, connection, dbSerial);
        load(helper, entries, info.uuid, info.revision);
        startUpdating(info.descriptor.name, info.uuid, info.revision);

        for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::sync));
        serial = _applicationObserverTopic->applicationAdded(dbSerial, info);
    }
    catch(const DatabaseException& ex)
    {
        halt(_communicator, ex);
    }

    _applicationObserverTopic->waitForSyncedSubscribers(serial); // Wait for replicas to be updated.

    //
    // Mark the application as updated. All the replicas received the update so it's now safe
    // for the nodes to start the servers.
    //
    {
        Lock sync(*this);
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
        catch(const DeploymentException& ex)
        {
            try
            {
                Lock sync(*this);
                entries.clear();
                unload(ApplicationHelper(_communicator, info.descriptor), entries);
                dbSerial = removeApplication(info.descriptor.name, _connectionPool->getConnection());

                for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::sync));
                serial = _applicationObserverTopic->applicationRemoved(dbSerial, info.descriptor.name);
            }
            catch(const DeploymentException& ex)
            {
                Ice::Error err(_traceLevels->logger);
                err << "failed to rollback previous application `" << info.descriptor.name << "':\n" << ex.reason;
            }
            catch(const DatabaseException& ex)
            {
                halt(_communicator, ex);
            }
            _applicationObserverTopic->waitForSyncedSubscribers(serial);
            for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::waitForSyncNoThrow));
            finishUpdating(info.descriptor.name);
            throw ex;
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
    IceUtil::UniquePtr<ApplicationHelper> previous;
    IceUtil::UniquePtr<ApplicationHelper> helper;
    try
    {
        Lock sync(*this);
        checkSessionLock(session);

        waitForUpdate(update.descriptor.name);

        DatabaseConnectionPtr connection = _connectionPool->getConnection();
        ApplicationsWrapperPtr applicationsWrapper = _connectionPool->getApplications(connection);
        try
        {
            oldApp = applicationsWrapper->find(update.descriptor.name);
        }
        catch(const NotFoundException&)
        {
            throw ApplicationNotExistException(update.descriptor.name);
        }

        if(update.revision < 0)
        {
            update.revision = oldApp.revision + 1;
        }

        previous.reset(new ApplicationHelper(_communicator, oldApp.descriptor));
        helper.reset(new ApplicationHelper(_communicator, previous->update(update.descriptor), true));

        startUpdating(update.descriptor.name, oldApp.uuid, oldApp.revision + 1);
    }
    catch(const DatabaseException& ex)
    {
        halt(_communicator, ex);
    }

    finishApplicationUpdate(update, oldApp, *previous, *helper, session, noRestart, dbSerial);
}

void
Database::syncApplicationDescriptor(const ApplicationDescriptor& newDesc, bool noRestart, AdminSessionI* session)
{
    assert(_master);

    ApplicationUpdateInfo update;
    ApplicationInfo oldApp;
    IceUtil::UniquePtr<ApplicationHelper> previous;
    IceUtil::UniquePtr<ApplicationHelper> helper;
    try
    {
        Lock sync(*this);
        checkSessionLock(session);

        waitForUpdate(newDesc.name);

        DatabaseConnectionPtr connection = _connectionPool->getConnection();
        ApplicationsWrapperPtr applicationsWrapper = _connectionPool->getApplications(connection);
        try
        {
            oldApp = applicationsWrapper->find(newDesc.name);
        }
        catch(const NotFoundException&)
        {
            throw ApplicationNotExistException(newDesc.name);
        }

        previous.reset(new ApplicationHelper(_communicator, oldApp.descriptor));
        helper.reset(new ApplicationHelper(_communicator, newDesc, true));

        update.updateTime = IceUtil::Time::now().toMilliSeconds();
        update.updateUser = _lockUserId;
        update.revision = oldApp.revision + 1;
        update.descriptor = helper->diff(*previous);

        startUpdating(update.descriptor.name, oldApp.uuid, oldApp.revision + 1);
    }
    catch(const DatabaseException& ex)
    {
        halt(_communicator, ex);
    }

    finishApplicationUpdate(update, oldApp, *previous, *helper, session, noRestart);
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
    IceUtil::UniquePtr<ApplicationHelper> previous;
    IceUtil::UniquePtr<ApplicationHelper> helper;

    try
    {
        Lock sync(*this);
        checkSessionLock(session);

        waitForUpdate(application);

        DatabaseConnectionPtr connection = _connectionPool->getConnection();
        ApplicationsWrapperPtr applicationsWrapper = _connectionPool->getApplications(connection);
        try
        {
            oldApp = applicationsWrapper->find(application);
        }
        catch(const NotFoundException&)
        {
            throw ApplicationNotExistException(application);
        }

        previous.reset(new ApplicationHelper(_communicator, oldApp.descriptor));
        helper.reset(new ApplicationHelper(_communicator, previous->instantiateServer(node, instance), true));

        update.updateTime = IceUtil::Time::now().toMilliSeconds();
        update.updateUser = _lockUserId;
        update.revision = oldApp.revision + 1;
        update.descriptor = helper->diff(*previous);

        startUpdating(update.descriptor.name, oldApp.uuid, oldApp.revision + 1);
    }
    catch(const DatabaseException& ex)
    {
        halt(_communicator, ex);
    }

    finishApplicationUpdate(update, oldApp, *previous, *helper, session, true);
}

void
Database::removeApplication(const string& name, AdminSessionI* session, Ice::Long dbSerial)
{
    assert(dbSerial != 0 || _master);
    ServerEntrySeq entries;

    int serial = 0; // Initialize to prevent warning.
    try
    {
        Lock sync(*this);
        checkSessionLock(session);

        waitForUpdate(name);

        DatabaseConnectionPtr connection = _connectionPool->getConnection();
        ApplicationInfo appInfo;
        try
        {
            ApplicationsWrapperPtr applicationsWrapper = _connectionPool->getApplications(connection);
            appInfo = applicationsWrapper->find(name);
        }
        catch(const NotFoundException&)
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

        dbSerial = removeApplication(name, connection, dbSerial);
        startUpdating(name, appInfo.uuid, appInfo.revision);

        for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::sync));
        serial = _applicationObserverTopic->applicationRemoved(dbSerial, name);
    }
    catch(const DatabaseException& ex)
    {
        halt(_communicator, ex);
    }
    _applicationObserverTopic->waitForSyncedSubscribers(serial);

    if(_master)
    {
        for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::waitForSyncNoThrow));
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
    DatabaseConnectionPtr connection = _connectionPool->newConnection();
    try
    {
        ApplicationsWrapperPtr applicationsWrapper = _connectionPool->getApplications(connection);
        return applicationsWrapper->find(name);
    }
    catch(const NotFoundException&)
    {
        throw ApplicationNotExistException(name);
    }
}

Ice::StringSeq
Database::getAllApplications(const string& expression)
{
    DatabaseConnectionPtr connection = _connectionPool->newConnection();
    ApplicationsWrapperPtr applicationsWrapper = _connectionPool->getApplications(connection);
    return getMatchingKeys<map<string, ApplicationInfo> >(applicationsWrapper->getMap(), expression);
}

void
Database::waitForApplicationUpdate(const AMD_NodeSession_waitForApplicationUpdatePtr& cb,
                                   const string& uuid,
                                   int revision)
{
    Lock sync(*this);

    vector<UpdateInfo>::iterator p = find(_updating.begin(), _updating.end(), make_pair(uuid, revision));
    if(p != _updating.end() && !p->updated)
    {
        p->cbs.push_back(cb);
    }
    else
    {
        cb->ice_response();
    }
}

NodeCache&
Database::getNodeCache()
{
    return _nodeCache;
}

NodeEntryPtr
Database::getNode(const string& name, bool create) const
{
    return _nodeCache.get(name, create);
}

ReplicaCache&
Database::getReplicaCache()
{
    return _replicaCache;
}

ReplicaEntryPtr
Database::getReplica(const string& name) const
{
    return _replicaCache.get(name);
}

ServerCache&
Database::getServerCache()
{
    return _serverCache;
}

ServerEntryPtr
Database::getServer(const string& id) const
{
    return _serverCache.get(id);
}

AllocatableObjectCache&
Database::getAllocatableObjectCache()
{
    return _allocatableObjectCache;
}

AllocatableObjectEntryPtr
Database::getAllocatableObject(const Ice::Identity& id) const
{
    return _allocatableObjectCache.get(id);
}

void
Database::setAdapterDirectProxy(const string& adapterId, const string& replicaGroupId, const Ice::ObjectPrx& proxy,
                                Ice::Long dbSerial)
{
    assert(dbSerial != 0 || _master);

    int serial = 0; // Initialize to prevent warning.
    {
        Lock sync(*this);
        if(_adapterCache.has(adapterId))
        {
            throw AdapterExistsException(adapterId);
        }

        AdapterInfo info;
        info.id = adapterId;
        info.proxy = proxy;
        info.replicaGroupId = replicaGroupId;

        bool updated = false;
        for(;;)
        {
            try
            {
                DatabaseConnectionPtr connection = _connectionPool->getConnection();
                TransactionHolder txHolder(connection);
                AdaptersWrapperPtr adaptersWrapper = _connectionPool->getAdapters(connection);
                try
                {
                    adaptersWrapper->find(adapterId);
                    updated = true;
                }
                catch(const NotFoundException&)
                {
                }

                if(proxy)
                {
                    adaptersWrapper->put(adapterId, info);
                }
                else
                {
                    adaptersWrapper->erase(adapterId);
                }
                dbSerial = adaptersWrapper->updateSerial(dbSerial);
                txHolder.commit();
                break;
            }
            catch(const DeadlockException&)
            {
                continue;
            }
            catch(const DatabaseException& ex)
            {
                halt(_communicator, ex);
            }
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

Ice::ObjectPrx
Database::getAdapterDirectProxy(const string& id, const Ice::EncodingVersion& encoding)
{
    DatabaseConnectionPtr connection = _connectionPool->newConnection();
    AdaptersWrapperPtr adaptersWrapper = _connectionPool->getAdapters(connection);
    try
    {
        return adaptersWrapper->find(id).proxy;
    }
    catch(const NotFoundException&)
    {
    }

    Ice::EndpointSeq endpoints;
    vector<AdapterInfo> infos = adaptersWrapper->findByReplicaGroupId(id);
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

    throw AdapterNotExistException(id);
}

void
Database::removeAdapter(const string& adapterId)
{
    assert(_master);

    int serial = 0; // Initialize to prevent warning.
    {
        Lock sync(*this);
        if(_adapterCache.has(adapterId))
        {
            AdapterEntryPtr adpt = _adapterCache.get(adapterId);
            DeploymentException ex;
            ex.reason = "removing adapter `" + adapterId + "' is not allowed:\n";
            ex.reason += "the adapter was added with the application descriptor `" + adpt->getApplication() + "'";
            throw ex;
        }

        AdapterInfoSeq infos;
        Ice::Long dbSerial = 0;
        for(;;)
        {
            try
            {
                DatabaseConnectionPtr connection = _connectionPool->getConnection();
                TransactionHolder txHolder(connection);
                AdaptersWrapperPtr adaptersWrapper = _connectionPool->getAdapters(connection);
                try
                {
                    adaptersWrapper->find(adapterId);
                    adaptersWrapper->erase(adapterId);
                }
                catch(const NotFoundException&)
                {
                    infos = adaptersWrapper->findByReplicaGroupId(adapterId);
                    if(infos.empty())
                    {
                        throw AdapterNotExistException(adapterId);
                    }
                    for(AdapterInfoSeq::iterator p = infos.begin(); p != infos.end(); ++p)
                    {
                        p->replicaGroupId.clear();
                        adaptersWrapper->put(p->id, *p);
                    }
                }
                dbSerial = adaptersWrapper->updateSerial();
                txHolder.commit();
                break;
            }
            catch(const DeadlockException&)
            {
                continue;
            }
            catch(const DatabaseException& ex)
            {
                halt(_communicator, ex);
            }
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

AdapterPrx
Database::getAdapterProxy(const string& adapterId, const string& replicaGroupId, bool upToDate)
{
    Lock sync(*this); // Make sure this isn't call during an update.
    return _adapterCache.get(adapterId)->getProxy(replicaGroupId, upToDate);
}

void
Database::getLocatorAdapterInfo(const string& id,
                                LocatorAdapterInfoSeq& adpts,
                                int& count,
                                bool& replicaGroup,
                                bool& roundRobin,
                                const set<string>& excludes)
{
    Lock sync(*this); // Make sure this isn't call during an update.
    _adapterCache.get(id)->getLocatorAdapterInfo(adpts, count, replicaGroup, roundRobin, excludes);
}

bool
Database::addAdapterSyncCallback(const string& id,
                                 const SynchronizationCallbackPtr& callback,
                                 const std::set<std::string>& excludes)
{
    Lock sync(*this); // Make sure this isn't call during an update.
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
    try
    {
        Lock sync(*this); // Make sure this isn't call during an update.
        return _adapterCache.get(id)->getAdapterInfo();
    }
    catch(AdapterNotExistException&)
    {
    }

    //
    // Otherwise, we check the adapter endpoint table -- if there's an
    // entry the adapter is managed by the registry itself.
    //
    DatabaseConnectionPtr connection = _connectionPool->newConnection();
    AdaptersWrapperPtr adaptersWrapper = _connectionPool->getAdapters(connection);
    AdapterInfoSeq infos;
    try
    {
        infos.push_back(adaptersWrapper->find(id));
    }
    catch(const NotFoundException&)
    {
        //
        // If it's not a regular object adapter, perhaps it's a replica
        // group...
        //
        infos = adaptersWrapper->findByReplicaGroupId(id);
        if(infos.size() == 0)
        {
            throw AdapterNotExistException(id);
        }
    }
    return infos;
}

Ice::StringSeq
Database::getAllAdapters(const string& expression)
{
    Lock sync(*this);
    vector<string> result;
    vector<string> ids = _adapterCache.getAll(expression);
    result.swap(ids);
    set<string> groups;

    DatabaseConnectionPtr connection = _connectionPool->getConnection();
    AdaptersWrapperPtr adaptersWrapper = _connectionPool->getAdapters(connection);
    map<string, AdapterInfo> adapters = adaptersWrapper->getMap();
    for(map<string, AdapterInfo>::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
        if(expression.empty() || IceUtilInternal::match(p->first, expression, true))
        {
            result.push_back(p->first);
        }
        string replicaGroupId = p->second.replicaGroupId;
        if(!replicaGroupId.empty() && (expression.empty() || IceUtilInternal::match(replicaGroupId, expression, true)))
        {
            groups.insert(replicaGroupId);
        }
    }
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
        Lock sync(*this);
        const Ice::Identity id = info.proxy->ice_getIdentity();

        if(_objectCache.has(id))
        {
            throw ObjectExistsException(id);
        }

        Ice::Long dbSerial = 0;
        for(;;)
        {
            try
            {
                DatabaseConnectionPtr connection = _connectionPool->getConnection();
                TransactionHolder txHolder(connection);
                ObjectsWrapperPtr objectsWrapper = _connectionPool->getObjects(connection);
                try
                {
                    objectsWrapper->find(id);
                    throw ObjectExistsException(id);
                }
                catch(const NotFoundException&)
                {
                }
                objectsWrapper->put(id, info);
                dbSerial = objectsWrapper->updateSerial();
                txHolder.commit();
                break;
            }
            catch(const DeadlockException&)
            {
                continue;
            }
            catch(const DatabaseException& ex)
            {
                halt(_communicator, ex);
            }
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
Database::addOrUpdateObject(const ObjectInfo& info, Ice::Long dbSerial)
{
    assert(dbSerial != 0 || _master);

    int serial = 0; // Initialize to prevent warning.
    {
        Lock sync(*this);
        const Ice::Identity id = info.proxy->ice_getIdentity();

        if(_objectCache.has(id))
        {
            throw ObjectExistsException(id);
        }

        bool update = false;
        for(;;)
        {
            try
            {
                DatabaseConnectionPtr connection = _connectionPool->getConnection();
                TransactionHolder txHolder(connection);
                ObjectsWrapperPtr objectsWrapper = _connectionPool->getObjects(connection);
                try
                {
                    objectsWrapper->find(id);
                    update = true;
                }
                catch(const NotFoundException&)
                {
                }
                objectsWrapper->put(id, info);
                dbSerial = objectsWrapper->updateSerial(dbSerial);
                txHolder.commit();
                break;
            }
            catch(const DeadlockException&)
            {
                continue;
            }
            catch(const DatabaseException& ex)
            {
                halt(_communicator, ex);
            }
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
Database::removeObject(const Ice::Identity& id, Ice::Long dbSerial)
{
    assert(dbSerial != 0 || _master);

    int serial = 0; // Initialize to prevent warning.
    {
        Lock sync(*this);
        if(_objectCache.has(id))
        {
            DeploymentException ex;
            ex.reason = "removing object `" + _communicator->identityToString(id) + "' is not allowed:\n";
            ex.reason += "the object was added with the application descriptor `";
            ex.reason += _objectCache.get(id)->getApplication();
            ex.reason += "'";
            throw ex;
        }

        for(;;)
        {
            try
            {
                DatabaseConnectionPtr connection = _connectionPool->getConnection();
                TransactionHolder txHolder(connection);
                ObjectsWrapperPtr objectsWrapper = _connectionPool->getObjects(connection);
                try
                {
                    objectsWrapper->find(id);
                }
                catch(const NotFoundException&)
                {
                    ObjectNotRegisteredException ex;
                    ex.id = id;
                    throw ex;
                }

                objectsWrapper->erase(id);
                dbSerial = objectsWrapper->updateSerial(dbSerial);
                txHolder.commit();
                break;
            }
            catch(const DeadlockException&)
            {
                continue;
            }
            catch(const DatabaseException& ex)
            {
                halt(_communicator, ex);
            }
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
Database::updateObject(const Ice::ObjectPrx& proxy)
{
    assert(_master);

    int serial = 0;
    {
        Lock sync(*this);

        const Ice::Identity id = proxy->ice_getIdentity();
        if(_objectCache.has(id))
        {
            DeploymentException ex;
            ex.reason = "updating object `" + _communicator->identityToString(id) + "' is not allowed:\n";
            ex.reason += "the object was added with the application descriptor `";
            ex.reason += _objectCache.get(id)->getApplication();
            ex.reason += "'";
            throw ex;
        }

        ObjectInfo info;
        Ice::Long dbSerial = 0;
        for(;;)
        {
            try
            {
                DatabaseConnectionPtr connection = _connectionPool->getConnection();
                TransactionHolder txHolder(connection);
                ObjectsWrapperPtr objectsWrapper = _connectionPool->getObjects(connection);
                try
                {
                    info = objectsWrapper->find(id);
                }
                catch(const NotFoundException&)
                {
                    ObjectNotRegisteredException ex;
                    ex.id = id;
                    throw ex;
                }

                info.proxy = proxy;
                objectsWrapper->put(id, info);
                dbSerial = objectsWrapper->updateSerial();
                txHolder.commit();
                break;
            }
            catch(const DeadlockException&)
            {
                continue;
            }
            catch(const DatabaseException& ex)
            {
                halt(_communicator, ex);
            }
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
    Lock sync(*this);
    for(;;)
    {
        try
        {
            DatabaseConnectionPtr connection = _connectionPool->getConnection();
            TransactionHolder txHolder(connection);
            ObjectsWrapperPtr objectsWrapper = _connectionPool->getObjects(connection);
            for(ObjectInfoSeq::const_iterator p = objects.begin(); p != objects.end(); ++p)
            {
                objectsWrapper->put(p->proxy->ice_getIdentity(), *p);
            }
            txHolder.commit();
            break;
        }
        catch(const DeadlockException&)
        {
            continue;
        }
        catch(const DatabaseException& ex)
        {
            halt(_communicator, ex);
        }
    }
    return _objectObserverTopic->wellKnownObjectsAddedOrUpdated(objects);
}

int
Database::removeRegistryWellKnownObjects(const ObjectInfoSeq& objects)
{
    Lock sync(*this);
    for(;;)
    {
        try
        {
            DatabaseConnectionPtr connection = _connectionPool->getConnection();
            TransactionHolder txHolder(connection);
            ObjectsWrapperPtr objectsWrapper = _connectionPool->getObjects(connection);
            for(ObjectInfoSeq::const_iterator p = objects.begin(); p != objects.end(); ++p)
            {
                objectsWrapper->erase(p->proxy->ice_getIdentity());
            }
            txHolder.commit();
            break;
        }
        catch(const DeadlockException&)
        {
            continue;
        }
        catch(const DatabaseException& ex)
        {
            halt(_communicator, ex);
        }
    }
    return _objectObserverTopic->wellKnownObjectsRemoved(objects);
}

Ice::ObjectPrx
Database::getObjectProxy(const Ice::Identity& id)
{
    try
    {
        //
        // Only return proxies for non allocatable objects.
        //
        return _objectCache.get(id)->getProxy();
    }
    catch(ObjectNotRegisteredException&)
    {
    }

    DatabaseConnectionPtr connection = _connectionPool->newConnection();
    try
    {
        ObjectsWrapperPtr objectsWrapper = _connectionPool->getObjects(connection);
        return objectsWrapper->find(id).proxy;
    }
    catch(const NotFoundException&)
    {
        ObjectNotRegisteredException ex;
        ex.id = id;
        throw ex;
    }
}

Ice::ObjectPrx
Database::getObjectByType(const string& type)
{
    Ice::ObjectProxySeq objs = getObjectsByType(type);
    if(objs.empty())
    {
        return 0;
    }
    return objs[IceUtilInternal::random(static_cast<int>(objs.size()))];
}

Ice::ObjectPrx
Database::getObjectByTypeOnLeastLoadedNode(const string& type, LoadSample sample)
{
    Ice::ObjectProxySeq objs = getObjectsByType(type);
    if(objs.empty())
    {
        return 0;
    }

    RandomNumberGenerator rng;
    random_shuffle(objs.begin(), objs.end(), rng);
    vector<pair<Ice::ObjectPrx, float> > objectsWithLoad;
    objectsWithLoad.reserve(objs.size());
    for(Ice::ObjectProxySeq::const_iterator p = objs.begin(); p != objs.end(); ++p)
    {
        float load = 1.0f;
        if(!(*p)->ice_getAdapterId().empty())
        {
            try
            {
                load = _adapterCache.get((*p)->ice_getAdapterId())->getLeastLoadedNodeLoad(sample);
            }
            catch(const AdapterNotExistException&)
            {
            }
        }
        objectsWithLoad.push_back(make_pair(*p, load));
    }
    return min_element(objectsWithLoad.begin(), objectsWithLoad.end(), ObjectLoadCI())->first;
}


Ice::ObjectProxySeq
Database::getObjectsByType(const string& type)
{
    Ice::ObjectProxySeq proxies = _objectCache.getObjectsByType(type);

    DatabaseConnectionPtr connection = _connectionPool->newConnection();
    ObjectsWrapperPtr objectsWrapper = _connectionPool->getObjects(connection);
    vector<ObjectInfo> infos = objectsWrapper->findByType(type);
    for(unsigned int i = 0; i < infos.size(); ++i)
    {
        proxies.push_back(infos[i].proxy);
    }
    return proxies;
}

ObjectInfo
Database::getObjectInfo(const Ice::Identity& id)
{
    try
    {
        ObjectEntryPtr object = _objectCache.get(id);
        return object->getObjectInfo();
    }
    catch(ObjectNotRegisteredException&)
    {
    }

    DatabaseConnectionPtr connection = _connectionPool->newConnection();
    ObjectsWrapperPtr objectsWrapper = _connectionPool->getObjects(connection);
    try
    {
        return objectsWrapper->find(id);
    }
    catch(const NotFoundException&)
    {
        throw ObjectNotRegisteredException(id);
    }
}

ObjectInfoSeq
Database::getAllObjectInfos(const string& expression)
{
    ObjectInfoSeq infos = _objectCache.getAll(expression);

    DatabaseConnectionPtr connection = _connectionPool->newConnection();
    ObjectsWrapperPtr objectsWrapper = _connectionPool->getObjects(connection);
    map<Ice::Identity, ObjectInfo> objects = objectsWrapper->getMap();
    for(map<Ice::Identity, ObjectInfo>::const_iterator p = objects.begin(); p != objects.end(); ++p)
    {
        if(expression.empty() || IceUtilInternal::match(_communicator->identityToString(p->first), expression, true))
        {
            infos.push_back(p->second);
        }
    }
    return infos;
}

ObjectInfoSeq
Database::getObjectInfosByType(const string& type)
{
    ObjectInfoSeq infos = _objectCache.getAllByType(type);

    DatabaseConnectionPtr connection = _connectionPool->newConnection();
    ObjectsWrapperPtr objectsWrapper = _connectionPool->getObjects(connection);
    ObjectInfoSeq dbInfos = objectsWrapper->findByType(type);
    for(unsigned int i = 0; i < dbInfos.size(); ++i)
    {
        infos.push_back(dbInfos[i]);
    }
    return infos;
}

void
Database::addInternalObject(const ObjectInfo& info, bool replace)
{
    Lock sync(*this);
    const Ice::Identity id = info.proxy->ice_getIdentity();

    for(;;)
    {
        try
        {
            DatabaseConnectionPtr connection = _connectionPool->getConnection();
            TransactionHolder txHolder(connection);
            ObjectsWrapperPtr internalObjectsWrapper = _connectionPool->getInternalObjects(connection);
            if(!replace)
            {
                try
                {
                    internalObjectsWrapper->find(id);
                    throw ObjectExistsException(id);
                }
                catch(const NotFoundException&)
                {
                }
            }
            internalObjectsWrapper->put(id, info);
            txHolder.commit();
            break;
        }
        catch(const DeadlockException&)
        {
            continue;
        }
        catch(const DatabaseException& ex)
        {
            halt(_communicator, ex);
        }
    }
}

void
Database::removeInternalObject(const Ice::Identity& id)
{
    Lock sync(*this);

    for(;;)
    {
        try
        {
            DatabaseConnectionPtr connection = _connectionPool->getConnection();
            TransactionHolder txHolder(connection);
            ObjectsWrapperPtr internalObjectsWrapper = _connectionPool->getInternalObjects(connection);
            try
            {
                internalObjectsWrapper->find(id);
            }
            catch(const NotFoundException&)
            {
                ObjectNotRegisteredException ex;
                ex.id = id;
                throw ex;
            }
            internalObjectsWrapper->erase(id);
            txHolder.commit();
            break;
        }
        catch(const DeadlockException&)
        {
            continue;
        }
        catch(const DatabaseException& ex)
        {
            halt(_communicator, ex);
        }
    }
}

Ice::ObjectProxySeq
Database::getInternalObjectsByType(const string& type)
{
    Ice::ObjectProxySeq proxies;

    DatabaseConnectionPtr connection = _connectionPool->newConnection();
    ObjectsWrapperPtr internalObjectsWrapper = _connectionPool->getInternalObjects(connection);
    vector<ObjectInfo> infos = internalObjectsWrapper->findByType(type);
    for(unsigned int i = 0; i < infos.size(); ++i)
    {
        proxies.push_back(infos[i].proxy);
    }
    return proxies;
}

void
Database::checkForAddition(const ApplicationHelper& app, const DatabaseConnectionPtr& connection)
{
    set<string> serverIds;
    set<string> adapterIds;
    set<Ice::Identity> objectIds;

    app.getIds(serverIds, adapterIds, objectIds);

    for_each(serverIds.begin(), serverIds.end(), objFunc(*this, &Database::checkServerForAddition));
    if(!adapterIds.empty())
    {
        AdaptersWrapperPtr adaptersWrapper = _connectionPool->getAdapters(connection);
        for(set<string>::const_iterator p = adapterIds.begin(); p != adapterIds.end(); ++p)
        {
            checkAdapterForAddition(*p, adaptersWrapper);
        }
    }
    if(!objectIds.empty())
    {
        ObjectsWrapperPtr objectsWrapper = _connectionPool->getObjects(connection);
        for(set<Ice::Identity>::const_iterator p = objectIds.begin(); p != objectIds.end(); ++p)
        {
            checkObjectForAddition(*p, objectsWrapper);
        }
    }

    set<string> repGrps;
    set<string> adptRepGrps;
    app.getReplicaGroups(repGrps, adptRepGrps);
    for_each(adptRepGrps.begin(), adptRepGrps.end(), objFunc(*this, &Database::checkReplicaGroupExists));
}

void
Database::checkForUpdate(const ApplicationHelper& origApp,
                         const ApplicationHelper& newApp,
                         const DatabaseConnectionPtr& connection)
{
    set<string> oldSvrs, newSvrs;
    set<string> oldAdpts, newAdpts;
    set<Ice::Identity> oldObjs, newObjs;

    origApp.getIds(oldSvrs, oldAdpts, oldObjs);
    newApp.getIds(newSvrs, newAdpts, newObjs);

    Ice::StringSeq addedSvrs;
    set_difference(newSvrs.begin(), newSvrs.end(), oldSvrs.begin(), oldSvrs.end(), back_inserter(addedSvrs));
    for_each(addedSvrs.begin(), addedSvrs.end(), objFunc(*this, &Database::checkServerForAddition));

    Ice::StringSeq addedAdpts;
    set_difference(newAdpts.begin(), newAdpts.end(), oldAdpts.begin(), oldAdpts.end(), back_inserter(addedAdpts));
    if(!addedAdpts.empty())
    {
        AdaptersWrapperPtr adaptersWrapper = _connectionPool->getAdapters(connection);
        for(Ice::StringSeq::const_iterator p = addedAdpts.begin(); p != addedAdpts.end(); ++p)
        {
            checkAdapterForAddition(*p, adaptersWrapper);
        }
    }

    vector<Ice::Identity> addedObjs;
    set_difference(newObjs.begin(), newObjs.end(), oldObjs.begin(), oldObjs.end(), back_inserter(addedObjs));
    if(!addedObjs.empty())
    {
        ObjectsWrapperPtr objectsWrapper = _connectionPool->getObjects(connection);
        for(vector<Ice::Identity>::const_iterator p = addedObjs.begin(); p != addedObjs.end(); ++p)
        {
            checkObjectForAddition(*p, objectsWrapper);
        }
    }

    set<string> oldRepGrps, newRepGrps;
    set<string> oldAdptRepGrps, newAdptRepGrps;
    origApp.getReplicaGroups(oldRepGrps, oldAdptRepGrps);
    newApp.getReplicaGroups(newRepGrps, newAdptRepGrps);

    set<string> rmRepGrps;
    set_difference(oldRepGrps.begin(), oldRepGrps.end(), newRepGrps.begin(),newRepGrps.end(), set_inserter(rmRepGrps));
    for_each(rmRepGrps.begin(), rmRepGrps.end(), objFunc(*this, &Database::checkReplicaGroupForRemove));

    set<string> addedAdptRepGrps;
    set_difference(newAdptRepGrps.begin(),newAdptRepGrps.end(), oldAdptRepGrps.begin(), oldAdptRepGrps.end(),
                   set_inserter(addedAdptRepGrps));
    for_each(addedAdptRepGrps.begin(), addedAdptRepGrps.end(), objFunc(*this, &Database::checkReplicaGroupExists));

    vector<string> invalidAdptRepGrps;
    set_intersection(rmRepGrps.begin(), rmRepGrps.end(), newAdptRepGrps.begin(), newAdptRepGrps.end(),
                     back_inserter(invalidAdptRepGrps));
    if(!invalidAdptRepGrps.empty())
    {
        DeploymentException ex;
        ex.reason = "couldn't find replica group `" + invalidAdptRepGrps.front() + "'";
        throw ex;
    }
}

void
Database::checkForRemove(const ApplicationHelper& app)
{
    set<string> replicaGroups;
    set<string> adapterReplicaGroups;
    app.getReplicaGroups(replicaGroups, adapterReplicaGroups);
    for_each(replicaGroups.begin(), replicaGroups.end(), objFunc(*this, &Database::checkReplicaGroupForRemove));
}

void
Database::checkServerForAddition(const string& id)
{
    if(_serverCache.has(id))
    {
        DeploymentException ex;
        ex.reason = "server `" + id + "' is already registered";
        throw ex;
    }
}

void
Database::checkAdapterForAddition(const string& id, const AdaptersWrapperPtr& adaptersWrapper)
{
    bool found = false;
    if(_adapterCache.has(id))
    {
        found = true;
    }
    else
    {
        try
        {
            adaptersWrapper->find(id);
            found = true;
        }
        catch(const NotFoundException&)
        {
            if(adaptersWrapper->findByReplicaGroupId(id).size() != 0)
            {
                found = true;
            }
        }
    }

    if(found)
    {
        DeploymentException ex;
        ex.reason = "adapter `" + id + "' is already registered";
        throw ex;
    }
}

void
Database::checkObjectForAddition(const Ice::Identity& objectId, const ObjectsWrapperPtr& objectsWrapper)
{
    bool found = false;
    if(_objectCache.has(objectId) || _allocatableObjectCache.has(objectId))
    {
        found = true;
    }
    else
    {
        try
        {
            objectsWrapper->find(objectId);
            found = true;
        }
        catch(const NotFoundException&)
        {
        }
    }

    if(found)
    {
        DeploymentException ex;
        ex.reason = "object `" + _communicator->identityToString(objectId) + "' is already registered";
        throw ex;
    }
}

void
Database::checkReplicaGroupExists(const string& replicaGroup)
{
    ReplicaGroupEntryPtr entry;
    try
    {
        entry = ReplicaGroupEntryPtr::dynamicCast(_adapterCache.get(replicaGroup));
    }
    catch(const AdapterNotExistException&)
    {
    }

    if(!entry)
    {
        DeploymentException ex;
        ex.reason = "couldn't find replica group `" + replicaGroup + "'";
        throw ex;
    }
}

void
Database::checkReplicaGroupForRemove(const string& replicaGroup)
{
    ReplicaGroupEntryPtr entry;
    try
    {
        entry = ReplicaGroupEntryPtr::dynamicCast(_adapterCache.get(replicaGroup));
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
        DeploymentException ex;
        ex.reason = "couldn't remove application because the replica group `" + replicaGroup +
            "' is used by object adapters from other applications.";
        throw ex;
    }
}

void
Database::load(const ApplicationHelper& app, ServerEntrySeq& entries, const string& uuid, int revision)
{
    const NodeDescriptorDict& nodes = app.getInstance().nodes;
    const string application = app.getInstance().name;
    for(NodeDescriptorDict::const_iterator n = nodes.begin(); n != nodes.end(); ++n)
    {
        _nodeCache.get(n->first, true)->addDescriptor(application, n->second);
    }

    const ReplicaGroupDescriptorSeq& adpts = app.getInstance().replicaGroups;
    for(ReplicaGroupDescriptorSeq::const_iterator r = adpts.begin(); r != adpts.end(); ++r)
    {
        assert(!r->id.empty());
        _adapterCache.addReplicaGroup(*r, application);
        for(ObjectDescriptorSeq::const_iterator o = r->objects.begin(); o != r->objects.end(); ++o)
        {
            _objectCache.add(toObjectInfo(_communicator, *o, r->id), application);
        }
    }

    map<string, ServerInfo> servers = app.getServerInfos(uuid, revision);
    for(map<string, ServerInfo>::const_iterator p = servers.begin(); p != servers.end(); ++p)
    {
        entries.push_back(_serverCache.add(p->second));
    }
}

void
Database::unload(const ApplicationHelper& app, ServerEntrySeq& entries)
{
    map<string, ServerInfo> servers = app.getServerInfos("", 0);
    for(map<string, ServerInfo>::const_iterator p = servers.begin(); p != servers.end(); ++p)
    {
        entries.push_back(_serverCache.remove(p->first));
    }

    const ReplicaGroupDescriptorSeq& adpts = app.getInstance().replicaGroups;
    for(ReplicaGroupDescriptorSeq::const_iterator r = adpts.begin(); r != adpts.end(); ++r)
    {
        for(ObjectDescriptorSeq::const_iterator o = r->objects.begin(); o != r->objects.end(); ++o)
        {
            _objectCache.remove(o->id);
        }
        _adapterCache.removeReplicaGroup(r->id);
    }

    const NodeDescriptorDict& nodes = app.getInstance().nodes;
    const string application = app.getInstance().name;
    for(NodeDescriptorDict::const_iterator n = nodes.begin(); n != nodes.end(); ++n)
    {
        _nodeCache.get(n->first)->removeDescriptor(application);
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
    map<string, ServerInfo> oldServers = oldApp.getServerInfos(uuid, revision);
    map<string, ServerInfo> newServers = newApp.getServerInfos(uuid, revision);
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
            ServerEntryPtr server = _serverCache.get(p->first);
            server->update(q->second, noRestart); // Just update the server revision on the node.
            entries.push_back(server);
        }
    }
    for(map<string, ServerInfo>::const_iterator p = oldServers.begin(); p != oldServers.end(); ++p)
    {
        map<string, ServerInfo>::const_iterator q = newServers.find(p->first);
        if(q == newServers.end())
        {
            entries.push_back(_serverCache.remove(p->first));
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
            ReplicaGroupEntryPtr entry = ReplicaGroupEntryPtr::dynamicCast(_adapterCache.get(r->id));
            assert(entry);
            entry->update(application, r->loadBalancing);
        }
        catch(const AdapterNotExistException&)
        {
            _adapterCache.addReplicaGroup(*r, application);
        }

        for(ObjectDescriptorSeq::const_iterator o = r->objects.begin(); o != r->objects.end(); ++o)
        {
            _objectCache.add(toObjectInfo(_communicator, *o, r->id), application);
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

Ice::Long
Database::saveApplication(const ApplicationInfo& info, const DatabaseConnectionPtr& connection, Ice::Long dbSerial)
{
    assert(dbSerial != 0 || _master);
    for(;;)
    {
        try
        {
            ApplicationsWrapperPtr applicationsWrapper = _connectionPool->getApplications(connection);
            TransactionHolder txHolder(connection);
            applicationsWrapper->put(info.descriptor.name, info);
            dbSerial = applicationsWrapper->updateSerial(dbSerial);
            txHolder.commit();
            break;
        }
        catch(const DeadlockException&)
        {
            continue;
        }
        catch(const DatabaseException& ex)
        {
            halt(_communicator, ex);
        }
    }
    return dbSerial;
}

Ice::Long
Database::removeApplication(const string& name, const DatabaseConnectionPtr& connection, Ice::Long dbSerial)
{
    assert(dbSerial != 0 || _master);
    for(;;)
    {
        try
        {
            ApplicationsWrapperPtr applicationsWrapper = _connectionPool->getApplications(connection);
            TransactionHolder txHolder(connection);
            applicationsWrapper->erase(name);
            dbSerial = applicationsWrapper->updateSerial(dbSerial);
            txHolder.commit();
            break;
        }
        catch(const DeadlockException&)
        {
            continue;
        }
        catch(const DatabaseException& ex)
        {
            halt(_communicator, ex);
        }
    }
    return dbSerial;
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
    if(noRestart)
    {
        for(p = oldServers.begin(); p != oldServers.end(); ++p)
        {
            map<string, ServerInfo>::const_iterator q = newServers.find(p->first);
            if(q == newServers.end())
            {
                servers.push_back(p->first);
                reasons.push_back("server `" + p->first + "' needs to be removed");
            }
        }
    }

    vector<CheckUpdateResultPtr> results;
    set<string> unreachableNodes;
    for(p = newServers.begin(); p != newServers.end(); ++p)
    {
        map<string, ServerInfo>::const_iterator q = oldServers.find(p->first);
        if(q != oldServers.end() && isServerUpdated(p->second, q->second))
        {
            if(noRestart && isServerUpdated(p->second, q->second, true)) // Ignore properties
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
                    CheckUpdateResultPtr result = _serverCache.get(p->first)->checkUpdate(p->second, noRestart);
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

    for(vector<CheckUpdateResultPtr>::const_iterator q = results.begin(); q != results.end(); ++q)
    {
        try
        {
            (*q)->getResult();
        }
        catch(const NodeUnreachableException& ex)
        {
            unreachableNodes.insert(ex.name);
        }
        catch(const DeploymentException& ex)
        {
            servers.push_back((*q)->getServer());
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
#if defined(__SUNPRO_CC) && defined(_RWSTD_NO_MEMBER_TEMPLATES)
                    Ice::StringSeq nodes;
                    for(set<string>::const_iterator p = unreachableNodes.begin(); p != unreachableNodes.end(); ++p)
                    {
                        nodes.push_back(*p);
                    }
#else
                    Ice::StringSeq nodes(unreachableNodes.begin(), unreachableNodes.end());
#endif
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
                    for(vector<string>::const_iterator p = reasons.begin(); p != reasons.end(); ++p)
                    {
                        out << "\n" << *p;
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
#if defined(__SUNPRO_CC) && defined(_RWSTD_NO_MEMBER_TEMPLATES)
                Ice::StringSeq nodes;
                for(set<string>::const_iterator p = unreachableNodes.begin(); p != unreachableNodes.end\
                        (); ++p)
                {
                    nodes.push_back(*p);
                }
#else
                Ice::StringSeq nodes(unreachableNodes.begin(), unreachableNodes.end());
#endif
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
        for(vector<string>::const_iterator p = reasons.begin(); p != reasons.end(); ++p)
        {
            os << "\n" << *p;
        }
        throw DeploymentException(os.str());
    }
}

void
Database::finishApplicationUpdate(const ApplicationUpdateInfo& update,
                                  const ApplicationInfo& oldApp,
                                  const ApplicationHelper& previous,
                                  const ApplicationHelper& helper,
                                  AdminSessionI* /*session*/,
                                  bool noRestart,
                                  Ice::Long dbSerial)
{
    const ApplicationDescriptor& newDesc = helper.getDefinition();
    DatabaseConnectionPtr connection = _connectionPool->newConnection();

    ServerEntrySeq entries;
    int serial = 0;
    try
    {
        if(_master)
        {
            checkUpdate(previous, helper, oldApp.uuid, oldApp.revision, noRestart);
        }

        Lock sync(*this);
        checkForUpdate(previous, helper, connection);
        reload(previous, helper, entries, oldApp.uuid, oldApp.revision + 1, noRestart);

        for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::sync));

        ApplicationInfo info = oldApp;
        info.updateTime = update.updateTime;
        info.updateUser = update.updateUser;
        info.revision = update.revision;
        info.descriptor = newDesc;
        dbSerial = saveApplication(info, connection, dbSerial);

        serial = _applicationObserverTopic->applicationUpdated(dbSerial, update);
    }
    catch(const DeploymentException&)
    {
        finishUpdating(update.descriptor.name);
        throw;
    }

    _applicationObserverTopic->waitForSyncedSubscribers(serial); // Wait for replicas to be updated.

    //
    // Mark the application as updated. All the replicas received the update so it's now safe
    // for the nodes to start servers.
    //
    {
        Lock sync(*this);
        vector<UpdateInfo>::iterator p = find(_updating.begin(), _updating.end(), update.descriptor.name);
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
        catch(const DeploymentException& ex)
        {
            ApplicationUpdateInfo newUpdate;
            {
                Lock sync(*this);
                entries.clear();
                ApplicationHelper previous(_communicator, newDesc);
                ApplicationHelper helper(_communicator, oldApp.descriptor);

                ApplicationInfo info = oldApp;
                info.revision = update.revision + 1;
                dbSerial = saveApplication(info, connection);
                reload(previous, helper, entries, info.uuid, info.revision, noRestart);

                newUpdate.updateTime = IceUtil::Time::now().toMilliSeconds();
                newUpdate.updateUser = _lockUserId;
                newUpdate.revision = info.revision;
                newUpdate.descriptor = helper.diff(previous);

                vector<UpdateInfo>::iterator p = find(_updating.begin(), _updating.end(), update.descriptor.name);
                assert(p != _updating.end());
                p->unmarkUpdated();

                for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::sync));

                serial = _applicationObserverTopic->applicationUpdated(dbSerial, newUpdate);
            }
            _applicationObserverTopic->waitForSyncedSubscribers(serial); // Wait for subscriber to be updated.
            for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::waitForSyncNoThrow));
            finishUpdating(newDesc.name);
            throw ex;
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
Database::waitForUpdate(const string& name)
{
    while(find(_updating.begin(), _updating.end(), name) != _updating.end())
    {
        wait();
    }
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
    Lock sync(*this);

    vector<UpdateInfo>::iterator p = find(_updating.begin(), _updating.end(), name);
    assert(p != _updating.end());
    p->markUpdated();
    _updating.erase(p);
    notifyAll();
}
