// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/UUID.h>
#include <IceGrid/RegistryI.h>
#include <IceGrid/AdminSessionI.h>
#include <IceGrid/AdminI.h>
#include <IceGrid/Database.h>

#include <IceSSL/Plugin.h>

using namespace std;
using namespace IceGrid;

FileIteratorI::FileIteratorI(const AdminSessionIPtr& session, 
                             const FileReaderPrx& reader, 
                             const string& filename,
                             Ice::Long offset,
                             int messageSizeMax) :
    _session(session),
    _reader(reader),
    _filename(filename),
    _offset(offset),
    _messageSizeMax(messageSizeMax - 256) // Room for the header
{
}

bool
FileIteratorI::read(int size, Ice::StringSeq& lines, const Ice::Current& current)
{
    try
    {
        return _reader->read(_filename, _offset, size > _messageSizeMax ? _messageSizeMax : size, _offset, lines);
    }
    catch(const Ice::LocalException& ex)
    {
        ostringstream os;
        os << ex;
        throw FileNotAvailableException(os.str());
    }
    return false; // Keep the compiler happy.
}

void
FileIteratorI::destroy(const Ice::Current& current)
{
    _session->removeFileIterator(current.id, current);
}

AdminSessionI::AdminSessionI(const string& id, const DatabasePtr& db, int timeout, const string& replicaName) :
    BaseSessionI(id, "admin", db),
    _timeout(timeout),
    _replicaName(replicaName)
{
}

AdminSessionI::~AdminSessionI()
{
}

Ice::ObjectPrx
AdminSessionI::registerWithServantLocator(const SessionServantLocatorIPtr& servantLoc, 
                                          const Ice::ConnectionPtr& con,
                                          const RegistryIPtr& registry)
{
    Ice::ObjectPrx proxy = BaseSessionI::registerWithServantLocator(servantLoc, con);
    _admin = AdminPrx::uncheckedCast(servantLoc->add(new AdminI(_database, registry, this), con));
    return proxy;
}

Ice::ObjectPrx
AdminSessionI::registerWithObjectAdapter(const Ice::ObjectAdapterPtr& adapter, 
                                         const RegistryIPtr& registry,
                                         const Glacier2::SessionControlPrx& ctl)
{
    Ice::ObjectPrx proxy = BaseSessionI::registerWithObjectAdapter(adapter);
    Ice::Identity identity;
    identity.category = _database->getInstanceName();
    identity.name = IceUtil::generateUUID();
    _admin = AdminPrx::uncheckedCast(adapter->add(new AdminI(_database, registry, this), identity));
    _sessionControl = ctl;
    return proxy;
}

AdminPrx
AdminSessionI::getAdmin(const Ice::Current& current) const
{
    return _admin;
}

void
AdminSessionI::setObservers(const RegistryObserverPrx& registryObserver, 
                            const NodeObserverPrx& nodeObserver, 
                            const ApplicationObserverPrx& appObserver,
                            const AdapterObserverPrx& adapterObserver,
                            const ObjectObserverPrx& objectObserver,
                            const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
        Ice::ObjectNotExistException ex(__FILE__, __LINE__);
        ex.id = current.id;
        throw ex;
    }

    if(registryObserver)
    {
        setupObserverSubscription(RegistryObserverTopicName, registryObserver->ice_timeout(_timeout * 1000));
    }
    else
    {
        setupObserverSubscription(RegistryObserverTopicName, Ice::ObjectPrx());
    }

    if(nodeObserver)
    {
        setupObserverSubscription(NodeObserverTopicName, nodeObserver->ice_timeout(_timeout * 1000));
    }
    else
    {
        setupObserverSubscription(NodeObserverTopicName, Ice::ObjectPrx());
    }

    if(appObserver)
    {
        setupObserverSubscription(ApplicationObserverTopicName, appObserver->ice_timeout(_timeout * 1000));
    }
    else
    {
        setupObserverSubscription(ApplicationObserverTopicName, Ice::ObjectPrx());
    }

    if(adapterObserver)
    {
        setupObserverSubscription(AdapterObserverTopicName, adapterObserver->ice_timeout(_timeout * 1000));
    }
    else
    {
        setupObserverSubscription(AdapterObserverTopicName, Ice::ObjectPrx());
    }

    if(objectObserver)
    {
        setupObserverSubscription(ObjectObserverTopicName, objectObserver->ice_timeout(_timeout * 1000));
    }
    else
    {
        setupObserverSubscription(ObjectObserverTopicName, Ice::ObjectPrx());
    }
}

void
AdminSessionI::setObserversByIdentity(const Ice::Identity& registryObserver, 
                                      const Ice::Identity& nodeObserver,
                                      const Ice::Identity& appObserver,
                                      const Ice::Identity& adapterObserver,
                                      const Ice::Identity& objectObserver,
                                      const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
        Ice::ObjectNotExistException ex(__FILE__, __LINE__);
        ex.id = current.id;
        throw ex;
    }

    setupObserverSubscription(RegistryObserverTopicName, toProxy(registryObserver, current.con));
    setupObserverSubscription(NodeObserverTopicName, toProxy(nodeObserver, current.con));
    setupObserverSubscription(ApplicationObserverTopicName, toProxy(appObserver, current.con));
    setupObserverSubscription(AdapterObserverTopicName, toProxy(adapterObserver, current.con));
    setupObserverSubscription(ObjectObserverTopicName, toProxy(objectObserver, current.con));

}

int
AdminSessionI::startUpdate(const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
        Ice::ObjectNotExistException ex(__FILE__, __LINE__);
        ex.id = current.id;
        throw ex;
    }

    int serial = _database->lock(this, _id);
    return serial;
}

void
AdminSessionI::finishUpdate(const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
        Ice::ObjectNotExistException ex(__FILE__, __LINE__);
        ex.id = current.id;
        throw ex;
    }

    _database->unlock(this);
}

string
AdminSessionI::getReplicaName(const Ice::Current& current) const
{
    return _replicaName;
}

FileIteratorPrx
AdminSessionI::openServerLog(const string& id, const string& path, int nLines, const Ice::Current& current)
{
    return addFileIterator(_database->getServer(id)->getProxy(), "#" + path, nLines, current);
}

FileIteratorPrx 
AdminSessionI::openServerStdOut(const string& id, int nLines, const Ice::Current& current)
{
    return addFileIterator(_database->getServer(id)->getProxy(), "stdout", nLines, current);
}

FileIteratorPrx 
AdminSessionI::openServerStdErr(const string& id, int nLines, const Ice::Current& current)
{
    return addFileIterator(_database->getServer(id)->getProxy(), "stderr", nLines, current);
}

FileIteratorPrx 
AdminSessionI::openNodeStdOut(const string& name, int nLines, const Ice::Current& current)
{
    return addFileIterator(_database->getNode(name)->getProxy(), "stdout", nLines, current);
}

FileIteratorPrx 
AdminSessionI::openNodeStdErr(const string& name, int nLines, const Ice::Current& current)
{
    return addFileIterator(_database->getNode(name)->getProxy(), "stderr", nLines, current);
}

FileIteratorPrx 
AdminSessionI::openRegistryStdOut(const string& name, int nLines, const Ice::Current& current)
{
    FileReaderPrx reader;
    if(name == _replicaName)
    {
        reader = _database->getReplicaCache().getInternalRegistry();
    }
    else
    {
        reader = _database->getReplica(name)->getProxy();
    }
    return addFileIterator(reader, "stdout", nLines, current);
}

FileIteratorPrx
AdminSessionI::openRegistryStdErr(const string& name, int nLines, const Ice::Current& current)
{
    FileReaderPrx reader;
    if(name == _replicaName)
    {
        reader = _database->getReplicaCache().getInternalRegistry();
    }
    else
    {
        reader = _database->getReplica(name)->getProxy();
    }
    return addFileIterator(reader, "stderr", nLines, current);
}

void
AdminSessionI::destroy(const Ice::Current&)
{
    destroyImpl(false);
}

void
AdminSessionI::setupObserverSubscription(TopicName name, const Ice::ObjectPrx& observer)
{
    if(_observers[name] && _observers[name] != observer)
    {
        _database->getObserverTopic(name)->unsubscribe(_observers[name]);
        _observers[name] = 0;
    }

    if(observer)
    {
        _observers[name] = observer;
        _database->getObserverTopic(name)->subscribe(_observers[name]); 
    }
}

Ice::ObjectPrx
AdminSessionI::toProxy(const Ice::Identity& id, const Ice::ConnectionPtr& connection)
{
    return id.name.empty() ? Ice::ObjectPrx() : connection->createProxy(id);
}

FileIteratorPrx
AdminSessionI::addFileIterator(const FileReaderPrx& reader, 
                               const string& filename, 
                               int nLines, 
                               const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
        Ice::ObjectNotExistException ex(__FILE__, __LINE__);
        ex.id = current.id;
        throw ex;
    }

    //
    // Always call getOffsetFromEnd even if nLines < 0. This allows to
    // throw right away if the file doesn't exit.
    //
    Ice::Long offset;
    try
    {
        offset = reader->getOffsetFromEnd(filename, nLines);
    }
    catch(const Ice::LocalException& ex)
    {
        ostringstream os;
        os << ex;
        throw FileNotAvailableException(os.str());
    }

    Ice::PropertiesPtr properties = reader->ice_getCommunicator()->getProperties();
    int messageSizeMax = properties->getPropertyAsIntWithDefault("Ice.MessageSizeMax", 1024) * 1024;

    Ice::ObjectPrx obj;
    Ice::ObjectPtr servant = new FileIteratorI(this, reader, filename, offset, messageSizeMax);
    if(_servantLocator)
    {
        obj = _servantLocator->add(servant, current.con);
    }
    else
    {
        assert(_adapter);
        obj = _adapter->addWithUUID(servant);
        
        if(_sessionControl)
        {
            try
            {
                Ice::IdentitySeq ids;
                ids.push_back(obj->ice_getIdentity());
                _sessionControl->identities()->add(ids);
            }
            catch(const Ice::LocalException&)
            {
            }
        }
    }
    _iterators.insert(obj->ice_getIdentity());
    return FileIteratorPrx::uncheckedCast(obj);
}

void
AdminSessionI::removeFileIterator(const Ice::Identity& id, const Ice::Current& current)
{
    Lock sync(*this);
    if(_servantLocator)
    {
        _servantLocator->remove(id);
    }
    else
    {
        try
        {
            assert(_adapter && _sessionControl);
            _adapter->remove(id);
        }
        catch(const Ice::ObjectAdapterDeactivatedException&)
        {
        }

        if(_sessionControl)
        {
            try
            {
                Ice::IdentitySeq ids;
                ids.push_back(id);
                _sessionControl->identities()->remove(ids);
            }
            catch(const Ice::LocalException&)
            {
            }
        }
    }
    _iterators.erase(id);
}

void
AdminSessionI::destroyImpl(bool shutdown)
{
    BaseSessionI::destroyImpl(shutdown);

    try
    {
        _database->unlock(this);
    }
    catch(AccessDeniedException&)
    {
    }

    //
    // Unregister the admin servant from the session servant locator
    // or object adapter.
    //
    if(!shutdown)
    {
        if(_servantLocator)
        {
            _servantLocator->remove(_admin->ice_getIdentity());
        }
        else if(_adapter)
        {
            try
            {
                _adapter->remove(_admin->ice_getIdentity());
            }
            catch(const Ice::ObjectAdapterDeactivatedException&)
            {
            }
        }

        //
        // Unregister the iterators from the session servant locator or
        // object adapter.
        //
        for(set<Ice::Identity>::const_iterator p = _iterators.begin(); p != _iterators.end(); ++p)
        {
            if(_servantLocator)
            {
                _servantLocator->remove(*p);
            }
            else if(_adapter)
            {
                try
                {
                    _adapter->remove(*p);
                }
                catch(const Ice::ObjectAdapterDeactivatedException&)
                {
                }
            }
        }

        //
        // Unsubscribe from the topics.
        //
        setupObserverSubscription(RegistryObserverTopicName, 0);
        setupObserverSubscription(NodeObserverTopicName, 0);
        setupObserverSubscription(ApplicationObserverTopicName, 0);
        setupObserverSubscription(AdapterObserverTopicName, 0);
        setupObserverSubscription(ObjectObserverTopicName, 0);
    }
}

AdminSessionFactory::AdminSessionFactory(const Ice::ObjectAdapterPtr& adapter,
                                         const DatabasePtr& database,
                                         const ReapThreadPtr& reaper,
                                         const RegistryIPtr& registry) :
    _adapter(adapter),
    _database(database), 
    _timeout(registry->getSessionTimeout()),
    _reaper(reaper),
    _registry(registry)
{
}

Glacier2::SessionPrx
AdminSessionFactory::createGlacier2Session(const string& sessionId, const Glacier2::SessionControlPrx& ctl)
{
    assert(_adapter);

    AdminSessionIPtr session = createSessionServant(sessionId);
    Ice::ObjectPrx proxy = session->registerWithObjectAdapter(_adapter, _registry, ctl);

    Ice::Identity queryId;
    queryId.category = _database->getInstanceName();
    queryId.name = "Query";

    Ice::IdentitySeq ids; // Identities of the object the session is allowed to access.
    ids.push_back(queryId); // The IceGrid::Query object
    ids.push_back(proxy->ice_getIdentity()); // The session object.
    ids.push_back(session->getAdmin()->ice_getIdentity()); // The per-session admin object.

    int timeout = 0;
    if(ctl)
    {
        try
        {
            ctl->identities()->add(ids);
        }
        catch(const Ice::LocalException&)
        {
            session->destroy(Ice::Current());
            return 0;
        }
        timeout = ctl->getSessionTimeout();
    }

    if(timeout > 0)
    {
        _reaper->add(new SessionReapable<AdminSessionI>(_database->getTraceLevels()->logger, session), timeout);
    }

    return Glacier2::SessionPrx::uncheckedCast(proxy);
}

AdminSessionIPtr
AdminSessionFactory::createSessionServant(const string& id)
{
    return new AdminSessionI(id, _database, _timeout, _registry->getName());
}

const TraceLevelsPtr&
AdminSessionFactory::getTraceLevels() const
{
    return _database->getTraceLevels(); 
}

AdminSessionManagerI::AdminSessionManagerI(const AdminSessionFactoryPtr& factory) : _factory(factory)
{
}

Glacier2::SessionPrx
AdminSessionManagerI::create(const string& userId, const Glacier2::SessionControlPrx& ctl, const Ice::Current& current)
{
    return _factory->createGlacier2Session(userId, ctl);
}

AdminSSLSessionManagerI::AdminSSLSessionManagerI(const AdminSessionFactoryPtr& factory) : _factory(factory)
{
}

Glacier2::SessionPrx
AdminSSLSessionManagerI::create(const Glacier2::SSLInfo& info, 
                                const Glacier2::SessionControlPrx& ctl,
                                const Ice::Current& current)
{
    string userDN;
    if(!info.certs.empty()) // TODO: Require userDN?
    {
        try
        {
            IceSSL::CertificatePtr cert = IceSSL::Certificate::decode(info.certs[0]);
            userDN = cert->getSubjectDN();
        }
        catch(const Ice::Exception& ex)
        {
            // This shouldn't happen, the SSLInfo is supposed to be encoded by Glacier2.
            Ice::Error out(_factory->getTraceLevels()->logger);
            out << "SSL session manager couldn't decode SSL certificates:\n" << ex;
            return 0;
        }
    }

    return _factory->createGlacier2Session(userDN, ctl);
}

