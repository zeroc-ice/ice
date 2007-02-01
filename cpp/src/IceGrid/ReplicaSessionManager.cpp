// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        try
        {
            _database->setAdapterDirectProxy(info.id, info.replicaGroupId, info.proxy);
        }
        catch(const AdapterExistsException&)
        {
            failure = "adapter `" + info.id + "' already exists and belongs to an application";
        }
        receivedUpdate(AdapterObserverTopicName, getSerial(current.ctx), failure);
    }

    virtual void 
    adapterUpdated(const AdapterInfo& info, const Ice::Current& current)
    {
        string failure;
        try
        {
            _database->setAdapterDirectProxy(info.id, info.replicaGroupId, info.proxy);
        }
        catch(const AdapterExistsException&)
        {
            failure = "adapter `" + info.id + "' already exists and belongs to an application";
        }
        receivedUpdate(AdapterObserverTopicName, getSerial(current.ctx), failure);
    }

    virtual void 
    adapterRemoved(const std::string& id, const Ice::Current& current)
    {
        string failure;
        try
        {
            _database->setAdapterDirectProxy(id, "", 0);
        }
        catch(const AdapterExistsException&)
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
            _database->addOrUpdateObject(info);
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
            _database->addOrUpdateObject(info);
        }
        catch(const ObjectExistsException& ex)
        {
            ostringstream os;
            os << ex << ":\n";
            os << "id: " << info.proxy->ice_getCommunicator()->identityToString(info.proxy->ice_getIdentity());
            failure = os.str();
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
                              const InternalReplicaInfoPtr& info,
                              const DatabasePtr& database,
                              const WellKnownObjectsManagerPtr& wellKnownObjects,
                              const InternalRegistryPrx& internalRegistry)
{
    Ice::CommunicatorPtr comm = database->getCommunicator();
    {
        Lock sync(*this);

        Ice::Identity id;
        id.category = comm->getDefaultLocator()->ice_getIdentity().category;
        id.name = "InternalRegistry-Master";
        
        _master = InternalRegistryPrx::uncheckedCast(comm->stringToProxy(comm->identityToString(id)));
        _name = name;
        _info = info;
        _internalRegistry = internalRegistry;
        _database = database;
        _wellKnownObjects = wellKnownObjects;
        _traceLevels = _database->getTraceLevels();

        //
        // Initialize the IceGrid::Query objects. The IceGrid::Query
        // interface is used to lookup the registry proxy in case it
        // becomes unavailable. Since replicas might not always have
        // an up to date registry proxy, we need to query all the
        // replicas.
        //
        Ice::EndpointSeq endpoints = comm->getDefaultLocator()->ice_getEndpoints();
        id.name = "Query";
        QueryPrx query = QueryPrx::uncheckedCast(comm->stringToProxy(comm->identityToString(id)));
        for(Ice::EndpointSeq::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
        {
            Ice::EndpointSeq singleEndpoint;
            singleEndpoint.push_back(*p);
            _queryObjects.push_back(QueryPrx::uncheckedCast(query->ice_endpoints(singleEndpoint)));
        }

        _thread = new Thread(*this, _master);
        _thread->start();
        notifyAll();
    }    

    _thread->tryCreateSession();
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

    _thread->setRegistry(replica);
    _thread->tryCreateSession();
}

NodePrxSeq
ReplicaSessionManager::getNodes(const NodePrxSeq& nodes) const
{
    assert(_thread && _thread->getRegistry());
    try
    {
        return _thread->getRegistry()->getNodes();
    }
    catch(const Ice::LocalException&)
    {
        return nodes;
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

    _database = 0;
    _wellKnownObjects = 0;
}

void
ReplicaSessionManager::registerAllWellKnownObjects()
{
    //
    // Always register first the well-known objects with the
    // database. Then, if there's a session, we register them with the
    // session and this will eventually override the ones with just
    // registered with the ones from the master.
    //
    _wellKnownObjects->registerAll();

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
ReplicaSessionManager::createSession(InternalRegistryPrx& registry, IceUtil::Time& timeout)
{
    ReplicaSessionPrx session;
    auto_ptr<Ice::Exception> exception;
    try
    {
        if(_traceLevels && _traceLevels->replica > 1)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
            out << "trying to establish session with master replica";
        }
        
        set<InternalRegistryPrx> used;
        if(!registry->ice_getEndpoints().empty())
        {
            try
            {
                session = createSessionImpl(registry, timeout);
            }
            catch(const Ice::LocalException& ex)
            {
                exception.reset(ex.ice_clone());
                used.insert(registry);
                registry = InternalRegistryPrx::uncheckedCast(registry->ice_endpoints(Ice::EndpointSeq()));
            }
        }

        if(!session)
        {
            for(vector<QueryPrx>::const_iterator p = _queryObjects.begin(); p != _queryObjects.end(); ++p)
            {
                InternalRegistryPrx newRegistry;
                try
                {
                    Ice::ObjectPrx obj = (*p)->findObjectById(registry->ice_getIdentity());
                    newRegistry = InternalRegistryPrx::uncheckedCast(obj);
                    if(newRegistry && used.find(newRegistry) == used.end())
                    {
                        session = createSessionImpl(newRegistry, timeout);
                        registry = newRegistry;
                        break;
                    }
                }
                catch(const Ice::LocalException& ex)
                {
                    exception.reset(ex.ice_clone());
                    if(newRegistry)
                    {
                        used.insert(newRegistry);
                    }
                }
            }
        }
    }
    catch(const ReplicaActiveException& ex)
    {
        if(_traceLevels)
        {
            _traceLevels->logger->error("a replica with the same name is already registered and active");
        }
        exception.reset(ex.ice_clone());
    }
    catch(const Ice::Exception& ex)
    {
        exception.reset(ex.ice_clone());
    }

    if(session)
    {
        //
        // Register all the well-known objects with the replica session.
        //
        _wellKnownObjects->registerAll(session);
    }
    else
    {
        //
        // Re-register all the well known objects with the local database.
        //
        _wellKnownObjects->registerAll();
    }

    if(_traceLevels && _traceLevels->replica > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
        if(session)
        {
            out << "established session with master replica";
        }
        else
        {
            out << "failed to establish session with master replica:\n";
            if(exception.get())
            {
                out << *exception.get();
            }
            else
            {
                out << "failed to get replica proxy";
            }
        }
    }
    return session;
}

ReplicaSessionPrx
ReplicaSessionManager::createSessionImpl(const InternalRegistryPrx& registry, IceUtil::Time& timeout)
{           
    try
    {
        ReplicaSessionPrx session = registry->registerReplica(_info, _internalRegistry);
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
        return session;
    }
    catch(const Ice::LocalException&)
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
        throw;
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

