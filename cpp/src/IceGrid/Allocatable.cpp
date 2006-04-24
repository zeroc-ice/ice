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
    Lock sync(*this);
    assert(!_allocatable);
    if(_canceled)
    {
	return false;
    }
    _allocatable = allocatable;
    allocated(_allocatable);
    return true;
}

void 
AllocationRequest::cancel() 
{
    Lock sync(*this);
    if(_canceled)
    {
	return;
    }
    _canceled = true; 
    canceled();
}

bool 
AllocationRequest::checkTimeout(const IceUtil::Time& now) 
{
    assert(_timeout > 0);
    {
	Lock sync(*this);
	if(_canceled)
	{
	    return true;
	}
	_canceled = _expiration < now;
	if(!_canceled)
	{
	    return false;
	}
	timeout();
    }
    _session->removeAllocationRequest(this);
    return true;
}

void
AllocationRequest::allocate()
{
    _session->addAllocationRequest(this);
}

void 
AllocationRequest::release(const SessionIPtr& session)
{
    //
    // Check if the session releasing the object is indeed the session 
    // which initiated the allocation request.
    //
    if(_session != session)
    {
	throw AllocationException("can't release object which is not allocated");
    }
    _session->removeAllocationRequest(this);
}

bool
AllocationRequest::operator<(const AllocationRequest& r) const
{
    return this < &r;
}

AllocationRequest::AllocationRequest(const SessionIPtr& session) :
    _session(session),
    _timeout(_session->getAllocationTimeout()),
    _expiration(_timeout > 0 ? (IceUtil::Time::now() + IceUtil::Time::milliSeconds(_timeout)) : IceUtil::Time()),
    _canceled(false)
{
}

Allocatable::Allocatable(bool allocatable) : _allocatable(allocatable), _allocated(false)
{
}

Allocatable::~Allocatable()
{
}

void 
Allocatable::allocate(const AllocationRequestPtr& request, bool allocateOnce)
{
    IceUtil::Mutex::Lock sync(_allocateMutex);
    if(_allocatable)
    {
	if(_allocated)
	{
	    if(_allocated->getSession() == request->getSession())
	    {
		if(allocateOnce)
		{
		    throw AllocationException("object already allocated by the session");
		}
		request->setAllocatable(this);
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
	}
    }
    else
    {
	if(allocateOnce)
	{
	    throw AllocationException("can't allocate non allocatable object");
	}
	bool rc = request->setAllocatable(this);
	assert(rc);
    }
}

bool 
Allocatable::tryAllocate(const AllocationRequestPtr& request)
{
    IceUtil::Mutex::Lock sync(_allocateMutex);
    if(_allocatable && _allocated)
    {
	return false;
    }
    else if(request->setAllocatable(this))
    {
	_allocated = request;
	_allocated->allocate();
    }
    return true;
}

void
Allocatable::release(const SessionIPtr& session)
{
    IceUtil::Mutex::Lock sync(_allocateMutex);
    if(!_allocated)
    {
	throw AllocationException("object not allocated");
    }
    _allocated->release(session);
    while(!_requests.empty())
    {
	_allocated = _requests.front();
	_requests.pop_front();
	if(_allocated->setAllocatable(this))
	{
	    return;
	}
    }
    _allocated = 0;
}
