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

using namespace std;
using namespace IceGrid;

class MasterRegistryObserverI : public RegistryObserver
{
public:

    MasterRegistryObserverI(const DatabasePtr& database) : _database(database)
    {
    }

    virtual void 
    init(int serial, 
	 const ApplicationDescriptorSeq& applications, 
	 const AdapterInfoSeq& adapters, 
	 const ObjectInfoSeq& objects, 
	 const Ice::Current&)
    {
	_database->initReplica(serial, applications, adapters, objects);
    }

    virtual void 
    applicationAdded(int serial, const ApplicationDescriptor& application, const Ice::Current&)
    {
	_database->addApplicationDescriptor(0, application, serial);
    }

    virtual void 
    applicationRemoved(int serial, const std::string& name, const Ice::Current&)
    {
	_database->removeApplicationDescriptor(0, name, serial);
    }

    virtual void 
    applicationUpdated(int serial, const ApplicationUpdateDescriptor& update, const Ice::Current&)
    {
	_database->updateApplicationDescriptor(0, update, serial);
    }

    virtual void 
    adapterAdded(int serial, const AdapterInfo& info, const Ice::Current&)
    {
	_database->setAdapterDirectProxy(info.id, info.replicaGroupId, info.proxy, serial);
    }

    virtual void 
    adapterUpdated(int serial, const AdapterInfo& info, const Ice::Current&)
    {
	_database->setAdapterDirectProxy(info.id, info.replicaGroupId, info.proxy, serial);
    }

    virtual void 
    adapterRemoved(int serial, const std::string& id, const Ice::Current&)
    {
	_database->setAdapterDirectProxy(id, "", 0, serial);
    }
    
    virtual void 
    objectAdded(int serial, const ObjectInfo& info, const Ice::Current&)
    {
	_database->addObject(info, false, serial);
    }

    virtual void 
    objectUpdated(int serial, const ObjectInfo& info, const Ice::Current&)
    {
	_database->updateObject(info.proxy, serial);
    }

    virtual void 
    objectRemoved(int serial, const Ice::Identity& id, const Ice::Current&)
    {
	_database->removeObject(id, serial);
    }

private:
    
    const DatabasePtr _database;
};

ReplicaSessionKeepAliveThread::ReplicaSessionKeepAliveThread(const std::string& name,
							     const InternalRegistryPrx& master,
							     const InternalRegistryPrx& replica,
							     const ReplicaInfo& info,
							     const DatabasePtr& database) : 
    _name(name),
    _master(master),
    _replica(replica),
    _info(info),
    _database(database),
    _shutdown(false)
{
}

void
ReplicaSessionKeepAliveThread::run()
{
    //
    // Keep alive the session.
    //
    ReplicaSessionPrx session;
    IceUtil::Time timeout = IceUtil::Time::seconds(5); 
    TraceLevelsPtr traceLevels = _database->getTraceLevels();
    while(true)
    {
	if(session)
	{
	    try
	    {
		if(traceLevels && traceLevels->replica > 2)
		{
		    Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		    out << "sending keep alive message to master replica";
		}

		session->keepAlive();
	    }
	    catch(const Ice::LocalException& ex)
	    {
		if(traceLevels && traceLevels->replica > 0)
		{
		    Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		    out << "lost session with master replica:\n" << ex;
		}
		session = 0;
	    }
	}

	if(!session)
	{
	    try
	    {
		if(traceLevels && traceLevels->replica > 1)
		{
		    Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		    out << "trying to establish session with master replica";
		}

		session = _master->registerReplica(_name, _replica, _info);
		int t = session->getTimeout();
		if(t > 0)
		{
		    timeout = IceUtil::Time::seconds(t);
		}

		if(traceLevels && traceLevels->replica > 0)
		{
		    Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		    out << "established session with master replica";
		}
	    }
	    catch(const ReplicaActiveException&)
	    {
		if(traceLevels)
		{
		    traceLevels->logger->error("a replica with the same name is already registered and active");
		}
	    }
	    catch(const Ice::LocalException& ex)
	    {
		ObjectInfo info;
		info.type = InternalRegistry::ice_staticId();
		info.proxy = _replica;
		_database->addObject(info, true);
		
		Ice::Identity id;
		id.category = _replica->ice_getIdentity().category;
		id.name = "Query";
		info.type = Query::ice_staticId();
		info.proxy = _info.clientProxy->ice_identity(id);
		_database->addObject(info, true);
		
		if(traceLevels && traceLevels->replica > 1)
		{
		    Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		    out << "failed to establish session with master replica:\n" << ex;
		}
	    }
	}

	{
	    Lock sync(*this);
	    if(!_shutdown)
	    {
		timedWait(timeout);
	    }
	    if(_shutdown)
	    {
		break;
	    }
	}	
    }
    
    //
    // Destroy the session.
    //
    if(session)
    {
	try
	{
	    session->destroy();

	    if(traceLevels && traceLevels->replica > 0)
	    {
		Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		out << "destroyed master replica session";
	    }
	}
	catch(const Ice::LocalException& ex)
	{
	    if(traceLevels && traceLevels->replica > 1)
	    {
		Ice::Trace out(traceLevels->logger, traceLevels->replicaCat);
		out << "couldn't destroy master replica session:\n" << ex;
	    }
	}
    }
}

void
ReplicaSessionKeepAliveThread::terminate()
{
    Lock sync(*this);
    _shutdown = true;
    notifyAll();
}

ReplicaSessionManager::ReplicaSessionManager()
{
}

void
ReplicaSessionManager::create(const string& name, 
			      const DatabasePtr& database,
			      const InternalRegistryPrx& replica,
			      const Ice::ObjectAdapterPtr& clientAdapter,
			      const Ice::ObjectAdapterPtr& serverAdapter)
{
    Ice::CommunicatorPtr communicator = database->getCommunicator();
    Ice::PropertiesPtr properties = communicator->getProperties();

    Ice::LocatorPrx locator = communicator->getDefaultLocator();
    assert(locator);
    string instanceName = locator->ice_getIdentity().category;

    InternalRegistryPrx master = 
	InternalRegistryPrx::uncheckedCast(communicator->stringToProxy(instanceName + "/InternalRegistry"));

    Ice::ObjectPrx observer = database->getInternalAdapter()->addWithUUID(new MasterRegistryObserverI(database));

    ReplicaInfo info;
    info.clientProxy = clientAdapter->createDirectProxy(communicator->stringToIdentity("dummy"));
    info.serverProxy = serverAdapter->createDirectProxy(communicator->stringToIdentity("dummy"));
    info.observer = RegistryObserverPrx::uncheckedCast(observer);

    _session = new ReplicaSessionKeepAliveThread(name, master, replica, info, database);
    _session->start();
}

void
ReplicaSessionManager::destroy()
{
    if(_session)
    {
	_session->terminate();
	_session->getThreadControl().join();
    }
}




