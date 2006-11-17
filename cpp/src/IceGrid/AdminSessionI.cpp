// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

FileIteratorI::FileIteratorI(const AdminSessionIPtr& session, const FileReaderPrx& reader, const string& filename) :
    _session(session),
    _reader(reader),
    _filename(filename),
    _offset(0)
{
}

Ice::StringSeq
FileIteratorI::read(int nlines, const Ice::Current& current)
{
    try
    {
	return _reader->readLines(_filename, _offset, nlines, _offset);
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

AdminSessionI::AdminSessionI(const string& id, const DatabasePtr& db, int timeout, const string& replicaName) :
    BaseSessionI(id, "admin", db),
    _timeout(timeout),
    _replicaName(replicaName)
{
}

AdminSessionI::~AdminSessionI()
{
}

void
AdminSessionI::setAdmin(const AdminPrx& admin)
{
    const_cast<AdminPrx&>(_admin) = admin;
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
AdminSessionI::openServerStdOut(const std::string& id, const Ice::Current& current)
{
    return addFileIterator(_database->getServer(id), "stdout", current);
}

FileIteratorPrx 
AdminSessionI::openServerStdErr(const std::string& id, const Ice::Current& current)
{
    return addFileIterator(_database->getServer(id), "stderr", current);
}

FileIteratorPrx 
AdminSessionI::openNodeStdOut(const std::string& name, const Ice::Current& current)
{
    return addFileIterator(_database->getNode(name), "stdout", current);
}

FileIteratorPrx 
AdminSessionI::openNodeStdErr(const std::string& name, const Ice::Current& current)
{
    return addFileIterator(_database->getNode(name), "stderr", current);
}

FileIteratorPrx 
AdminSessionI::openRegistryStdOut(const std::string& name, const Ice::Current& current)
{
    FileReaderPrx reader;
    if(name == _replicaName)
    {
	Ice::Identity internalRegistryId;
	internalRegistryId.category = _database->getInstanceName();
	internalRegistryId.name = "InternalRegistry-" + _replicaName;

	Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();
	string proxyStr = communicator->identityToString(internalRegistryId);
	reader = FileReaderPrx::uncheckedCast(communicator->stringToProxy(proxyStr));
    }
    else 
    {
	reader = _database->getReplica(name);
    }

    return addFileIterator(reader, "stdout", current);
}

FileIteratorPrx
AdminSessionI::openRegistryStdErr(const std::string& name, const Ice::Current& current)
{
    FileReaderPrx reader;
    if(name == _replicaName)
    {
	Ice::Identity internalRegistryId;
	internalRegistryId.category = _database->getInstanceName();
	internalRegistryId.name = "InternalRegistry-" + _replicaName;

	Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();
	string proxyStr = communicator->identityToString(internalRegistryId);
	reader = FileReaderPrx::uncheckedCast(communicator->stringToProxy(proxyStr));
    }
    else 
    {
	reader = _database->getReplica(name);
    }

    return addFileIterator(reader, "stderr", current);
}

void
AdminSessionI::destroy(const Ice::Current& current)
{
    BaseSessionI::destroy(current);
    
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
    if(_servantLocator)
    {
	_servantLocator->remove(_admin->ice_getIdentity());
    }
    else if(current.adapter)
    {
	current.adapter->remove(_admin->ice_getIdentity());
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
	else if(current.adapter)
	{
	    try
	    {
		current.adapter->remove(*p);
	    }
	    catch(const Ice::LocalException&)
	    {
	    }
	}
    }

    //
    // Unsubscribe from the topics.
    //
    if(current.adapter) // Not shutting down
    {
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

    Ice::IdentitySeq ids; // Identities of the object the session is allowed to access.

    Ice::Identity id;
    id.category = _database->getInstanceName();

    // The per-session admin object.
    id.name = IceUtil::generateUUID();
    AdminSessionIPtr session = createSessionServant(sessionId);
    AdminPrx admin = AdminPrx::uncheckedCast(_adapter->add(new AdminI(_database, _registry, session), id));
    session->setAdmin(admin);
    ids.push_back(id);

    // The session admin object.
    id.name = IceUtil::generateUUID();
    Glacier2::SessionPrx s = Glacier2::SessionPrx::uncheckedCast(_adapter->add(session, id));
    ids.push_back(id);

    // The IceGrid::Query object
    id.name = "Query";
    ids.push_back(id);

    int timeout = 0;
    if(ctl)
    {
	try
	{
	    ctl->identities()->add(ids);
	}
	catch(const Ice::LocalException&)
	{
	    s->destroy();
	    return 0;
	}
	timeout = ctl->getSessionTimeout();
    }

    if(timeout > 0)
    {
	_reaper->add(new SessionReapable(_adapter, session, s->ice_getIdentity()), timeout);
    }

    return s;
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
AdminSessionI::addFileIterator(const FileReaderPrx& reader, const string& filename, const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }

    Ice::ObjectPrx obj;
    Ice::ObjectPtr servant = new FileIteratorI(this, reader, filename);
    if(_servantLocator)
    {
	obj = _servantLocator->add(servant, current.con);
    }
    else
    {
	obj = current.adapter->addWithUUID(servant);
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
	    current.adapter->remove(id);
	}
	catch(const Ice::LocalException&)
	{
	}
    }
    _iterators.erase(id);
}
