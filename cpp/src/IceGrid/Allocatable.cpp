// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceGrid/Allocatable.h>
#include <IceGrid/SessionI.h>

using namespace std;
using namespace IceGrid;

AllocationRequest::~AllocationRequest()
{
}

bool
AllocationRequest::pending()
{
    Lock sync(*this);
    assert(_state == Initial);
    
    if(_timeout == 0)
    {
	_state = Canceled;
	canceled(AllocationTimeoutException());
	return false;
    }
    else if(!_session->addAllocationRequest(this))
    {
	_state = Canceled;
	canceled(AllocationException("session destroyed"));
	return false;
    }

    if(_timeout > 0)
    {
	_session->getWaitQueue()->add(this, IceUtil::Time::milliSeconds(_timeout));
    }
    _state = Pending;
    return true;
}

bool
AllocationRequest::finish(const AllocatablePtr& allocatable, const SessionIPtr& session)
{
    Lock sync(*this);
    switch(_state)
    {
    case Initial:
	break;
    case Canceled:
	return false;
    case Pending:
	if(_timeout > 0)
	{
	    _session->getWaitQueue()->remove(this);
	}
	_session->removeAllocationRequest(this);
	break;
    case Allocated:
	assert(false);
	break;
    }

    //
    // Check if the allocatable is already allocated by the session
    // and if it's allowed to allocate multiple times the same
    // allocatable.
    //
    if(allocateOnce() && _session == session)
    {
	_state = Canceled;
	canceled(AllocationException("already allocated by the session"));
	return false;
    }
    else if(allocated(allocatable, _session))
    {
	_state = Allocated;
	return true;
    }
    else
    {
	_state = Canceled;
	return false;
    }
}

void 
AllocationRequest::cancel(const AllocationException& ex) 
{
    Lock sync(*this);
    switch(_state)
    {
    case Initial:
	break;
    case Canceled:
    case Allocated:
	return;
    case Pending:
	if(_timeout > 0)
	{
	    _session->getWaitQueue()->remove(this);
	}
	_session->removeAllocationRequest(this);
	break;
    }    

    _state = Canceled;
    canceled(ex);
}

void
AllocationRequest::expired(bool destroyed)
{
    Lock sync(*this);
    switch(_state)
    {
    case Initial:
	assert(false);
    case Canceled:
    case Allocated:
	return;
    case Pending:
	_session->removeAllocationRequest(this);
	break;
    }
    
    _state = Canceled;
    canceled(AllocationTimeoutException());
}

bool
AllocationRequest::isCanceled() const
{
    Lock sync(*this);
    return _state == Canceled;
}

bool
AllocationRequest::operator<(const AllocationRequest& r) const
{
    return this < &r;
}

AllocationRequest::AllocationRequest(const SessionIPtr& session) :
    _session(session),
    _timeout(_session->getAllocationTimeout()), // The session timeout can be updated so we need to cache it here.
    _state(Initial)
{
}

ParentAllocationRequest::ParentAllocationRequest(const AllocationRequestPtr& request, 
						 const AllocatablePtr& allocatable) :
    AllocationRequest(request->getSession()),
    _request(request),
    _allocatable(allocatable)
{
}

bool
ParentAllocationRequest::allocated(const AllocatablePtr& allocatable, const SessionIPtr& session)
{
    try
    {
	if(_allocatable->allocate(_request, false))
	{
	    assert(_allocatable->getSession() == _request->getSession());
	    return true;
	}
	return false;
    }
    catch(const AllocationException& ex)
    {
	_request->cancel(ex);
	return false;
    }
}

void 
ParentAllocationRequest::canceled(const AllocationException& ex)
{
    _request->canceled(ex);
}

Allocatable::Allocatable(bool allocatable, const AllocatablePtr& parent) : 
    _allocatable(allocatable || parent && parent->allocatable()),
    _parent((parent && parent->allocatable()) ? parent : AllocatablePtr()),
    _count(0)
{
    assert(!_parent || _parent->allocatable()); // Parent is only set if it's allocatable.
}

Allocatable::~Allocatable()
{
}

bool
Allocatable::allocate(const AllocationRequestPtr& request, bool checkParent)
{
    IceUtil::RecMutex::Lock sync(_allocateMutex);
    if(!_allocatable)
    {
	throw NotAllocatableException("not allocatable");
    }
    else if(_session == request->getSession())
    {
	if(request->finish(this, _session))
	{
	    ++_count;
	    return true; // Allocated
	}
	return false;
    }

    if(_parent && checkParent)
    {
	return _parent->allocate(new ParentAllocationRequest(request, this), true);
    }

    if(_session)
    {
	if(request->pending())
	{
	    _requests.push_back(request);
	}
    } 
    else if(request->finish(this, _session) && allocated(request->getSession()))
    {
	assert(_count == 0);
	_session = request->getSession();
	++_count;
	return true; // Allocated
    }
    return false;
}

bool 
Allocatable::tryAllocateWithSession(const SessionIPtr& session, const AllocatablePtr& child)
{
    IceUtil::RecMutex::Lock sync(_allocateMutex);
    assert(_allocatable);
    if(_session && _session != session)
    {
	//
	// The allocatable is already allocated by another session.
	//
	// We keep track of the allocation attempt of a child. E.g.:
	// if a session tries to allocate an object and it can't be
	// allocated because the adapter is already allocated, we keep
	// track of the object here. This will be used by release() to
	// notify the object cache when the adapter is released that
	// the object is potentially available.
	//
	_attempts.insert(child);
	return false;
    }
    else if(_session == session)
    {
	//
	// The allocatable is allocated by this session, we just 
	// increment the allocation count and return a true to 
	// indicate a successfull allocation.
	//
	++_count;
	return true;
    }

    //
    // This allocatable isn't allocated, so we now have to check if
    // the parent is allocated or not. If the allocation of the parent
    // is succsefull (returns "true"), we allocate this allocatable.
    //
    if(_parent && !_parent->tryAllocateWithSession(session, child))
    {
	return false;
    }

    if(allocated(session))
    {
	assert(_count == 0);
	_session = session;
	++_count;
	return true; // Successfull allocation
    }
    return false;
}

bool 
Allocatable::tryAllocate(const AllocationRequestPtr& request)
{
    IceUtil::RecMutex::Lock sync(_allocateMutex);
    
    //
    // If not allocatable or already allocated, the allocation attempt
    // fails.
    //
    if(!_allocatable || _session)
    {
	return false;
    }
    
    //
    // Try to allocate the parent. This should succeed if the parent 
    // is not already allocated or if it's already allocated by the
    // session.
    //
    if(_parent && !_parent->tryAllocateWithSession(request->getSession(), this))
    {
	return false;
    }
    
    //
    // The parent could be allocated, we allocate this allocatable.
    // 
    if(request->finish(this, _session) && allocated(request->getSession()))
    {
	assert(_count == 0);
	_session = request->getSession();
	++_count;
	return true; // The allocatable was allocated.
    }

    //
    // If we reach here, either the request was canceled or the session
    // destroyed. If that's the case, we need to release the parent.
    //
    if(_parent)
    {
	set<AllocatablePtr> releasedAllocatables;
	if(_parent->release(request->getSession(), false, releasedAllocatables))
	{
	    assert(releasedAllocatables.empty());
	}
    }

    return true; // The request was canceled.
}

bool
Allocatable::release(const SessionIPtr&)
{
    assert(false);
    return false;
}

bool
Allocatable::release(const SessionIPtr& session, bool all, set<AllocatablePtr>& releasedAllocatables)
{
    IceUtil::RecMutex::Lock sync(_allocateMutex);
    if(!_allocatable)
    {
	throw NotAllocatableException("not allocatable");
    }
    else if(!_session || _session != session)
    {
	throw AllocationException("can't release object which is not allocated");
    }

    if(!all && --_count)
    {
	return false;
    }
    _session = 0;
    _count = 0;

    released(session); 

    if(_parent)
    {
	assert(_requests.empty());
	_parent->release(session, false, releasedAllocatables);
    }
    else
    {
	//
	// Allocate the allocatable to another session.
	//
	while(!_requests.empty())
	{
	    AllocationRequestPtr request = _requests.front();
	    _requests.pop_front();
	    if(request->finish(this, _session) && allocated(request->getSession()))
	    {
		_session = request->getSession();
		++_count;
		
		//
		// Check if there's other requests from the session
		// waiting to allocate this allocatable.
		//
		list<AllocationRequestPtr>::iterator p = _requests.begin();
		while(p != _requests.end())
		{
		    if((*p)->getSession() == _session)
		    {
			if((*p)->finish(this, _session))
			{
			    ++_count;
			}
			p = _requests.erase(p);
		    }
		    else
		    {
			++p;
		    }
		}
		return false; // Not released yet! 
	    }
	}
    }

    releasedAllocatables.insert(_attempts.begin(), _attempts.end());
    _attempts.clear();
    return true; // The allocatable is released.
}

bool
Allocatable::isAllocated() const
{
    IceUtil::RecMutex::Lock sync(_allocateMutex);
    return _session || _parent && _parent->isAllocated();
}

SessionIPtr
Allocatable::getSession() const
{
    IceUtil::RecMutex::Lock sync(_allocateMutex);
    return _session;
}

bool
Allocatable::operator<(const Allocatable& r) const
{
    return this < &r;
}

