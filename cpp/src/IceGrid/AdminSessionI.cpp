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

AdminSessionI::AdminSessionI(const string& id, 
			     const DatabasePtr& db,
			     int timeout,
			     const RegistryObserverTopicPtr& registryObserverTopic,
			     const NodeObserverTopicPtr& nodeObserverTopic) :
    BaseSessionI(id, "admin", db, timeout),
    _registryObserverTopic(registryObserverTopic),
    _nodeObserverTopic(nodeObserverTopic)
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
    // Subscribe to the topics.
    //
    if(registryObserver)
    {
	if(_registryObserver)
	{
	    _registryObserverTopic->unsubscribe(_registryObserver);
	}
	_registryObserver = RegistryObserverPrx::uncheckedCast(registryObserver->ice_timeout(_timeout * 1000));
	_registryObserverTopic->subscribe(_registryObserver); 
    }
    if(nodeObserver)
    {
	if(_nodeObserver)
	{
	    _nodeObserverTopic->unsubscribe(_nodeObserver);
	}
	_nodeObserver = NodeObserverPrx::uncheckedCast(nodeObserver->ice_timeout(_timeout * 1000));
	_nodeObserverTopic->subscribe(_nodeObserver);
    }
}

void
AdminSessionI::setObserversByIdentity(const Ice::Identity& registryObserver, 
				      const Ice::Identity& nodeObserver,
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
    // Subscribe to the topics.
    //
    if(!registryObserver.name.empty())
    {
	if(_registryObserver)
	{
	    _registryObserverTopic->unsubscribe(_registryObserver);
	}
	_registryObserver = RegistryObserverPrx::uncheckedCast(current.con->createProxy(registryObserver));
	_registryObserverTopic->subscribe(_registryObserver);
    }
    if(!nodeObserver.name.empty())
    {
	if(_nodeObserver)
	{
	    _nodeObserverTopic->unsubscribe(_nodeObserver);
	}
	_nodeObserver = NodeObserverPrx::uncheckedCast(current.con->createProxy(nodeObserver));
	_nodeObserverTopic->subscribe(_nodeObserver);
    }
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
    // Unsubscribe from the topics.
    //
    if(current.adapter) // Not shutting down
    {
	if(_registryObserver) // Immutable once _destroy = true
	{
	    _registryObserverTopic->unsubscribe(_registryObserver);
	    _registryObserver = 0;
	}
	if(_nodeObserver)
	{
	    _nodeObserverTopic->unsubscribe(_nodeObserver);
	    _nodeObserver = 0;
	}
    }
}

AdminSessionFactory::AdminSessionFactory(const Ice::ObjectAdapterPtr& adapter,
					 const DatabasePtr& database,
					 int sessionTimeout,
					 const RegistryObserverTopicPtr& regTopic,
					 const NodeObserverTopicPtr& nodeTopic,
					 const RegistryIPtr& registry) :
    _adapter(adapter),
    _database(database), 
    _timeout(sessionTimeout),
    _registryObserverTopic(regTopic),
    _nodeObserverTopic(nodeTopic),
    _registry(registry)
{
}

Glacier2::SessionPrx
AdminSessionFactory::createGlacier2Session(const string& sessionId, const Glacier2::SessionControlPrx& ctl)
{
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
    }

    return s;
}

AdminSessionIPtr
AdminSessionFactory::createSessionServant(const string& id)
{
    return new AdminSessionI(id, _database, _timeout, _registryObserverTopic, _nodeObserverTopic);
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
