// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/StringUtil.h>
#include <Freeze/Freeze.h>
#include <IceGrid/Database.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/Util.h>
#include <IceGrid/DescriptorHelper.h>
#include <IceGrid/NodeSessionI.h>

#include <algorithm>
#include <functional>
#include <iterator>

using namespace std;
using namespace IceGrid;

const string Database::_descriptorDbName = "applications";
const string Database::_adapterDbName = "adapters";
const string Database::_objectDbName = "objects";

namespace IceGrid
{

struct AddAdapterId : std::unary_function<ComponentDescriptorPtr&, void>
{
    AddAdapterId(set<string>& ids, set<string>& replicatedIds) : _ids(ids), _replicatedIds(replicatedIds)
    {
    }

    void 
    operator()(const ComponentDescriptorPtr& desc)
    {
	for(AdapterDescriptorSeq::const_iterator p = desc->adapters.begin(); p != desc->adapters.end(); ++p)
	{
	    if(p->id.empty())
	    {
		DeploymentException ex;
		ex.reason = "empty adapter id for adapter `" + p->name + "' in `" + desc->name + "'";
		throw ex;
	    }
	    if(!_ids.insert(p->id).second && _replicatedIds.find(p->id) == _replicatedIds.end())
	    {
		DeploymentException ex;
		ex.reason = "duplicated adapter id `" + p->id + "'";
		throw ex;
	    }
	}
    }

    set<string>& _ids;
    const set<string>& _replicatedIds;
};

struct AddReplicatedAdapterId : std::unary_function<ReplicatedAdapterDescriptor&, void>
{
    AddReplicatedAdapterId(set<string>& ids) : _ids(ids)
    {
    }

    void 
    operator()(const ReplicatedAdapterDescriptor& desc)
    {
	if(!_ids.insert(desc.id).second)
	{
	    DeploymentException ex;
	    ex.reason = "duplicated replicated adapter id `" + desc.id + "'";
	    throw ex;
	}
    }

    set<string>& _ids;
};

struct AddObjectId : std::unary_function<ComponentDescriptorPtr&, void>
{
    AddObjectId(set<Ice::Identity>& ids) : _ids(ids)
    {
    }

    void 
    operator()(const ComponentDescriptorPtr& desc)
    {
	for(AdapterDescriptorSeq::const_iterator p = desc->adapters.begin(); p != desc->adapters.end(); ++p)
	{
	    for(ObjectDescriptorSeq::const_iterator q = p->objects.begin(); q != p->objects.end(); ++q)
	    {
		if(!_ids.insert(q->id).second)
		{
		    DeploymentException ex;
		    ex.reason = "duplicated object id `" + Ice::identityToString(q->id) + "'";
		    throw ex;
		}
	    }
	}
    }

    set<Ice::Identity>& _ids;
};

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
	cb->ice_response(_database->getAdapterDirectProxy(current.id.name));
    }

    virtual Ice::ObjectPrx 
    getDirectProxy(const Ice::Current& current) const
    {
	return _database->getAdapterDirectProxy(current.id.name);
    }

    virtual void 
    setDirectProxy(const ::Ice::ObjectPrx& proxy, const ::Ice::Current& current)
    {
	_database->setAdapterDirectProxy(current.id.name, proxy);
    }

    virtual void 
    destroy(const ::Ice::Current& current)
    {
	_database->setAdapterDirectProxy(current.id.name, 0);
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

}

Database::Database(const Ice::ObjectAdapterPtr& adapter,
		   const string& envName,
		   int nodeSessionTimeout,
		   const TraceLevelsPtr& traceLevels) :
    _communicator(adapter->getCommunicator()),
    _internalAdapter(adapter),
    _envName(envName),
    _nodeSessionTimeout(nodeSessionTimeout),
    _traceLevels(traceLevels),
    _objectCache(_communicator),
    _serverCache(*this, _nodeCache, _adapterCache, _objectCache),
    _connection(Freeze::createConnection(adapter->getCommunicator(), envName)),
    _descriptors(_connection, _descriptorDbName),
    _objects(_connection, _objectDbName),
    _adapters(_connection, _adapterDbName),
    _serial(0)
{
    //
    // Register a default servant to manage manually registered object adapters.
    //
    _internalAdapter->addServantLocator(new AdapterServantLocator(this), "IceGridAdapter");

    //
    // Cache the servers & adapters.
    //
    for(StringApplicationDescriptorDict::const_iterator p = _descriptors.begin(); p != _descriptors.end(); ++p)
    {
	for(ReplicatedAdapterDescriptorSeq::const_iterator r = p->second->replicatedAdapters.begin();
	    r != p->second->replicatedAdapters.end(); ++r)
	{
	    _adapterCache.get(r->id, true)->enableReplication(r->loadBalancing);
	}

	ServerInstanceDescriptorSeq::const_iterator q;
	for(q = p->second->servers.begin(); q != p->second->servers.end(); ++q)
	{
	    addServer(p->first, *q);
	}
    }
}

Database::~Database()
{
}

void
Database::setObservers(const RegistryObserverPrx& registryObserver, const NodeObserverPrx& nodeObserver)
{
    int serial;
    ApplicationDescriptorSeq applications;
    {
	Lock sync(*this);
	_registryObserver = registryObserver;
	_nodeObserver = nodeObserver;
	serial = _serial;

	for(StringApplicationDescriptorDict::const_iterator p = _descriptors.begin(); p != _descriptors.end(); ++p)
	{
	    applications.push_back(p->second);
	}
    }

    //
    // Notify the observers.
    //
    _registryObserver->init(serial, applications);
}

void
Database::checkSessionLock(ObserverSessionI* session)
{
    if(_lock != 0 && session != _lock)
    {
	AccessDenied ex;
	ex.lockUserId = _lockUserId;
	throw ex;
    }
}

void
Database::lock(int serial, ObserverSessionI* session, const string& userId)
{
    Lock sync(*this);
    checkSessionLock(session);

    if(serial != _serial)
    {
	throw CacheOutOfDate();
    }

    _lock = session;
    _lockUserId = userId;
}

void
Database::unlock(ObserverSessionI* session)
{
    Lock sync(*this);
    assert(_lock == session);
    _lock = 0;
    _lockUserId.clear();
}

void
Database::addApplicationDescriptor(ObserverSessionI* session, const ApplicationDescriptorPtr& newApp)
{
    ServerEntrySeq entries;
    ApplicationDescriptorPtr descriptor;
    int serial;
    {
	Lock sync(*this);
	
	checkSessionLock(session);

	//
	// We first ensure that the application doesn't already exist
	// and that the application components don't already exist.
	//
	if(_descriptors.find(newApp->name) != _descriptors.end())
	{
	    ApplicationExistsException ex;
	    ex.name = newApp->name;
	    throw ex;
	}

	try
	{
	    ApplicationDescriptorHelper helper(_communicator, newApp);
	    helper.instantiate();
	    descriptor = helper.getDescriptor();
	}
	catch(const string& msg)
	{
	    DeploymentException ex;
	    ex.reason = msg;
	    throw ex;
	}

	//
	// Ensure that the application servers, adapters and objects
	// aren't already registered.
	//
	set<string> servers;
	for_each(descriptor->servers.begin(), descriptor->servers.end(), AddServerName(servers));
	try
	{
	    for_each(servers.begin(), servers.end(), objFunc(*this, &Database::checkServerForAddition));
	}
	catch(const ServerExistsException& e)
	{
	    DeploymentException ex;
	    ex.reason = "server `" + e.name + "' is already registered"; 
	    throw ex;
	}
	
	set<string> replicatedAdapterIds;
	AddReplicatedAdapterId addReplicatedAdpt(replicatedAdapterIds);
	for_each(descriptor->replicatedAdapters.begin(), descriptor->replicatedAdapters.end(), addReplicatedAdpt);
	try
	{
	    ObjFunc<Database, const string&> func = objFunc(*this, &Database::checkAdapterForAddition);
	    for_each(replicatedAdapterIds.begin(), replicatedAdapterIds.end(), func);
	}
	catch(const AdapterExistsException& e)
	{
	    DeploymentException ex;
	    ex.reason = "replicated adapter `" + e.id + "' is already registered"; 
	    throw ex;
	}

	set<string> adapterIds;
	AddAdapterId addAdpt(adapterIds, replicatedAdapterIds);
	for_each(descriptor->servers.begin(), descriptor->servers.end(), forEachComponent(addAdpt));
	try
	{
	    for_each(adapterIds.begin(), adapterIds.end(), objFunc(*this, &Database::checkAdapterForAddition));
	}
	catch(const AdapterExistsException& e)
	{
	    DeploymentException ex;
	    ex.reason = "adapter `" + e.id + "' is already registered"; 
	    throw ex;
	}

	set<Ice::Identity> objectIds;
	for_each(descriptor->servers.begin(), descriptor->servers.end(), forEachComponent(AddObjectId(objectIds)));
	try
	{
	    for_each(objectIds.begin(), objectIds.end(), objFunc(*this, &Database::checkObjectForAddition));
	}
	catch(const ObjectExistsException& e)
	{
	    DeploymentException ex;
	    ex.reason = "object `" + Ice::identityToString(e.id) + "' is already registered"; 
	    throw ex;
	}

	//
	// Register the replicated adapters.
	//
	for(ReplicatedAdapterDescriptorSeq::const_iterator p = descriptor->replicatedAdapters.begin();
	    p != descriptor->replicatedAdapters.end(); ++p)
	{
	    _adapterCache.get(p->id, true)->enableReplication(p->loadBalancing);
	}

	//
	// Register the application servers.
	//
	addServers(descriptor->name, descriptor->servers, servers, entries);

	//
	// Save the application descriptor.
	//
	_descriptors.put(make_pair(descriptor->name, descriptor));

	serial = ++_serial;
    }

    //
    // Notify the observers.
    //
    _registryObserver->applicationAdded(serial, descriptor);

    if(_traceLevels->application > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->applicationCat);
	out << "added application `" << descriptor->name << "'";
    }

    //
    // Synchronize the servers on the nodes.
    //
    for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::sync));
}

void
Database::updateApplicationDescriptor(ObserverSessionI* session, const ApplicationUpdateDescriptor& update)
{
    ServerEntrySeq entries;
    int serial;
    ApplicationUpdateDescriptor newUpdate;
    {
	Lock sync(*this);	
	checkSessionLock(session);

	StringApplicationDescriptorDict::const_iterator p = _descriptors.find(update.name);
	if(p == _descriptors.end())
	{
	    ApplicationNotExistException ex;
	    ex.name = update.name;
	    throw ex;
	}

	//
	// Update the application descriptor.
	//
	ApplicationDescriptorPtr descriptor;
	try
	{
	    ApplicationDescriptorHelper helper(_communicator, p->second);
	    newUpdate = helper.update(update);
	    descriptor = helper.getDescriptor();
	}
	catch(const string& msg)
	{
	    DeploymentException ex;
	    ex.reason = msg;
	    throw ex;
	}

	//
	// Synchronize the application descriptor.
	//
	syncApplicationDescriptorNoSync(p->second, descriptor, entries);

	serial = ++_serial;
    }    

    //
    // Notify the observers.
    //
    _registryObserver->applicationUpdated(serial, newUpdate);

    if(_traceLevels->application > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->applicationCat);
	out << "updated application `" << update.name << "'";
    }

    for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::sync));
}

void
Database::syncApplicationDescriptor(ObserverSessionI* session, const ApplicationDescriptorPtr& newDesc)
{
    ServerEntrySeq entries;
    int serial;
    ApplicationUpdateDescriptor update;
    {
	Lock sync(*this);
	checkSessionLock(session);

	StringApplicationDescriptorDict::const_iterator p = _descriptors.find(newDesc->name);
	if(p == _descriptors.end())
	{
	    ApplicationNotExistException ex;
	    ex.name = newDesc->name;
	    throw ex;
	}

	ApplicationDescriptorPtr descriptor;
	try
	{
	    ApplicationDescriptorHelper helper(_communicator, newDesc);
	    helper.instantiate();
	    update = helper.diff(p->second);
	    descriptor = helper.getDescriptor();
	}
	catch(const string& msg)
	{
	    DeploymentException ex;
	    ex.reason = msg;
	    throw ex;
	}
	
	//
	// Synchronize the application descriptor.
	//
	syncApplicationDescriptorNoSync(p->second, descriptor, entries);

	serial = ++_serial;
    }

    //
    // Notify the observers.
    //
    _registryObserver->applicationUpdated(serial, update);

    if(_traceLevels->application > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->applicationCat);
	out << "synced application `" << newDesc->name << "'";
    }

    for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::sync));
}

void
Database::syncApplicationDescriptorNoSync(const ApplicationDescriptorPtr& origDesc,
					  const ApplicationDescriptorPtr& newDesc,
					  ServerEntrySeq& entries)
{
    //
    // Ensure that the new application servers aren't already
    // registered.
    //
    set<string> oldSvrs;
    set<string> newSvrs;
    for_each(origDesc->servers.begin(), origDesc->servers.end(), AddServerName(oldSvrs));
    for_each(newDesc->servers.begin(), newDesc->servers.end(), AddServerName(newSvrs));

    set<string> added, removed, updated; 
    set_intersection(newSvrs.begin(), newSvrs.end(), oldSvrs.begin(), oldSvrs.end(), set_inserter(updated));
    set_difference(oldSvrs.begin(), oldSvrs.end(), newSvrs.begin(), newSvrs.end(), set_inserter(removed));
    set_difference(newSvrs.begin(), newSvrs.end(), oldSvrs.begin(), oldSvrs.end(), set_inserter(added));
    try
    {
	for_each(added.begin(), added.end(), objFunc(*this, &Database::checkServerForAddition));
    }
    catch(const ServerExistsException& e)
    {
	DeploymentException ex;
	ex.reason = "server `" + e.name + "' is already registered"; 
	throw ex;
    }

    //
    // Ensure that the new application replicated adapters aren't
    // already registered.
    //
    set<string> oldReplicatedAdapterIds;
    set<string> newReplicatedAdapterIds;
    AddReplicatedAdapterId addOldReplicatedAdpt(oldReplicatedAdapterIds);
    for_each(origDesc->replicatedAdapters.begin(), origDesc->replicatedAdapters.end(), addOldReplicatedAdpt);
    AddReplicatedAdapterId addNewReplicatedAdpt(newReplicatedAdapterIds);
    for_each(newDesc->replicatedAdapters.begin(), newDesc->replicatedAdapters.end(), addNewReplicatedAdpt);

    set<string> addedReplicatedAdpts;
    set_difference(newReplicatedAdapterIds.begin(), newReplicatedAdapterIds.end(), oldReplicatedAdapterIds.begin(), 
		   oldReplicatedAdapterIds.end(), set_inserter(addedReplicatedAdpts));
    try
    {
	ObjFunc<Database, const string&> func = objFunc(*this, &Database::checkAdapterForAddition);
	for_each(addedReplicatedAdpts.begin(), addedReplicatedAdpts.end(), func);
    }
    catch(const AdapterExistsException& e)
    {
	DeploymentException ex;
	ex.reason = "replicated adapter `" + e.id + "' is already registered"; 
	throw ex;
    }

    //
    // Ensure that the new application adapters aren't already
    // registered.
    //
    set<string> oldAdpts;
    set<string> newAdpts;
    AddAdapterId addOldAdpt(oldAdpts, oldReplicatedAdapterIds);
    for_each(origDesc->servers.begin(), origDesc->servers.end(), forEachComponent(addOldAdpt));
    AddAdapterId addNewAdpt(newAdpts, newReplicatedAdapterIds);
    for_each(newDesc->servers.begin(), newDesc->servers.end(), forEachComponent(addNewAdpt));

    set<string> addedAdpts;
    set_difference(newAdpts.begin(), newAdpts.end(), oldAdpts.begin(), oldAdpts.end(), set_inserter(addedAdpts));
    try
    {
	for_each(addedAdpts.begin(), addedAdpts.end(), objFunc(*this, &Database::checkAdapterForAddition));
    }
    catch(const AdapterExistsException& e)
    {
	DeploymentException ex;
	ex.reason = "adapter `" + e.id + "' is already registered"; 
	throw ex;
    }

    //
    // Ensure that the new application objects aren't already
    // registered.
    //
    set<Ice::Identity> oldObjs;
    set<Ice::Identity> newObjs;
    for_each(origDesc->servers.begin(), origDesc->servers.end(), forEachComponent(AddObjectId(oldObjs)));
    for_each(newDesc->servers.begin(), newDesc->servers.end(), forEachComponent(AddObjectId(newObjs)));

    set<Ice::Identity> addedObjs;
    set_difference(newObjs.begin(), newObjs.end(), oldObjs.begin(), oldObjs.end(), set_inserter(addedObjs));
    try
    {
	for_each(addedObjs.begin(), addedObjs.end(), objFunc(*this, &Database::checkObjectForAddition));
    }
    catch(const ObjectExistsException& e)
    {
	DeploymentException ex;
	ex.reason = "object `" + Ice::identityToString(e.id) + "' is already registered"; 
	throw ex;
    }	

    //
    // Update the replicated adapters.
    //
    for(ReplicatedAdapterDescriptorSeq::const_iterator p = origDesc->replicatedAdapters.begin();
	p != origDesc->replicatedAdapters.end(); ++p)
    {
	_adapterCache.get(p->id)->disableReplication();
    }
    for(ReplicatedAdapterDescriptorSeq::const_iterator p = newDesc->replicatedAdapters.begin();
	p != newDesc->replicatedAdapters.end(); ++p)
    {
	_adapterCache.get(p->id, true)->enableReplication(p->loadBalancing);
    }


    //
    // Register the new servers, unregister the old ones and
    // update the updated ones.
    //
    addServers(newDesc->name, newDesc->servers, added, entries);
    updateServers(origDesc, newDesc, updated, entries);
    removeServers(origDesc->name, origDesc->servers, removed, entries);

    _descriptors.put(make_pair(newDesc->name, newDesc));
}

void
Database::removeApplicationDescriptor(ObserverSessionI* session, const std::string& name)
{
    ApplicationDescriptorPtr descriptor;
    ServerEntrySeq entries;
    int serial;
    {
	Lock sync(*this);
	checkSessionLock(session);

	StringApplicationDescriptorDict::iterator p = _descriptors.find(name);
	if(p == _descriptors.end())
	{
	    throw ApplicationNotExistException();
	}
	
	descriptor = p->second;
	_descriptors.erase(p);

	for(ReplicatedAdapterDescriptorSeq::const_iterator q = descriptor->replicatedAdapters.begin();
	    q != descriptor->replicatedAdapters.end(); ++q)
	{
	    _adapterCache.get(q->id)->disableReplication();
	}
	
	set<string> servers;
	for_each(descriptor->servers.begin(), descriptor->servers.end(), AddServerName(servers));
	removeServers(descriptor->name, descriptor->servers, servers, entries);

	serial = ++_serial;
    }

    //
    // Notify the observers
    //
    _registryObserver->applicationRemoved(serial, descriptor->name);

    if(_traceLevels->application > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->applicationCat);
	out << "removed application `" << name << "'";
    }

    for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::sync));
}

ApplicationDescriptorPtr
Database::getApplicationDescriptor(const std::string& name)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringApplicationDescriptorDict descriptors(connection, _descriptorDbName); 
    
    StringApplicationDescriptorDict::const_iterator p = descriptors.find(name);
    if(p == descriptors.end())
    {
	ApplicationNotExistException ex;
	ex.name = name;
	throw ex;
    }

    return p->second;
}

Ice::StringSeq
Database::getAllApplications(const string& expression)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringApplicationDescriptorDict descriptors(connection, _descriptorDbName);
    return getMatchingKeys<StringApplicationDescriptorDict>(descriptors, expression);
}

void 
Database::addNode(const string& name, const NodeSessionIPtr& session)
{
    _nodeCache.get(name, true)->setSession(session);
}

NodePrx 
Database::getNode(const string& name) const
{
    return _nodeCache.get(name)->getProxy();
}

void 
Database::removeNode(const string& name)
{
    _nodeCache.get(name)->setSession(0);

    try
    {
	_nodeObserver->nodeDown(name);
    }
    catch(const Ice::LocalException&)
    {
	// TODO: Log a warning?
    }
}

Ice::StringSeq 
Database::getAllNodes(const string& expression)
{
    return _nodeCache.getAll(expression);
}

ServerInstanceDescriptor
Database::getServerDescriptor(const std::string& name)
{
    ApplicationDescriptorPtr app = getApplicationDescriptor(getServerApplication(name));

    for(ServerInstanceDescriptorSeq::const_iterator p = app->servers.begin(); p != app->servers.end(); ++p)
    {
	if(p->descriptor->name == name)
	{
	    return *p;
	}
    }

    ServerNotExistException ex;
    ex.name = name;
    throw ex;
}

string
Database::getServerApplication(const string& name)
{
    return _serverCache.get(name)->getApplication();
}

ServerPrx
Database::getServer(const string& name)
{
    int activationTimeout, deactivationTimeout;
    return getServerWithTimeouts(name, activationTimeout, deactivationTimeout);
}

ServerPrx
Database::getServerWithTimeouts(const string& name, int& activationTimeout, int& deactivationTimeout)
{
    return _serverCache.get(name)->getProxy(activationTimeout, deactivationTimeout);
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

void
Database::setAdapterDirectProxy(const string& id, const Ice::ObjectPrx& proxy)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxyDict adapters(connection, _adapterDbName); 
    if(proxy)
    {
	StringObjectProxyDict::iterator p = adapters.find(id);
	if(p != adapters.end())
	{
	    p.set(proxy);

	    if(_traceLevels->adapter > 0)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
		out << "added adapter `" << id << "'";
	    }
	}
	else
	{
	    adapters.put(make_pair(id, proxy));

	    if(_traceLevels->adapter > 0)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
		out << "updated adapter `" << id << "'";
	    }
	}
    }
    else
    {
	adapters.erase(id);
    }
}

Ice::ObjectPrx
Database::getAdapterDirectProxy(const string& id)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxyDict adapters(connection, _adapterDbName); 
    StringObjectProxyDict::const_iterator p = adapters.find(id);
    if(p != adapters.end())
    {
	return p->second;
    }
    return 0;
}

AdapterPrx
Database::getAdapter(const string& id, const string& serverId)
{
    //
    // TODO: Perhaps we should also cache the adapter proxies here
    // instead of doing multiple lookups.
    //

    //
    // First we check if the given adapter id is associated to a
    // server, if that's the case we get the adapter proxy from the
    // server.
    //
    try
    {
	return _adapterCache.get(id)->getProxy(serverId);
    }
    catch(const AdapterNotExistException&)
    {
    }

    //
    // Otherwise, we check the adapter endpoint table -- if there's an
    // entry the adapter is managed by the registry itself.
    //
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxyDict adapters(connection, _adapterDbName); 
    if(adapters.find(id) != adapters.end())
    {
	Ice::Identity identity;
	identity.category = "IceGridAdapter";
	identity.name = id;
	return AdapterPrx::uncheckedCast(_internalAdapter->createDirectProxy(identity));
    }

    AdapterNotExistException ex;
    ex.id = id;
    throw ex;
}

Ice::StringSeq
Database::getAllAdapters(const string& expression)
{
    Lock sync(*this);
    vector<string> result;
    vector<string> ids = _adapterCache.getAll(expression);
    result.swap(ids);
    ids = getMatchingKeys<StringObjectProxyDict>(_adapters, expression);
    result.insert(result.end(), ids.begin(), ids.end());
    return result;
}

void
Database::addObject(const ObjectInfo& info)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectInfoDict objects(connection, _objectDbName); 
    const Ice::Identity id = info.proxy->ice_getIdentity();
    if(objects.find(id) != objects.end())
    {
	ObjectExistsException ex;
	ex.id = id;
	throw ex;
    }
    objects.put(make_pair(id, info));

    if(_traceLevels->object > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
	out << "added object `" << Ice::identityToString(id) << "'";
    }
}

void
Database::removeObject(const Ice::Identity& id)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectInfoDict objects(connection, _objectDbName); 
    if(objects.find(id) == objects.end())
    {
	ObjectNotExistException ex;
	ex.id = id;
	throw ex;
    }
    objects.erase(id);

    if(_traceLevels->object > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
	out << "removed object `" << Ice::identityToString(id) << "'";
    }
}

void
Database::updateObject(const Ice::ObjectPrx& proxy)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectInfoDict objects(connection, _objectDbName); 
    const Ice::Identity id = proxy->ice_getIdentity();
    IdentityObjectInfoDict::iterator p = objects.find(id);
    if(p == objects.end())
    {
	ObjectNotExistException ex;
	ex.id = id;
	throw ex;
    }
    ObjectInfo info = p->second;
    info.proxy = proxy;
    p.set(info);

    if(_traceLevels->object > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
	out << "updated object `" << Ice::identityToString(id) << "'";
    }
}

Ice::ObjectPrx
Database::getObjectProxy(const Ice::Identity& id, string& adapterId)
{
    try
    {
	ObjectEntryPtr object = _objectCache.get(id);
	adapterId = object->getAdapterId();
	return object->getProxy();
    }
    catch(ObjectNotExistException&)
    {
    }

    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectInfoDict objects(connection, _objectDbName);
    IdentityObjectInfoDict::const_iterator p = objects.find(id);
    if(p == objects.end())
    {
	ObjectNotExistException ex;
	ex.id = id;
	throw ex;
    }
    adapterId = "";
    return p->second.proxy;
}

Ice::ObjectPrx
Database::getObjectByType(const string& type)
{
    Ice::ObjectProxySeq objs = getObjectsWithType(type);
    return objs[rand() % objs.size()];
}

Ice::ObjectProxySeq
Database::getObjectsWithType(const string& type)
{
    Ice::ObjectProxySeq proxies = _objectCache.getObjectsWithType(type);

    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectInfoDict objects(connection, _objectDbName);    
    for(IdentityObjectInfoDict::const_iterator p = objects.findByType(type); p != objects.end(); ++p)
    {
	proxies.push_back(p->second.proxy);
    }
    if(proxies.empty())
    {
	throw ObjectNotExistException();
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
    catch(ObjectNotExistException&)
    {
    }

    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectInfoDict objects(connection, _objectDbName);
    IdentityObjectInfoDict::const_iterator p = objects.find(id);
    if(p == objects.end())
    {
	ObjectNotExistException ex;
	ex.id = id;
	throw ex;
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
	if(expression.empty() || IceUtil::match(Ice::identityToString(p->first), expression, true))
	{
	    infos.push_back(p->second);
	}
    }
    return infos;
}

const TraceLevelsPtr&
Database::getTraceLevels() const
{
    return _traceLevels;
}

int
Database::getNodeSessionTimeout() const
{
    return _nodeSessionTimeout;
}

void
Database::checkServerForAddition(const string& name)
{
    if(_serverCache.has(name))
    {
	ServerExistsException ex;
	ex.name = name;
	throw ex;
    }
}

void
Database::checkAdapterForAddition(const string& id)
{
    if(_adapterCache.has(id) || _adapters.find(id) != _adapters.end())
    {
	AdapterExistsException ex;
	ex.id = id;
	throw ex;
    }
}

void
Database::checkObjectForAddition(const Ice::Identity& objectId)
{
    if(_objectCache.has(objectId) || _objects.find(objectId) != _objects.end())
    {
	ObjectExistsException ex;
	ex.id = objectId;
	throw ex;
    }
}

void 
Database::addServers(const string& application, const ServerInstanceDescriptorSeq& servers, const set<string>& names, 
		     ServerEntrySeq& entries)
{
    for(ServerInstanceDescriptorSeq::const_iterator p = servers.begin(); p != servers.end(); ++p)
    {
	if(names.find(p->descriptor->name) == names.end())
	{
	    continue;
	}
	entries.push_back(addServer(application, *p));
    }
}

void 
Database::updateServers(const ApplicationDescriptorPtr& oldAppDesc, const ApplicationDescriptorPtr& newAppDesc,
			const set<string>& names, ServerEntrySeq& entries)
{
    ApplicationDescriptorHelper oldAppDescHelper(_communicator, oldAppDesc);
    ApplicationDescriptorHelper newAppDescHelper(_communicator, newAppDesc);

    ServerInstanceDescriptorSeq::const_iterator p;
    for(p = newAppDesc->servers.begin(); p != newAppDesc->servers.end(); ++p)
    {
	if(names.find(p->descriptor->name) == names.end())
	{
	    continue;
	}

	ServerInstanceDescriptorSeq::const_iterator q;
	for(q = oldAppDesc->servers.begin(); q != oldAppDesc->servers.end(); ++q)
	{
	    if(p->descriptor->name == q->descriptor->name)
	    {
		if(q->node != p->node || 
		   ServerDescriptorHelper(oldAppDescHelper, q->descriptor) != 
		   ServerDescriptorHelper(newAppDescHelper, p->descriptor))
		{
		    entries.push_back(updateServer(*p));
		}
		break;
	    }
	}
    }
}

void
Database::removeServers(const string& application, const ServerInstanceDescriptorSeq& servers, 
			const set<string>& names, ServerEntrySeq& entries)
{
    for(ServerInstanceDescriptorSeq::const_iterator p = servers.begin(); p != servers.end(); ++p)
    {
	if(names.find(p->descriptor->name) == names.end())
	{
	    continue;
	}
	entries.push_back(removeServer(application, *p));
    }
}

ServerEntryPtr
Database::addServer(const string& application, const ServerInstanceDescriptor& instance)
{
    return _serverCache.add(instance.descriptor->name, instance, application);
}

ServerEntryPtr
Database::updateServer(const ServerInstanceDescriptor& instance)
{
    return _serverCache.update(instance);
}

ServerEntryPtr
Database::removeServer(const string& application, const ServerInstanceDescriptor& instance)
{
    return _serverCache.remove(instance.descriptor->name);
}
