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

    MasterDatabaseObserverI(const DatabasePtr& database, ReplicaSessionManager& manager) : 
	_database(database),
	_manager(manager)
    {
    }

    virtual void 
    applicationInit(int, const ApplicationInfoSeq& applications, const Ice::Current&)
    {
	_database->syncApplications(applications);
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
	_manager.receivedUpdate("application", getSerial(current.ctx, "application"), failure);
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
	_manager.receivedUpdate("application", getSerial(current.ctx, "application"), failure);
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
	_manager.receivedUpdate("application", getSerial(current.ctx, "application"), failure);
    }

    virtual void
    adapterInit(const AdapterInfoSeq& adapters, const Ice::Current& current)
    {
	_database->syncAdapters(adapters);
    }

    virtual void 
    adapterAdded(const AdapterInfo& info, const Ice::Current& current)
    {
	string failure;
	if(!_database->setAdapterDirectProxy(info.id, info.replicaGroupId, info.proxy))
	{
	    failure = "adapter `" + info.id + "' already exists and belongs to an application";
	}
	_manager.receivedUpdate("adapter", getSerial(current.ctx, "adapter"), failure);
    }

    virtual void 
    adapterUpdated(const AdapterInfo& info, const Ice::Current& current)
    {
	string failure;
	if(!_database->setAdapterDirectProxy(info.id, info.replicaGroupId, info.proxy))
	{
	    failure = "adapter `" + info.id + "' already exists and belongs to an application";
	}
	_manager.receivedUpdate("adapter", getSerial(current.ctx, "adapter"), failure);
    }

    virtual void 
    adapterRemoved(const std::string& id, const Ice::Current& current)
    {
	string failure;
	if(!_database->setAdapterDirectProxy(id, "", 0))
	{
	    failure = "adapter `" + id + "' already exists and belongs to an application";
	}
	_manager.receivedUpdate("adapter", getSerial(current.ctx, "adapter"), failure);
    }

    virtual void
    objectInit(const ObjectInfoSeq& objects, const Ice::Current& current)
    {
	_database->syncObjects(objects);
    }

    virtual void 
    objectAdded(const ObjectInfo& info, const Ice::Current& current)
    {
	string failure;
	try
	{
	    _database->addObject(info, true);
	}
	catch(const ObjectExistsException& ex)
	{
	    ostringstream os;
	    os << ex << ":\n";
	    os << "id: " << info.proxy->ice_getCommunicator()->identityToString(info.proxy->ice_getIdentity());
	    failure = os.str();
	}
	_manager.receivedUpdate("object", getSerial(current.ctx, "object"), failure);
    }

    virtual void 
    objectUpdated(const ObjectInfo& info, const Ice::Current& current)
    {
	string failure;
	try
	{
	    _database->addObject(info, true);
	}
	catch(const DeploymentException& ex)
	{
	    ostringstream os;
	    os << ex << ":\n" << ex.reason;
	    failure = os.str();
	}
	_manager.receivedUpdate("object", getSerial(current.ctx, "object"), failure);
    }

    virtual void 
    objectRemoved(const Ice::Identity& id, const Ice::Current& current)
    {
	string failure;
	try
	{
	    _database->removeObject(id);
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
	_manager.receivedUpdate("object", getSerial(current.ctx, "object"), failure);
    }

private:

    int 
    getSerial(const Ice::Context& context, const string& name)
    {
	Ice::Context::const_iterator p = context.find(name);
	if(p != context.end())
	{
	    int serial;
	    istringstream is(p->second);
	    is >> serial;
	    return serial;
	}
	return -1;
    }
    
    const DatabasePtr _database;
    ReplicaSessionManager& _manager;
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
    Ice::CommunicatorPtr communicator = database->getCommunicator();
    string instanceName = communicator->getDefaultLocator()->ice_getIdentity().category;

    {
	Lock sync(*this);
	
	_master = 
	    InternalRegistryPrx::uncheckedCast(communicator->stringToProxy(instanceName + "/InternalRegistry-Master"));
	
	Ice::ObjectPrx obsv = 
	    database->getInternalAdapter()->addWithUUID(new MasterDatabaseObserverI(database, *this));

	_observer = DatabaseObserverPrx::uncheckedCast(obsv);
	
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

    _thread->tryCreateSession(_master);
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
    _thread = 0;
}

void
ReplicaSessionManager::receivedUpdate(const string& update, int serial, const string& failure)
{
    ReplicaSessionPrx session = _thread->getSession();
    if(session)
    {
	try
	{
	    session->receivedUpdate(update, serial, failure);
	}
	catch(const Ice::LocalException&)
	{
	}
    }
    if(!failure.empty())
    {
	_thread->destroyActiveSession();
    }
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

	ReplicaSessionPrx session = registry->registerReplica(_name, _info, _internalRegistry, _observer);
	int t = session->getTimeout();
	if(t > 0)
	{
	    timeout = IceUtil::Time::seconds(t / 2);
	}

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
}

