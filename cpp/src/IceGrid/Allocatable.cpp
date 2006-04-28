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
    else if(_timeout > 0)
    {
	_session->getWaitQueue()->add(this, IceUtil::Time::milliSeconds(_timeout));
    }
    _state = Pending;
    _session->addAllocationRequest(this);
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
    else
    {
	_state = Allocated;
	allocated(allocatable, _session);
	return true;
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

void 
ParentAllocationRequest::allocated(const AllocatablePtr& allocatable, const SessionIPtr& session)
{
    try
    {
	_allocatable->allocate(_request, false);
	assert(_allocatable->getSession() == _request->getSession());
    }
    catch(const AllocationException& ex)
    {
	_request->cancel(ex);
    }
}

void 
ParentAllocationRequest::canceled(const AllocationException& ex)
{
    _request->canceled(ex);
}

Allocatable::Allocatable() : _allocatable(false), _count(0)
{
}

Allocatable::~Allocatable()
{
}

void
Allocatable::allocate(const AllocationRequestPtr& request, bool checkParent)
{
    IceUtil::RecMutex::Lock sync(_allocateMutex);
    if(!_allocatable)
    {
	throw NotAllocatableException("not allocatable");
    }

    if(_session == request->getSession())
    {
	if(request->finish(this, _session))
	{
	    ++_count;
	}
	return;
    }

    if(_parent && checkParent)
    {
	_parent->allocate(new ParentAllocationRequest(request, this), true);
	return;
    }

    if(_session)
    {
	if(request->pending())
	{
	    _requests.push_back(request);
	}
    } 
    else if(request->finish(this, _session))
    {
	assert(_count == 0);
	_session = request->getSession();
	++_count;
	allocated(_session);
    }
}

bool 
Allocatable::tryAllocateWithSession(const SessionIPtr& session, const AllocatablePtr& child)
{
    IceUtil::RecMutex::Lock sync(_allocateMutex);
    assert(_allocatable);
    if(_session && _session != session)
    {
	_attempts.insert(child); // Remember the allocation attempts of a child
	return false;
    }
    else if(_session == session)
    {
	++_count;
	return true;
    }

    if(_parent && !_parent->tryAllocateWithSession(session, child))
    {
	return false;
    }

    assert(_count == 0);

    _session = session;
    ++_count;
    allocated(_session);
    return true;
}

bool 
Allocatable::tryAllocate(const AllocationRequestPtr& request)
{
    IceUtil::RecMutex::Lock sync(_allocateMutex);

    //
    // If not allocatable or already allocated or if the parent is
    // allocated by a session other than the session from the given
    // request, we can't allocate the allocatable.
    //
    if(!_allocatable || _session)
    {
	return false;
    }

    if(_parent && !_parent->tryAllocateWithSession(request->getSession(), this))
    {
	return false;
    }

    if(request->finish(this, _session))
    {
	assert(_count == 0);
	_session = request->getSession();
	++_count;
	allocated(_session);
    }
    return true; // The allocatable was allocated or the request was canceled.
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
    if(!_session || _session != session)
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
	    if(request->finish(this, _session))
	    {
		_session = request->getSession();
		++_count;
		allocated(_session);
		
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
		return false;
	    }
	}
    }

    releasedAllocatables.insert(_attempts.begin(), _attempts.end());
    _attempts.clear();
    return true;
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

