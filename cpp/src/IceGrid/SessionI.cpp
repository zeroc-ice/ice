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

BaseSessionI::BaseSessionI(const string& userId, 
			   const string& prefix, 
			   const DatabasePtr& database,
			   int timeout) :
    _userId(userId), 
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
	out << _prefix << " session `" << _userId << "' created";
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
	out << _prefix << " session `" << _userId << "' keep alive";
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

    if(current.adapter)
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
	out << _prefix << " session `" << _userId << "' destroyed";
    }
}

IceUtil::Time
BaseSessionI::timestamp() const
{
    Lock sync(*this);
    return _timestamp;
}

SessionI::SessionI(const string& userId, 
		   const DatabasePtr& database, 
		   int timeout,
		   const WaitQueuePtr& waitQueue,
		   const Glacier2::SessionControlPrx& sessionControl) :
    BaseSessionI(userId, "client", database, timeout),
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
	    assert(false);
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
    //
    // TODO: XXX: Update the Glacier2 allowable table to allow access to this object!
    //
    return Glacier2::SessionPrx::uncheckedCast(current.adapter->addWithUUID(create(user, ctl)));
}

SessionIPtr
ClientSessionManagerI::create(const string& userId, const Glacier2::SessionControlPrx& ctl)
{
    return new SessionI(userId, _database, _timeout, _waitQueue, ctl);
}
