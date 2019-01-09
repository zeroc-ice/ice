// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/UUID.h>
#include <IceGrid/RegistryI.h>
#include <IceGrid/AdminSessionI.h>
#include <IceGrid/AdminI.h>
#include <IceGrid/Database.h>

#include <IceSSL/Plugin.h>

using namespace std;
using namespace IceGrid;

namespace
{

class SubscriberForwarderI : public Ice::BlobjectArrayAsync
{
    class CallbackI: public IceUtil::Shared
    {
    public:

        virtual void
        exception(const Ice::Exception& ex, const Ice::AMD_Object_ice_invokePtr& amdCB)
        {
            try
            {
                ex.ice_throw();
            }
            catch(const Ice::Exception&)
            {
                // Throw ObjectNotExistException, the subscriber is unreachable
                amdCB->ice_exception(Ice::ObjectNotExistException(__FILE__, __LINE__));
            }
        }

        virtual void
        response(bool ok,
                 const pair<const Ice::Byte*, const Ice::Byte*>& outP,
                 const Ice::AMD_Object_ice_invokePtr& amdCB)
        {
            amdCB->ice_response(ok, outP);
        }
    };

public:

    SubscriberForwarderI(const Ice::ObjectPrx& proxy) :
        _proxy(proxy),
        _callback(newCallback_Object_ice_invoke(new CallbackI(), &CallbackI::response, &CallbackI::exception))
    {
    }

    virtual void
    ice_invoke_async(const Ice::AMD_Object_ice_invokePtr& amdCB,
                     const pair<const Ice::Byte*, const Ice::Byte*>& inParams,
                     const Ice::Current& current)
    {
        _proxy->begin_ice_invoke(current.operation, current.mode, inParams, current.ctx, _callback, amdCB);
    }

private:

    const Ice::ObjectPrx _proxy;
    const Ice::Callback_Object_ice_invokePtr _callback;
};

}

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

    if(con)
    {
        category = _database->getInstanceName() + "-" + Ice::generateUUID();

        Ice::Identity templateId;
        templateId.name = "pick-a-name";
        templateId.category = category;

        _adminCallbackTemplate = _registry->createAdminCallbackProxy(templateId);
    }

    Ice::ObjectPrx session = _servantManager->addSession(this, con, category);

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
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, "", "");
    }

    const int t = _timeout * 1000;
    const Ice::LocatorPrx l = _registry->getLocator();
    if(registryObserver)
    {
        setupObserverSubscription(RegistryObserverTopicName,
                                  addForwarder(registryObserver->ice_timeout(t)->ice_locator(l)));
    }
    else
    {
        setupObserverSubscription(RegistryObserverTopicName, Ice::ObjectPrx());
    }

    if(nodeObserver)
    {
        setupObserverSubscription(NodeObserverTopicName,
                                  addForwarder(nodeObserver->ice_timeout(t)->ice_locator(l)));
    }
    else
    {
        setupObserverSubscription(NodeObserverTopicName, Ice::ObjectPrx());
    }

    if(appObserver)
    {
        setupObserverSubscription(ApplicationObserverTopicName,
                                  addForwarder(appObserver->ice_timeout(t)->ice_locator(l)));
    }
    else
    {
        setupObserverSubscription(ApplicationObserverTopicName, Ice::ObjectPrx());
    }

    if(adapterObserver)
    {
        setupObserverSubscription(AdapterObserverTopicName,
                                  addForwarder(adapterObserver->ice_timeout(t)->ice_locator(l)));
    }
    else
    {
        setupObserverSubscription(AdapterObserverTopicName, Ice::ObjectPrx());
    }

    if(objectObserver)
    {
        setupObserverSubscription(ObjectObserverTopicName,
                                  addForwarder(objectObserver->ice_timeout(t)->ice_locator(l)));
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
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, "", "");
    }

    setupObserverSubscription(RegistryObserverTopicName, addForwarder(registryObserver, current), true);
    setupObserverSubscription(NodeObserverTopicName, addForwarder(nodeObserver, current), true);
    setupObserverSubscription(ApplicationObserverTopicName, addForwarder(appObserver, current), true);
    setupObserverSubscription(AdapterObserverTopicName, addForwarder(adapterObserver, current), true);
    setupObserverSubscription(ObjectObserverTopicName, addForwarder(objectObserver, current), true);
}

int
AdminSessionI::startUpdate(const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, "", "");
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
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, "", "");
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
AdminSessionI::setupObserverSubscription(TopicName name, const Ice::ObjectPrx& observer, bool forwarder)
{
    if(_observers.find(name) != _observers.end() && _observers[name].first != observer)
    {
        _database->getObserverTopic(name)->unsubscribe(_observers[name].first);
        if(_observers[name].second)
        {
            try
            {
                // Unregister forwarder object
                _registry->getRegistryAdapter()->remove(_observers[name].first->ice_getIdentity());
            }
            catch(const Ice::ObjectAdapterDeactivatedException&)
            {
            }
        }
        _observers.erase(name);
    }

    if(observer)
    {
        _observers[name].first = observer;
        _observers[name].second = forwarder;
        _database->getObserverTopic(name)->subscribe(observer);
    }
}

Ice::ObjectPrx
AdminSessionI::addForwarder(const Ice::Identity& id, const Ice::Current& current)
{
    if(id.name.empty())
    {
        return Ice::ObjectPrx();
    }
    return addForwarder(current.con->createProxy(id)->ice_encodingVersion(current.encoding));
}

Ice::ObjectPrx
AdminSessionI::addForwarder(const Ice::ObjectPrx& prx)
{
    return _registry->getRegistryAdapter()->addWithUUID(new SubscriberForwarderI(prx));
}

FileIteratorPrx
AdminSessionI::addFileIterator(const FileReaderPrx& reader, const string& filename, int nLines,
                               const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, "", "");
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
    catch(const AccessDeniedException&)
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
    _timeout(registry->getSessionTimeout(Ice::emptyCurrent)),
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

            throw Glacier2::CannotCreateSessionException("internal server error");
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
        catch(const Ice::Exception& ex)
        {
            // This shouldn't happen, the SSLInfo is supposed to be encoded by Glacier2.
            Ice::Error out(_factory->getTraceLevels()->logger);
            out << "SSL session manager couldn't decode SSL certificates:\n" << ex;
            throw Glacier2::CannotCreateSessionException("internal server error");
        }
    }

    return _factory->createGlacier2Session(userDN, ctl);
}
