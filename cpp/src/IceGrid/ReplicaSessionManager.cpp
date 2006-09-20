// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/ReplicaSessionManager.h>
#include <IceGrid/TraceLevels.h>
#include <IceGrid/Database.h>
#include <IceGrid/WellKnownObjectsManager.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

class MasterDatabaseObserverI : public DatabaseObserver, public IceUtil::Mutex
{
public:

    MasterDatabaseObserverI(const ReplicaSessionManager::ThreadPtr& thread,
			    const DatabasePtr& database, 
			    const ReplicaSessionPrx& session) : 
	_thread(thread),
	_database(database),
	_session(session)
    {
    }

    virtual void 
    applicationInit(int, const ApplicationInfoSeq& applications, const Ice::Current& current)
    {
	_database->syncApplications(applications);
	receivedUpdate(ApplicationObserverTopicName, getSerial(current.ctx));
    }

    virtual void 
    applicationAdded(int, const ApplicationInfo& application, const Ice::Current& current)
    {
	string failure;
	try
	{
	    _database->addApplication(application);
	}
	catch(const DeploymentException& ex)
	{
	    ostringstream os;
	    os << ex << ":\n" << ex.reason;
	    failure = os.str();
	}
	receivedUpdate(ApplicationObserverTopicName, getSerial(current.ctx), failure);
    }

    virtual void 
    applicationRemoved(int, const std::string& name, const Ice::Current& current)
    {
	string failure;
	try
	{
	    _database->removeApplication(name);
	}
	catch(const ApplicationNotExistException& ex)
	{
	    ostringstream os;
	    os << ex << ":\napplication: " << ex.name;
	    failure = os.str();
	}
	receivedUpdate(ApplicationObserverTopicName, getSerial(current.ctx), failure);
    }

    virtual void 
    applicationUpdated(int, const ApplicationUpdateInfo& update, const Ice::Current& current)
    {
	string failure;
	try
	{
	    _database->updateApplication(update);
	}
	catch(const DeploymentException& ex)
	{
	    ostringstream os;
	    os << ex << ":\n" << ex.reason;
	    failure = os.str();
	}
	catch(const ApplicationNotExistException& ex)
	{
	    ostringstream os;
	    os << ex << ":\napplication: " << ex.name;
	    failure = os.str();
	}
	receivedUpdate(ApplicationObserverTopicName, getSerial(current.ctx), failure);
    }

    virtual void
    adapterInit(const AdapterInfoSeq& adapters, const Ice::Current& current)
    {
	_database->syncAdapters(adapters);
	receivedUpdate(AdapterObserverTopicName, getSerial(current.ctx));
    }

    virtual void 
    adapterAdded(const AdapterInfo& info, const Ice::Current& current)
    {
	string failure;
	if(!_database->setAdapterDirectProxy(info.id, info.replicaGroupId, info.proxy))
	{
	    failure = "adapter `" + info.id + "' already exists and belongs to an application";
	}
	receivedUpdate(AdapterObserverTopicName, getSerial(current.ctx), failure);
    }

    virtual void 
    adapterUpdated(const AdapterInfo& info, const Ice::Current& current)
    {
	string failure;
	if(!_database->setAdapterDirectProxy(info.id, info.replicaGroupId, info.proxy))
	{
	    failure = "adapter `" + info.id + "' already exists and belongs to an application";
	}
	receivedUpdate(AdapterObserverTopicName, getSerial(current.ctx), failure);
    }

    virtual void 
    adapterRemoved(const std::string& id, const Ice::Current& current)
    {
	string failure;
	if(!_database->setAdapterDirectProxy(id, "", 0))
	{
	    failure = "adapter `" + id + "' already exists and belongs to an application";
	}
	receivedUpdate(AdapterObserverTopicName, getSerial(current.ctx), failure);
    }

    virtual void
    objectInit(const ObjectInfoSeq& objects, const Ice::Current& current)
    {
	_database->syncObjects(objects);
	receivedUpdate(ObjectObserverTopicName, getSerial(current.ctx));
    }

    virtual void 
    objectAdded(const ObjectInfo& info, const Ice::Current& current)
    {
	string failure;
	try
	{
	    const Ice::Identity& id = info.proxy->ice_getIdentity();
	    if(id.category != _database->getInstanceName() || id.name.find("Node-") != 0)
	    {
		// Don't replicate node well-known objects. These objects are 
		// maintained by each replica with each node session.
		_database->addObject(info, true);
	    }
	}
	catch(const ObjectExistsException& ex)
	{
	    ostringstream os;
	    os << ex << ":\n";
	    os << "id: " << info.proxy->ice_getCommunicator()->identityToString(info.proxy->ice_getIdentity());
	    failure = os.str();
	}
	receivedUpdate(ObjectObserverTopicName, getSerial(current.ctx), failure);
    }

    virtual void 
    objectUpdated(const ObjectInfo& info, const Ice::Current& current)
    {
	string failure;
	try
	{
	    const Ice::Identity& id = info.proxy->ice_getIdentity();
	    if(id.category != _database->getInstanceName() || id.name.find("Node-") != 0)
	    {
		// Don't replicate node well-known objects. These objects are 
		// maintained by each replica with each node session.
		_database->addObject(info, true);
	    }
	}
	catch(const DeploymentException& ex)
	{
	    ostringstream os;
	    os << ex << ":\n" << ex.reason;
	    failure = os.str();
	}
	receivedUpdate(ObjectObserverTopicName, getSerial(current.ctx), failure);
    }

    virtual void 
    objectRemoved(const Ice::Identity& id, const Ice::Current& current)
    {
	string failure;
	try
	{
	    if(id.category != _database->getInstanceName() || id.name.find("Node-") != 0)
	    {
		// Don't replicate node well-known objects. These objects are 
		// maintained by each replica with each node session.
		_database->removeObject(id);
	    }
	}
	catch(const DeploymentException& ex)
	{
	    ostringstream os;
	    os << ex << ":\n" << ex.reason;
	    failure = os.str();
	}
	catch(const ObjectNotRegisteredException&)
	{
	}
	receivedUpdate(ObjectObserverTopicName, getSerial(current.ctx), failure);
    }

private:

    int 
    getSerial(const Ice::Context& context)
    {
	Ice::Context::const_iterator p = context.find("serial");
	if(p != context.end())
	{
	    int serial;
	    istringstream is(p->second);
	    is >> serial;
	    return serial;
	}
	return -1;
    }
    
    void 
    receivedUpdate(TopicName name, int serial, const string& failure = string())
    {
	try
	{
	    _session->receivedUpdate(name, serial, failure);
	}
	catch(const Ice::LocalException&)
	{
	}
	if(!failure.empty())
	{
	    _thread->destroyActiveSession();
	}
    }

    const ReplicaSessionManager::ThreadPtr _thread;
    const DatabasePtr _database;
    const ReplicaSessionPrx _session;
};


};

ReplicaSessionManager::ReplicaSessionManager()
{
}

void
ReplicaSessionManager::create(const string& name, 
			      const RegistryInfo& info,
			      const DatabasePtr& database,
			      const WellKnownObjectsManagerPtr& wellKnownObjects,
			      const InternalRegistryPrx& internalRegistry)
{
    Ice::CommunicatorPtr comm = database->getCommunicator();
    string instName = comm->getDefaultLocator()->ice_getIdentity().category;

    {
	Lock sync(*this);
	
	_master = InternalRegistryPrx::uncheckedCast(comm->stringToProxy(instName + "/InternalRegistry-Master"));
	
	_name = name;
	_info = info;
	_internalRegistry = internalRegistry;
	_database = database;
	_wellKnownObjects = wellKnownObjects;
	_traceLevels = _database->getTraceLevels();

	_thread = new Thread(*this, _master);
	_thread->start();
	notifyAll();
    }    

    _thread->tryCreateSession(0);
}

void
ReplicaSessionManager::create(const InternalRegistryPrx& replica)
{
    {
	Lock sync(*this);
	while(!_master) // Wait to be initialized.
	{
	    wait();
	}
    }

    if(replica->ice_getIdentity() != _master->ice_getIdentity())
    {
	_database->getTraceLevels()->logger->error("can only create sessions with the master replica");
	return;
    }

    _thread->tryCreateSession(replica);
}

NodePrxSeq
ReplicaSessionManager::getNodes() const
{
    try
    {
	return _master->getNodes();
    }
    catch(const Ice::LocalException&)
    {
	return _internalRegistry->getNodes();
    }
}

void
ReplicaSessionManager::destroy()
{
    {
	Lock sync(*this);
	if(!_thread)
	{
	    return;
	}
    }

    _thread->terminate();
    _thread->getThreadControl().join();
}

void
ReplicaSessionManager::registerAllWellKnownObjects()
{
    //
    // Try to create the session if it doesn't already exists.
    //
    _thread->tryCreateSession(0);

    //
    // If there's an active session, register the well-known objects
    // with the session.
    //
    ReplicaSessionPrx session = _thread->getSession();
    if(session)
    {
	try
	{
	    _wellKnownObjects->registerAll(session);
	    return;
	}
	catch(const Ice::LocalException&)
	{
	}
    }
}

bool
ReplicaSessionManager::keepAlive(const ReplicaSessionPrx& session)
{
    try
    {
	if(_traceLevels && _traceLevels->replica > 2)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
	    out << "sending keep alive message to master replica";
	}

	session->keepAlive();
	return true;
    }
    catch(const Ice::LocalException& ex)
    {
	if(_traceLevels && _traceLevels->replica > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
	    out << "lost session with master replica:\n" << ex;
	}
	return false;
    }
}

ReplicaSessionPrx
ReplicaSessionManager::createSession(const InternalRegistryPrx& registry, IceUtil::Time& timeout)
{
    try
    {
	if(_traceLevels && _traceLevels->replica > 1)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
	    out << "trying to establish session with master replica";
	}

	ReplicaSessionPrx session = registry->registerReplica(_name, _info, _internalRegistry);
	int t = session->getTimeout();
	if(t > 0)
	{
	    timeout = IceUtil::Time::seconds(t / 2);
	}
	
	//
	// Create a new database observer servant and give its proxy
	// to the session so that it can subscribe it. This call only
	// returns once the observer is subscribed and initialized.
	//
	DatabaseObserverPtr servant = new MasterDatabaseObserverI(_thread, _database, session);
	_observer = DatabaseObserverPrx::uncheckedCast(_database->getInternalAdapter()->addWithUUID(servant));	
	session->setDatabaseObserver(_observer);

	//
	// Register all the well-known objects with the replica session.
	//
	_wellKnownObjects->registerAll(session);

	if(_traceLevels && _traceLevels->replica > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
	    out << "established session with master replica";
	}
	
	return session;
    }
    catch(const ReplicaActiveException&)
    {
	if(_traceLevels)
	{
	    _traceLevels->logger->error("a replica with the same name is already registered and active");
	}
	return 0;
    }
    catch(const Ice::LocalException& ex)
    {
	if(_observer)
	{
	    try
	    {
		_database->getInternalAdapter()->remove(_observer->ice_getIdentity());
	    }
	    catch(const Ice::LocalException&)
	    {
	    }
	    _observer = 0;
	}

	//
	// Re-register all the well known objects with the local database.
	//
	_wellKnownObjects->registerAll();

	if(_traceLevels && _traceLevels->replica > 1)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
	    out << "failed to establish session with master replica:\n" << ex;
	}
	return 0;
    }
}

void
ReplicaSessionManager::destroySession(const ReplicaSessionPrx& session)
{
    try
    {
	session->destroy();

	if(_traceLevels && _traceLevels->replica > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
	    out << "destroyed master replica session";
	}
    }
    catch(const Ice::LocalException& ex)
    {
	if(_traceLevels && _traceLevels->replica > 1)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
	    out << "couldn't destroy master replica session:\n" << ex;
	}
    }

    if(_observer)
    {
	try
	{
	    _database->getInternalAdapter()->remove(_observer->ice_getIdentity());
	}
	catch(const Ice::LocalException&)
	{
	}
	_observer = 0;
    }
}

