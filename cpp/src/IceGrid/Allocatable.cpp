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
    // Check if the allocatable is already allocated by the session.
    //
    if(_session == session)
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

Allocatable::Allocatable(bool allocatable, const AllocatablePtr& parent) : 
    _allocatable(allocatable || parent && parent->isAllocatable()),
    _parent((parent && parent->isAllocatable()) ? parent : AllocatablePtr()),
    _count(0),
    _releasing(false)
{
    assert(!_parent || _parent->isAllocatable()); // Parent is only set if it's allocatable.
}

Allocatable::~Allocatable()
{
}

bool
Allocatable::allocate(const AllocationRequestPtr& request, bool fromRelease)
{
    if(!_allocatable)
    {
	throw NotAllocatableException("not allocatable");
    }

    try
    {
	return allocate(request, false, fromRelease);
    }
    catch(const AllocationException&)
    {
	return false; // The session was destroyed
    }
}

bool
Allocatable::tryAllocate(const AllocationRequestPtr& request, bool fromRelease)
{
    if(!_allocatable)
    {
	return false;
    }

    return allocate(request, true, fromRelease);
}

void
Allocatable::release(const SessionIPtr& session, bool fromRelease)
{
    if(!_allocatable)
    {
	throw NotAllocatableException("not allocatable");
    }

    bool isReleased = false;
    bool hasRequests = false;
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_allocateMutex);
	if(!fromRelease)
	{
	    while(_releasing)
	    {
		_allocateMutex.wait();
	    }
	}

	if(!_session || _session != session)
	{
	    throw AllocationException("can't release object which is not allocated");
	}
	
	if(--_count == 0)
	{
	    _session = 0;

	    released(session);

	    if(!_releasing)
	    {
		if(!_parent && !_requests.empty())
		{
		    _releasing = true; // Prevent new allocations.
		    hasRequests = true;
		}
		
		isReleased = true;
	    }
	}
    }

    if(_parent)
    {
	_parent->release(session, fromRelease);
	return;
    }

    if(hasRequests)
    {
	while(true)
	{
	    AllocationRequestPtr request;
	    AllocatablePtr allocatable = dequeueAllocationAttempt(request);
	    if(!allocatable)
	    {
		IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_allocateMutex);
		assert(_count == 0 && _requests.empty());
		_releasing = false;
		_allocateMutex.notifyAll();
		return;
	    }
	    
	    //
	    // Try to allocate the allocatable with the request or if
	    // there's no request, just notify the allocatable that it can
	    // be allocated again.
	    //
	    if(request && allocatable->allocate(request, true) || !request && allocatable->canTryAllocate())
	    {
		IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_allocateMutex);
		assert(_count);
		
		//
		// Check if there's other requests from the session
		// waiting to allocate this allocatable.
		//
		list<pair<AllocatablePtr, AllocationRequestPtr> >::iterator p = _requests.begin();
		while(p != _requests.end())
		{
		    if(p->second && p->second->getSession() == _session)
		    {
			if(p->second->finish(this, _session))
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
		_releasing = false;
		_allocateMutex.notifyAll();
		return; // We're done, the allocatable is allocated again!
	    }
	}
    }
    else if(isReleased)
    {
	canTryAllocate(); // Notify that this allocatable can be allocated.
    }
}

SessionIPtr
Allocatable::getSession() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_allocateMutex);
    return _session;
}

bool
Allocatable::operator<(const Allocatable& r) const
{
    return this < &r;
}

void
Allocatable::queueAllocationAttempt(const AllocatablePtr& allocatable, 
				    const AllocationRequestPtr& request, 
				    bool tryAllocate)
{
    assert(!_parent);
    if(!tryAllocate)
    {
	if(request->pending())
	{
	    _requests.push_back(make_pair(allocatable, request));
	}
    }
    else
    {
	if(_attempts.insert(allocatable).second)
	{
	    _requests.push_back(make_pair(allocatable, AllocationRequestPtr()));
	}
    }    
}

AllocatablePtr
Allocatable::dequeueAllocationAttempt(AllocationRequestPtr& request)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_allocateMutex);
    if(_requests.empty())
    {
	return 0;
    }

    pair<AllocatablePtr, AllocationRequestPtr> alloc = _requests.front();
    _requests.pop_front();
    if(alloc.second)
    {
	request = alloc.second;
    }
    else
    {
	_attempts.erase(alloc.first);
    }
    return alloc.first;
}

bool
Allocatable::allocate(const AllocationRequestPtr& request, bool tryAllocate, bool fromRelease)
{
    if(_parent && !_parent->allocateFromChild(request, this, tryAllocate, fromRelease))
    {
	return false;
    }

    try
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_allocateMutex);
	if(!_session && (fromRelease || !_releasing))
	{
	    if(request->finish(this, _session))
	    {
		allocated(request->getSession()); // This might throw.
		assert(_count == 0);
		_session = request->getSession();
		++_count;
		return true; // Allocated
	    }
	}
	else if(_session == request->getSession())
	{
	    if(!tryAllocate && request->finish(this, _session))
	    {
		assert(_count > 0);
		++_count;
		return true; // Allocated
	    }
	}
	else
	{
	    queueAllocationAttempt(this, request, tryAllocate);
	}
    }
    catch(const AllocationException& ex)
    {
	if(_parent)
	{
	    _parent->release(request->getSession(), fromRelease);
	}
	throw ex;
    }
    if(_parent)
    {
	_parent->release(request->getSession(), fromRelease);
    }
    return false;
}

bool
Allocatable::allocateFromChild(const AllocationRequestPtr& request, 
			       const AllocatablePtr& child, 
			       bool tryAllocate,
			       bool fromRelease)
{
    assert(_allocatable);

    if(_parent && !_parent->allocateFromChild(request, child, tryAllocate, fromRelease))
    {
	return false;
    }

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_allocateMutex);
    if(!_session && (fromRelease || !_releasing) || _session == request->getSession())
    {
	if(!_session)
	{
	    try
	    {
		allocated(request->getSession());
	    }
	    catch(const AllocationException&)
	    {
		// Ignore
	    }
	}
	_session = request->getSession();
	++_count;
	return true; // Allocated	    
    }
    else
    {
	queueAllocationAttempt(child, request, tryAllocate);
    }

    return false;
}

