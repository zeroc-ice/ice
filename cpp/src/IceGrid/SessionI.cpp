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
#include <IceGrid/LocatorI.h>
#include <IceGrid/Database.h>

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
		   const WaitQueuePtr& waitQueue,
		   const Ice::LocatorRegistryPrx& registry,
		   int timeout) :
    _userId(userId), 
    _prefix(prefix),
    _timeout(timeout),
    _traceLevels(database->getTraceLevels()),
    _database(database),
    _waitQueue(waitQueue),
    _destroyed(false),
    _timestamp(IceUtil::Time::now()),
    _allocationTimeout(-1)
{
    if(_traceLevels && _traceLevels->session > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->sessionCat);
	out << _prefix << " session `" << _userId << "' created";
    }
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
    set<AllocationRequestPtr> requests;
    set<AllocatablePtr> allocations;
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

	requests.swap(_requests);
	allocations.swap(_allocations);
    }

    for(set<AllocationRequestPtr>::const_iterator p = requests.begin(); p != requests.end(); ++p)
    {
	(*p)->cancel(AllocationException("session destroyed"));
    }
    for(set<AllocatablePtr>::const_iterator q = allocations.begin(); q != allocations.end(); ++q)
    {
	try
	{
	    (*q)->release(this);
	}
	catch(const AllocationException&)
	{
	    assert(false);
	}
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
    _requests.erase(request);
}

bool
SessionI::addAllocation(const AllocatablePtr& allocatable)
{
    Lock sync(*this);
    if(_destroyed)
    {
	return false;
    }
    _allocations.insert(allocatable);
    return true;
}

void
SessionI::removeAllocation(const AllocatablePtr& allocatable)
{
    Lock sync(*this);
    _allocations.erase(allocatable);
}

ClientSessionI::ClientSessionI(const string& userId, 
			       const DatabasePtr& database, 
			       const Ice::ObjectAdapterPtr& adapter,
			       const WaitQueuePtr& waitQueue,
			       const Ice::LocatorRegistryPrx& registry,
			       int timeout) :
    SessionI(userId, "client", database, adapter, waitQueue, registry, timeout)
{
}

ClientSessionManagerI::ClientSessionManagerI(const DatabasePtr& database,
					     const ReapThreadPtr& reaper,
					     const WaitQueuePtr& waitQueue,
					     const Ice::LocatorRegistryPrx& registry,
					     int timeout) :
    _database(database), 
    _reaper(reaper),
    _waitQueue(waitQueue),
    _registry(registry),
    _timeout(timeout)
{
    //
    // TODO: XXX: Remove _registry attribute
    //
}

Glacier2::SessionPrx
ClientSessionManagerI::create(const string& userId, const Glacier2::SessionControlPrx&, const Ice::Current& current)
{
    //
    // We don't add the session to the reaper thread, Glacier2 takes
    // care of reaping the session.
    //
    SessionIPtr session = new ClientSessionI(userId, _database, current.adapter, _waitQueue, _registry, _timeout);
    return Glacier2::SessionPrx::uncheckedCast(current.adapter->addWithUUID(session)); // TODO: XXX: category = userid?
}

SessionPrx
ClientSessionManagerI::createLocalSession(const string& userId, const Ice::Current& current)
{
    SessionIPtr session = new ClientSessionI(userId, _database, current.adapter, _waitQueue, _registry, _timeout);
    SessionPrx proxy = SessionPrx::uncheckedCast(current.adapter->addWithUUID(session));
    _reaper->add(new SessionReapable(session, proxy));
    return proxy;
}

