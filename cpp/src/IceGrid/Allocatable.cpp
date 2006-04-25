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
AllocationRequest::setAllocatable(const AllocatablePtr& allocatable)
{
    assert(allocatable);

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
	break;
    case Allocated:
	assert(false);
	break;
    }

    _allocatable = allocatable;
    _state = Allocated;
    allocated(_allocatable);
    return true;
}

void 
AllocationRequest::cancel() 
{
    AllocatablePtr allocatable;
    {
	Lock sync(*this);
	switch(_state)
	{
	case Initial:
	    assert(false);
	case Canceled:
	    return;
	case Pending:
	    if(_timeout > 0)
	    {
		_session->getWaitQueue()->remove(this);
	    }
	    canceled();
	    break;
	case Allocated:
	    allocatable = _allocatable;
	    break;
	}
	
	_state = Canceled;
    }

    if(allocatable)
    {
	try
	{
	    allocatable->release(_session);
	}
	catch(const AllocationException&)
	{
	    // Ignore, the allocatable might already have been released.
	}
    }
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
	return;
    case Pending:
	timeout();
	break;
    case Allocated:
	return;
    }
    
    _state = Canceled;
}

void
AllocationRequest::allocate()
{
    _session->addAllocationRequest(this);

    Lock sync(*this);
    assert(_allocatable || _state == Initial);
    if(!_allocatable)
    {
	if(_timeout > 0)
	{
	    _session->getWaitQueue()->add(this, IceUtil::Time::milliSeconds(_timeout));
	}
	_state = Pending;
    }
}

void 
AllocationRequest::release()
{
    _session->removeAllocationRequest(this);
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

Allocatable::Allocatable(bool allocatable) : _allocatable(allocatable), _allocated(false)
{
}

Allocatable::~Allocatable()
{
}

void 
Allocatable::allocate(const AllocationRequestPtr& request, bool once)
{
    IceUtil::Mutex::Lock sync(_allocateMutex);
    if(_allocatable)
    {
	if(_allocated)
	{
	    if(_allocated->getSession() == request->getSession())
	    {
		if(once)
		{
		    throw AllocationException("object already allocated by the session");
		}
		else
		{
		    request->setAllocatable(this);
		}
	    }
	    else if(request->getTimeout())
	    {
		request->allocate();
		_requests.push_back(request); // TODO: XXX: monitor request timeout if timeout != -1
	    }
	    else
	    {
		request->timeout();
	    }
	}
	else if(request->setAllocatable(this))
	{
	    _allocated = request;
	    _allocated->allocate();
	    allocated();
	}
    }
    else
    {
	throw AllocationException("can't allocate non allocatable object");
    }
}

bool 
Allocatable::tryAllocate(const AllocationRequestPtr& request)
{
    IceUtil::Mutex::Lock sync(_allocateMutex);
    if(_allocatable)
    {
	if(_allocated)
	{
	    if(_allocated->getSession() == request->getSession())
	    {
		throw AllocationException("object already allocated by the session");
	    }
	    return false;
	}
	else if(request->setAllocatable(this))
	{
	    _allocated = request;
	    _allocated->allocate();
	    allocated();
	}
	return true; // The allocatable was allocated or the request was canceled.
    }
    else
    {
	return false;
    }
}

bool
Allocatable::release(const SessionIPtr& session)
{
    IceUtil::Mutex::Lock sync(_allocateMutex);
    if(!_allocated || _allocated->getSession() != session)
    {
	throw AllocationException("can't release object which is not allocated");
    }
    _allocated->release();

    //
    // Allocate the allocatable to another client.
    //
    while(!_requests.empty())
    {
	_allocated = _requests.front();
	_requests.pop_front();
	if(_allocated->setAllocatable(this))
	{
	    return false;
	}
    }
    released();
    _allocated = 0;
    return true;
}

bool
Allocatable::isAllocated() const
{
    IceUtil::Mutex::Lock sync(_allocateMutex);
    return _allocated;
}
