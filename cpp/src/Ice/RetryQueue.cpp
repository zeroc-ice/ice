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
    if(_queue->remove(this))
    {
        _outAsync->__processRetry(false);
    }
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
    for(set<RetryTaskPtr>::const_iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
        _instance->timer()->cancel(*p);
        (*p)->destroy();
    }
    _requests.clear();
}

bool
IceInternal::RetryQueue::remove(const RetryTaskPtr& task)
{
    Lock sync(*this);
    return _requests.erase(task) > 0;
}

