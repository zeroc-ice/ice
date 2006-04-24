// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/SessionI.h>
#include <IceGrid/QueryI.h>
#include <IceGrid/Database.h>

using namespace std;
using namespace IceGrid;

class AllocateObject : public ObjectAllocationRequest
{
public:

    AllocateObject(const SessionIPtr& session, const AMD_Session_allocateObjectPtr& cb) :
	ObjectAllocationRequest(session), _cb(cb)
    {
    }

    virtual void response(const Ice::ObjectPrx& proxy)
    {
	assert(_cb);
	if(proxy)
	{
	    _cb->ice_response();
	}
	else
	{
	    //
	    // TODO: The request might also have been canceled!
	    //

	    _cb->ice_exception(AllocationTimeoutException());
	}
	_cb = 0;
    }

private:

    AMD_Session_allocateObjectPtr _cb;
};

SessionReapable::SessionReapable(const SessionIPtr& session, const SessionPrx& proxy) : 
    _session(session),
    _proxy(proxy)
{
}

SessionReapable::~SessionReapable()
{
}

IceUtil::Time
SessionReapable::timestamp() const
{
    return _session->timestamp();
}

void
SessionReapable::destroy()
{
    _proxy->destroy();
}

SessionI::SessionI(const string& userId, 
		   const string& prefix, 
		   const DatabasePtr& database,
		   const Ice::ObjectAdapterPtr& adapter,
		   int timeout) :
    _userId(userId), 
    _prefix(prefix),
    _timeout(timeout),
    _traceLevels(database->getTraceLevels()),
    _database(database),
    _destroyed(false),
    _allocationTimeout(-1)
{
    if(_traceLevels && _traceLevels->session > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->sessionCat);
	out << _prefix << " session `" << _userId << "' created";
    }

    //
    // Register session based query and locator interfaces
    //
    _query = QueryPrx::uncheckedCast(adapter->addWithUUID(new QueryI(adapter->getCommunicator(), _database, this)));
    //_locator = adapter->addWithUUID(new LocatorI());
}

SessionI::~SessionI()
{
}

void
SessionI::keepAlive(const Ice::Current& current)
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
	out << _prefix << " session `" << _userId << "' keep alive";
    }
}

int
SessionI::getTimeout(const Ice::Current&) const
{
    return _timeout;
}

QueryPrx
SessionI::getQuery(const Ice::Current& current) const
{
    return _query;
}

Ice::LocatorPrx
SessionI::getLocator(const Ice::Current& current) const
{
    return _locator;
}

void
SessionI::allocateObject_async(const AMD_Session_allocateObjectPtr& cb, const Ice::ObjectPrx& prx, const Ice::Current&)
{
    //
    // TODO: Check if the proxy points to a replicated object and eventually throw if that's the case.
    //
    if(!prx)
    {
	throw AllocationException("proxy is null");
    }
    _database->allocateObject(prx->ice_getIdentity(), new AllocateObject(this, cb), true);
}

void
SessionI::releaseObject(const Ice::ObjectPrx& prx, const Ice::Current&)
{
    //
    // TODO: Check if the proxy points to a replicated object and eventually throw if that's the case.
    //
    if(!prx)
    {
	throw AllocationException("proxy is null");
    }
    _database->releaseObject(prx->ice_getIdentity(), this);
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
    Lock sync(*this);
    if(_destroyed)
    {
	Ice::ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = current.id;
	throw ex;
    }
    _destroyed = true;
    current.adapter->remove(current.id);

    if(_traceLevels && _traceLevels->session > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->sessionCat);
	out << _prefix << " session `" << _userId << "' destroyed";
    }
}

IceUtil::Time
SessionI::timestamp() const
{
    Lock sync(*this);
    return _timestamp;
}

int
SessionI::getAllocationTimeout() const
{
    Lock sync(*this);
    return _allocationTimeout;
}

void
SessionI::addAllocationRequest(const AllocationRequestPtr& request)
{
    Lock sync(*this);
    _allocations.insert(request);
}

void
SessionI::removeAllocationRequest(const AllocationRequestPtr& request)
{
    Lock sync(*this);
    _allocations.erase(request);
}

ClientSessionI::ClientSessionI(const string& userId, const DatabasePtr& database, const Ice::ObjectAdapterPtr& adapter,
			       int timeout) :
    SessionI(userId, "client", database, adapter, timeout)
{
}

ClientSessionManagerI::ClientSessionManagerI(const DatabasePtr& database,
					     const ReapThreadPtr& reaper,
					     int sessionTimeout) :
    _database(database), 
    _reaper(reaper),
    _sessionTimeout(sessionTimeout)
{
}

Glacier2::SessionPrx
ClientSessionManagerI::create(const string& userId, const Glacier2::SessionControlPrx&, const Ice::Current& current)
{
    //
    // We don't add the session to the reaper thread, Glacier2 takes
    // care of reaping the session.
    //
    SessionIPtr session = new ClientSessionI(userId, _database, current.adapter, _sessionTimeout);
    return Glacier2::SessionPrx::uncheckedCast(current.adapter->addWithUUID(session)); // TODO: XXX: category = userid?
}

SessionPrx
ClientSessionManagerI::createLocalSession(const string& userId, const Ice::Current& current)
{
    SessionIPtr session = new ClientSessionI(userId, _database, current.adapter, _sessionTimeout);
    SessionPrx proxy = SessionPrx::uncheckedCast(current.adapter->addWithUUID(session));
    _reaper->add(new SessionReapable(session, proxy));
    return proxy;
}

