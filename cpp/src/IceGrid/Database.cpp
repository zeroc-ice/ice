// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/StringUtil.h>
#include <IceUtil/Random.h>
#include <Freeze/Freeze.h>
#include <IceGrid/Database.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/Util.h>
#include <IceGrid/DescriptorHelper.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/ReplicaSessionI.h>
#include <IceGrid/Session.h>
#include <IceGrid/Topics.h>

#include <algorithm>
#include <functional>
#include <iterator>

using namespace std;
using namespace IceGrid;

const string Database::_applicationDbName = "applications";
const string Database::_adapterDbName = "adapters";
const string Database::_objectDbName = "objects";
const string Database::_internalObjectDbName = "internal-objects";

namespace IceGrid
{

struct ObjectLoadCI : binary_function<pair<Ice::ObjectPrx, float>&, pair<Ice::ObjectPrx, float>&, bool>
{
    bool operator()(const pair<Ice::ObjectPrx, float>& lhs, const pair<Ice::ObjectPrx, float>& rhs)
    {
        return lhs.second < rhs.second;
    }
};

}

Database::Database(const Ice::ObjectAdapterPtr& registryAdapter,
                   const IceStorm::TopicManagerPrx& topicManager,
                   const string& instanceName,
                   const TraceLevelsPtr& traceLevels,
                   const RegistryInfo& info) :
    _communicator(registryAdapter->getCommunicator()),
    _internalAdapter(registryAdapter),
    _topicManager(topicManager),
    _envName("Registry"),
    _instanceName(instanceName),
    _traceLevels(traceLevels),  
    _master(info.name == "Master"),
    _replicaCache(_communicator, topicManager),
    _nodeCache(_communicator, _replicaCache, info.name),
    _adapterCache(_communicator),
    _objectCache(_communicator),
    _allocatableObjectCache(_communicator),
    _serverCache(_communicator, _nodeCache, _adapterCache, _objectCache, _allocatableObjectCache),
    _connection(Freeze::createConnection(registryAdapter->getCommunicator(), _envName)),
    _applications(_connection, _applicationDbName),
    _adapters(_connection, _adapterDbName),
    _objects(_connection, _objectDbName),
    _internalObjects(_connection, _internalObjectDbName),
    _lock(0), 
    _applicationSerial(0)
{
    ServerEntrySeq entries;
    for(StringApplicationInfoDict::iterator p = _applications.begin(); p != _applications.end(); ++p)
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
    _applicationObserverTopic = new ApplicationObserverTopic(_topicManager, _applications);
    _adapterObserverTopic = new AdapterObserverTopic(_topicManager, _adapters);
    _objectObserverTopic = new ObjectObserverTopic(_topicManager, _objects);

    _registryObserverTopic->registryUp(info);
}

Database::~Database()
{
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
    
    return _applicationSerial;
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
Database::syncApplications(const ApplicationInfoSeq& applications)
{
    int serial;
    {
        Lock sync(*this);
    
        Freeze::TransactionHolder txHolder(_connection);
        ServerEntrySeq entries;
        set<string> names;
        for(ApplicationInfoSeq::const_iterator p = applications.begin(); p != applications.end(); ++p)
        {
            try
            {
                StringApplicationInfoDict::const_iterator s = _applications.find(p->descriptor.name);
                if(s != _applications.end())
                {
                    ApplicationHelper previous(_communicator, s->second.descriptor);
                    ApplicationHelper helper(_communicator, p->descriptor);
                    reload(previous, helper, entries, p->uuid, p->revision);
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
            _applications.put(StringApplicationInfoDict::value_type(p->descriptor.name, *p));
            names.insert(p->descriptor.name);
        }

        StringApplicationInfoDict::iterator s = _applications.begin();
        while(s != _applications.end())
        {
            if(names.find(s->first) == names.end())
            {
                unload(ApplicationHelper(_communicator, s->second.descriptor), entries);
                _applications.erase(s++);
            }
            else
            {
                ++s;
            }
        }
        ++_applicationSerial;
    
        serial = _applicationObserverTopic->applicationInit(_applicationSerial, applications);

        txHolder.commit();
    }
    _applicationObserverTopic->waitForSyncedSubscribers(serial);
}

void
Database::syncAdapters(const AdapterInfoSeq& adapters)
{
    int serial;
    {
        Lock sync(*this);
        Freeze::TransactionHolder txHolder(_connection);
        _adapters.clear();
        for(AdapterInfoSeq::const_iterator r = adapters.begin(); r != adapters.end(); ++r)
        {
            _adapters.put(StringAdapterInfoDict::value_type(r->id, *r));
        }
        serial = _adapterObserverTopic->adapterInit(adapters);
        txHolder.commit();
    }
    _adapterObserverTopic->waitForSyncedSubscribers(serial);
}

void
Database::syncObjects(const ObjectInfoSeq& objects)
{
    int serial;
    {
        Lock sync(*this);
        Freeze::TransactionHolder txHolder(_connection);
        _objects.clear();
        for(ObjectInfoSeq::const_iterator q = objects.begin(); q != objects.end(); ++q)
        {
            _objects.put(IdentityObjectInfoDict::value_type(q->proxy->ice_getIdentity(), *q));
        }
        serial = _objectObserverTopic->objectInit(objects);
        txHolder.commit();
    }
    _objectObserverTopic->waitForSyncedSubscribers(serial);
}

void
Database::addApplication(const ApplicationInfo& info, AdminSessionI* session)
{
    ServerEntrySeq entries;
    {
        Lock sync(*this);
        checkSessionLock(session);

        while(_updating.find(info.descriptor.name) != _updating.end())
        {
            wait();
        }

        if(_applications.find(info.descriptor.name) != _applications.end())
        {
            throw DeploymentException("application `" + info.descriptor.name + "' already exists");
        }       

        ApplicationHelper helper(_communicator, info.descriptor, true);
        checkForAddition(helper);
        load(helper, entries, info.uuid, info.revision);
        startUpdating(info.descriptor.name);
    }

    if(_master)
    {
        try
        {
            for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::syncAndWait));
        }
        catch(const DeploymentException& ex)
        {
            try
            {
                Lock sync(*this);
                entries.clear();
                unload(ApplicationHelper(_communicator, info.descriptor), entries);
            }
            catch(const DeploymentException& ex)
            {
                Ice::Error err(_traceLevels->logger);
                err << "failed to rollback previous application `" << info.descriptor.name << "':\n" << ex.reason;
            }
            finishUpdating(info.descriptor.name);
            throw ex;
        }
    }

    int serial;
    {
        Lock sync(*this);
        ++_applicationSerial;   
        _applications.put(StringApplicationInfoDict::value_type(info.descriptor.name, info));

        serial = _applicationObserverTopic->applicationAdded(_applicationSerial, info);
    
        if(_traceLevels->application > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->applicationCat);
            out << "added application `" << info.descriptor.name << "'";
        }
    }

    _applicationObserverTopic->waitForSyncedSubscribers(serial);

    finishUpdating(info.descriptor.name);
}

void
Database::updateApplication(const ApplicationUpdateInfo& updt, AdminSessionI* session)
{
    ServerEntrySeq entries;
    ApplicationInfo oldApp;
    ApplicationDescriptor newDesc;
    ApplicationUpdateInfo update = updt;
    {
        Lock sync(*this);       
        checkSessionLock(session);

        while(_updating.find(update.descriptor.name) != _updating.end())
        {
            wait();
        }

        StringApplicationInfoDict::const_iterator p = _applications.find(update.descriptor.name);
        if(p == _applications.end())
        {
            throw ApplicationNotExistException(update.descriptor.name);
        }
        oldApp = p->second;

        if(update.revision < 0)
        {
            update.revision = oldApp.revision + 1;
        }

        ApplicationHelper previous(_communicator, oldApp.descriptor);
        ApplicationHelper helper(_communicator, previous.update(update.descriptor), true);

        checkForUpdate(previous, helper);
        reload(previous, helper, entries, oldApp.uuid, oldApp.revision + 1);

        newDesc = helper.getDefinition();

        startUpdating(update.descriptor.name);
    }

    finishApplicationUpdate(entries, update, oldApp, newDesc, session);
}

void
Database::syncApplicationDescriptor(const ApplicationDescriptor& newDesc, AdminSessionI* session)
{
    ServerEntrySeq entries;
    ApplicationUpdateInfo update;
    ApplicationInfo oldApp;
    {
        Lock sync(*this);
        checkSessionLock(session);

        while(_updating.find(update.descriptor.name) != _updating.end())
        {
            wait();
        }

        StringApplicationInfoDict::const_iterator p = _applications.find(newDesc.name);
        if(p == _applications.end())
        {
            throw ApplicationNotExistException(newDesc.name);
        }
        oldApp = p->second;

        ApplicationHelper previous(_communicator, oldApp.descriptor);
        ApplicationHelper helper(_communicator, newDesc, true);

        update.updateTime = IceUtil::Time::now().toMilliSeconds();
        update.updateUser = _lockUserId;
        update.revision = oldApp.revision + 1;
        update.descriptor = helper.diff(previous);
        
        checkForUpdate(previous, helper);       
        reload(previous, helper, entries, oldApp.uuid, oldApp.revision + 1);

        startUpdating(update.descriptor.name);
    }

    finishApplicationUpdate(entries, update, oldApp, newDesc, session);
}

void
Database::instantiateServer(const string& application, 
                            const string& node, 
                            const ServerInstanceDescriptor& instance,
                            AdminSessionI* session)
{
    ServerEntrySeq entries;
    ApplicationUpdateInfo update;
    ApplicationInfo oldApp;
    ApplicationDescriptor newDesc;
    {
        Lock sync(*this);       
        checkSessionLock(session);

        while(_updating.find(application) != _updating.end())
        {
            wait();
        }

        StringApplicationInfoDict::const_iterator p = _applications.find(application);
        if(p == _applications.end())
        {
            throw ApplicationNotExistException(application);
        }
        oldApp = p->second;

        ApplicationHelper previous(_communicator, oldApp.descriptor);
        ApplicationHelper helper(_communicator, previous.instantiateServer(node, instance), true);

        update.updateTime = IceUtil::Time::now().toMilliSeconds();
        update.updateUser = _lockUserId;
        update.revision = oldApp.revision + 1;
        update.descriptor = helper.diff(previous);

        checkForUpdate(previous, helper);       
        reload(previous, helper, entries, oldApp.uuid, oldApp.revision + 1);

        newDesc = helper.getDefinition();

        startUpdating(update.descriptor.name);
    }

    finishApplicationUpdate(entries, update, oldApp, newDesc, session);
}

void
Database::removeApplication(const string& name, AdminSessionI* session)
{
    ServerEntrySeq entries;
    int serial;
    {
        Lock sync(*this);
        checkSessionLock(session);

        while(_updating.find(name) != _updating.end())
        {
            wait();
        }

        StringApplicationInfoDict::iterator p = _applications.find(name);
        if(p == _applications.end())
        {
            throw ApplicationNotExistException(name);
        }

        try
        {
            ApplicationHelper helper(_communicator, p->second.descriptor);
            unload(helper, entries);
        }
        catch(const DeploymentException&)
        {
            //
            // For some reasons the application became invalid. If
            // it's invalid, it's most likely not loaded either. So we
            // ignore the error and erase the descriptor.
            //
        }
        
        startUpdating(name);
    }

    if(_master)
    {
        for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::sync));
        for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::waitNoThrow));
    }

    {
        Lock sync(*this);
        _applications.erase(name);
        ++_applicationSerial;

        serial = _applicationObserverTopic->applicationRemoved(_applicationSerial, name);

        if(_traceLevels->application > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->applicationCat);
            out << "removed application `" << name << "'";
        }
    }

    _applicationObserverTopic->waitForSyncedSubscribers(serial);

    finishUpdating(name);
}

ApplicationInfo
Database::getApplicationInfo(const std::string& name)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringApplicationInfoDict descriptors(connection, _applicationDbName); 
    
    StringApplicationInfoDict::const_iterator p = descriptors.find(name);
    if(p == descriptors.end())
    {
        throw ApplicationNotExistException(name);
    }

    return p->second;
}

Ice::StringSeq
Database::getAllApplications(const string& expression)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringApplicationInfoDict descriptors(connection, _applicationDbName);
    return getMatchingKeys<StringApplicationInfoDict>(descriptors, expression);
}

void
Database::waitForApplicationUpdate(const AMD_NodeSession_waitForApplicationUpdatePtr& cb,
                                   const string& application, 
                                   int revision)
{
    Lock sync(*this);
    map<string, vector<AMD_NodeSession_waitForApplicationUpdatePtr> >::iterator p = _updating.find(application);
    if(p != _updating.end())
    {
        p->second.push_back(cb);
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
Database::setAdapterDirectProxy(const string& adapterId, const string& replicaGroupId, const Ice::ObjectPrx& proxy)
{
    int serial;
    {
        Lock sync(*this);
        if(_adapterCache.has(adapterId))
        {
            throw AdapterExistsException(adapterId);
        }

        StringAdapterInfoDict::iterator p = _adapters.find(adapterId);
        AdapterInfo info;
        bool updated = false;
        if(proxy)
        {
            if(p != _adapters.end())
            {
                info = p->second;
                info.proxy = proxy;
                info.replicaGroupId = replicaGroupId;
                p.set(info);
                updated = true;
            }
            else
            {
                info.id = adapterId;
                info.proxy = proxy;
                info.replicaGroupId = replicaGroupId;
                _adapters.put(StringAdapterInfoDict::value_type(adapterId, info));
            }   
        }
        else
        {
            if(p == _adapters.end())
            {
                return;
            }
            _adapters.erase(p);
        }

        if(_traceLevels->adapter > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
            out << (proxy ? (updated ? "updated" : "added") : "removed") << " adapter `" << adapterId << "'";
            if(!replicaGroupId.empty())
            {
                out << " with replica group `" << replicaGroupId << "'";
            }
        }
    
        if(proxy)
        {
            if(updated)
            {
                serial = _adapterObserverTopic->adapterUpdated(info);
            }
            else
            {
                serial = _adapterObserverTopic->adapterAdded(info);
            }
        }
        else
        {
            serial = _adapterObserverTopic->adapterRemoved(adapterId);
        }
    }
    _adapterObserverTopic->waitForSyncedSubscribers(serial);
}

Ice::ObjectPrx
Database::getAdapterDirectProxy(const string& id)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringAdapterInfoDict adapters(connection, _adapterDbName); 
    StringAdapterInfoDict::const_iterator p = adapters.find(id);
    if(p != adapters.end())
    {
        return p->second.proxy;
    }

    Ice::EndpointSeq endpoints;
    for(p = adapters.findByReplicaGroupId(id, true); p != adapters.end(); ++p)
    {
        Ice::EndpointSeq edpts = p->second.proxy->ice_getEndpoints();
        endpoints.insert(endpoints.end(), edpts.begin(), edpts.end());
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
        
        Freeze::TransactionHolder txHolder(_connection); // Required because of the iterator
        
        StringAdapterInfoDict::iterator p = _adapters.find(adapterId);
        AdapterInfoSeq infos;
        if(p != _adapters.end())
        {
            _adapters.erase(p);
        }
        else
        {
            p = _adapters.findByReplicaGroupId(adapterId, true);
            if(p == _adapters.end())
            {
                throw AdapterNotExistException(adapterId);
            }
            
            while(p != _adapters.end())
            {
                AdapterInfo info = p->second;
                info.replicaGroupId = "";
                infos.push_back(info);
                _adapters.put(StringAdapterInfoDict::value_type(p->first, info));
                ++p;
            }
        }
        
        if(_traceLevels->adapter > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
            out << "removed " << (infos.empty() ? "adapter" : "replica group") << " `" << adapterId << "'";
        }
        
        if(infos.empty())
        {
            serial = _adapterObserverTopic->adapterRemoved(adapterId);
        }
        else
        {
            for(AdapterInfoSeq::const_iterator p = infos.begin(); p != infos.end(); ++p)
            {
                serial = _adapterObserverTopic->adapterUpdated(*p);
            }
        }
        
        txHolder.commit();
    }
    _adapterObserverTopic->waitForSyncedSubscribers(serial);
}

AdapterEntryPtr
Database::getAdapter(const string& id) const
{
    return _adapterCache.get(id);
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
        return _adapterCache.get(id)->getAdapterInfo();
    }
    catch(AdapterNotExistException&)
    {
    }

    //
    // Otherwise, we check the adapter endpoint table -- if there's an
    // entry the adapter is managed by the registry itself.
    //
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringAdapterInfoDict adapters(connection, _adapterDbName); 
    StringAdapterInfoDict::const_iterator p = adapters.find(id);
    if(p != adapters.end())
    {
        AdapterInfoSeq infos;
        infos.push_back(p->second);
        return infos;
    }

    //
    // If it's not a regular object adapter, perhaps it's a replica
    // group...
    //
    p = adapters.findByReplicaGroupId(id, true);
    if(p != adapters.end())
    {
        AdapterInfoSeq infos;
        while(p != adapters.end())
        {
            infos.push_back(p->second);
            ++p;
        }
        return infos;
    }

    throw AdapterNotExistException(id);
}


Ice::StringSeq
Database::getAllAdapters(const string& expression)
{
    Lock sync(*this);
    vector<string> result;
    vector<string> ids = _adapterCache.getAll(expression);
    result.swap(ids);
    set<string> groups;
    for(StringAdapterInfoDict::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
    {
        if(expression.empty() || IceUtil::match(p->first, expression, true))
        {
            result.push_back(p->first);
        }
        string replicaGroupId = p->second.replicaGroupId;
        if(!replicaGroupId.empty() && (expression.empty() || IceUtil::match(replicaGroupId, expression, true)))
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
    int serial;
    {
        Lock sync(*this);       
        const Ice::Identity id = info.proxy->ice_getIdentity();
        
        if(_objectCache.has(id))
        {
            throw ObjectExistsException(id);
        }
        
        if(_objects.find(id) != _objects.end())
        {
            throw ObjectExistsException(id);
        }
        _objects.put(IdentityObjectInfoDict::value_type(id, info));
        
        serial = _objectObserverTopic->objectAdded(info);

        if(_traceLevels->object > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
            out << "added object `" << _communicator->identityToString(id) << "'";
        }
    }
    _objectObserverTopic->waitForSyncedSubscribers(serial);
}

void
Database::addOrUpdateObject(const ObjectInfo& info)
{
    int serial;
    {
        Lock sync(*this);       
        const Ice::Identity id = info.proxy->ice_getIdentity();
        
        if(_objectCache.has(id))
        {
            throw ObjectExistsException(id);
        }
        
        bool update = _objects.find(id) != _objects.end();
        _objects.put(IdentityObjectInfoDict::value_type(id, info));
        
        if(update)
        {
            serial = _objectObserverTopic->objectUpdated(info);
        }
        else
        {
            serial = _objectObserverTopic->objectAdded(info);
        }
        
        if(_traceLevels->object > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
            out << (!update ? "added" : "updated") << " object `" << _communicator->identityToString(id) << "'";
        }
    }
    _objectObserverTopic->waitForSyncedSubscribers(serial);
}

void
Database::removeObject(const Ice::Identity& id)
{
    int serial;
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
        
        IdentityObjectInfoDict::iterator p = _objects.find(id);
        if(p == _objects.end())
        {
            ObjectNotRegisteredException ex;
            ex.id = id;
            throw ex;
        }
        _objects.erase(p);
        
        serial = _objectObserverTopic->objectRemoved(id);
        
        if(_traceLevels->object > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
            out << "removed object `" << _communicator->identityToString(id) << "'";
        }
    }
    _objectObserverTopic->waitForSyncedSubscribers(serial);
}

void
Database::updateObject(const Ice::ObjectPrx& proxy)
{
    int serial;
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
    
        IdentityObjectInfoDict::iterator p = _objects.find(id);
        if(p == _objects.end())
        {
            ObjectNotRegisteredException ex;
            ex.id = id;
            throw ex;
        }
        
        ObjectInfo info;
        info = p->second;
        info.proxy = proxy;
        p.set(info);
    
        serial = _objectObserverTopic->objectUpdated(info);
        
        if(_traceLevels->object > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
            out << "updated object `" << _communicator->identityToString(id) << "'";
        }
    }
    _objectObserverTopic->waitForSyncedSubscribers(serial);
}

int
Database::addOrUpdateObjectsInDatabase(const ObjectInfoSeq& objects)
{
    Lock sync(*this);
    Freeze::TransactionHolder txHolder(_connection);
    for(ObjectInfoSeq::const_iterator p = objects.begin(); p != objects.end(); ++p)
    {
        _objects.put(IdentityObjectInfoDict::value_type(p->proxy->ice_getIdentity(), *p));
    }
    int serial = _objectObserverTopic->objectsAddedOrUpdated(objects);
    txHolder.commit();
    return serial;
}

void
Database::removeObjectsInDatabase(const ObjectInfoSeq& objects)
{
    Lock sync(*this);
    Freeze::TransactionHolder txHolder(_connection);
    for(ObjectInfoSeq::const_iterator p = objects.begin(); p != objects.end(); ++p)
    {
        _objects.erase(p->proxy->ice_getIdentity());
    }
    _objectObserverTopic->objectsRemoved(objects);
    txHolder.commit();
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

    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectInfoDict objects(connection, _objectDbName);
    IdentityObjectInfoDict::const_iterator p = objects.find(id);
    if(p == objects.end())
    {
        ObjectNotRegisteredException ex;
        ex.id = id;
        throw ex;
    }
    return p->second.proxy;
}

Ice::ObjectPrx
Database::getObjectByType(const string& type)
{
    Ice::ObjectProxySeq objs = getObjectsByType(type);
    if(objs.empty())
    {
        return 0;
    }
    return objs[IceUtil::random(static_cast<int>(objs.size()))];
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
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectInfoDict objects(connection, _objectDbName);    
    for(IdentityObjectInfoDict::const_iterator p = objects.findByType(type); p != objects.end(); ++p)
    {
        proxies.push_back(p->second.proxy);
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

    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectInfoDict objects(connection, _objectDbName);
    IdentityObjectInfoDict::const_iterator p = objects.find(id);
    if(p == objects.end())
    {
        throw ObjectNotRegisteredException(id);
    }
    return p->second;
}

ObjectInfoSeq
Database::getAllObjectInfos(const string& expression)
{
    ObjectInfoSeq infos = _objectCache.getAll(expression);
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectInfoDict objects(connection, _objectDbName); 
    for(IdentityObjectInfoDict::const_iterator p = objects.begin(); p != objects.end(); ++p)
    {
        if(expression.empty() || IceUtil::match(_communicator->identityToString(p->first), expression, true))
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
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectInfoDict objects(connection, _objectDbName);    
    for(IdentityObjectInfoDict::const_iterator p = objects.findByType(type); p != objects.end(); ++p)
    {
        infos.push_back(p->second);
    }
    return infos;
}

void
Database::addInternalObject(const ObjectInfo& info, bool replace)
{
    Lock sync(*this);   
    const Ice::Identity id = info.proxy->ice_getIdentity();
    if(!replace && _internalObjects.find(id) != _internalObjects.end())
    {
        throw ObjectExistsException(id);
    }
    _internalObjects.put(IdentityObjectInfoDict::value_type(id, info));
}

void
Database::removeInternalObject(const Ice::Identity& id)
{
    Lock sync(*this);
    IdentityObjectInfoDict::iterator p = _internalObjects.find(id);
    if(p == _internalObjects.end())
    {
        ObjectNotRegisteredException ex;
        ex.id = id;
        throw ex;
    }
    _internalObjects.erase(p);
}

Ice::ObjectProxySeq
Database::getInternalObjectsByType(const string& type)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectInfoDict internalObjects(connection, _internalObjectDbName);    
    Ice::ObjectProxySeq proxies;
    for(IdentityObjectInfoDict::const_iterator p = internalObjects.findByType(type); p != internalObjects.end(); ++p)
    {
        proxies.push_back(p->second.proxy);
    }
    return proxies;
}

void
Database::checkForAddition(const ApplicationHelper& app)
{
    set<string> serverIds;
    set<string> adapterIds;
    set<Ice::Identity> objectIds;

    app.getIds(serverIds, adapterIds, objectIds);

    for_each(serverIds.begin(), serverIds.end(), objFunc(*this, &Database::checkServerForAddition));
    for_each(adapterIds.begin(), adapterIds.end(), objFunc(*this, &Database::checkAdapterForAddition));
    for_each(objectIds.begin(), objectIds.end(), objFunc(*this, &Database::checkObjectForAddition)); 
}
     
void
Database::checkForUpdate(const ApplicationHelper& origApp, const ApplicationHelper& newApp)
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
    set_difference(newAdpts.begin(), newAdpts.end(), oldAdpts.begin(), oldAdpts.end(), set_inserter(addedAdpts));
    for_each(addedAdpts.begin(), addedAdpts.end(), objFunc(*this, &Database::checkAdapterForAddition));

    vector<Ice::Identity> addedObjs;
    set_difference(newObjs.begin(), newObjs.end(), oldObjs.begin(), oldObjs.end(), set_inserter(addedObjs));
    for_each(addedObjs.begin(), addedObjs.end(), objFunc(*this, &Database::checkObjectForAddition));
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
Database::checkAdapterForAddition(const string& id)
{
    if(_adapterCache.has(id) ||
       _adapters.find(id) != _adapters.end() || 
       _adapters.findByReplicaGroupId(id) != _adapters.end())
    {
        DeploymentException ex;
        ex.reason = "adapter `" + id + "' is already registered"; 
        throw ex;
    }
}

void
Database::checkObjectForAddition(const Ice::Identity& objectId)
{
    if(_objectCache.has(objectId) ||
       _allocatableObjectCache.has(objectId) ||
       _objects.find(objectId) != _objects.end())
    {
        DeploymentException ex;
        ex.reason = "object `" + _communicator->identityToString(objectId) + "' is already registered"; 
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
            ObjectInfo info;
            info.type = o->type;
            info.proxy = _communicator->stringToProxy("\"" + _communicator->identityToString(o->id) + "\" @ " + r->id);
            _objectCache.add(info, application);
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
                 int revision)
{
    const string application = oldApp.getInstance().name;

    //
    // Remove destroyed servers.
    //
    map<string, ServerInfo> oldServers = oldApp.getServerInfos(uuid, revision);
    map<string, ServerInfo> newServers = newApp.getServerInfos(uuid, revision);
    vector<ServerInfo> load;
    map<string, ServerInfo>::const_iterator p;
    for(p = newServers.begin(); p != newServers.end(); ++p)
    {
        map<string, ServerInfo>::const_iterator q = oldServers.find(p->first);
        if(q == oldServers.end())
        {
            load.push_back(p->second);
        } 
        else
        {
            _serverCache.remove(p->first, false); // Don't destroy the server if it was updated.
            load.push_back(p->second);
        }
    }
    for(p = oldServers.begin(); p != oldServers.end(); ++p)
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
    ReplicaGroupDescriptorSeq::const_iterator r;
    for(r = oldAdpts.begin(); r != oldAdpts.end(); ++r)
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
    NodeDescriptorDict::const_iterator n;
    for(n = oldNodes.begin(); n != oldNodes.end(); ++n)
    {
        _nodeCache.get(n->first)->removeDescriptor(application);
    }

    //
    // Add back node descriptors.
    //
    const NodeDescriptorDict& newNodes = newApp.getInstance().nodes;
    for(n = newNodes.begin(); n != newNodes.end(); ++n)
    {
        _nodeCache.get(n->first, true)->addDescriptor(application, n->second);
    }

    //
    // Add back replica groups.
    //
    for(r = newAdpts.begin(); r != newAdpts.end(); ++r)
    {
        try
        {
            ReplicaGroupEntryPtr entry = ReplicaGroupEntryPtr::dynamicCast(_adapterCache.get(r->id));
            assert(entry);
            entry->update(r->loadBalancing);
        }
        catch(const AdapterNotExistException&)
        {
            _adapterCache.addReplicaGroup(*r, application);
        }

        for(ObjectDescriptorSeq::const_iterator o = r->objects.begin(); o != r->objects.end(); ++o)
        {
            ObjectInfo info;
            info.type = o->type;
            info.proxy = _communicator->stringToProxy(_communicator->identityToString(o->id) + "@" + r->id);
            _objectCache.add(info, application);
        }
    }

    //
    // Add back servers.
    //
    for(vector<ServerInfo>::const_iterator q = load.begin(); q != load.end(); ++q)
    {
        entries.push_back(_serverCache.add(*q));
    }
}

void
Database::finishApplicationUpdate(ServerEntrySeq& entries, 
                                  const ApplicationUpdateInfo& update,
                                  const ApplicationInfo& oldApp, 
                                  const ApplicationDescriptor& newDesc,
                                  AdminSessionI* session)
{
    if(_master)
    {
        //
        // Load the servers on the nodes. If a server couldn't be
        // deployed we unload the application and throw.
        //
        try
        {
            for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::syncAndWait));
        }
        catch(const DeploymentException& ex)
        {
            ApplicationUpdateInfo newUpdate;
            {
                Lock sync(*this);
                entries.clear();
                ApplicationHelper previous(_communicator, newDesc);
                ApplicationHelper helper(_communicator, oldApp.descriptor);
                reload(previous, helper, entries, oldApp.uuid, oldApp.revision);
            }

            try
            {
                for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::syncAndWait));
            }
            catch(const DeploymentException& ex)
            {
                Ice::Error err(_traceLevels->logger);
                err << "failed to rollback previous application `" << oldApp.descriptor.name << "':\n" << ex.reason;
            }

            finishUpdating(newDesc.name);
            throw ex;
        }
    }

    //
    // Save the application descriptor.
    //
    int serial;
    {
        Lock sync(*this);
        
        ApplicationInfo info = oldApp;
        info.updateTime = update.updateTime;
        info.updateUser = update.updateUser;
        info.revision = update.revision;
        info.descriptor = newDesc;
        
        _applications.put(StringApplicationInfoDict::value_type(update.descriptor.name, info));
        ++_applicationSerial;
    
        if(_traceLevels->application > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->applicationCat);
            out << "updated application `" << update.descriptor.name << "'";
        }
        
        serial = _applicationObserverTopic->applicationUpdated(_applicationSerial, update);
    }

    _applicationObserverTopic->waitForSyncedSubscribers(serial);

    finishUpdating(update.descriptor.name);
}

void
Database::startUpdating(const string& name)
{
    // Must be called within the synchronization.
    _updating.insert(make_pair(name, vector<AMD_NodeSession_waitForApplicationUpdatePtr>()));
}

void
Database::finishUpdating(const string& name)
{
    Lock sync(*this);
    
    map<string, vector<AMD_NodeSession_waitForApplicationUpdatePtr> >::iterator p = _updating.find(name);
    assert(p != _updating.end());
    for(vector<AMD_NodeSession_waitForApplicationUpdatePtr>::const_iterator q = p->second.begin(); 
        q != p->second.end(); ++q)
    {
        (*q)->ice_response();
    }
    _updating.erase(p);

    notifyAll();
}
