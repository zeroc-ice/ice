// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
        int serial;
        _database->syncApplications(applications, getSerials(current.ctx, serial));
        receivedUpdate(ApplicationObserverTopicName, serial);
    }

    virtual void
    applicationAdded(int, const ApplicationInfo& application, const Ice::Current& current)
    {
        int serial;
        string failure;
        try
        {
            _database->addApplication(application, 0, getSerials(current.ctx, serial));
        }
        catch(const DeploymentException& ex)
        {
            ostringstream os;
            os << ex << ":\n" << ex.reason;
            failure = os.str();
        }
        receivedUpdate(ApplicationObserverTopicName, serial, failure);
    }

    virtual void
    applicationRemoved(int, const std::string& name, const Ice::Current& current)
    {
        int serial;
        string failure;
        try
        {
            _database->removeApplication(name, 0, getSerials(current.ctx, serial));
        }
        catch(const ApplicationNotExistException& ex)
        {
            ostringstream os;
            os << ex << ":\napplication: " << ex.name;
            failure = os.str();
        }
        receivedUpdate(ApplicationObserverTopicName, serial, failure);
    }

    virtual void
    applicationUpdated(int, const ApplicationUpdateInfo& update, const Ice::Current& current)
    {
        int serial;
        string failure;
        try
        {
            _database->updateApplication(update, false, 0, getSerials(current.ctx, serial));
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
        receivedUpdate(ApplicationObserverTopicName, serial, failure);
    }

    virtual void
    adapterInit(const AdapterInfoSeq& adapters, const Ice::Current& current)
    {
        int serial;
        _database->syncAdapters(adapters, getSerials(current.ctx, serial));
        receivedUpdate(AdapterObserverTopicName, serial);
    }

    virtual void
    adapterAdded(const AdapterInfo& info, const Ice::Current& current)
    {
        int serial;
        string failure;
        try
        {
            _database->setAdapterDirectProxy(info.id, info.replicaGroupId, info.proxy, getSerials(current.ctx, serial));
        }
        catch(const AdapterExistsException&)
        {
            failure = "adapter `" + info.id + "' already exists and belongs to an application";
        }
        receivedUpdate(AdapterObserverTopicName, serial, failure);
    }

    virtual void
    adapterUpdated(const AdapterInfo& info, const Ice::Current& current)
    {
        int serial;
        string failure;
        try
        {
            _database->setAdapterDirectProxy(info.id, info.replicaGroupId, info.proxy, getSerials(current.ctx, serial));
        }
        catch(const AdapterExistsException&)
        {
            failure = "adapter `" + info.id + "' already exists and belongs to an application";
        }
        receivedUpdate(AdapterObserverTopicName, serial, failure);
    }

    virtual void
    adapterRemoved(const std::string& id, const Ice::Current& current)
    {
        int serial;
        string failure;
        try
        {
            _database->setAdapterDirectProxy(id, "", 0, getSerials(current.ctx, serial));
        }
        catch(const AdapterExistsException&)
        {
            failure = "adapter `" + id + "' already exists and belongs to an application";
        }
        receivedUpdate(AdapterObserverTopicName, serial, failure);
    }

    virtual void
    objectInit(const ObjectInfoSeq& objects, const Ice::Current& current)
    {
        int serial;
        _database->syncObjects(objects, getSerials(current.ctx, serial));
        receivedUpdate(ObjectObserverTopicName, serial);
    }

    virtual void
    objectAdded(const ObjectInfo& info, const Ice::Current& current)
    {
        int serial;
        string failure;
        try
        {
            _database->addOrUpdateObject(info, getSerials(current.ctx, serial));
        }
        catch(const ObjectExistsException& ex)
        {
            ostringstream os;
            os << ex << ":\n";
            os << "id: " << _database->getCommunicator()->identityToString(info.proxy->ice_getIdentity());
            failure = os.str();
        }
        receivedUpdate(ObjectObserverTopicName, serial, failure);
    }

    virtual void
    objectUpdated(const ObjectInfo& info, const Ice::Current& current)
    {
        int serial;
        string failure;
        try
        {
            _database->addOrUpdateObject(info, getSerials(current.ctx, serial));
        }
        catch(const ObjectExistsException& ex)
        {
            ostringstream os;
            os << ex << ":\n";
            os << "id: " << _database->getCommunicator()->identityToString(info.proxy->ice_getIdentity());
            failure = os.str();
        }
        catch(const DeploymentException& ex)
        {
            ostringstream os;
            os << ex << ":\n" << ex.reason;
            failure = os.str();
        }
        receivedUpdate(ObjectObserverTopicName, serial, failure);
    }

    virtual void
    objectRemoved(const Ice::Identity& id, const Ice::Current& current)
    {
        int serial;
        string failure;
        try
        {
            _database->removeObject(id, getSerials(current.ctx, serial));
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
        receivedUpdate(ObjectObserverTopicName, serial, failure);
    }

private:

    Ice::Long
    getSerials(const Ice::Context& context, int& serial)
    {
        Ice::Context::const_iterator p = context.find("serial");
        if(p != context.end())
        {
            istringstream is(p->second);
            is >> serial;
        }
        else
        {
            serial = -1;
        }

        p = context.find("dbSerial");
        if(p != context.end())
        {
            Ice::Long dbSerial;
            istringstream is(p->second);
            is >> dbSerial;
            return dbSerial;
        }
        else
        {
            return -1;
        }
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

ReplicaSessionManager::ReplicaSessionManager(const Ice::CommunicatorPtr& communicator, const string& instanceName) :
    SessionManager(communicator, instanceName)
{
}

void
ReplicaSessionManager::create(const string& name,
                              const InternalReplicaInfoPtr& info,
                              const DatabasePtr& database,
                              const WellKnownObjectsManagerPtr& wellKnownObjects,
                              const InternalRegistryPrx& internalRegistry)
{
    {
        Lock sync(*this);

        _name = name;
        _info = info;
        _internalRegistry = internalRegistry;
        _database = database;
        _wellKnownObjects = wellKnownObjects;
        _traceLevels = _database->getTraceLevels();

        _thread = new Thread(*this, _master, _traceLevels->logger);
        _thread->start();
        notifyAll();
    }

    _thread->tryCreateSession();
    _thread->waitTryCreateSession();
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
    _thread->waitTryCreateSession();
}

NodePrxSeq
ReplicaSessionManager::getNodes(const NodePrxSeq& nodes) const
{
    assert(_thread && _thread->getRegistry());
    if(_thread->getSession())
    {
        try
        {
            return _thread->getRegistry()->getNodes();
        }
        catch(const Ice::LocalException&)
        {
            return nodes;
        }
    }
    else
    {
        return nodes;
    }
}

void
ReplicaSessionManager::destroy()
{
    ThreadPtr thread;
    {
        Lock sync(*this);
        if(!_communicator)
        {
            return;
        }
        thread = _thread;
        _thread = 0;

        _communicator = 0;
    }

    if(thread)
    {
        thread->terminate();
        thread->getThreadControl().join();
    }

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

IceGrid::InternalRegistryPrx
ReplicaSessionManager::findInternalRegistryForReplica(const Ice::Identity& id)
{
    vector<Ice::AsyncResultPtr> results;
    vector<QueryPrx> queryObjects = findAllQueryObjects(true);
    for(vector<QueryPrx>::const_iterator q = queryObjects.begin(); q != queryObjects.end(); ++q)
    {
        results.push_back((*q)->begin_findObjectById(id));
    }

    for(vector<Ice::AsyncResultPtr>::const_iterator p = results.begin(); p != results.end(); ++p)
    {
        QueryPrx query = QueryPrx::uncheckedCast((*p)->getProxy());
        try
        {
            return InternalRegistryPrx::checkedCast(query->end_findObjectById(*p));
        }
        catch(const Ice::Exception&)
        {
        }
    }

    return 0;
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
    IceInternal::UniquePtr<Ice::Exception> exception;
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
            vector<Ice::AsyncResultPtr> results;
            vector<QueryPrx> queryObjects = findAllQueryObjects(false);
            for(vector<QueryPrx>::const_iterator q = queryObjects.begin(); q != queryObjects.end(); ++q)
            {
                results.push_back((*q)->begin_findObjectById(registry->ice_getIdentity()));
            }

            for(vector<Ice::AsyncResultPtr>::const_iterator p = results.begin(); p != results.end(); ++p)
            {
                QueryPrx query = QueryPrx::uncheckedCast((*p)->getProxy());
                if(isDestroyed())
                {
                    break;
                }

                InternalRegistryPrx newRegistry;
                try
                {
                    Ice::ObjectPrx obj = query->end_findObjectById(*p);
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
    catch(const DeploymentException& ex)
    {
        if(_traceLevels)
        {
            _traceLevels->logger->error("database synchronization with master failed:\n" + ex.reason);
        }
        exception.reset(ex.ice_clone());
    }
    catch(const PermissionDeniedException& ex)
    {
        if(_traceLevels)
        {
            _traceLevels->logger->error("connection to master was denied:\n" + ex.reason);
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
    ReplicaSessionPrx session;
    try
    {
        session = registry->registerReplica(_info, _internalRegistry);
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
        StringLongDict serials = _database->getSerials();
        IceUtil::Optional<StringLongDict> serialsOpt;
        if(!serials.empty())
        {
            serialsOpt = serials; // Don't provide serials parameter if serials aren't supported.
        }
        session->setDatabaseObserver(_observer, serialsOpt);
        return session;
    }
    catch(const Ice::Exception&)
    {
        destroySession(session);
        throw;
    }
}

void
ReplicaSessionManager::destroySession(const ReplicaSessionPrx& session)
{
    if(session)
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
