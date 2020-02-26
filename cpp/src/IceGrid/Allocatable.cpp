//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceGrid/Allocatable.h>
#include <IceGrid/SessionI.h>

using namespace std;
using namespace IceGrid;

bool
AllocationRequest::pending()
{
    lock_guard lock(_mutex);
    assert(_state == Initial);

    if(_timeout == 0)
    {
        _state = Canceled;
        canceled(make_exception_ptr(AllocationTimeoutException()));
        return false;
    }
    else if(!_session->addAllocationRequest(shared_from_this()))
    {
        _state = Canceled;
        canceled(make_exception_ptr(AllocationException("session destroyed")));
        return false;
    }

    if(_timeout > 0)
    {
        try
        {
            _session->getTimer()->schedule(shared_from_this(), IceUtil::Time::milliSeconds(_timeout));
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
AllocationRequest::allocate(const shared_ptr<Allocatable>&, const shared_ptr<SessionI>& session)
{
    lock_guard lock(_mutex);
    switch(_state)
    {
    case Initial:
        break;
    case Canceled:
        return false;
    case Pending:
        if(_timeout > 0)
        {
            _session->getTimer()->cancel(shared_from_this());
        }
        _session->removeAllocationRequest(shared_from_this());
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
        canceled(make_exception_ptr(AllocationException("already allocated by the session")));
        return false;
    }
    else
    {
        _state = Allocated;
        return true;
    }
}

void
AllocationRequest::cancel(exception_ptr ex)
{
    lock_guard lock(_mutex);
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
            _session->getTimer()->cancel(shared_from_this());
        }
        _session->removeAllocationRequest(shared_from_this());
        break;
    }

    _state = Canceled;
    canceled(ex);
}

void
AllocationRequest::runTimerTask() // TimerTask::runTimerTask() method implementation
{
    lock_guard lock(_mutex);
    switch(_state)
    {
    case Initial:
        assert(false);
    case Canceled:
    case Allocated:
        return;
    case Pending:
        _session->removeAllocationRequest(shared_from_this());
        break;
    }

    _state = Canceled;
    canceled(make_exception_ptr(AllocationTimeoutException()));
}

bool
AllocationRequest::isCanceled() const
{
    lock_guard lock(_mutex);
    return _state == Canceled;
}

bool
AllocationRequest::operator<(const AllocationRequest& r) const
{
    return this < &r;
}

AllocationRequest::AllocationRequest(const shared_ptr<SessionI>& session) :
    _session(session),
    _timeout(_session->getAllocationTimeout()), // The session timeout can be updated so we need to cache it here.
    _state(Initial)
{
}

Allocatable::Allocatable(bool allocatable, const shared_ptr<Allocatable>& parent) :
    _allocatable(allocatable || (parent && parent->isAllocatable())),
    _parent((parent && parent->isAllocatable()) ? parent : nullptr),
    _count(0),
    _releasing(false)
{
    assert(!_parent || _parent->isAllocatable()); // Parent is only set if it's allocatable.
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
Allocatable::allocate(const shared_ptr<AllocationRequest>& request, bool fromRelease)
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
Allocatable::tryAllocate(const shared_ptr<AllocationRequest>& request, bool fromRelease)
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
Allocatable::release(const shared_ptr<SessionI>& session, bool fromRelease)
{
    bool isReleased = false;
    bool hasRequests = false;
    {
        unique_lock lock(_mutex);
        if(!fromRelease)
        {
            _condVar.wait(lock, [this] { return !_releasing; });
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
                shared_ptr<AllocationRequest> request;
                shared_ptr<Allocatable> allocatable;
                {
                    lock_guard lock(_mutex);
                    allocatable = dequeueAllocationAttempt(request);
                    if(!allocatable)
                    {
                        assert(_requests.empty());
                        assert(_count == 0);
                        _releasing = false;
                        _condVar.notify_all();
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
                                lock_guard lock(_mutex);
                                assert(_count);

                                allocatable = 0;
                                request = 0;

                                //
                                // Check if there's other requests from the session
                                // waiting to allocate this allocatable.
                                //
                                auto p = _requests.begin();
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
                                    _condVar.notify_all();
                                    return; // We're done, the allocatable was released (but is allocated again)!
                                }
                            }

                            try
                            {
                                assert(allocatable && request);
                                allocatable->allocate(request, true);
                            }
                            catch(const Ice::UserException&)
                            {
                                request->cancel(current_exception());
                            }
                        }
                    }
                }
                catch(const Ice::UserException&)
                {
                    request->cancel(current_exception());
                }
            }
        }
        else if(isReleased)
        {
            canTryAllocate(); // Notify that this allocatable can be allocated.
        }
    }
}

shared_ptr<SessionI>
Allocatable::getSession() const
{
    lock_guard lock(_mutex);
    return _session;
}

bool
Allocatable::operator<(const Allocatable& r) const
{
    return this < &r;
}

void
Allocatable::queueAllocationAttempt(const shared_ptr<Allocatable>& allocatable,
                                    const shared_ptr<AllocationRequest>& request,
                                    bool tryAllocate)
{
    assert(!_parent);
    if(!tryAllocate)
    {
        if(request->pending())
        {
            _requests.push_back({ allocatable, request });
        }
    }
    else
    {
        _requests.push_back({ allocatable, nullptr });
    }
}

void
Allocatable::queueAllocationAttemptFromChild(const shared_ptr<Allocatable>& allocatable)
{
    if(_parent)
    {
        _parent->queueAllocationAttemptFromChild(allocatable);
        return;
    }

    lock_guard lock(_mutex);
    _requests.push_back({ allocatable, nullptr });
}

shared_ptr<Allocatable>
Allocatable::dequeueAllocationAttempt(shared_ptr<AllocationRequest>& request)
{
    if(_requests.empty())
    {
        return nullptr;
    }

    auto alloc = _requests.front();
    _requests.pop_front();
    if(alloc.second)
    {
        request = alloc.second;
    }
    return alloc.first;
}

bool
Allocatable::allocate(const shared_ptr<AllocationRequest>& request, bool tryAllocate, bool fromRelease)
{
    if(_parent && !_parent->allocateFromChild(request, shared_from_this(), tryAllocate, fromRelease))
    {
        return false;
    }

    bool queueWithParent = false;
    int allocationCount = 0;
    try
    {
        lock_guard lock(_mutex);
        checkAllocatable();

        if(!_session && (fromRelease || !_releasing))
        {
            if(request->allocate(shared_from_this(), _session))
            {
                try
                {
                    allocated(request->getSession()); // This might throw SessionDestroyedException
                }
                catch(const SessionDestroyedException&)
                {
                    request->canceled(make_exception_ptr(AllocationException("session destroyed")));
                    throw;
                }
                assert(_count == 0);
                _session = request->getSession();
                request->allocated(shared_from_this(), request->getSession());
                ++_count;
                allocationCount = _count;
            }
        }
        else if(_session == request->getSession())
        {
            if(!tryAllocate)
            {
                if(request->allocate(shared_from_this(), _session))
                {
                    assert(_count > 0);
                    ++_count;
                    request->allocated(shared_from_this(), _session);
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
            queueAllocationAttempt(shared_from_this(), request, tryAllocate);
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
            _parent->queueAllocationAttemptFromChild(shared_from_this());
        }
        _parent->release(request->getSession(), fromRelease);
    }
    return allocationCount > 0;
}

bool
Allocatable::allocateFromChild(const shared_ptr<AllocationRequest>& request,
                               const shared_ptr<Allocatable>& child,
                               bool tryAllocate,
                               bool fromRelease)
{
    if(_parent && !_parent->allocateFromChild(request, child, tryAllocate, fromRelease))
    {
        return false;
    }

    int allocationCount = 0;
    {
        lock_guard lock(_mutex);
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
