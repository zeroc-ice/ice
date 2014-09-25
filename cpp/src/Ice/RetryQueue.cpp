// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/RetryQueue.h>
#include <Ice/OutgoingAsync.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(RetryQueue* p) { return p; }

IceInternal::RetryTask::RetryTask(const RetryQueuePtr& queue, const OutgoingAsyncMessageCallbackPtr& outAsync) :
    _queue(queue), _outAsync(outAsync)
{
}

void
IceInternal::RetryTask::runTimerTask()
{
    _outAsync->__processRetry(false);

    //
    // NOTE: this must be called last, destroy() blocks until all task
    // are removed to prevent the client thread pool to be destroyed
    // (we still need the client thread pool at this point to call
    // exception callbacks with CommunicatorDestroyedException).
    //
    _queue->remove(this);
}

void
IceInternal::RetryTask::destroy()
{
    _outAsync->__processRetry(true);
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
IceInternal::RetryQueue::add(const OutgoingAsyncMessageCallbackPtr& out, int interval)
{
    Lock sync(*this);
    if(!_instance)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    RetryTaskPtr task = new RetryTask(this, out);
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

    set<RetryTaskPtr>::const_iterator p = _requests.begin();
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


