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

struct AddComponent : std::unary_function<ComponentDescriptorPtr&, void>
{
    AddComponent(Database& database, const Database::ServerEntryPtr& entry) : _database(database), _entry(entry)
    {
    }

    void
    operator()(const ComponentDescriptorPtr& desc)
    {
	_database.addComponent(_entry, desc);
    }

    Database& _database;
    const Database::ServerEntryPtr _entry;
};

struct AddAdapterId : std::unary_function<ComponentDescriptorPtr&, void>
{
    AddAdapterId(set<string>& ids) : _ids(ids)
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
		ex.reason = "invalid descriptor: empty adapter id for adapter `" + p->name + "' in `" + desc->name 
		    + "'";
		throw ex;
	    }
	    if(!_ids.insert(p->id).second)
	    {
		DeploymentException ex;
		ex.reason = "invalid descriptor: duplicated adapter id `" + p->id + "'";
		throw ex;
	    }
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
		if(!q->proxy)
		{
		    DeploymentException ex;
		    ex.reason = "invalid descriptor: object proxy is null in `" + desc->name + "'";
		    throw ex;
		}
		if(!_ids.insert(q->proxy->ice_getIdentity()).second)
		{
		    DeploymentException ex;
		    ex.reason = "invalid descriptor: duplicated object id `" + 
			Ice::identityToString(q->proxy->ice_getIdentity()) + "'";
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
    // Cache the servers.
    //
    for(StringApplicationDescriptorDict::const_iterator p = _descriptors.begin(); p != _descriptors.end(); ++p)
    {
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

	set<string> adapterIds;
	for_each(descriptor->servers.begin(), descriptor->servers.end(), forEachComponent(AddAdapterId(adapterIds)));
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
    for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&Database::ServerEntry::sync));
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

    for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&Database::ServerEntry::sync));
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

    for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&Database::ServerEntry::sync));
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
    // Ensure that the new application adapters aren't already
    // registered.
    //
    set<string> oldAdpts;
    set<string> newAdpts;
    for_each(origDesc->servers.begin(), origDesc->servers.end(), forEachComponent(AddAdapterId(oldAdpts)));
    for_each(newDesc->servers.begin(), newDesc->servers.end(), forEachComponent(AddAdapterId(newAdpts)));

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

    for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&Database::ServerEntry::sync));
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
Database::addNode(const string& name, const NodeSessionIPtr& node)
{
    ServerEntrySeq entries;    
    {
	Lock sync(*this);

	if(_nodes.find(name) != _nodes.end())
	{
	    throw NodeActiveException();
	}

	if(_traceLevels->node > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->nodeCat);
	    out << "added node `" << name << "'";
	}

	_nodes.insert(make_pair(name, node));

	//
	// Get all the node servers and see if they need to be synced.
	//
	map<string, set<string> >::const_iterator p = _serversByNode.find(name);
	if(p == _serversByNode.end())
	{
	    return;
	}
	for(set<string>::const_iterator q = p->second.begin() ; q != p->second.end(); ++q)
	{
	    ServerEntryPtr entry = _servers[*q];
	    assert(entry);
	    if(entry->needsSync())
	    {
		entries.push_back(entry);
	    }
	}
    }

    for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&Database::ServerEntry::sync));
}

NodePrx 
Database::getNode(const string& name) const
{
    Lock sync(*this);

    map<string, NodeSessionIPtr>::const_iterator p = _nodes.find(name);
    if(p == _nodes.end())
    {
	if(_serversByNode.find(name) == _serversByNode.end())
	{
	    throw NodeNotExistException();
	}
	else
	{
	    throw NodeUnreachableException();
	}
    }
    return p->second->getNode();
}

void 
Database::removeNode(const string& name)
{
    {
	Lock sync(*this);
	if(_nodes.erase(name) > 0)
	{
	    if(_traceLevels->node > 0)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->nodeCat);
		out << "removed node `" << name << "'";
	    }
	}
    }

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
    Lock sync(*this);
    set<string> nodes;
    Ice::StringSeq r = getMatchingKeys<map<string, set<string> > >(_serversByNode, expression);
    nodes.insert(r.begin(), r.end());
    r = getMatchingKeys<map<string, NodeSessionIPtr> >(_nodes, expression);
    nodes.insert(r.begin(), r.end());
    return Ice::StringSeq(nodes.begin(), nodes.end());
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
    Lock sync(*this);
    map<string, string>::const_iterator p = _applicationsByServerName.find(name);
    if(p == _applicationsByServerName.end())
    {
	ServerNotExistException ex;
	ex.name = name;
	throw ex;
    }

    return p->second;
}

ServerPrx
Database::getServer(const string& name)
{
    ServerEntryPtr entry;
    {
	Lock sync(*this);
	map<string, ServerEntryPtr>::const_iterator p = _servers.find(name);
	if(p != _servers.end())
	{
	    entry = p->second;
	}
    }
    if(!entry)
    {
	ServerNotExistException ex;
	ex.name = name;
	throw ex;
    }
    return entry->getProxy();
}

Ice::StringSeq
Database::getAllServers(const string& expression)
{
    Lock sync(*this);
    return getMatchingKeys<map<string, ServerEntryPtr> >(_servers, expression);
}

Ice::StringSeq
Database::getAllNodeServers(const string& node)
{
    Lock sync(*this);
    map<string, set<string> >::const_iterator p = _serversByNode.find(node);
    if(p == _serversByNode.end())
    {
	return Ice::StringSeq();
    }
    return Ice::StringSeq(p->second.begin(), p->second.end());
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
Database::getAdapter(const string& id)
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
    ServerEntryPtr entry;
    {
	Lock sync(*this);
	map<string, ServerEntryPtr>::const_iterator p = _serversByAdapterId.find(id);
	if(p != _serversByAdapterId.end())
	{
	    entry = p->second;
	}
    }
    if(entry)
    {
	return entry->getAdapter(id);
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
    vector<string> ids;
    ids = getMatchingKeys<map<string, ServerEntryPtr> >(_serversByAdapterId, expression);
    result.swap(ids);
    ids = getMatchingKeys<StringObjectProxyDict>(_adapters, expression);
    result.insert(result.end(), ids.begin(), ids.end());
    return result;
}

void
Database::addObjectDescriptor(const ObjectDescriptor& object)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectDescDict objects(connection, _objectDbName); 
    const Ice::Identity id = object.proxy->ice_getIdentity();
    if(objects.find(id) != objects.end())
    {
	ObjectExistsException ex;
	ex.id = id;
	throw ex;
    }
    objects.put(make_pair(id, object));

    if(_traceLevels->object > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
	out << "added object `" << Ice::identityToString(id) << "'";
    }
}

void
Database::removeObjectDescriptor(const Ice::Identity& id)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectDescDict objects(connection, _objectDbName); 
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
Database::updateObjectDescriptor(const Ice::ObjectPrx& proxy)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectDescDict objects(connection, _objectDbName); 
    const Ice::Identity id = proxy->ice_getIdentity();
    IdentityObjectDescDict::iterator p = objects.find(id);
    if(p == objects.end())
    {
	ObjectNotExistException ex;
	ex.id = id;
	throw ex;
    }
    ObjectDescriptor desc = p->second;
    desc.proxy = proxy;
    p.set(desc);

    if(_traceLevels->object > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
	out << "updated object `" << Ice::identityToString(id) << "'";
    }
}

ObjectDescriptor
Database::getObjectDescriptor(const Ice::Identity& id)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectDescDict objects(connection, _objectDbName);    
    IdentityObjectDescDict::const_iterator p = objects.find(id);
    if(p == objects.end())
    {
	ObjectNotExistException ex;
	ex.id = id;
	throw ex;
    }
    return p->second;
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
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectDescDict objects(connection, _objectDbName);    
    Ice::ObjectProxySeq proxies;
    for(IdentityObjectDescDict::const_iterator p = objects.findByType(type); p != objects.end(); ++p)
    {
	proxies.push_back(p->second.proxy);
    }
    if(proxies.empty())
    {
	throw ObjectNotExistException();
    }
    return proxies;
}

ObjectDescriptorSeq
Database::getAllObjectDescriptors(const string& expression)
{
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, _envName);
    IdentityObjectDescDict objects(connection, _objectDbName); 
    ObjectDescriptorSeq descriptors;
    for(IdentityObjectDescDict::const_iterator p = objects.begin(); p != objects.end(); ++p)
    {
	if(expression.empty() ||
	   IceUtil::match(Ice::identityToString(p->second.proxy->ice_getIdentity()), expression, true))
	{
	    descriptors.push_back(p->second);
	}
    }
    return descriptors;
}

void
Database::checkServerForAddition(const string& name)
{
    if(_servers.find(name) != _servers.end())
    {
	ServerExistsException ex;
	ex.name = name;
	throw ex;
    }
}

void
Database::checkAdapterForAddition(const string& id)
{
    if(_serversByAdapterId.find(id) != _serversByAdapterId.end() || _adapters.find(id) != _adapters.end())
    {
	AdapterExistsException ex;
	ex.id = id;
	throw ex;
    }
}

void
Database::checkObjectForAddition(const Ice::Identity& objectId)
{
    if(_objects.find(objectId) != _objects.end())
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

Database::ServerEntryPtr
Database::addServer(const string& application, const ServerInstanceDescriptor& instance)
{
    ServerEntryPtr entry;
    map<string, ServerEntryPtr>::const_iterator q = _servers.find(instance.descriptor->name);
    if(q != _servers.end())
    {
	entry = q->second;
	entry->update(instance);
    }
    else
    {
	entry = new ServerEntry(*this, instance);
	_servers.insert(make_pair(instance.descriptor->name, entry));
    }
    
    map<string, set<string> >::iterator p = _serversByNode.find(instance.node);
    if(p == _serversByNode.end())
    {
	p = _serversByNode.insert(make_pair(instance.node, set<string>())).first;
    }
    p->second.insert(p->second.begin(), instance.descriptor->name);

    _applicationsByServerName.insert(make_pair(instance.descriptor->name, application));

    forEachComponent(AddComponent(*this, entry))(instance);
    return entry;
}

Database::ServerEntryPtr
Database::updateServer(const ServerInstanceDescriptor& instance)
{
    //
    // Get the server entry and the current descriptor then check
    // if the server descriptor really changed.	
    //
    ServerEntryPtr entry;
    map<string, ServerEntryPtr>::const_iterator q = _servers.find(instance.descriptor->name);
    assert(q != _servers.end());

    entry = q->second;
    ServerInstanceDescriptor old = entry->getDescriptor();

    //
    // If the node changed, move the server from the old node to the
    // new one.
    //
    if(old.node != instance.node)
    {
	map<string, set<string> >::iterator p = _serversByNode.find(old.node);
	assert(p != _serversByNode.end());
	p->second.erase(instance.descriptor->name);
	if(p->second.empty())
	{
	    _serversByNode.erase(p);
	}
	p = _serversByNode.find(instance.node);
	if(p == _serversByNode.end())
	{
	    p = _serversByNode.insert(make_pair(instance.node, set<string>())).first;
	}
	p->second.insert(p->second.begin(), instance.descriptor->name);
    }
    
    //
    // Remove the object adapters and objects from the old descriptor. 
    //
    forEachComponent(objFunc(*this, &Database::removeComponent))(old);
    
    //
    // Update the server entry.
    //
    entry->update(instance);

    //
    // Add the new object adapters and objects.
    //
    forEachComponent(AddComponent(*this, entry))(instance);
    return entry;
}

Database::ServerEntryPtr
Database::removeServer(const string& application, const ServerInstanceDescriptor& instance)
{
    ServerEntryPtr entry;
    map<string, ServerEntryPtr>::iterator q = _servers.find(instance.descriptor->name);
    assert(q != _servers.end());
    
    map<string, set<string> >::iterator p = _serversByNode.find(instance.node);
    assert(p != _serversByNode.end());
    p->second.erase(instance.descriptor->name);
    if(p->second.empty())
    {
	_serversByNode.erase(p);
    }
    
    entry = q->second;
    entry->destroy();

    _applicationsByServerName.erase(instance.descriptor->name);
    
    //
    // Remove the object adapters and objects.
    //
    forEachComponent(objFunc(*this, &Database::removeComponent))(instance);
    return entry;
}

void
Database::clearServer(const std::string& name)
{
    Lock sync(*this);
    map<string, ServerEntryPtr>::iterator p = _servers.find(name);
    if(p != _servers.end())
    {
	if(p->second->canRemove())
	{
	    _servers.erase(p);
	}
    }
}

void
Database::addComponent(const ServerEntryPtr& entry, const ComponentDescriptorPtr& component)
{
    for(AdapterDescriptorSeq::const_iterator q = component->adapters.begin() ; q != component->adapters.end(); ++q)
    {
	_serversByAdapterId.insert(make_pair(q->id, entry));
	for(ObjectDescriptorSeq::const_iterator r = q->objects.begin(); r != q->objects.end(); ++r)
	{
	    _objects.put(make_pair(r->proxy->ice_getIdentity(), *r));
	}
    }
}

void
Database::removeComponent(const ComponentDescriptorPtr& component)
{
    for(AdapterDescriptorSeq::const_iterator q = component->adapters.begin() ; q != component->adapters.end(); ++q)
    {
	_serversByAdapterId.erase(q->id);
	for(ObjectDescriptorSeq::const_iterator r = q->objects.begin(); r != q->objects.end(); ++r)
	{
	    _objects.erase(r->proxy->ice_getIdentity());
	}
    }
}

Database::ServerEntry::ServerEntry(Database& database, const ServerInstanceDescriptor& descriptor) :
    _database(database), 
    _synchronizing(false)
{
    _load.reset(new ServerInstanceDescriptor());
    *_load = descriptor;
}

void
Database::ServerEntry::sync()
{
    map<string, AdapterPrx> adapters;
    try
    {
	sync(adapters);
    }
    catch(const NodeUnreachableException&)
    {
    }
}

bool
Database::ServerEntry::needsSync() const
{
    Lock sync(*this);
    return _failed;
}

void
Database::ServerEntry::update(const ServerInstanceDescriptor& instance)
{
    Lock sync(*this);

    auto_ptr<ServerInstanceDescriptor> descriptor(new ServerInstanceDescriptor());
    *descriptor = instance;

    if(_loaded.get() && descriptor->node != _loaded->node)
    {
	assert(!_destroy.get());
	_destroy = _loaded;
    }
    else if(_load.get() && descriptor->node != _load->node)
    {
	assert(!_destroy.get());
	_destroy = _load;
    }

    _load = descriptor;
    _loaded.reset(0);
    _proxy = 0;
    _adapters.clear();
}

void
Database::ServerEntry::destroy()
{
    Lock sync(*this);
    if(_loaded.get())
    {
	assert(!_destroy.get());
	_destroy = _loaded;
    }
    else if(_load.get())
    {
	assert(!_destroy.get());
	_destroy = _load;
    }

    _load.reset(0);
    _loaded.reset(0);
    _proxy = 0;
    _adapters.clear();
}

ServerInstanceDescriptor
Database::ServerEntry::getDescriptor()
{
    Lock sync(*this);
    if(_proxy)
    {
	return *_loaded.get();
    }
    else 
    {
	return *_load.get();
    }
}

ServerPrx
Database::ServerEntry::getProxy()
{
    ServerPrx proxy;
    {
	Lock sync(*this);
	if(_proxy) // Synced
	{
	    proxy = _proxy;
	}
    }

    if(proxy)
    {
	try
	{
	    proxy->ice_ping();
	    return proxy;
	}
	catch(const Ice::LocalException& ex)
	{
	}
    }

    StringAdapterPrxDict adapters;
    return sync(adapters);
}

AdapterPrx
Database::ServerEntry::getAdapter(const string& id)
{
    AdapterPrx proxy;
    {
	Lock sync(*this);
	if(_proxy) // Synced
	{
	    proxy = _adapters[id];
	}
    }

    if(proxy)
    {
	try
	{
	    proxy->ice_ping();
	    return proxy;
	}
	catch(const Ice::LocalException& ex)
	{
	}
    }

    StringAdapterPrxDict adapters;
    sync(adapters);
    return adapters[id];
}

ServerPrx
Database::ServerEntry::sync(map<string, AdapterPrx>& adapters)
{
    ServerDescriptorPtr load;
    string loadNode;
    ServerDescriptorPtr destroy;
    string destroyNode;
    {
	Lock sync(*this);
	while(_synchronizing)
	{
	    wait();
	}

	if(!_load.get() && !_destroy.get())
	{
	    _load = _loaded; // Re-load the current server.
	}

	_synchronizing = true;
	_failed = false;
	if(_load.get())
	{
	    load = _load->descriptor;
	    loadNode = _load->node;
	}
	if(_destroy.get())
	{
	    destroy = _destroy->descriptor;
	    destroyNode = _destroy->node;
	}
    }

    ServerPrx proxy;
    try
    {
	if(destroy)
	{
	    try
	    {
		_database.getNode(destroyNode)->destroyServer(destroy->name);
	    }
	    catch(const NodeNotExistException& ex)
	    {
		if(!load)
		{
		    throw NodeUnreachableException();
		}
	    }
	    catch(const Ice::LocalException& ex)
	    {
		if(!load)
		{
		    throw NodeUnreachableException();
		}
	    }
	}

	if(load)
	{
	    try
	    {
		proxy = _database.getNode(loadNode)->loadServer(load, adapters);
	    }
	    catch(const NodeNotExistException& ex)
	    {
		throw NodeUnreachableException();
	    }
	    catch(const DeploymentException& ex)
	    {
		// TODO: Warning
	    }
	    catch(const Ice::LocalException& ex)
	    {
		throw NodeUnreachableException();
	    }
	}
    }
    catch(const NodeUnreachableException& ex)
    {
	{
	    Lock sync(*this);
	    _synchronizing = false;
	    _destroy.reset(0);
	    _failed = true;
	    notifyAll();
	}
	if(!load && destroy)
	{
	    _database.clearServer(destroy->name);
	}
	throw;
    }

    {
	Lock sync(*this);
	_synchronizing = false;
	_loaded = _load;
	_load.reset(0);
	_destroy.reset(0);
	_proxy = proxy ? ServerPrx::uncheckedCast(proxy->ice_timeout(_database._nodeSessionTimeout)) : ServerPrx();
	_adapters.clear();
	for(StringAdapterPrxDict::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
	{
	    AdapterPrx adapter = AdapterPrx::uncheckedCast(p->second->ice_timeout(_database._nodeSessionTimeout));
	    _adapters.insert(make_pair(p->first, adapter));
	}
	notifyAll();
    }
    if(!load && destroy)
    {
	_database.clearServer(destroy->name);
    }
    return proxy;
}

bool
Database::ServerEntry::canRemove()
{
     Lock sync(*this);
     return !_loaded.get() && !_load.get() && !_destroy.get();
}
