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

namespace IceGrid
{

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

};

ReplicaSessionManager::ReplicaSessionManager()
{
}

void
ReplicaSessionManager::create(const string& name, const DatabasePtr& database, const InternalRegistryPrx& replica)
{
    Ice::CommunicatorPtr communicator = database->getCommunicator();
    string instanceName = communicator->getDefaultLocator()->ice_getIdentity().category;

    Lock sync(*this);

    _master = InternalRegistryPrx::uncheckedCast(communicator->stringToProxy(instanceName + "/InternalRegistry"));

    Ice::ObjectPrx observer = database->getInternalAdapter()->addWithUUID(new MasterRegistryObserverI(database));
    _observer = RegistryObserverPrx::uncheckedCast(observer);

    _name = name;
    _replica = replica;
    _database = database;
    _traceLevels = _database->getTraceLevels();

    _thread = new Thread(*this, _master);
    _thread->start();
    notifyAll();
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

void
ReplicaSessionManager::activate()
{
    ReplicaSessionPrx session = _thread->getSession();
    if(session)
    {
	session->setClientAndServerProxies(_database->getClientProxy(), _database->getServerProxy());
    }
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
	return _replica->getNodes();
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

bool
ReplicaSessionManager::keepAlive(const ReplicaSessionPrx& session) const
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
ReplicaSessionManager::createSession(const InternalRegistryPrx& registry, IceUtil::Time& timeout) const
{
    try
    {
	if(_traceLevels && _traceLevels->replica > 1)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
	    out << "trying to establish session with master replica";
	}

	ReplicaSessionPrx session = registry->registerReplica(_name, _replica, _observer);
	int t = session->getTimeout();
	if(t > 0)
	{
	    timeout = IceUtil::Time::seconds(t);
	}		
	session->setClientAndServerProxies(_database->getClientProxy(), _database->getServerProxy());

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
	ObjectInfo info;
	info.type = InternalRegistry::ice_staticId();
	info.proxy = _replica;
	_database->addObject(info, true);
	_database->updateReplicatedWellKnownObjects();
		
	if(_traceLevels && _traceLevels->replica > 1)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
	    out << "failed to establish session with master replica:\n" << ex;
	}
	return 0;
    }
}

void
ReplicaSessionManager::destroySession(const ReplicaSessionPrx& session) const
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
