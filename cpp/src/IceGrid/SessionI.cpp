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
#include <IceGrid/SessionI.h>
#include <IceGrid/QueryI.h>
#include <IceGrid/LocatorI.h>
#include <IceGrid/Database.h>

#include <IceSSL/Plugin.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

template<class T>
class AllocateObject : public ObjectAllocationRequest
{
    typedef IceUtil::Handle<T> TPtr;

public:

    AllocateObject(const SessionIPtr& session, const TPtr& cb) :
	ObjectAllocationRequest(session), _cb(cb)
    {
    }

    virtual void
    response(const Ice::ObjectPrx& proxy)
    {
	assert(_cb);
	_cb->ice_response(proxy);
	_cb = 0;
    }

    virtual void
    exception(const AllocationException& ex)
    {
	assert(_cb);
	_cb->ice_exception(ex);
	_cb = 0;
    }

private:

    TPtr _cb;
};

template<class T> static AllocateObject<T>*
newAllocateObject(const SessionIPtr& session, const IceUtil::Handle<T>& cb)
{
    return new AllocateObject<T>(session, cb);
}

};

BaseSessionI::BaseSessionI(const string& id, 
			   const string& prefix, 
			   const DatabasePtr& database,
			   int timeout) :
    _id(id), 
    _prefix(prefix),
    _timeout(timeout),
    _traceLevels(database->getTraceLevels()),
    _database(database),
    _destroyed(false),
    _timestamp(IceUtil::Time::now())
{
    if(_traceLevels && _traceLevels->session > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->sessionCat);
	out << _prefix << " session `" << _id << "' created";
    }
}

BaseSessionI::~BaseSessionI()
{
}

void
BaseSessionI::keepAlive(const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }

    _timestamp = IceUtil::Time::now();

    if(_traceLevels->session > 1)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->sessionCat);
	out << _prefix << " session `" << _id << "' keep alive";
    }
}

int
BaseSessionI::getTimeout(const Ice::Current&) const
{
    return _timeout;
}

void
BaseSessionI::destroy(const Ice::Current& current)
{
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }
    _destroyed = true;

    if(_servantLocator)
    {
	_servantLocator->remove(current.id);
    }
    else if(current.adapter)
    {
	try
	{
	    current.adapter->remove(current.id);
	}
	catch(const Ice::ObjectAdapterDeactivatedException&)
	{
	}
    }
	
    if(_traceLevels && _traceLevels->session > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->sessionCat);
	out << _prefix << " session `" << _id << "' destroyed";
    }
}

IceUtil::Time
BaseSessionI::timestamp() const
{
    Lock sync(*this);
    return _timestamp;
}

void
BaseSessionI::setServantLocator(const SessionServantLocatorIPtr& servantLocator)
{
    //
    // This is supposed to be called after creation only.
    //
    const_cast<SessionServantLocatorIPtr&>(_servantLocator) = servantLocator;
}

SessionI::SessionI(const string& id, 
		   const DatabasePtr& database, 
		   int timeout,
		   const WaitQueuePtr& waitQueue,
		   const Glacier2::SessionControlPrx& sessionControl) :
    BaseSessionI(id, "client", database, timeout),
    _waitQueue(waitQueue),
    _sessionControl(sessionControl),
    _allocationTimeout(-1)
{
}

SessionI::~SessionI()
{
}

void
SessionI::allocateObjectById_async(const AMD_Session_allocateObjectByIdPtr& cb,
				   const Ice::Identity& id, 
				   const Ice::Current&)
{
    _database->allocateObject(id, newAllocateObject(this, cb));
}

void
SessionI::allocateObjectByType_async(const AMD_Session_allocateObjectByTypePtr& cb, 
				     const string& type,
				     const Ice::Current&)
{
    _database->allocateObjectByType(type, newAllocateObject(this, cb));
}

void
SessionI::releaseObject(const Ice::Identity& id, const Ice::Current&)
{
    _database->releaseObject(id, this);
}

void
SessionI::setAllocationTimeout(int timeout, const Ice::Current&)
{
    Lock sync(*this);
    _allocationTimeout = timeout;
}

void
SessionI::destroy(const Ice::Current& current)
{
    BaseSessionI::destroy(current);

    //
    // NOTE: The _requests and _allocations attributes are immutable
    // once the session is destroyed so we don't need mutex protection
    // here to access them.
    //

    for(set<AllocationRequestPtr>::const_iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
	(*p)->cancel(AllocationException("session destroyed"));
    }
    _requests.clear();

    for(set<AllocatablePtr>::const_iterator q = _allocations.begin(); q != _allocations.end(); ++q)
    {
	try
	{
	    (*q)->release(this);
	}
	catch(const AllocationException&)
	{
	}
    }
    _allocations.clear();
}

int
SessionI::getAllocationTimeout() const
{
    Lock sync(*this);
    return _allocationTimeout;
}

bool
SessionI::addAllocationRequest(const AllocationRequestPtr& request)
{
    Lock sync(*this);
    if(_destroyed)
    {
	return false;
    }
    _requests.insert(request);
    return true;
}

void
SessionI::removeAllocationRequest(const AllocationRequestPtr& request)
{
    Lock sync(*this);
    if(_destroyed)
    {
	return;
    }
    _requests.erase(request);
}

void
SessionI::addAllocation(const AllocatablePtr& allocatable)
{
    Lock sync(*this);
    if(_destroyed)
    {
	throw SessionDestroyedException();
    }
    _allocations.insert(allocatable);
}

void
SessionI::removeAllocation(const AllocatablePtr& allocatable)
{
    Lock sync(*this);
    if(_destroyed)
    {
	return;
    }
    _allocations.erase(allocatable);
}

ClientSessionManagerI::ClientSessionManagerI(const DatabasePtr& database, int timeout, const WaitQueuePtr& waitQueue) :
    _database(database), 
    _timeout(timeout),
    _waitQueue(waitQueue)
{
}

Glacier2::SessionPrx
ClientSessionManagerI::create(const string& user, const Glacier2::SessionControlPrx& ctl, const Ice::Current& current)
{
    Ice::Identity id;
    id.name = IceUtil::generateUUID();
    id.category = current.id.category;
    Glacier2::SessionPrx s = Glacier2::SessionPrx::uncheckedCast(current.adapter->add(create(user, ctl), id));
    if(ctl)
    {
	try
	{
	    //
	    // Restrict the objects the session is allowed to access to the session object itself,
	    // the query and registry objects.
	    //
	    Ice::IdentitySeq ids;
	    Ice::Identity id;
	    id.category = "IceGrid";
	    id.name = "Query";
	    ids.push_back(id);
	    id.name = "Registry";
	    ids.push_back(id);
	    ids.push_back(s->ice_getIdentity());
	    ctl->objectIdFilter()->addAccept(ids);
	}
	catch(const Ice::LocalException&)
	{
	    s->destroy();
	    return 0;
	}
    }
    return s;
}

SessionIPtr
ClientSessionManagerI::create(const string& userId, const Glacier2::SessionControlPrx& ctl)
{
    return new SessionI(userId, _database, _timeout, _waitQueue, ctl);
}

ClientSSLSessionManagerI::ClientSSLSessionManagerI(const DatabasePtr& db, int timeout, const WaitQueuePtr& waitQueue) :
    _database(db), 
    _timeout(timeout),
    _waitQueue(waitQueue)
{
}

Glacier2::SessionPrx
ClientSSLSessionManagerI::create(const Glacier2::SSLInfo& info, const Glacier2::SessionControlPrx& ctl, 
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
	    Ice::Error out(_database->getTraceLevels()->logger);
	    out << "SSL session manager couldn't decode SSL certificates:\n" << ex;
	    return 0;
	}
    }
	
    SessionIPtr session = new SessionI(userDN, _database, _timeout, _waitQueue, ctl);
    Ice::Identity id;
    id.name = IceUtil::generateUUID();
    id.category = current.id.category;
    Glacier2::SessionPrx s = Glacier2::SessionPrx::uncheckedCast(current.adapter->add(session, id));
    if(ctl)
    {
	try
	{
	    //
	    // Restrict the objects the session is allowed to access to the session object itself,
	    // the query and registry objects.
	    //
	    Ice::IdentitySeq ids;
	    Ice::Identity id;
	    id.category = "IceGrid";
	    id.name = "Query";
	    ids.push_back(id);
	    id.name = "Registry";
	    ids.push_back(id);
	    ids.push_back(s->ice_getIdentity());
	    ctl->objectIdFilter()->addAccept(ids);
	}
	catch(const Ice::LocalException&)
	{
	    s->destroy();
	    return 0;
	}
    }
    return s;
}
