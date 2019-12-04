//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Glacier2/RequestQueue.h>
#include <Glacier2/Instance.h>
#include <Glacier2/SessionRouterI.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::Request::Request(shared_ptr<ObjectPrx> proxy, const std::pair<const Byte*, const Byte*>& inParams,
                 const Current& current, bool forwardContext, const Ice::Context& sslContext,
                 function<void(bool, pair<const Byte*, const Byte*>)> response,
                 function<void(exception_ptr)> exception) :
    _proxy(move(proxy)),
    _inParams(inParams.first, inParams.second),
    _current(current),
    _forwardContext(forwardContext),
    _sslContext(sslContext),
    _response(move(response)),
    _exception(move(exception))
{
    Context::const_iterator p = current.ctx.find("_ovrd");
    if(p != current.ctx.end())
    {
        const_cast<string&>(_override) = p->second;
    }
}

void
Glacier2::Request::invoke(function<void(bool, pair<const Byte*, const Byte*>)>&& response,
                          function<void(exception_ptr)>&& exception,
                          std::function<void(bool)>&& sent)
{
    pair<const Byte*, const Byte*> inPair;
    if(_inParams.size() == 0)
    {
        inPair.first = inPair.second = nullptr;
    }
    else
    {
        inPair.first = _inParams.data();
        inPair.second = inPair.first + _inParams.size();
    }

    if(_forwardContext)
    {
        if(_sslContext.size() > 0)
        {
            Ice::Context ctx = _current.ctx;
            ctx.insert(_sslContext.begin(), _sslContext.end());
            _proxy->ice_invokeAsync(_current.operation, _current.mode, inPair,
                                    move(response), move(exception), move(sent), ctx);
        }
        else
        {
            _proxy->ice_invokeAsync(_current.operation, _current.mode, inPair,
                                    move(response), move(exception), move(sent), _current.ctx);
        }
    }
    else
    {
        if(_sslContext.size() > 0)
        {
            _proxy->ice_invokeAsync(_current.operation, _current.mode, inPair,
                                    move(response), move(exception), move(sent), _sslContext);
        }
        else
        {
            _proxy->ice_invokeAsync(_current.operation, _current.mode, inPair,
                                    move(response), move(exception), move(sent));
        }
    }
}

bool
Glacier2::Request::override(const shared_ptr<Request>& other) const
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
    return Ice::targetEqualTo(_proxy, other->_proxy);
}

void
Glacier2::Request::response(bool ok, const pair<const Byte*, const Byte*>& outParams)
{
    assert(_proxy->ice_isTwoway());
    _response(ok, outParams);
}

void
Glacier2::Request::exception(exception_ptr ex)
{
    //
    // Only for twoways, oneway dispatches are finished when queued, see queued().
    //
    if(_proxy->ice_isTwoway())
    {
        _exception(ex);
    }
}

void
Glacier2::Request::queued()
{
    if(!_proxy->ice_isTwoway())
    {
        _response(true, { nullptr, nullptr });
    }
}

Glacier2::RequestQueue::RequestQueue(shared_ptr<RequestQueueThread> requestQueueThread,
                                     shared_ptr<Instance> instance,
                                     shared_ptr<Ice::Connection> connection) :
    _requestQueueThread(move(requestQueueThread)),
    _instance(move(instance)),
    _connection(move(connection)),
    _pendingSend(false),
    _destroyed(false)
{
}

bool
Glacier2::RequestQueue::addRequest(shared_ptr<Request> request)
{
    lock_guard<mutex> lg(_mutex);
    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    if(request->hasOverride())
    {
        for(auto& r : _requests)
        {
            //
            // If the new request overrides an old one, then abort the old
            // request and replace it with the new request.
            //
            if(request->override(r))
            {
                if(_observer)
                {
                    _observer->overridden(!_connection);
                }
                request->queued();
                r = move(request);
                return true;
            }
        }
    }

    //
    // No override, we add the new request.
    //
    if(_requests.empty() && (!_connection || !_pendingSend))
    {
        _requestQueueThread->flushRequestQueue(shared_from_this()); // This might throw if the thread is destroyed.
    }
    request->queued();
    _requests.push_back(move(request));
    if(_observer)
    {
        _observer->queued(!_connection);
    }
    return false;
}

void
Glacier2::RequestQueue::flushRequests()
{
    lock_guard<mutex> lg(_mutex);
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
        for(const auto& request : _requests)
        {
            if(_observer)
            {
                _observer->forwarded(!_connection);
            }
            auto self = shared_from_this();
            request->invoke(
                [self, request](bool ok, const pair<const Byte*, const Byte*>& outParams)
                {
                    self->response(ok, outParams, request);
                },
                [self, request](exception_ptr e)
                {
                    self->exception(e, request);
                }
            );
        }
        _requests.clear();
    }
}

void
Glacier2::RequestQueue::destroy()
{
    lock_guard<mutex> lg(_mutex);
    _destroyed = true;
}

void
Glacier2::RequestQueue::updateObserver(shared_ptr<Glacier2::Instrumentation::SessionObserver> observer)
{
    lock_guard<mutex> lg(_mutex);
    _observer = observer;
}

void
Glacier2::RequestQueue::flush()
{
    assert(_connection);
    _pendingSend = false;
    _pendingSendRequest = nullptr;

    deque<shared_ptr<Request>>::iterator p;
    for(p = _requests.begin(); p != _requests.end(); ++p)
    {
        if(_observer)
        {
            _observer->forwarded(!_connection);
        }

        shared_ptr<promise<void>> isSent = make_shared<promise<void>>();
        shared_ptr<promise<void>> completedExceptionally = make_shared<promise<void>>();

        auto self = shared_from_this();
        auto request = *p;

        request->invoke(
            [self, request](bool ok, const pair<const Byte*, const Byte*>& outParams)
            {
                self->response(ok, outParams, request);
            },
            [self, request, completedExceptionally](exception_ptr e)
            {
                completedExceptionally->set_value();
                self->exception(e, request);
            },
            [self, request, isSent](bool sentSynchronously)
            {
                isSent->set_value();
                self->sent(sentSynchronously, request);
            }
        );

        if((isSent->get_future().wait_for(0s) != future_status::ready) &&
            (completedExceptionally->get_future().wait_for(0s) != future_status::ready))
        {
            _pendingSend = true;
            _pendingSendRequest = *p++;
            break;
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
}

void
Glacier2::RequestQueue::response(bool ok, const pair<const Byte*, const Byte*>& outParams, const shared_ptr<Request>& request)
{
    assert(request);
    request->response(ok, outParams);
}

void
Glacier2::RequestQueue::exception(exception_ptr ex, const shared_ptr<Request>& request)
{
    //
    // If the connection has been lost, destroy the session.
    //
    if(_connection)
    {
        try
        {
            rethrow_exception(ex);
        }
        catch(const Ice::LocalException& e)
        {
            if(dynamic_cast<const Ice::SocketException*>(&e) ||
               dynamic_cast<const Ice::TimeoutException*>(&e) ||
               dynamic_cast<const Ice::ProtocolException*>(&e))
            {
                try
                {
                    _instance->sessionRouter()->destroySession(_connection);
                }
                catch(const Exception&)
                {
                }
            }
        }

        lock_guard<mutex> lg(_mutex);
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
Glacier2::RequestQueue::sent(bool sentSynchronously, const shared_ptr<Request>& request)
{
    if(_connection && !sentSynchronously)
    {
        lock_guard<mutex> lg(_mutex);
        if(request == _pendingSendRequest)
        {
            flush();
        }
    }
}

Glacier2::RequestQueueThread::RequestQueueThread(std::chrono::milliseconds sleepTime) :
    _sleepTime(move(sleepTime)),
    _destroy(false),
    _sleep(false),
    _thread([this] { run(); })
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
        lock_guard<mutex> lg(_mutex);
        assert(!_destroy);
        _destroy = true;
        _sleep = false;
    }

    _condVar.notify_one();

    _thread.join();
}

void
Glacier2::RequestQueueThread::flushRequestQueue(shared_ptr<RequestQueue> queue)
{
    lock_guard<mutex> lg(_mutex);

    if(_destroy)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    if(_queues.empty() && !_sleep)
    {
        _condVar.notify_one();
    }
    _queues.push_back(move(queue));
}

void
Glacier2::RequestQueueThread::run()
{
    while(true)
    {
        vector<shared_ptr<RequestQueue>> queues;

        {
            unique_lock<mutex> lock(_mutex);

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
                    auto now = chrono::steady_clock::now();

                    if(_condVar.wait_for(lock, _sleepDuration) == cv_status::no_timeout)
                    {
                        _sleepDuration = 0ns;
                    }
                    else
                    {
                        _sleepDuration -= chrono::steady_clock::now() - now;
                    }

                    if(_sleepDuration <= 0ns)
                    {
                        _sleep = false;
                    }
                }
                else
                {
                    _condVar.wait(lock);
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

            if(_sleepTime > 0ms)
            {
                _sleep = true;
                _sleepDuration = _sleepTime;
            }
        }

        for(const auto& queue : queues)
        {
            queue->flushRequests();
        }
    }
}
