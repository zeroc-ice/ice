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
    _timeout(IceUtil::Time::seconds(5)), 
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
    while(true)
    {
	keepAlive(session);

	{
	    Lock sync(*this);

	    session = _session;	    

	    if(!_shutdown)
	    {
		timedWait(_timeout);
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
	}
	catch(const Ice::LocalException&)
	{
	    //
	    // TODO: XXX: TRACE?
	    //
//	    ostringstream os;
//	    os << "couldn't contact the IceGrid registry to destroy the node session:\n" << ex;
//	    _database->getTraceLevels()->logger->warning(os.str());
	}
    }
}

void
ReplicaSessionKeepAliveThread::waitForCreate()
{
    Lock sync(*this);
    while(!_session)
    {
	wait();
    }
}

void
ReplicaSessionKeepAliveThread::terminate()
{
    Lock sync(*this);
    _shutdown = true;
    notifyAll();
}

void
ReplicaSessionKeepAliveThread::keepAlive(const ReplicaSessionPrx& session)
{
    if(session)
    {
	try
	{
	    session->keepAlive();
	    return; // We're done!
	}
	catch(const Ice::LocalException&)
	{
	}
    }

    try
    {
	ReplicaSessionPrx newSession = _master->registerReplica(_name, _replica, _info);
	int timeout = newSession->getTimeout();
	{
	    Lock sync(*this);
	    if(timeout > 0)
	    {
		_timeout = IceUtil::Time::seconds(timeout);
	    }
	    _session = newSession;
	    notifyAll();
	}
    }
    catch(const ReplicaActiveException&)
    {
 	_database->getTraceLevels()->logger->error("a replica with the same name is already registered and active");
    }
    catch(const Ice::LocalException&)
    {
	//
	// TODO: FIX THIS SHOULD BE A TRACE
	//
//	ostringstream os;
//	os << "couldn't contact the IceGrid registry:\n" << ex;
//	_database->getTraceLevels()->logger->warning(os.str());
    }
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




