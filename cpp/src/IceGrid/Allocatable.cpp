//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        try
        {
            _session->getTimer()->schedule(this, IceUtil::Time::milliSeconds(_timeout));
        }
        catch(const IceUtil::Exception&)
        {
            // Ignore, timer is destroyed because of shutdown
        }
    }
    _state = Pending;
    return true;
}

bool
AllocationRequest::allocate(const AllocatablePtr& /*allocatable*/, const SessionIPtr& session)
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
            _session->getTimer()->cancel(this);
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
    else
    {
        _state = Allocated;
        return true;
    }
}

void
AllocationRequest::cancel(const Ice::UserException& ex)
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
            _session->getTimer()->cancel(this);
        }
        _session->removeAllocationRequest(this);
        break;
    }

    _state = Canceled;
    canceled(ex);
}

void
AllocationRequest::runTimerTask() // TimerTask::runTimerTask() method implementation
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
    _allocatable(allocatable || (parent && parent->isAllocatable())),
    _count(0),
    _releasing(false)
{
    //
    // COMPILERFIX: the constructor initializaton:
    //
    //     _parent((parent && parent->isAllocatable()) ? parent : AllocatablePtr())
    //
    // doesn't work on HP-UX (aCC: HP ANSI C++ B3910B A.03.56). It
    // results in a SEGFAULT at runtime.
    //
    if(parent && parent->isAllocatable())
    {
        const_cast<AllocatablePtr&>(_parent) = parent;
    }
    assert(!_parent || _parent->isAllocatable()); // Parent is only set if it's allocatable.
}

Allocatable::~Allocatable()
{
}

void
Allocatable::checkAllocatable()
{
    if(!isAllocatable())
    {
        throw AllocationException("not allocatable");
    }
}

bool
Allocatable::allocate(const AllocationRequestPtr& request, bool fromRelease)
{
    try
    {
        return allocate(request, false, fromRelease);
    }
    catch(const SessionDestroyedException&)
    {
        return false; // The session was destroyed
    }
}

bool
Allocatable::tryAllocate(const AllocationRequestPtr& request, bool fromRelease)
{
    try
    {
        return allocate(request, true, fromRelease);
    }
    catch(const AllocationException&)
    {
        return false; // Not allocatable
    }
}

void
Allocatable::release(const SessionIPtr& session, bool fromRelease)
{
    bool isReleased = false;
    bool hasRequests = false;
    {
        Lock sync(*this);
        if(!fromRelease)
        {
            while(_releasing)
            {
                wait();
            }
            assert(!_releasing);
        }

        if(!_session || _session != session)
        {
            throw AllocationException("can't release object which is not allocated");
        }

        if(--_count == 0)
        {
            _session = 0;

            released(session);

            isReleased = true;

            if(!fromRelease && !_requests.empty())
            {
                assert(!_parent);
                _releasing = true; // Prevent new allocations.
                hasRequests = true;
            }
        }
    }

    if(isReleased)
    {
        releasedNoSync(session);
    }

    if(_parent)
    {
        _parent->release(session, fromRelease);
    }
    else if(!fromRelease)
    {
        if(hasRequests)
        {
            while(true)
            {
                AllocationRequestPtr request;
                AllocatablePtr allocatable;
                {
                    Lock sync(*this);
                    allocatable = dequeueAllocationAttempt(request);
                    if(!allocatable)
                    {
                        assert(_requests.empty());
                        assert(_count == 0);
                        _releasing = false;
                        notifyAll();
                        return;
                    }
                }

                //
                // Try to allocate the allocatable with the request or if
                // there's no request, just notify the allocatable that it can
                // be allocated again.
                //
                try
                {
                    if((request && allocatable->allocate(request, true)) || (!request && allocatable->canTryAllocate()))
                    {
                        while(true)
                        {
                            {
                                Lock sync(*this);
                                assert(_count);

                                allocatable = 0;
                                request = 0;

                                //
                                // Check if there's other requests from the session
                                // waiting to allocate this allocatable.
                                //
                                list<pair<AllocatablePtr, AllocationRequestPtr> >::iterator p = _requests.begin();
                                while(p != _requests.end())
                                {
                                    if(p->second && p->second->getSession() == _session)
                                    {
                                        allocatable = p->first;
                                        request = p->second;
                                        _requests.erase(p);
                                        break;
                                    }
                                    ++p;
                                }
                                if(!allocatable)
                                {
                                    _releasing = false;
                                    notifyAll();
                                    return; // We're done, the allocatable was released (but is allocated again)!
                                }
                            }

                            try
                            {
                                assert(allocatable && request);
                                allocatable->allocate(request, true);
                            }
                            catch(const Ice::UserException& ex)
                            {
                                request->cancel(ex);
                            }
                        }
                    }
                }
                catch(const Ice::UserException& ex)
                {
                    request->cancel(ex);
                }
            }
        }
        else if(isReleased)
        {
            canTryAllocate(); // Notify that this allocatable can be allocated.
        }
    }
}

SessionIPtr
Allocatable::getSession() const
{
    Lock sync(*this);
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
        _requests.push_back(make_pair(allocatable, AllocationRequestPtr()));
    }
}

void
Allocatable::queueAllocationAttemptFromChild(const AllocatablePtr& allocatable)
{
    if(_parent)
    {
        _parent->queueAllocationAttemptFromChild(allocatable);
        return;
    }

    Lock sync(*this);
    _requests.push_back(make_pair(allocatable, AllocationRequestPtr()));
}

AllocatablePtr
Allocatable::dequeueAllocationAttempt(AllocationRequestPtr& request)
{
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
    return alloc.first;
}

bool
Allocatable::allocate(const AllocationRequestPtr& request, bool tryAllocate, bool fromRelease)
{
    if(_parent && !_parent->allocateFromChild(request, this, tryAllocate, fromRelease))
    {
        return false;
    }

    bool queueWithParent = false;
    int allocationCount = 0;
    try
    {
        Lock sync(*this);
        checkAllocatable();

        if(!_session && (fromRelease || !_releasing))
        {
            if(request->allocate(this, _session))
            {
                try
                {
                    allocated(request->getSession()); // This might throw SessionDestroyedException
                }
                catch(const SessionDestroyedException&)
                {
                    request->canceled(AllocationException("session destroyed"));
                    throw;
                }
                assert(_count == 0);
                _session = request->getSession();
                request->allocated(this, request->getSession());
                ++_count;
                allocationCount = _count;
            }
        }
        else if(_session == request->getSession())
        {
            if(!tryAllocate)
            {
                if(request->allocate(this, _session))
                {
                    assert(_count > 0);
                    ++_count;
                    request->allocated(this, _session);
                    allocationCount = _count;
                }
            }
            else
            {
                queueWithParent = true;
            }
        }
        else
        {
            queueAllocationAttempt(this, request, tryAllocate);
        }
    }
    catch(...)
    {
        if(_parent)
        {
            _parent->release(request->getSession(), fromRelease);
        }
        throw;
    }

    if(allocationCount == 1)
    {
        allocatedNoSync(request->getSession());
    }
    else if(allocationCount == 0 && _parent)
    {
        if(queueWithParent)
        {
            _parent->queueAllocationAttemptFromChild(this);
        }
        _parent->release(request->getSession(), fromRelease);
    }
    return allocationCount > 0;
}

bool
Allocatable::allocateFromChild(const AllocationRequestPtr& request,
                               const AllocatablePtr& child,
                               bool tryAllocate,
                               bool fromRelease)
{
    if(_parent && !_parent->allocateFromChild(request, child, tryAllocate, fromRelease))
    {
        return false;
    }

    int allocationCount = 0;
    {
        Lock sync(*this);
        if((!_session || _session == request->getSession()) && (fromRelease || !_releasing))
        {
            if(!_session)
            {
                try
                {
                    allocated(request->getSession());
                }
                catch(const SessionDestroyedException&)
                {
                    // Ignore
                }
            }
            _session = request->getSession();
            ++_count;
            allocationCount = _count;
        }
        else
        {
            queueAllocationAttempt(child, request, tryAllocate);
        }
    }

    if(allocationCount == 1)
    {
        allocatedNoSync(request->getSession());
    }
    return allocationCount > 0;
}
