//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Glacier2/RequestQueue.h>
#include <Glacier2/Instance.h>
#include <Glacier2/SessionRouterI.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::Request::Request(const ObjectPrx& proxy, const std::pair<const Byte*, const Byte*>& inParams,
                           const Current& current, bool forwardContext, const Ice::Context& sslContext,
                           const AMD_Object_ice_invokePtr& amdCB) :
    _proxy(proxy),
    _inParams(inParams.first, inParams.second),
    _current(current),
    _forwardContext(forwardContext),
    _sslContext(sslContext),
    _amdCB(amdCB)
{
    Context::const_iterator p = current.ctx.find("_ovrd");
    if(p != current.ctx.end())
    {
        const_cast<string&>(_override) = p->second;
    }
}

void
Glacier2::Request::addBatchProxy(set<Ice::ObjectPrx>& batchProxies)
{
    set<Ice::ObjectPrx>::const_iterator p = batchProxies.find(_proxy);
    if(p == batchProxies.end())
    {
        batchProxies.insert(_proxy);
    }
    else if(p->get() != _proxy.get())
    {
        const_cast<Ice::ObjectPrx&>(_proxy) = *p;
    }
}

Ice::AsyncResultPtr
Glacier2::Request::invoke(const Callback_Object_ice_invokePtr& cb)
{
    pair<const Byte*, const Byte*> inPair;
    if(_inParams.size() == 0)
    {
        inPair.first = inPair.second = 0;
    }
    else
    {
        inPair.first = &_inParams[0];
        inPair.second = inPair.first + _inParams.size();
    }

    if(_proxy->ice_isBatchOneway() || _proxy->ice_isBatchDatagram())
    {
        ByteSeq outParams;
        if(_forwardContext)
        {
            if(_sslContext.size() > 0)
            {
                Ice::Context ctx = _current.ctx;
                ctx.insert(_sslContext.begin(), _sslContext.end());
                _proxy->ice_invoke(_current.operation, _current.mode, inPair, outParams, ctx);
            }
            else
            {
                _proxy->ice_invoke(_current.operation, _current.mode, inPair, outParams, _current.ctx);
            }
        }
        else
        {
            if(_sslContext.size() > 0)
            {
                _proxy->ice_invoke(_current.operation, _current.mode, inPair, outParams, _sslContext);
            }
            else
            {
                _proxy->ice_invoke(_current.operation, _current.mode, inPair, outParams);
            }
        }
        return 0;
    }
    else
    {
        Ice::AsyncResultPtr result;
        if(_forwardContext)
        {
            if(_sslContext.size() > 0)
            {
                Ice::Context ctx = _current.ctx;
                ctx.insert(_sslContext.begin(), _sslContext.end());
                result = _proxy->begin_ice_invoke(_current.operation, _current.mode, inPair, ctx, cb, this);
            }
            else
            {
                result = _proxy->begin_ice_invoke(_current.operation, _current.mode, inPair, _current.ctx, cb, this);
            }
        }
        else
        {
            if(_sslContext.size() > 0)
            {
                result = _proxy->begin_ice_invoke(_current.operation, _current.mode, inPair, _sslContext, cb, this);
            }
            else
            {
                result = _proxy->begin_ice_invoke(_current.operation, _current.mode, inPair, cb, this);
            }
        }

        return result;
    }
}

bool
Glacier2::Request::override(const RequestPtr& other) const
{
    //
    // Both override values have to be non-empty.
    //
    if(_override.empty() || other->_override.empty())
    {
        return false;
    }

    //
    // Override does not work for twoways, because a response is
    // expected for each request.
    //
    if(_proxy->ice_isTwoway() || other->_proxy->ice_isTwoway())
    {
        return false;
    }

    //
    // Don't override if the override isn't the same.
    //
    if(_override != other->_override)
    {
        return false;
    }

    //
    // We cannot override if the proxies differ.
    //
    return _proxy == other->_proxy;
}

void
Glacier2::Request::response(bool ok, const pair<const Ice::Byte*, const Ice::Byte*>& outParams)
{
    assert(_proxy->ice_isTwoway());
    _amdCB->ice_response(ok, outParams);
}

void
Glacier2::Request::exception(const Ice::Exception& ex)
{
    //
    // Only for twoways, oneway or batch oneway dispatches are finished
    // when queued, see queued().
    //
    if(_proxy->ice_isTwoway())
    {
        _amdCB->ice_exception(ex);
    }
}

void
Glacier2::Request::queued()
{
    if(!_proxy->ice_isTwoway())
    {
#if defined(_MSC_VER)
        _amdCB->ice_response(true, pair<const Byte*, const Byte*>(static_cast<const Byte*>(nullptr),
                                                                  static_cast<const Byte*>(nullptr)));
#else
        _amdCB->ice_response(true, pair<const Byte*, const Byte*>(0, 0));
#endif
    }
}

Glacier2::RequestQueue::RequestQueue(const RequestQueueThreadPtr& requestQueueThread,
                                     const InstancePtr& instance,
                                     const Ice::ConnectionPtr& connection) :
    _requestQueueThread(requestQueueThread),
    _instance(instance),
    _connection(connection),
    _callback(newCallback_Object_ice_invoke(this, &RequestQueue::response, &RequestQueue::exception,
                                            &RequestQueue::sent)),
    _flushCallback(newCallback_Connection_flushBatchRequests(this, &RequestQueue::exception, &RequestQueue::sent)),
    _pendingSend(false),
    _destroyed(false)
{
}

bool
Glacier2::RequestQueue::addRequest(const RequestPtr& request)
{
    IceUtil::Mutex::Lock lock(*this);
    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    if(request->hasOverride())
    {
        for(deque<RequestPtr>::iterator p = _requests.begin(); p != _requests.end(); ++p)
        {
            //
            // If the new request overrides an old one, then abort the old
            // request and replace it with the new request.
            //
            if(request->override(*p))
            {
                if(_observer)
                {
                    _observer->overridden(!_connection);
                }
                request->queued();
                *p = request;
                return true;
            }
        }
    }

    if(!_connection)
    {
        //
        // If it's a batch request, we make sure we use a unique batch proxy object for the queued
        // batch requests. We want all the requests for the same batch proxy to be queued on the
        // same proxy object.
        //
        request->addBatchProxy(_batchProxies);
    }

    //
    // No override, we add the new request.
    //
    if(_requests.empty() && (!_connection || !_pendingSend))
    {
        _requestQueueThread->flushRequestQueue(this); // This might throw if the thread is destroyed.
    }
    _requests.push_back(request);
    request->queued();
    if(_observer)
    {
        _observer->queued(!_connection);
    }
    return false;
}

void
Glacier2::RequestQueue::flushRequests()
{
    IceUtil::Mutex::Lock lock(*this);
    if(_connection)
    {
        if(_pendingSend)
        {
            return;
        }
        flush();
    }
    else
    {
        for(deque<RequestPtr>::const_iterator p = _requests.begin(); p != _requests.end(); ++p)
        {
            try
            {
                if(_observer)
                {
                    _observer->forwarded(!_connection);
                }
                assert(_callback);
                (*p)->invoke(_callback);
            }
            catch(const Ice::LocalException&)
            {
                // Ignore, this can occur for batch requests.
            }
        }
        _requests.clear();

        for(set<Ice::ObjectPrx>::const_iterator q = _batchProxies.begin(); q != _batchProxies.end(); ++q)
        {
            (*q)->begin_ice_flushBatchRequests();
        }
        _batchProxies.clear();
    }

    if(_destroyed && _requests.empty())
    {
        destroyInternal();
    }
}

void
Glacier2::RequestQueue::destroy()
{
    IceUtil::Mutex::Lock lock(*this);

    _destroyed = true;

    //
    // Although the session has been destroyed, we cannot destroy this queue
    // until all requests have completed.
    //
    if(_requests.empty())
    {
        destroyInternal();
    }
}

void
Glacier2::RequestQueue::updateObserver(const Glacier2::Instrumentation::SessionObserverPtr& observer)
{
    IceUtil::Mutex::Lock lock(*this);
    _observer = observer;
}

void
Glacier2::RequestQueue::flush()
{
    assert(_connection);
    _pendingSend = false;
    _pendingSendRequest = 0;

    bool flushBatchRequests = false;
    deque<RequestPtr>::iterator p;
    for(p = _requests.begin(); p != _requests.end(); ++p)
    {
        try
        {
            assert(_callback);
            if(_observer)
            {
                _observer->forwarded(!_connection);
            }
            Ice::AsyncResultPtr result = (*p)->invoke(_callback);
            if(!result)
            {
                flushBatchRequests = true;
            }
            else if(!result->sentSynchronously() && !result->isCompleted())
            {
                _pendingSend = true;
                _pendingSendRequest = *p++;
                break;
            }
        }
        catch(const Ice::LocalException&)
        {
            // Ignore, this can occur for batch requests.
        }
    }

    if(p == _requests.end())
    {
        _requests.clear();
    }
    else
    {
        _requests.erase(_requests.begin(), p);
    }

    if(flushBatchRequests)
    {
        Ice::AsyncResultPtr result = _connection->begin_flushBatchRequests(ICE_SCOPED_ENUM(CompressBatch, BasedOnProxy), _flushCallback);
        if(!result->sentSynchronously() && !result->isCompleted())
        {
            _pendingSend = true;
            _pendingSendRequest = 0;
        }
    }
}

void
Glacier2::RequestQueue::destroyInternal()
{
    //
    // Must be called with the mutex locked.
    //

    //
    // Remove cyclic references.
    //
    const_cast<Ice::Callback_Object_ice_invokePtr&>(_callback) = 0;
    const_cast<Ice::Callback_Connection_flushBatchRequestsPtr&>(_flushCallback) = 0;
}

void
Glacier2::RequestQueue::response(bool ok, const pair<const Byte*, const Byte*>& outParams, const RequestPtr& request)
{
    assert(request);
    request->response(ok, outParams);
}

void
Glacier2::RequestQueue::exception(const Ice::Exception& ex, const RequestPtr& request)
{
    //
    // If the connection has been lost, destroy the session.
    //
    if(_connection)
    {
        if(dynamic_cast<const Ice::SocketException*>(&ex) ||
           dynamic_cast<const Ice::TimeoutException*>(&ex) ||
           dynamic_cast<const Ice::ProtocolException*>(&ex))
        {
            try
            {
                _instance->sessionRouter()->destroySession(_connection);
            }
            catch(const Exception&)
            {
            }
        }

        IceUtil::Mutex::Lock lock(*this);
        if(request == _pendingSendRequest)
        {
            flush();
        }
    }

    if(request)
    {
        request->exception(ex);
    }
}

void
Glacier2::RequestQueue::sent(bool sentSynchronously, const RequestPtr& request)
{
    if(_connection && !sentSynchronously)
    {
        IceUtil::Mutex::Lock lock(*this);
        if(request == _pendingSendRequest)
        {
            flush();
        }
    }
}

Glacier2::RequestQueueThread::RequestQueueThread(const IceUtil::Time& sleepTime) :
    IceUtil::Thread("Glacier2 request queue thread"),
    _sleepTime(sleepTime),
    _destroy(false),
    _sleep(false)
{
}

Glacier2::RequestQueueThread::~RequestQueueThread()
{
    assert(_destroy);
    assert(_queues.empty());
}

void
Glacier2::RequestQueueThread::destroy()
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

        assert(!_destroy);
        _destroy = true;
        _sleep = false;
        notify();
    }

    try
    {
        getThreadControl().join();
    }
    catch(const IceUtil::ThreadNotStartedException&)
    {
        // Expected if start() failed.
    }
}

void
Glacier2::RequestQueueThread::flushRequestQueue(const RequestQueuePtr& queue)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    if(_destroy)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    if(_queues.empty() && !_sleep)
    {
        notify();
    }
    _queues.push_back(queue);
}

void
Glacier2::RequestQueueThread::run()
{
    while(true)
    {
        vector<RequestQueuePtr> queues;

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

            //
            // Wait indefinitely if there's no requests to
            // send. If the queue is being destroyed we still need to
            // wait until all the responses for twoway requests are
            // received.
            //
            while(!_destroy && (_queues.empty() || _sleep))
            {
                if(_sleep)
                {
                    IceUtil::Time now = IceUtil::Time::now(IceUtil::Time::Monotonic);
                    if(!timedWait(_sleepDuration))
                    {
                        _sleepDuration = IceUtil::Time();
                    }
                    else
                    {
                        _sleepDuration -= IceUtil::Time::now(IceUtil::Time::Monotonic) - now;
                    }

                    if(_sleepDuration <= IceUtil::Time())
                    {
                        _sleep = false;
                    }
                }
                else
                {
                    wait();
                }
            }

            //
            // If the queue is being destroyed and there's no requests or responses
            // to send, we're done.
            //
            if(_destroy && _queues.empty())
            {
                return;
            }

            assert(!_queues.empty() && !_sleep);

            queues.swap(_queues);

            if(_sleepTime > IceUtil::Time())
            {
                _sleep = true;
                _sleepDuration = _sleepTime;
            }
        }

        for(vector<RequestQueuePtr>::const_iterator p = queues.begin(); p != queues.end(); ++p)
        {
            (*p)->flushRequests();
        }
    }
}
