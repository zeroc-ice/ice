// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/RetryQueue.h>
#include <Ice/OutgoingAsync.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/LoggerUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(RetryQueue* p) { return p; }

IceInternal::RetryTask::RetryTask(const InstancePtr& instance,
                                  const RetryQueuePtr& queue,
                                  const ProxyOutgoingAsyncBasePtr& outAsync) :
    _instance(instance),
    _queue(queue),
    _outAsync(outAsync)
{
}

void
IceInternal::RetryTask::runTimerTask()
{
    _outAsync->retry(); // Retry again the invocation.

    //
    // NOTE: this must be called last, destroy() blocks until all task
    // are removed to prevent the client thread pool to be destroyed
    // (we still need the client thread pool at this point to call
    // exception callbacks with CommunicatorDestroyedException).
    //
    _queue->remove(this);
}

void
IceInternal::RetryTask::requestCanceled(OutgoingBase*, const Ice::LocalException&)
{
    assert(false);
}

void
IceInternal::RetryTask::asyncRequestCanceled(const OutgoingAsyncBasePtr& outAsync, const Ice::LocalException& ex)
{
    if(_queue->cancel(this))
    {
        if(_instance->traceLevels()->retry >= 1)
        {
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->retryCat);
            out << "operation retry canceled\n" << ex;
        }
        if(_outAsync->completed(ex))
        {
            _outAsync->invokeCompletedAsync();
        }
    }
}

void
IceInternal::RetryTask::destroy()
{
    try
    {
        _outAsync->abort(CommunicatorDestroyedException(__FILE__, __LINE__));
    }
    catch(const CommunicatorDestroyedException&)
    {
        // Abort can throw if there's no callback, ignore.
    }
}

bool
IceInternal::RetryTask::operator<(const RetryTask& rhs) const
{
    return this < &rhs;
}

IceInternal::RetryQueue::RetryQueue(const InstancePtr& instance) : _instance(instance)
{
}

void
IceInternal::RetryQueue::add(const ProxyOutgoingAsyncBasePtr& out, int interval)
{
    Lock sync(*this);
    if(!_instance)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    RetryTaskPtr task = new RetryTask(_instance, this, out);
    out->cancelable(task); // This will throw if the request is canceled.
    try
    {
        _instance->timer()->schedule(task, IceUtil::Time::milliSeconds(interval));
    }
    catch(const IceUtil::IllegalArgumentException&) // Expected if the communicator destroyed the timer.
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _requests.insert(task);
}

void
IceInternal::RetryQueue::destroy()
{
    Lock sync(*this);
    assert(_instance);

    set<RetryTaskPtr>::iterator p = _requests.begin();
    while(p != _requests.end())
    {
        if(_instance->timer()->cancel(*p))
        {
            (*p)->destroy();
            _requests.erase(p++);
        }
        else
        {
            ++p;
        }
    }

    _instance = 0;
    while(!_requests.empty())
    {
        wait();
    }
}

void
IceInternal::RetryQueue::remove(const RetryTaskPtr& task)
{
    Lock sync(*this);
    assert(_requests.find(task) != _requests.end());
    _requests.erase(task);
    if(!_instance && _requests.empty())
    {
        notify(); // If we are destroying the queue, destroy is probably waiting on the queue to be empty.
    }
}

bool
IceInternal::RetryQueue::cancel(const RetryTaskPtr& task)
{
    Lock sync(*this);
    if(_requests.erase(task) > 0)
    {
        if(!_instance && _requests.empty())
        {
            notify(); // If we are destroying the queue, destroy is probably waiting on the queue to be empty.
        }
        return _instance->timer()->cancel(task);
    }
    return false;
}
