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

#include <algorithm>
#include <functional>
#include <iterator>

using namespace std;
using namespace IceGrid;

const string Database::_descriptorDbName = "applications";
const string Database::_adapterDbName = "adapters";
const string Database::_objectDbName = "objects";
pointer_to_unary_function<int, int> Database::_rand(IceUtil::random);

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
	return _database->getAdapterDirectProxy(current.id.name);
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

Database::Database(const Ice::ObjectAdapterPtr& adapter,
		   const string& envName,
		   const string& instanceName,
		   int nodeSessionTimeout,
		   const TraceLevelsPtr& traceLevels) :
    _communicator(adapter->getCommunicator()),
    _internalAdapter(adapter),
    _envName(envName),
    _instanceName(instanceName),
    _traceLevels(traceLevels), 
    _nodeCache(nodeSessionTimeout),
    _objectCache(_communicator),
    _serverCache(_nodeCache, _adapterCache, _objectCache),
    _connection(Freeze::createConnection(adapter->getCommunicator(), envName)),
    _descriptors(_connection, _descriptorDbName),
    _objects(_connection, _objectDbName),
    _adapters(_connection, _adapterDbName),
    _lock(0), 
    _serial(0)
{
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
	
    _serverCache.setTraceLevels(_traceLevels);
    _nodeCache.setTraceLevels(_traceLevels);
    _adapterCache.setTraceLevels(_traceLevels);
    _objectCache.setTraceLevels(_traceLevels);

    //
    // Register a default servant to manage manually registered object adapters.
    //
    // NOTE: This must be done only once we're sure this constructor
    // won't throw. The servant locator is holding a handle on this
    // object and if an exception was thrown a bogus database object
    // won't be referenced from the servant locator.
    //
    _internalAdapter->addServantLocator(new AdapterServantLocator(this), "IceGridAdapter");
}

Database::~Database()
{
}

void
Database::destroy()
{
    _nodeCache.destroy(); // Break cyclic reference count.
}

std::string
Database::getInstanceName() const
{
    return _instanceName;
}

void
Database::setObservers(const RegistryObserverPrx& registryObserver, const NodeObserverPrx& nodeObserver)
{
    int serial;
    ApplicationDescriptorSeq applications;
    AdapterInfoSeq adapters;
    ObjectInfoSeq objects;
    {
	Lock sync(*this);
	_registryObserver = registryObserver;
	_nodeObserver = nodeObserver;
	serial = _serial;

	for(StringApplicationDescriptorDict::const_iterator p = _descriptors.begin(); p != _descriptors.end(); ++p)
	{
	    applications.push_back(p->second);
	}

	for(StringAdapterInfoDict::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
	{
	    adapters.push_back(p->second);
	    if(adapters.back().id.empty())
	    {
		adapters.back().id = p->first;
	    }
	}
	
	for(IdentityObjectInfoDict::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
	{
	    objects.push_back(p->second);
	}
    }

    //
    // Notify the observers.
    //
    _registryObserver->init(serial, applications, adapters, objects);
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
    DistributionDescriptor appDistrib;
    map<string, DistributionDescriptorDict> nodeDistrib;

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
	_descriptors.put(StringApplicationDescriptorDict::value_type(desc.name, desc));
	
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
	
	_descriptors.put(StringApplicationDescriptorDict::value_type(update.name, helper.getDescriptor()));

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
	
	_descriptors.put(StringApplicationDescriptorDict::value_type(newDesc.name, newDesc));

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

	try
	{
	    ApplicationHelper helper(p->second);
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

void
Database::instantiateServer(const string& application, const string& node, const ServerInstanceDescriptor& instance)
{
    ServerEntrySeq entries;
    int serial;
    ApplicationUpdateDescriptor update;
    {
	Lock sync(*this);	
	checkSessionLock(0);

	StringApplicationDescriptorDict::const_iterator p = _descriptors.find(application);
	if(p == _descriptors.end())
	{
	    throw ApplicationNotExistException(application);
	}

	ApplicationHelper previous(p->second);
	ApplicationHelper helper(p->second);
	helper.instantiateServer(node, instance);
	update = helper.diff(previous);

	checkForUpdate(previous, helper);
	
	reload(previous, helper, entries);
	
	_descriptors.put(StringApplicationDescriptorDict::value_type(application, helper.getDescriptor()));

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

NodeInfo
Database::getNodeInfo(const string& name) const
{
    return _nodeCache.get(name)->getInfo();
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
Database::getServerInfo(const std::string& id, bool resolve)
{
    return _serverCache.get(id)->getServerInfo(resolve);
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

	serial = ++_serial;
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
	    _registryObserver->adapterUpdated(serial, info);
	}
	else
	{
	    _registryObserver->adapterAdded(serial, info);
	}
    }
    else
    {
	_registryObserver->adapterRemoved(serial, adapterId);
    }
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

	Freeze::TransactionHolder txHolder(_connection);

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
	    serial = ++_serial;
	}
	else
	{
	    serial = _serial;
	    _serial += infos.size();
	}
    }

    if(_traceLevels->adapter > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->adapterCat);
	out << "removed " << (infos.empty() ? "adapter" : "replica group") << " `" << adapterId << "'";
    }
    
    if(infos.empty())
    {
	_registryObserver->adapterRemoved(serial, adapterId);
    }
    else
    {
	for(AdapterInfoSeq::const_iterator p = infos.begin(); p != infos.end(); ++p)
	{
	    _registryObserver->adapterUpdated(++serial, *p);
	}
    }
}

AdapterPrx
Database::getAdapter(const string& id, const string& replicaGroupId)
{
    return _adapterCache.getServerAdapter(id, false)->getProxy(replicaGroupId);
}

vector<pair<string, AdapterPrx> >
Database::getAdapters(const string& id, bool allRegistered, int& endpointCount)
{
    //
    // First we check if the given adapter id is associated to a
    // server, if that's the case we get the adapter proxy from the
    // server.
    //
    try
    {
	return _adapterCache.get(id)->getProxies(allRegistered, endpointCount);
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
	random_shuffle(adpts.begin(), adpts.end(), _rand);
	endpointCount = static_cast<int>(adpts.size());
	return adpts;
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
    const Ice::Identity id = info.proxy->ice_getIdentity();
    {
	Lock sync(*this);	
	if(_objectCache.has(id))
	{
	    throw ObjectExistsException(id);
	}
	
	if(_objects.find(id) != _objects.end())
	{
	    throw ObjectExistsException(id);
	}
	_objects.put(IdentityObjectInfoDict::value_type(id, info));
	
	serial = ++_serial;
    }
	
    //
    // Notify the observers.
    //
    _registryObserver->objectAdded(serial, info);

    if(_traceLevels->object > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
	out << "added object `" << Ice::identityToString(id) << "'";
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
	    ex.reason = "removing object `" + Ice::identityToString(id) + "' is not allowed:\n";
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

	serial = ++_serial;
    }

    //
    // Notify the observers.
    //
    _registryObserver->objectRemoved(serial, id);

    if(_traceLevels->object > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
	out << "removed object `" << Ice::identityToString(id) << "'";
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
	    ex.reason = "updating object `" + Ice::identityToString(id) + "' is not allowed:\n";
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

	serial = ++_serial;
    }

    //
    // Notify the observers.
    //
    _registryObserver->objectUpdated(serial, info);

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
    return objs[IceUtil::random(objs.size())];
}

Ice::ObjectPrx
Database::getObjectByTypeOnLeastLoadedNode(const string& type, LoadSample sample)
{
    Ice::ObjectProxySeq objs = getObjectsByType(type);
    random_shuffle(objs.begin(), objs.end(), _rand);
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
    const string application = app.getInstance().name;
    for(NodeDescriptorDict::const_iterator n = nodes.begin(); n != nodes.end(); ++n)
    {
	_nodeCache.get(n->first, true)->addDescriptor(application, n->second);	
    }

    const ReplicaGroupDescriptorSeq& adpts = app.getInstance().replicaGroups;
    for(ReplicaGroupDescriptorSeq::const_iterator r = adpts.begin(); r != adpts.end(); ++r)
    {
	assert(!r->id.empty());
	_adapterCache.getReplicaGroup(r->id, true)->set(application, r->loadBalancing);
	for(ObjectDescriptorSeq::const_iterator o = r->objects.begin(); o != r->objects.end(); ++o)
	{
	    _objectCache.add(application, r->id, "", *o);
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
    map<string, ServerInfo> servers = app.getServerInfos();
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
	_adapterCache.remove(r->id);
    }

    const NodeDescriptorDict& nodes = app.getInstance().nodes;
    const string application = app.getInstance().name;
    for(NodeDescriptorDict::const_iterator n = nodes.begin(); n != nodes.end(); ++n)
    {
	_nodeCache.get(n->first)->removeDescriptor(application);
    }
}

void
Database::reload(const ApplicationHelper& oldApp, const ApplicationHelper& newApp, ServerEntrySeq& entries)
{
    const string application = oldApp.getInstance().name;

    //
    // Remove destroyed servers.
    //
    map<string, ServerInfo> oldServers = oldApp.getServerInfos();
    map<string, ServerInfo> newServers = newApp.getServerInfos();
    vector<ServerInfo> load;
    map<string, ServerInfo>::const_iterator p;
    for(p = newServers.begin(); p != newServers.end(); ++p)
    {
	map<string, ServerInfo>::const_iterator q = oldServers.find(p->first);
	if(q == oldServers.end())
	{
	    load.push_back(p->second);
	} 
	else if(p->second.node != q->second.node || !descriptorEqual(p->second.descriptor, q->second.descriptor))
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
	    _adapterCache.remove(r->id);
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
	_adapterCache.getReplicaGroup(r->id, true)->set(application, r->loadBalancing);
	for(ObjectDescriptorSeq::const_iterator o = r->objects.begin(); o != r->objects.end(); ++o)
	{
	    _objectCache.add(application, r->id, "", *o);
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
