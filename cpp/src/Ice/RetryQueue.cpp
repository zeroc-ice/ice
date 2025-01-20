// Copyright (c) ZeroC, Inc.

#include "RetryQueue.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "Ice/OutgoingAsync.h"
#include "Instance.h"
#include "TraceLevels.h"

#include <stdexcept>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::RetryTask::RetryTask(InstancePtr instance, RetryQueuePtr queue, ProxyOutgoingAsyncBasePtr outAsync)
    : _instance(std::move(instance)),
      _queue(std::move(queue)),
      _outAsync(std::move(outAsync))
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
    _queue->remove(shared_from_this());
}

void
IceInternal::RetryTask::asyncRequestCanceled(const OutgoingAsyncBasePtr& /*outAsync*/, exception_ptr ex)
{
    if (_queue->cancel(shared_from_this()))
    {
        if (_instance->traceLevels()->retry >= 1)
        {
            try
            {
                rethrow_exception(ex);
            }
            catch (const std::exception& e)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->retryCat);
                out << "operation retry canceled\n" << e;
            }
        }
        if (_outAsync->exception(ex))
        {
            _outAsync->invokeExceptionAsync();
        }
    }
}

void
IceInternal::RetryTask::destroy()
{
    try
    {
        _outAsync->abort(make_exception_ptr(CommunicatorDestroyedException(__FILE__, __LINE__)));
    }
    catch (const CommunicatorDestroyedException&)
    {
        // Abort can throw if there's no callback, ignore.
    }
}

IceInternal::RetryQueue::RetryQueue(InstancePtr instance) : _instance(std::move(instance)) {}

void
IceInternal::RetryQueue::add(const ProxyOutgoingAsyncBasePtr& out, int interval)
{
    lock_guard<mutex> lock(_mutex);
    if (!_instance)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    RetryTaskPtr task = make_shared<RetryTask>(_instance, shared_from_this(), out);
    out->cancelable(task); // This will throw if the request is canceled.
    try
    {
        _instance->timer()->schedule(task, chrono::milliseconds(interval));
    }
    catch (const invalid_argument&) // Expected if the communicator destroyed the timer.
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _requests.insert(task);
}

void
IceInternal::RetryQueue::destroy()
{
    unique_lock<mutex> lock(_mutex);
    assert(_instance);

    auto p = _requests.begin();
    while (p != _requests.end())
    {
        if (_instance->timer()->cancel(*p))
        {
            (*p)->destroy();
            _requests.erase(p++);
        }
        else
        {
            ++p;
        }
    }

    _instance = nullptr;
    _conditionVariable.wait(lock, [this] { return _requests.empty(); });
}

void
IceInternal::RetryQueue::remove(const RetryTaskPtr& task)
{
    lock_guard<mutex> lock(_mutex);
    assert(_requests.find(task) != _requests.end());
    _requests.erase(task);
    if (!_instance && _requests.empty())
    {
        // If we are destroying the queue, destroy is probably waiting on the queue to be empty.
        _conditionVariable.notify_one();
    }
}

bool
IceInternal::RetryQueue::cancel(const RetryTaskPtr& task)
{
    lock_guard<mutex> lock(_mutex);
    if (_requests.erase(task) > 0)
    {
        if (_instance)
        {
            return _instance->timer()->cancel(task);
        }
        else if (_requests.empty())
        {
            // If we are destroying the queue, destroy is probably waiting on the queue to be empty.
            _conditionVariable.notify_one();
        }
    }
    return false;
}
