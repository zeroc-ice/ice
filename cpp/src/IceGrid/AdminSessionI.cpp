// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/AdminSessionI.h>
#include <IceGrid/Database.h>

#include <IceSSL/Plugin.h>

using namespace std;
using namespace IceGrid;

AdminSessionI::AdminSessionI(const string& id, 
			     const DatabasePtr& database,
			     int timeout,
			     const RegistryObserverTopicPtr& registryObserverTopic,
			     const NodeObserverTopicPtr& nodeObserverTopic) :
    BaseSessionI(id, "admin", database, timeout),
    _registryObserverTopic(registryObserverTopic),
    _nodeObserverTopic(nodeObserverTopic),
    _updating(false)
{
}

AdminSessionI::~AdminSessionI()
{
}

AdminPrx
AdminSessionI::getAdmin(const Ice::Current& current) const
{
    return AdminPrx::uncheckedCast(
	current.adapter->getCommunicator()->stringToProxy(_database->getInstanceName() + "/Admin"));
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
    _updating = true;
    return serial;
}

void
AdminSessionI::addApplication(const ApplicationDescriptor& app, const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }

    if(!_updating)
    {
	throw AccessDeniedException();
    }
    _database->addApplicationDescriptor(this, app);
}

void
AdminSessionI::updateApplication(const ApplicationUpdateDescriptor& update, const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }

    if(!_updating)
    {
	throw AccessDeniedException();
    }
    _database->updateApplicationDescriptor(this, update);
}

void
AdminSessionI::syncApplication(const ApplicationDescriptor& app, const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }

    if(!_updating)
    {
	throw AccessDeniedException();
    }
    _database->syncApplicationDescriptor(this, app);
}

void
AdminSessionI::removeApplication(const string& name, const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }

    if(!_updating)
    {
	throw AccessDeniedException();
    }
    _database->removeApplicationDescriptor(this, name);
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

    if(!_updating)
    {
	throw AccessDeniedException();
    }
    _database->unlock(this);
    _updating = false;
}

void
AdminSessionI::destroy(const Ice::Current& current)
{
    BaseSessionI::destroy(current);
    
    if(_updating) // Immutable once _destroy = true
    {
	_database->unlock(this);
	_updating = false;
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

AdminSessionManagerI::AdminSessionManagerI(const DatabasePtr& database,
					   int sessionTimeout,
					   const RegistryObserverTopicPtr& regTopic,
					   const NodeObserverTopicPtr& nodeTopic) :
    _database(database), 
    _timeout(sessionTimeout),
    _registryObserverTopic(regTopic),
    _nodeObserverTopic(nodeTopic)
{
}

Glacier2::SessionPrx
AdminSessionManagerI::create(const string& id, const Glacier2::SessionControlPrx&, const Ice::Current& current)
{
    return Glacier2::SessionPrx::uncheckedCast(current.adapter->addWithUUID(create(id)));
}

AdminSessionIPtr
AdminSessionManagerI::create(const string& id)
{
    return new AdminSessionI(id, _database, _timeout, _registryObserverTopic, _nodeObserverTopic);
}

AdminSSLSessionManagerI::AdminSSLSessionManagerI(const DatabasePtr& database,
						 int sessionTimeout,
						 const RegistryObserverTopicPtr& regTopic,
						 const NodeObserverTopicPtr& nodeTopic) :
    _database(database), 
    _timeout(sessionTimeout),
    _registryObserverTopic(regTopic),
    _nodeObserverTopic(nodeTopic)
{
}

Glacier2::SessionPrx
AdminSSLSessionManagerI::create(const Glacier2::SSLInfo& info, const Glacier2::SessionControlPrx&, 
				const Ice::Current& current)
{
    IceSSL::CertificatePtr cert = IceSSL::Certificate::decode(info.certs[0]);
    string id = cert->getSubjectDN();
    AdminSessionIPtr session = new AdminSessionI(id, _database, _timeout, _registryObserverTopic, _nodeObserverTopic);
    return Glacier2::SessionPrx::uncheckedCast(current.adapter->addWithUUID(session));
}

