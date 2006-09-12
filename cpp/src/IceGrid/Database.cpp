// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

namespace IceGrid
{

//
// A default servant for adapter objects registered directly in the
// registry database.
//
class AdapterI : public Adapter
{
public:

    AdapterI(const DatabasePtr& database) : _database(database)
    {
    }

    virtual void
    activate_async(const AMD_Adapter_activatePtr& cb, const Ice::Current& current)
    {
	assert(false);
    }

    virtual Ice::ObjectPrx
    getDirectProxy(const Ice::Current& current) const
    {
	Ice::ObjectPrx proxy = _database->getAdapterDirectProxy(current.id.name);
	if(!proxy)
	{
	    throw Ice::ObjectNotExistException(__FILE__, __LINE__);
	}
	return proxy;
    }

    virtual void 
    setDirectProxy(const ::Ice::ObjectPrx& proxy, const ::Ice::Current& current)
    {
	assert(false);
    }

private:

    const DatabasePtr _database;
};

//
// A servant locator for the default servant above.
//
class AdapterServantLocator : public Ice::ServantLocator
{
public:

    AdapterServantLocator(const DatabasePtr& database) : _adapter(new AdapterI(database))
    {
    }

    virtual Ice::ObjectPtr
    locate(const Ice::Current& current, Ice::LocalObjectPtr& cookie)
    {
	return _adapter;
    }

    virtual void
    finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&)
    {
    }

    virtual void
    deactivate(const std::string&)
    {
    }
    
private:

    const AdapterPtr _adapter;
};

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
		   int sessionTimeout,
		   const TraceLevelsPtr& traceLevels,
		   bool master) :
    _communicator(registryAdapter->getCommunicator()),
    _internalAdapter(registryAdapter),
    _topicManager(topicManager),
    _envName("Registry"),
    _instanceName(instanceName),
    _traceLevels(traceLevels),  
    _sessionTimeout(sessionTimeout),
    _master(master),
    _replicaCache(_communicator, topicManager),
    _nodeCache(_communicator, _replicaCache, _master),
    _objectCache(_communicator),
    _allocatableObjectCache(_communicator),
    _serverCache(_communicator, _nodeCache, _adapterCache, _objectCache, _allocatableObjectCache, sessionTimeout),
    _connection(Freeze::createConnection(registryAdapter->getCommunicator(), _envName)),
    _applications(_connection, _applicationDbName),
    _objects(_connection, _objectDbName),
    _adapters(_connection, _adapterDbName),
    _lock(0), 
    _applicationSerial(0),
    _adapterSerial(0),
    _objectSerial(0)
{
    ServerEntrySeq entries;
    for(StringApplicationInfoDict::const_iterator p = _applications.begin(); p != _applications.end(); ++p)
    {
	try
	{
	    load(ApplicationHelper(_communicator, p->second.descriptor), entries, p->second.uuid, p->second.revision);
	}
	catch(const DeploymentException& ex)
	{
	    Ice::Warning warn(_traceLevels->logger);
	    warn << "invalid application `" << p->first << "':\n" << ex.reason;
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

    //
    // Register a default servant to manage manually registered object adapters.
    //
    __setNoDelete(true);
    try
    {
	_internalAdapter->addServantLocator(new AdapterServantLocator(this), "IceGridAdapter");
    }
    catch(...)
    {
	__setNoDelete(false);
	throw;
    }
    __setNoDelete(false);
}

Database::~Database()
{
}

void
Database::destroy()
{
    _nodeCache.destroy(); // Break cyclic reference count.
    _replicaCache.destroy();
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
	assert(false);
	break;
    }
    return 0; // Keep the compiler happy.
}

void
Database::checkSessionLock(AdminSessionI* session)
{
    if(_lock == 0 && session)
    {
	throw AccessDeniedException(); // Sessions must first acquire the lock!
    }
    else if(_lock != 0 && session != _lock)
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
	serial = ++_applicationSerial;
    }

    _applicationObserverTopic->applicationInit(serial, applications);
}

void
Database::syncAdapters(const AdapterInfoSeq& adapters)
{
    int serial;
    {
	Lock sync(*this);
	_adapters.clear();
	for(AdapterInfoSeq::const_iterator r = adapters.end(); r != adapters.end(); ++r)
	{
	    _adapters.put(StringAdapterInfoDict::value_type(r->id, *r));
	}
	serial = ++_adapterSerial;
    }
    

    _adapterObserverTopic->adapterInit(serial, adapters);
}

void
Database::syncObjects(const ObjectInfoSeq& objects)
{
    int serial;
    {
	Lock sync(*this);
	_objects.clear();
	for(ObjectInfoSeq::const_iterator q = objects.begin(); q != objects.end(); ++q)
	{
	    _objects.put(IdentityObjectInfoDict::value_type(q->proxy->ice_getIdentity(), *q));
	}
	serial = ++_objectSerial;
    }
    _objectObserverTopic->objectInit(serial, objects);
}

Ice::ObjectPrx
Database::getReplicatedEndpoints(const string& name, const Ice::ObjectPrx& proxy)
{
    return _replicaCache.getEndpoints(name, proxy);
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

	ApplicationHelper helper(_communicator, info.descriptor);
	checkForAddition(helper);
	load(helper, entries, info.uuid, info.revision);
	_updating.insert(info.descriptor.name);

	_replicaCache.startApplicationReplication(info.uuid, info.revision);
    }

    if(_master)
    {
	try
	{
	    for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::sync));
	}
	catch(const DeploymentException& ex)
	{
	    try
	    {
		Lock sync(*this);
		entries.clear();
		unload(ApplicationHelper(_communicator, info.descriptor), entries);
		_updating.erase(info.descriptor.name);
		notifyAll();
	    }
	    catch(const DeploymentException& ex)
	    {
		Ice::Error err(_traceLevels->logger);
		err << "failed to rollback previous application `" << info.descriptor.name << "':\n" << ex.reason;
	    }
	    throw ex;
	}
    }

    //
    // Save the application descriptor.
    //
    int serial;
    {
	Lock sync(*this);
	_applications.put(StringApplicationInfoDict::value_type(info.descriptor.name, info));
	serial = ++_applicationSerial;	
	_updating.erase(info.descriptor.name);
	notifyAll();
    }

    _applicationObserverTopic->applicationAdded(serial, info);
    _replicaCache.waitForUpdateReplication("application", serial);
    _replicaCache.finishApplicationReplication(info.uuid, info.revision);

    if(_traceLevels->application > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->applicationCat);
	out << "added application `" << info.descriptor.name << "'";
    }
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
	ApplicationHelper helper(_communicator, previous.update(update.descriptor));

	checkForUpdate(previous, helper);
	reload(previous, helper, entries, oldApp.uuid, oldApp.revision + 1);

	newDesc = helper.getDefinition();

	_updating.insert(update.descriptor.name);
	_replicaCache.startApplicationReplication(oldApp.uuid, update.revision);
    }

    finishUpdate(entries, update, oldApp, newDesc, session);
}

void
Database::syncApplicationDescriptor(const ApplicationDescriptor& newDesc, AdminSessionI* session)
{
    assert(session);

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
	ApplicationHelper helper(_communicator, newDesc);

	update.updateTime = IceUtil::Time::now().toMilliSeconds();
	update.updateUser = _lockUserId;
	update.revision = oldApp.revision + 1;
	update.descriptor = helper.diff(previous);
	
	checkForUpdate(previous, helper);	
	reload(previous, helper, entries, oldApp.uuid, oldApp.revision + 1);

	_updating.insert(update.descriptor.name);
	_replicaCache.startApplicationReplication(oldApp.uuid, update.revision);
    }

    finishUpdate(entries, update, oldApp, newDesc, session);
}

void
Database::instantiateServer(const string& application, 
			    const string& node, 
			    const ServerInstanceDescriptor& instance,
			    AdminSessionI* session)
{
    assert(session);

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
	ApplicationHelper helper(_communicator, previous.instantiateServer(node, instance));

	update.updateTime = IceUtil::Time::now().toMilliSeconds();
	update.updateUser = _lockUserId;
	update.revision = oldApp.revision + 1;
	update.descriptor = helper.diff(previous);

	checkForUpdate(previous, helper);	
	reload(previous, helper, entries, oldApp.uuid, oldApp.revision + 1);

	newDesc = helper.getDefinition();

	_updating.insert(update.descriptor.name);
	_replicaCache.startApplicationReplication(oldApp.uuid, update.revision);
    }

    finishUpdate(entries, update, oldApp, newDesc, session);
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
	
	_applications.erase(p);

	serial = ++_applicationSerial;
    }

    _applicationObserverTopic->applicationRemoved(serial, name);
    _replicaCache.waitForUpdateReplication("application", serial);

    if(_traceLevels->application > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->applicationCat);
	out << "removed application `" << name << "'";
    }

    if(session)
    {
	for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::sync));
    }
    else
    {
	// TODO: XXX: synchronize the servers
    }
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
Database::addNode(const string& name, const NodeSessionIPtr& session)
{
    _nodeCache.get(name, true)->setSession(session);

//     //
//     // Only the master adds the node well-known proxy to its
//     // database. The well-known proxy will be transmitted to the
//     // replicas through the replication of the database.
//     //
//     if(_master)
//     {
	ObjectInfo info;
	info.type = Node::ice_staticId();
	info.proxy = session->getNode();
	addObject(info, true);
//    }
}

NodePrx 
Database::getNode(const string& name) const
{
    return _nodeCache.get(name)->getProxy();
}

NodeInfo
Database::getNodeInfo(const string& name) const
{
    return _nodeCache.get(name)->getInfo();
}

void 
Database::removeNode(const string& name, const NodeSessionIPtr& session, bool shutdown)
{
    //
    // If the registry isn't being shutdown and this registry is the
    // master we remove the node well-known proxy from the object
    // adapter. Replicas will be notified through the replication.
    // 
    if(!shutdown && _master)
    {
	removeObject(session->getNode()->ice_getIdentity());
    }

    //
    // We must notify the observer first (there's an assert in the
    // observer to ensure that only nodes which are up are teared
    // down).
    //
    _nodeObserverTopic->nodeDown(name);

    _nodeCache.get(name)->setSession(0);
}

Ice::StringSeq
Database::getAllNodes(const string& expression)
{
    return _nodeCache.getAll(expression);
}

void
Database::addReplica(const string& name, const ReplicaSessionIPtr& session)
{
    // 
    // NOTE: this must be done before we add the replica to the cache
    // in order for ReplicaCache::waitForUpdateReplication to work.
    //
    _applicationObserverTopic->subscribeAndWaitForSubscription(session->getObserver());
    _adapterObserverTopic->subscribeAndWaitForSubscription(session->getObserver());
    _objectObserverTopic->subscribeAndWaitForSubscription(session->getObserver());

    _replicaCache.add(name, session);
}

InternalRegistryPrx 
Database::getReplica(const string& name) const
{
    return _replicaCache.get(name)->getProxy();
}

RegistryInfo
Database::getReplicaInfo(const string& name) const
{
    return _replicaCache.get(name)->getInfo();
}

void
Database::replicaReceivedUpdate(const string& name, const string& update, int serial, const string& failure)
{
    _replicaCache.replicaReceivedUpdate(name, update, serial, failure);
}

void
Database::waitForApplicationReplication(const string& application, int revision)
{
    _replicaCache.waitForApplicationReplication(application, revision);
}

void
Database::removeReplica(const string& name, const ReplicaSessionIPtr& session)
{
    _replicaCache.remove(name);

    _applicationObserverTopic->unsubscribe(session->getObserver());
    _adapterObserverTopic->unsubscribe(session->getObserver());
    _objectObserverTopic->unsubscribe(session->getObserver());
}

Ice::StringSeq
Database::getAllReplicas(const string& expression)
{
    return _replicaCache.getAll(expression);
}

ServerInfo
Database::getServerInfo(const std::string& id, bool resolve)
{
    return _serverCache.get(id)->getServerInfo(resolve);
}

ServerPrx
Database::getServer(const string& id, bool upToDate)
{
    int activationTimeout, deactivationTimeout;
    string node;
    return getServerWithTimeouts(id, activationTimeout, deactivationTimeout, node, upToDate);
}

ServerPrx
Database::getServerWithTimeouts(const string& id, int& actTimeout, int& deactTimeout, string& node, bool upToDate)
{
    return _serverCache.get(id)->getProxy(actTimeout, deactTimeout, node, upToDate);
}

Ice::StringSeq
Database::getAllServers(const string& expression)
{
    return _serverCache.getAll(expression);
}

Ice::StringSeq
Database::getAllNodeServers(const string& node)
{
    return _nodeCache.get(node)->getServers();
}

bool
Database::setAdapterDirectProxy(const string& adapterId, const string& replicaGroupId, const Ice::ObjectPrx& proxy)
{
    AdapterInfo info;
    int serial;
    bool updated = false;
    {
	Lock sync(*this);
	if(_adapterCache.has(adapterId))
	{
	    return false;
	}

	StringAdapterInfoDict::iterator p = _adapters.find(adapterId);
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
		return true;
	    }
	    _adapters.erase(p);
	}

	serial = ++_adapterSerial;
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
	    _adapterObserverTopic->adapterUpdated(serial, info);
	}
	else
	{
	    _adapterObserverTopic->adapterAdded(serial, info);
	}
    }
    else
    {
	_adapterObserverTopic->adapterRemoved(serial, adapterId);
    }
    _replicaCache.waitForUpdateReplication("adapter", serial);
    return true;
}

Ice::ObjectPrx
Database::getAdapterDirectProxy(const string& adapterId)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringAdapterInfoDict adapters(connection, _adapterDbName); 
    StringAdapterInfoDict::const_iterator p = adapters.find(adapterId);
    if(p != adapters.end())
    {
	return p->second.proxy;
    }
    return 0;
}

void
Database::removeAdapter(const string& adapterId)
{
    AdapterInfoSeq infos;
    int serial;
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

	txHolder.commit();

	if(infos.empty())
	{
	    serial = ++_adapterSerial;
	}
	else
	{
	    serial = _adapterSerial + 1;
	    _adapterSerial += static_cast<int>(static_cast<int>(infos.size()));
	}
    }

    if(_traceLevels->adapter > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
	out << "removed " << (infos.empty() ? "adapter" : "replica group") << " `" << adapterId << "'";
    }

    if(infos.empty())
    {
	_adapterObserverTopic->adapterRemoved(serial, adapterId);
	_replicaCache.waitForUpdateReplication("adapter", serial);
    }
    else
    {
	int i = 0;
	AdapterInfoSeq::const_iterator p;
	for(p = infos.begin(); p != infos.end(); ++p, ++i)
	{
	    _adapterObserverTopic->adapterUpdated(serial + i, *p);
	}
	i = 0;
	for(p = infos.begin(); p != infos.end(); ++p, ++i)
	{
	    _replicaCache.waitForUpdateReplication("adapter", serial + i);
	}	
    }
}

AdapterPrx
Database::getAdapter(const string& id, const string& replicaGroupId, bool upToDate)
{
    return _adapterCache.getServerAdapter(id)->getProxy(replicaGroupId, upToDate);
}

vector<pair<string, AdapterPrx> >
Database::getAdapters(const string& id, int& endpointCount, bool& replicaGroup)
{
    //
    // First we check if the given adapter id is associated to a
    // server, if that's the case we get the adapter proxy from the
    // server.
    //
    try
    {
	return _adapterCache.get(id)->getProxies(endpointCount, replicaGroup);
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
	vector<pair<string, AdapterPrx> > adpts;
	Ice::Identity identity;
	identity.category = "IceGridAdapter";
	identity.name = id;
	Ice::ObjectPrx adpt = _internalAdapter->createDirectProxy(identity);
	adpts.push_back(make_pair(id, AdapterPrx::uncheckedCast(adpt)));
	replicaGroup = false;
	endpointCount = 1;
	return adpts;
    }

    //
    // If it's not a regular object adapter, perhaps it's a replica
    // group...
    //
    p = adapters.findByReplicaGroupId(id, true);
    if(p != adapters.end())
    {
	vector<pair<string, AdapterPrx> > adpts;
	while(p != adapters.end())
	{
	    Ice::Identity identity;
	    identity.category = "IceGridAdapter";
	    identity.name = p->first;
	    AdapterPrx adpt = AdapterPrx::uncheckedCast(_internalAdapter->createDirectProxy(identity));
	    adpts.push_back(make_pair(p->first, adpt));
	    ++p;
	}
	RandomNumberGenerator rng;
	random_shuffle(adpts.begin(), adpts.end(), rng);
	replicaGroup = true;
	endpointCount = static_cast<int>(adpts.size());
	return adpts;
    }

    throw AdapterNotExistException(id);
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
Database::addObject(const ObjectInfo& info, bool replaceIfExistsInDatabase)
{
    int serial;
    const Ice::Identity id = info.proxy->ice_getIdentity();
    bool update = false;
    {
	Lock sync(*this);	
	if(_objectCache.has(id))
	{
	    throw ObjectExistsException(id);
	}
	
	if(_objects.find(id) != _objects.end())
	{
	    if(!replaceIfExistsInDatabase)
	    {
		throw ObjectExistsException(id);
	    }
	    else
	    {
		update = true;
	    }
	}
	_objects.put(IdentityObjectInfoDict::value_type(id, info));
	
	serial = ++_objectSerial;
    }

    if(!update)
    {
	_objectObserverTopic->objectAdded(serial, info);
    }
    else
    {
	_objectObserverTopic->objectUpdated(serial, info);
    }
    _replicaCache.waitForUpdateReplication("object", serial);

    if(_traceLevels->object > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
	if(!update)
	{
	    out << "added object `" << _communicator->identityToString(id) << "'";
	}
	else
	{
	    out << "updated object `" << _communicator->identityToString(id) << "'";
	}
    }
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

	serial = ++_objectSerial;
    }

    _objectObserverTopic->objectRemoved(serial, id);
    _replicaCache.waitForUpdateReplication("object", serial);

    if(_traceLevels->object > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
	out << "removed object `" << _communicator->identityToString(id) << "'";
    }
}

void
Database::updateObject(const Ice::ObjectPrx& proxy)
{
    const Ice::Identity id = proxy->ice_getIdentity();
    int serial;
    ObjectInfo info;
    {
	Lock sync(*this);	
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
	
	info = p->second;
	info.proxy = proxy;
	p.set(info);

	serial = ++_objectSerial;
    }

    _objectObserverTopic->objectUpdated(serial, info);
    _replicaCache.waitForUpdateReplication("object", serial);

    if(_traceLevels->object > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
	out << "updated object `" << _communicator->identityToString(id) << "'";
    }
}

void
Database::addOrUpdateObjectsInDatabase(const ObjectInfoSeq& objects)
{
    int serial;
    vector<bool> updated;
    {
	Lock sync(*this);

	Freeze::TransactionHolder txHolder(_connection);
	for(ObjectInfoSeq::const_iterator p = objects.begin(); p != objects.end(); ++p)
	{
	    updated.push_back(_objects.find(p->proxy->ice_getIdentity()) != _objects.end());
	    _objects.put(IdentityObjectInfoDict::value_type(p->proxy->ice_getIdentity(), *p));
	}
	serial = _objectSerial;
	_objectSerial += static_cast<int>(static_cast<int>(objects.size()));
	txHolder.commit();
    }
	
    vector<bool>::const_iterator q = updated.begin();
    for(ObjectInfoSeq::const_iterator p = objects.begin(); p != objects.end(); ++p, ++q)
    {
	//
	// TODO: Add a better observer call?
	//
	if(!*q)
	{
	    _objectObserverTopic->objectAdded(++serial, *p);
	}
	else
	{
	    _objectObserverTopic->objectUpdated(++serial, *p);
	}

	if(_traceLevels->object > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
	    if(!*q)
	    {
		out << "added object `" << _communicator->identityToString(p->proxy->ice_getIdentity()) << "'";
	    }
	    else
	    {
		out << "updated object `" << _communicator->identityToString(p->proxy->ice_getIdentity()) << "'";
	    }
	}
    }
}

void
Database::removeObjectsInDatabase(const ObjectInfoSeq& objects)
{
    int serial;
    {
	Freeze::TransactionHolder txHolder(_connection);
	for(ObjectInfoSeq::const_iterator p = objects.begin(); p != objects.end(); ++p)
	{
	    _objects.erase(p->proxy->ice_getIdentity());
	}
	serial = _objectSerial;
	_objectSerial += static_cast<int>(static_cast<int>(objects.size()));
	txHolder.commit();
    }

    for(ObjectInfoSeq::const_iterator p = objects.begin(); p != objects.end(); ++p)
    {
	_objectObserverTopic->objectRemoved(++serial, p->proxy->ice_getIdentity());

	if(_traceLevels->object > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
	    out << "removed object `" << _communicator->identityToString(p->proxy->ice_getIdentity()) << "'";
	}
    }
}

void
Database::allocateObject(const Ice::Identity& id, const ObjectAllocationRequestPtr& request)
{
    _allocatableObjectCache.get(id)->allocate(request);
}

void
Database::allocateObjectByType(const string& type, const ObjectAllocationRequestPtr& request)
{
    _allocatableObjectCache.allocateByType(type, request);
}

void
Database::releaseObject(const Ice::Identity& id, const SessionIPtr& session)
{
    _allocatableObjectCache.get(id)->release(session);
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
    return objs[IceUtil::random(static_cast<int>(objs.size()))];
}

Ice::ObjectPrx
Database::getObjectByTypeOnLeastLoadedNode(const string& type, LoadSample sample)
{
    Ice::ObjectProxySeq objs = getObjectsByType(type);
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
    if(proxies.empty())
    {
	throw ObjectNotRegisteredException();
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
    if(_objectCache.has(objectId) 
       || _allocatableObjectCache.has(objectId) 
       || _objects.find(objectId) != _objects.end())
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
	_adapterCache.addReplicaGroup(r->id, application, r->loadBalancing);
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
    vector<string> updateRev;
    map<string, ServerInfo>::const_iterator p;
    for(p = newServers.begin(); p != newServers.end(); ++p)
    {
	map<string, ServerInfo>::const_iterator q = oldServers.find(p->first);
	if(q == oldServers.end())
	{
	    load.push_back(p->second);
	} 
	else if(p->second.node != q->second.node || 
	        !descriptorEqual(_communicator, p->second.descriptor, q->second.descriptor))
	{
	    _serverCache.remove(p->first, false); // Don't destroy the server if it was updated.
	    load.push_back(p->second);
	}
	else
	{
	    updateRev.push_back(p->first);
	}
    }
    for(p = oldServers.begin(); p != oldServers.end(); ++p)
    {
	map<string, ServerInfo>::const_iterator q = newServers.find(p->first);
	if(q == newServers.end())
	{
	    entries.push_back(_serverCache.remove(p->first));
	}
	else
	{
	    updateRev.push_back(p->first);
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
	    ReplicaGroupEntryPtr entry = _adapterCache.getReplicaGroup(r->id);
	    entry->update(r->loadBalancing);
	}
	catch(const AdapterNotExistException&)
	{
	    _adapterCache.addReplicaGroup(r->id, application, r->loadBalancing);
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

    //
    // Just update the revision of the server.
    //
    for(vector<string>::const_iterator rp = updateRev.begin(); rp != updateRev.end(); ++rp)
    {
	_serverCache.get(*rp)->updateRevision(revision);
    }
}

void
Database::finishUpdate(ServerEntrySeq& entries, 
		       const ApplicationUpdateInfo& update,
		       const ApplicationInfo& oldApp, 
		       const ApplicationDescriptor& newDesc,
		       AdminSessionI* session)
{
    if(_master)
    {
	//
	// Synchronize the servers on the nodes. If a server couldn't be
	// deployed we unload the application and throw.
	//
	try
	{
	    for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::sync));
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
		_updating.erase(newDesc.name);
		notifyAll();
	    }

	    try
	    {
		for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::sync));
	    }
	    catch(const DeploymentException& ex)
	    {
		Ice::Error err(_traceLevels->logger);
		err << "failed to rollback previous application `" << oldApp.descriptor.name << "':\n" << ex.reason;
	    }
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
	serial = ++_applicationSerial;
	_updating.erase(update.descriptor.name);
	notifyAll();
    }

    _applicationObserverTopic->applicationUpdated(serial, update);
    _replicaCache.waitForUpdateReplication("application", serial);
    _replicaCache.finishApplicationReplication(oldApp.uuid, update.revision);

    if(_traceLevels->application > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->applicationCat);
	out << "updated application `" << update.descriptor.name << "'";
    }
}
