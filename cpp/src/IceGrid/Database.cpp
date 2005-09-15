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
	cb->ice_response(getAdapterDirectProxy(current));
    }

    virtual Ice::ObjectPrx 
    getDirectProxy(const Ice::Current& current) const
    {
	return getAdapterDirectProxy(current);
    }

    virtual void 
    setDirectProxy(const ::Ice::ObjectPrx& proxy, const ::Ice::Current& current)
    {
	setAdapterDirectProxy(current, proxy);
    }

    virtual void 
    destroy(const ::Ice::Current& current)
    {
	setAdapterDirectProxy(current, 0);
    }

private:

    Ice::ObjectPrx
    getAdapterDirectProxy(const Ice::Current& current) const
    {
	string adapterId, serverId;
	getAdapterIdAndServerId(current, serverId, adapterId);
	return _database->getAdapterDirectProxy(serverId, adapterId);
    }

    void
    setAdapterDirectProxy(const Ice::Current& current, const Ice::ObjectPrx& proxy)
    {
	string adapterId, serverId;
	getAdapterIdAndServerId(current, serverId, adapterId);
	_database->setAdapterDirectProxy(serverId, adapterId, proxy);
    }
    
    void
    getAdapterIdAndServerId(const Ice::Current& current, string& serverId, string& adapterId) const
    {
	istringstream is(current.id.name);
	unsigned int size;
	is >> size;
	char c;
	is >> c;
	assert(c == '-');
	string id;
	is >> id;
	adapterId = id.substr(0, size);
	serverId = (id.size() > size) ? id.substr(size + 1) : string();
    }

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
    _traceLevels(traceLevels), 
    _nodeCache(nodeSessionTimeout, _traceLevels),
    _objectCache(_communicator, _traceLevels),
    _adapterCache(_traceLevels),
    _serverCache(_nodeCache, _adapterCache, _objectCache, _traceLevels),
    _connection(Freeze::createConnection(adapter->getCommunicator(), envName)),
    _descriptors(_connection, _descriptorDbName),
    _objects(_connection, _objectDbName),
    _adapters(_connection, _adapterDbName),
    _lock(0), 
    _serial(0)
{
    //
    // Register a default servant to manage manually registered object adapters.
    //
    _internalAdapter->addServantLocator(new AdapterServantLocator(this), "IceGridAdapter");

    //
    // Cache the servers & adapters.
    //
    ServerEntrySeq entries;
    for(StringApplicationDescriptorDict::const_iterator p = _descriptors.begin(); p != _descriptors.end(); ++p)
    {
	try
	{
	    load(ApplicationHelper(p->second), entries);
	}
	catch(const DeploymentException& ex)
	{
	    Ice::Warning warn(_traceLevels->logger);
	    warn << "invalid application `" << p->first << "':\n" << ex.reason;
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
	AccessDeniedException ex;
	ex.lockUserId = _lockUserId;
	throw ex;
    }
}

int
Database::lock(ObserverSessionI* session, const string& userId)
{
    Lock sync(*this);
    checkSessionLock(session);

    _lock = session;
    _lockUserId = userId;
    
    return _serial;
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
Database::addApplicationDescriptor(ObserverSessionI* session, const ApplicationDescriptor& desc)
{
    ServerEntrySeq entries;
    int serial;
    {
	Lock sync(*this);
	
	checkSessionLock(session);

	//
	// We first ensure that the application doesn't already exist
	// and that the application components don't already exist.
	//
	if(_descriptors.find(desc.name) != _descriptors.end())
	{
	    throw DeploymentException("application `" + desc.name + "' already exists");
	}

	ApplicationHelper helper(desc);
	
	//
	// Ensure that the application servers, adapters and objects
	// aren't already registered.
	//
	checkForAddition(helper);
	
	//
	// Register the application servers, adapters, objects.
	//
	load(helper, entries);
	
	//
	// Save the application descriptor.
	//
	_descriptors.put(make_pair(desc.name, desc));
	
	serial = ++_serial;
    }

    //
    // Notify the observers.
    //
    _registryObserver->applicationAdded(serial, desc);

    if(_traceLevels->application > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->applicationCat);
	out << "added application `" << desc.name << "'";
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
    {
	Lock sync(*this);	
	checkSessionLock(session);

	StringApplicationDescriptorDict::const_iterator p = _descriptors.find(update.name);
	if(p == _descriptors.end())
	{
	    throw ApplicationNotExistException(update.name);
	}

	ApplicationHelper previous(p->second);
	ApplicationHelper helper(p->second);
	helper.update(update);
	
	checkForUpdate(previous, helper);
	
	reload(previous, helper, entries);
	
	_descriptors.put(make_pair(update.name, helper.getDescriptor()));

	serial = ++_serial;
    }    

    //
    // Notify the observers.
    //
    _registryObserver->applicationUpdated(serial, update);

    if(_traceLevels->application > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->applicationCat);
	out << "updated application `" << update.name << "'";
    }

    for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::sync));
}

void
Database::syncApplicationDescriptor(ObserverSessionI* session, const ApplicationDescriptor& newDesc)
{
    ServerEntrySeq entries;
    int serial;
    ApplicationUpdateDescriptor update;
    {
	Lock sync(*this);
	checkSessionLock(session);

	StringApplicationDescriptorDict::const_iterator p = _descriptors.find(newDesc.name);
	if(p == _descriptors.end())
	{
	    throw ApplicationNotExistException(newDesc.name);
	}

	ApplicationHelper previous(p->second);
	ApplicationHelper helper(newDesc);
	update = helper.diff(previous);
	
	checkForUpdate(previous, helper);
	
	reload(previous, helper, entries);
	
	_descriptors.put(make_pair(newDesc.name, newDesc));

	serial = ++_serial;
    }

    //
    // Notify the observers.
    //
    _registryObserver->applicationUpdated(serial, update);

    if(_traceLevels->application > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->applicationCat);
	out << "synced application `" << newDesc.name << "'";
    }

    for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::sync));
}

void
Database::removeApplicationDescriptor(ObserverSessionI* session, const std::string& name)
{
    ServerEntrySeq entries;
    int serial;
    {
	Lock sync(*this);
	checkSessionLock(session);

	StringApplicationDescriptorDict::iterator p = _descriptors.find(name);
	if(p == _descriptors.end())
	{
	    throw ApplicationNotExistException(name);
	}
	
	ApplicationHelper helper(p->second);
	unload(helper, entries);
	
	_descriptors.erase(p);

	serial = ++_serial;
    }

    //
    // Notify the observers
    //
    _registryObserver->applicationRemoved(serial, name);

    if(_traceLevels->application > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->applicationCat);
	out << "removed application `" << name << "'";
    }

    for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::sync));
}

ApplicationDescriptor
Database::getApplicationDescriptor(const std::string& name)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringApplicationDescriptorDict descriptors(connection, _descriptorDbName); 
    
    StringApplicationDescriptorDict::const_iterator p = descriptors.find(name);
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
    //
    // We must notify the observer first (there's an assert in the
    // observer to ensure that only nodes which are up are teared
    // down).
    //
    _nodeObserver->nodeDown(name);
    _nodeCache.get(name)->setSession(0);
}

Ice::StringSeq 
Database::getAllNodes(const string& expression)
{
    return _nodeCache.getAll(expression);
}

ServerInfo
Database::getServerInfo(const std::string& id)
{
    return _serverCache.get(id)->getServerInfo();
}

ServerPrx
Database::getServer(const string& id)
{
    int activationTimeout, deactivationTimeout;
    string node;
    return getServerWithTimeouts(id, activationTimeout, deactivationTimeout, node);
}

ServerPrx
Database::getServerWithTimeouts(const string& id, int& activationTimeout, int& deactivationTimeout, string& node)
{
    return _serverCache.get(id)->getProxy(activationTimeout, deactivationTimeout, node);
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
Database::setAdapterDirectProxy(const string& serverId, const string& adapterId, const Ice::ObjectPrx& proxy)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxiesDict adapters(connection, _adapterDbName); 
    if(proxy)
    {
	StringObjectProxiesDict::iterator p = adapters.find(adapterId);
	if(p != adapters.end())
	{
	    StringObjectProxyDict proxies = p->second;
	    proxies[serverId] = proxy;
	    p.set(proxies);

	    if(_traceLevels->adapter > 0)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
		out << "added adapter `" << adapterId << "'";
	    }
	}
	else
	{
	    StringObjectProxyDict proxies;
	    proxies[serverId] = proxy;
	    adapters.put(make_pair(adapterId, proxies));

	    if(_traceLevels->adapter > 0)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
		out << "updated adapter `" << adapterId << "'";
	    }
	}
    }
    else
    {
	StringObjectProxiesDict::iterator p = adapters.find(adapterId);
	if(p != adapters.end())
	{
	    StringObjectProxyDict proxies = p->second;
	    if(proxies.erase(serverId) == 0)
	    {
		ServerNotExistException ex;
		ex.id = serverId;
		throw ex;
	    }

	    if(proxies.empty())
	    {
		adapters.erase(p);
	    }
	    else
	    {
		p.set(proxies);
	    }
	}
	else
	{
	    AdapterNotExistException ex;
	    ex.id = adapterId;
	    throw ex;
	}
    }
}

Ice::ObjectPrx
Database::getAdapterDirectProxy(const string& serverId, const string& adapterId)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxiesDict adapters(connection, _adapterDbName); 
    StringObjectProxiesDict::const_iterator p = adapters.find(adapterId);
    if(p != adapters.end())
    {
	StringObjectProxyDict::const_iterator q = p->second.find(serverId);
	if(q != p->second.end())
	{
	    return q->second;
	}
    }
    return 0;
}

void
Database::removeAdapter(const string& adapterId)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxiesDict adapters(connection, _adapterDbName); 
    StringObjectProxiesDict::iterator p = adapters.find(adapterId);
    if(p != adapters.end())
    {
	adapters.erase(p);
    }
    else
    {
	AdapterNotExistException ex;
	ex.id = adapterId;
	throw ex;
    }
}

AdapterPrx
Database::getAdapter(const string& id, const string& serverId)
{
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
    StringObjectProxiesDict adapters(connection, _adapterDbName); 
    StringObjectProxiesDict::const_iterator p = adapters.find(id);
    if(p != adapters.end())
    {
	StringObjectProxyDict::const_iterator q = p->second.find(serverId);
	if(q != p->second.end())
	{
	    Ice::Identity identity;
	    identity.category = "IceGridAdapter";
	    ostringstream os;
	    os << id.size() << "-" << id << "-" << serverId;
	    identity.name = os.str();
	    return AdapterPrx::uncheckedCast(_internalAdapter->createDirectProxy(identity));
	}
	else
	{
	    ServerNotExistException ex;
	    ex.id = serverId;
	    throw ex;
	}
    }

    AdapterNotExistException ex;
    ex.id = id;
    throw ex;
}

vector<pair<string, AdapterPrx> >
Database::getAdapters(const string& id, int& endpointCount)
{
    //
    // First we check if the given adapter id is associated to a
    // server, if that's the case we get the adapter proxy from the
    // server.
    //
    try
    {
	return _adapterCache.get(id)->getProxies(endpointCount);
    }
    catch(const AdapterNotExistException&)
    {
    }

    //
    // Otherwise, we check the adapter endpoint table -- if there's an
    // entry the adapter is managed by the registry itself.
    //
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    StringObjectProxiesDict adapters(connection, _adapterDbName); 
    StringObjectProxiesDict::const_iterator p = adapters.find(id);
    if(p != adapters.end())
    {
	vector<pair<string, AdapterPrx> > adapters;
	for(StringObjectProxyDict::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
	{
	    Ice::Identity identity;
	    identity.category = "IceGridAdapter";
	    ostringstream os;
	    os << id.size() << "-" << id << "-" << q->first;
	    identity.name = os.str();
	    adapters.push_back(
		make_pair(q->first, AdapterPrx::uncheckedCast(_internalAdapter->createDirectProxy(identity))));
	}
	random_shuffle(adapters.begin(), adapters.end());
	endpointCount = adapters.size();
	return adapters;
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
    ids = getMatchingKeys<StringObjectProxiesDict>(_adapters, expression);
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
Database::getObjectProxy(const Ice::Identity& id)
{
    try
    {
	return _objectCache.get(id)->getProxy();
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
    if(_adapterCache.has(id) || _adapters.find(id) != _adapters.end())
    {
	DeploymentException ex;
	ex.reason = "adapter `" + id + "' is already registered"; 
	throw ex;
    }
}

void
Database::checkObjectForAddition(const Ice::Identity& objectId)
{
    if(_objectCache.has(objectId) || _objects.find(objectId) != _objects.end())
    {
	DeploymentException ex;
	ex.reason = "object `" + Ice::identityToString(objectId) + "' is already registered"; 
	throw ex;
    }
}

void
Database::load(const ApplicationHelper& app, ServerEntrySeq& entries)
{
    const NodeDescriptorDict& nodes = app.getInstance().nodes;
    for(NodeDescriptorDict::const_iterator n = nodes.begin(); n != nodes.end(); ++n)
    {
	_nodeCache.get(n->first, true)->addDescriptor(app.getInstance().name, n->second);	
    }

    const ReplicatedAdapterDescriptorSeq& adpts = app.getInstance().replicatedAdapters;
    for(ReplicatedAdapterDescriptorSeq::const_iterator r = adpts.begin(); r != adpts.end(); ++r)
    {
	assert(!r->id.empty());
	_adapterCache.get(r->id, true)->enableReplication(r->loadBalancing);
	for(ObjectDescriptorSeq::const_iterator o = r->objects.begin(); o != r->objects.end(); ++o)
	{
	    _objectCache.add(r->id, "", *o);
	}
    }

    map<string, ServerInfo> servers = app.getServerInfos();
    for(map<string, ServerInfo>::const_iterator p = servers.begin(); p != servers.end(); ++p)
    {
	entries.push_back(_serverCache.add(p->second));
    }
}

void
Database::unload(const ApplicationHelper& app, ServerEntrySeq& entries)
{
    const NodeDescriptorDict& nodes = app.getInstance().nodes;
    for(NodeDescriptorDict::const_iterator n = nodes.begin(); n != nodes.end(); ++n)
    {
	_nodeCache.get(n->first)->removeDescriptor(app.getInstance().name);
    }

    const ReplicatedAdapterDescriptorSeq& adpts = app.getInstance().replicatedAdapters;
    for(ReplicatedAdapterDescriptorSeq::const_iterator r = adpts.begin(); r != adpts.end(); ++r)
    {
	for(ObjectDescriptorSeq::const_iterator o = r->objects.begin(); o != r->objects.end(); ++o)
	{
	    _objectCache.remove(o->id);
	}
	_adapterCache.get(r->id, false)->disableReplication();
    }

    map<string, ServerInfo> servers = app.getServerInfos();
    for(map<string, ServerInfo>::const_iterator p = servers.begin(); p != servers.end(); ++p)
    {
	entries.push_back(_serverCache.remove(p->first));
    }
}

void
Database::reload(const ApplicationHelper& oldApp, const ApplicationHelper& newApp, ServerEntrySeq& entries)
{
    const NodeDescriptorDict& oldNodes = oldApp.getInstance().nodes;
    NodeDescriptorDict::const_iterator n;
    for(n = oldNodes.begin(); n != oldNodes.end(); ++n)
    {
	_nodeCache.get(n->first)->removeDescriptor(oldApp.getInstance().name);
    }
    const NodeDescriptorDict& newNodes = newApp.getInstance().nodes;
    for(n = newNodes.begin(); n != newNodes.end(); ++n)
    {
	_nodeCache.get(n->first, true)->addDescriptor(newApp.getInstance().name, n->second);
    }

    //
    // Unload/load replicated adapters.
    //
    const ReplicatedAdapterDescriptorSeq& oldAdpts = oldApp.getInstance().replicatedAdapters;
    ReplicatedAdapterDescriptorSeq::const_iterator r;
    for(r = oldAdpts.begin(); r != oldAdpts.end(); ++r)
    {
	_adapterCache.get(r->id, false)->disableReplication();
    }
    const ReplicatedAdapterDescriptorSeq& newAdpts = newApp.getInstance().replicatedAdapters;
    for(r = newAdpts.begin(); r != newAdpts.end(); ++r)
    {
	_adapterCache.get(r->id, true)->enableReplication(r->loadBalancing);
    }

    map<string, ServerInfo> oldServers = oldApp.getServerInfos();
    map<string, ServerInfo> newServers = newApp.getServerInfos();

    //
    // Unload updated and removed servers and keep track of added and
    // updated servers to reload them after.
    //
    vector<ServerInfo> load;
    map<string, ServerInfo>::const_iterator p;
    for(p = newServers.begin(); p != newServers.end(); ++p)
    {
	map<string, ServerInfo>::const_iterator q = oldServers.find(p->first);
	if(q == oldServers.end())
	{
	    load.push_back(p->second);
	} 
	else if(p->second.node != q->second.node ||
		ServerHelper(p->second.descriptor) != ServerHelper(q->second.descriptor))
	{
	    entries.push_back(_serverCache.remove(p->first, false)); // Don't destroy the server if it was updated.
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
    // Load added servers and reload updated ones.
    //
    for(vector<ServerInfo>::const_iterator q = load.begin(); q != load.end(); ++q)
    {
	entries.push_back(_serverCache.add(*q));
    }
}
