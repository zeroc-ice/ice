// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/UUID.h>
#include <IceUtil/DisableWarnings.h>
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
FileIteratorI::read(int size, Ice::StringSeq& lines, const Ice::Current&)
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

AdminSessionI::AdminSessionI(const string& id, const DatabasePtr& db, int timeout, const RegistryIPtr& registry) :
    BaseSessionI(id, "admin", db),
    _timeout(timeout),
    _replicaName(registry->getName()),
    _registry(registry)
{
}

AdminSessionI::~AdminSessionI()
{
}

Ice::ObjectPrx
AdminSessionI::_register(const SessionServantManagerPtr& servantManager, const Ice::ConnectionPtr& con)
{
    //
    // This is supposed to be called after creation only, no need to synchronize.
    //
    _servantManager = servantManager;

    string category;

    if(con != 0)
    {
        category = _database->getInstanceName() + "-" + IceUtil::generateUUID();
     
        Ice::Identity templateId;
        templateId.name = "pick-a-name";
        templateId.category = category;
        
        _adminCallbackTemplate = _registry->createAdminCallbackProxy(templateId);
    }

    Ice::ObjectPrx session =  _servantManager->addSession(this, con, category);

    _admin = AdminPrx::uncheckedCast(_servantManager->add(new AdminI(_database, _registry, this), this));

    return session;
}

AdminPrx
AdminSessionI::getAdmin(const Ice::Current&) const
{
    return _admin;
}

Ice::ObjectPrx
AdminSessionI::getAdminCallbackTemplate(const Ice::Current&) const
{
    return _adminCallbackTemplate;
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

    setupObserverSubscription(RegistryObserverTopicName, toProxy(registryObserver, current.con, current.encoding));
    setupObserverSubscription(NodeObserverTopicName, toProxy(nodeObserver, current.con, current.encoding));
    setupObserverSubscription(ApplicationObserverTopicName, toProxy(appObserver, current.con, current.encoding));
    setupObserverSubscription(AdapterObserverTopicName, toProxy(adapterObserver, current.con, current.encoding));
    setupObserverSubscription(ObjectObserverTopicName, toProxy(objectObserver, current.con, current.encoding));

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
AdminSessionI::getReplicaName(const Ice::Current&) const
{
    return _replicaName;
}

FileIteratorPrx
AdminSessionI::openServerLog(const string& id, const string& path, int nLines, const Ice::Current& current)
{
    try
    {
        return addFileIterator(_database->getServer(id)->getProxy(false, 5), "#" + path, nLines, current);
    }
    catch(const SynchronizationException&)
    {
        throw DeploymentException("server is being updated");
        return 0;
    }
}

FileIteratorPrx 
AdminSessionI::openServerStdOut(const string& id, int nLines, const Ice::Current& current)
{
    try
    {
        return addFileIterator(_database->getServer(id)->getProxy(false, 5), "stdout", nLines, current);
    }
    catch(const SynchronizationException&)
    {
        throw DeploymentException("server is being updated");
        return 0;
    }
}

FileIteratorPrx 
AdminSessionI::openServerStdErr(const string& id, int nLines, const Ice::Current& current)
{
    try
    {
        return addFileIterator(_database->getServer(id)->getProxy(false, 5), "stderr", nLines, current);
    }
    catch(const SynchronizationException&)
    {
        throw DeploymentException("server is being updated");
        return 0;
    }
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
AdminSessionI::toProxy(const Ice::Identity& id, const Ice::ConnectionPtr& connection, const Ice::EncodingVersion& v)
{
    return id.name.empty() ? Ice::ObjectPrx() : connection->createProxy(id)->ice_encodingVersion(v);
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

    Ice::ObjectPrx obj = _servantManager->add(new FileIteratorI(this, reader, filename, offset, messageSizeMax), this);
    return FileIteratorPrx::uncheckedCast(obj);
}

void
AdminSessionI::removeFileIterator(const Ice::Identity& id, const Ice::Current&)
{
    Lock sync(*this);
    _servantManager->remove(id);
}

void
AdminSessionI::destroyImpl(bool shutdown)
{
    BaseSessionI::destroyImpl(shutdown);

    _servantManager->removeSession(this);

    try
    {
        _database->unlock(this);
    }
    catch(AccessDeniedException&)
    {
    }

    if(!shutdown)
    {
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

AdminSessionFactory::AdminSessionFactory(const SessionServantManagerPtr& servantManager,
                                         const DatabasePtr& database,
                                         const ReapThreadPtr& reaper,
                                         const RegistryIPtr& registry) :
    _servantManager(servantManager),
    _database(database), 
    _timeout(registry->getSessionTimeout()),
    _reaper(reaper),
    _registry(registry),
    _filters(false)
{
    if(_servantManager) // Not set if Glacier2 session manager adapter not enabled
    {
        Ice::PropertiesPtr props = database->getCommunicator()->getProperties();
        const_cast<bool&>(_filters) = props->getPropertyAsIntWithDefault("IceGrid.Registry.AdminSessionFilters", 0) > 0;
    }
}

Glacier2::SessionPrx
AdminSessionFactory::createGlacier2Session(const string& sessionId, const Glacier2::SessionControlPrx& ctl)
{
    assert(_servantManager);

    AdminSessionIPtr session = createSessionServant(sessionId);
    Ice::ObjectPrx proxy = session->_register(_servantManager, 0);

    int timeout = 0;
    if(ctl)
    {
        try
        {
            if(_filters)
            {
                Ice::IdentitySeq ids;
                Ice::Identity queryId;
                queryId.category = _database->getInstanceName();
                queryId.name = "Query";
                ids.push_back(queryId);
                
                _servantManager->setSessionControl(session, ctl, ids);
            }
            timeout = ctl->getSessionTimeout();
        }
        catch(const Ice::LocalException& e)
        {
            session->destroy(Ice::Current());

            Ice::Warning out(_database->getTraceLevels()->logger);
            out << "Failed to callback Glacier2 session control object:\n" << e;

            Glacier2::CannotCreateSessionException ex;
            ex.reason = "internal server error";
            throw ex;
        }
    }

    _reaper->add(new SessionReapable<AdminSessionI>(_database->getTraceLevels()->logger, session), timeout);
    return Glacier2::SessionPrx::uncheckedCast(proxy);
}

AdminSessionIPtr
AdminSessionFactory::createSessionServant(const string& id)
{
    return new AdminSessionI(id, _database, _timeout, _registry);
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
AdminSessionManagerI::create(const string& userId, const Glacier2::SessionControlPrx& ctl, const Ice::Current&)
{
    return _factory->createGlacier2Session(userId, ctl);
}

AdminSSLSessionManagerI::AdminSSLSessionManagerI(const AdminSessionFactoryPtr& factory) : _factory(factory)
{
}

Glacier2::SessionPrx
AdminSSLSessionManagerI::create(const Glacier2::SSLInfo& info, 
                                const Glacier2::SessionControlPrx& ctl,
                                const Ice::Current&)
{
    string userDN;
    if(!info.certs.empty()) // TODO: Require userDN?
    {
        try
        {
            IceSSL::CertificatePtr cert = IceSSL::Certificate::decode(info.certs[0]);
            userDN = cert->getSubjectDN();
        }
        catch(const Ice::Exception& e)
        {
            // This shouldn't happen, the SSLInfo is supposed to be encoded by Glacier2.
            Ice::Error out(_factory->getTraceLevels()->logger);
            out << "SSL session manager couldn't decode SSL certificates:\n" << e;

            Glacier2::CannotCreateSessionException ex;
            ex.reason = "internal server error";
            throw ex;
        }
    }

    return _factory->createGlacier2Session(userDN, ctl);
}

