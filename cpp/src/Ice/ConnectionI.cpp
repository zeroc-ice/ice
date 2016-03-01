// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/ConnectionI.h>
#include <Ice/Instance.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Properties.h>
#include <Ice/TraceUtil.h>
#include <Ice/TraceLevels.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Transceiver.h>
#include <Ice/ThreadPool.h>
#include <Ice/ACM.h>
#include <Ice/ObjectAdapterI.h> // For getThreadPool() and getServantManager().
#include <Ice/EndpointI.h>
#include <Ice/Outgoing.h>
#include <Ice/OutgoingAsync.h>
#include <Ice/Incoming.h>
#include <Ice/LocalException.h>
#include <Ice/RequestHandler.h> // For RetryException
#include <Ice/ReferenceFactory.h> // For createProxy().
#include <Ice/ProxyFactory.h> // For createProxy().
#include <Ice/BatchRequestQueue.h>

#ifdef ICE_HAS_BZIP2
#  include <bzlib.h>
#endif

using namespace std;
using namespace Ice;
using namespace Ice::Instrumentation;
using namespace IceInternal;

Ice::LocalObject* Ice::upCast(ConnectionI* p) { return p; }

namespace
{

const ::std::string __flushBatchRequests_name = "flushBatchRequests";


class TimeoutCallback : public IceUtil::TimerTask
{
public:

    TimeoutCallback(Ice::ConnectionI* connection) : _connection(connection)
    {
    }

    void
    runTimerTask()
    {
        _connection->timedOut();
    }

private:

    Ice::ConnectionI* _connection;
};

class DispatchCall : public DispatchWorkItem
{
public:

    DispatchCall(const ConnectionIPtr& connection, const ConnectionI::StartCallbackPtr& startCB,
                 const vector<ConnectionI::OutgoingMessage>& sentCBs, Byte compress, Int requestId,
                 Int invokeNum, const ServantManagerPtr& servantManager, const ObjectAdapterPtr& adapter,
                 const OutgoingAsyncBasePtr& outAsync, const ConnectionCallbackPtr& heartbeatCallback,
                 BasicStream& stream) :
        DispatchWorkItem(connection),
        _connection(connection),
        _startCB(startCB),
        _sentCBs(sentCBs),
        _compress(compress),
        _requestId(requestId),
        _invokeNum(invokeNum),
        _servantManager(servantManager),
        _adapter(adapter),
        _outAsync(outAsync),
        _heartbeatCallback(heartbeatCallback),
        _stream(stream.instance(), currentProtocolEncoding)
    {
        _stream.swap(stream);
    }

    virtual void
    run()
    {
        _connection->dispatch(_startCB, _sentCBs, _compress, _requestId, _invokeNum, _servantManager, _adapter,
                              _outAsync, _heartbeatCallback, _stream);
    }

private:

    const ConnectionIPtr _connection;
    const ConnectionI::StartCallbackPtr _startCB;
    const vector<ConnectionI::OutgoingMessage> _sentCBs;
    const Byte _compress;
    const Int _requestId;
    const Int _invokeNum;
    const ServantManagerPtr _servantManager;
    const ObjectAdapterPtr _adapter;
    const OutgoingAsyncBasePtr _outAsync;
    const ConnectionCallbackPtr _heartbeatCallback;
    BasicStream _stream;
};

class FinishCall : public DispatchWorkItem
{
public:

    FinishCall(const Ice::ConnectionIPtr& connection, bool close) :
        DispatchWorkItem(connection), _connection(connection), _close(close)
    {
    }

    virtual void
    run()
    {
        _connection->finish(_close);
    }

private:

    const ConnectionIPtr _connection;
    const bool _close;
};

ConnectionState connectionStateMap[] = {
    ConnectionStateValidating,   // StateNotInitialized
    ConnectionStateValidating,   // StateNotValidated
    ConnectionStateActive,       // StateActive
    ConnectionStateHolding,      // StateHolding
    ConnectionStateClosing,      // StateClosing
    ConnectionStateClosing,      // StateClosingPending
    ConnectionStateClosed,       // StateClosed
    ConnectionStateClosed,       // StateFinished
};

}

Ice::ConnectionI::Observer::Observer() : _readStreamPos(0), _writeStreamPos(0)
{
}

void
Ice::ConnectionI::Observer::startRead(const Buffer& buf)
{
    if(_readStreamPos)
    {
        assert(!buf.b.empty());
        _observer->receivedBytes(static_cast<int>(buf.i - _readStreamPos));
    }
    _readStreamPos = buf.b.empty() ? 0 : buf.i;
}

void
Ice::ConnectionI::Observer::finishRead(const Buffer& buf)
{
    if(_readStreamPos == 0)
    {
        return;
    }
    assert(buf.i >= _readStreamPos);
    _observer->receivedBytes(static_cast<int>(buf.i - _readStreamPos));
    _readStreamPos = 0;
}

void
Ice::ConnectionI::Observer::startWrite(const Buffer& buf)
{
    if(_writeStreamPos)
    {
        assert(!buf.b.empty());
        _observer->sentBytes(static_cast<int>(buf.i - _writeStreamPos));
    }
    _writeStreamPos = buf.b.empty() ? 0 : buf.i;
}

void
Ice::ConnectionI::Observer::finishWrite(const Buffer& buf)
{
    if(_writeStreamPos == 0)
    {
        return;
    }
    if(buf.i > _writeStreamPos)
    {
        _observer->sentBytes(static_cast<int>(buf.i - _writeStreamPos));
    }
    _writeStreamPos = 0;
}

void
Ice::ConnectionI::Observer::attach(const Ice::Instrumentation::ConnectionObserverPtr& observer)
{
    ObserverHelperT<Ice::Instrumentation::ConnectionObserver>::attach(observer);
    if(!observer)
    {
        _writeStreamPos = 0;
        _readStreamPos = 0;
    }
}


void
Ice::ConnectionI::OutgoingMessage::adopt(BasicStream* str)
{
    if(adopted)
    {
        if(str)
        {
            delete stream;
            stream = 0;
            adopted = false;
        }
        else
        {
            return; // Stream is already adopted.
        }
    }
    else if(!str)
    {
        if(out || outAsync)
        {
            return; // Adopting request stream is not necessary.
        }
        else
        {
            str = stream; // Adopt this stream
            stream = 0;
        }
    }

    assert(str);
    stream = new BasicStream(str->instance(), currentProtocolEncoding);
    stream->swap(*str);
    adopted = true;
}

void
Ice::ConnectionI::OutgoingMessage::canceled(bool adoptStream)
{
    assert((out || outAsync)); // Only requests can timeout.
    out = 0;
    outAsync = 0;
    if(adoptStream)
    {
        adopt(0); // Adopt the request stream
    }
    else
    {
        assert(!adopted);
    }
}

bool
Ice::ConnectionI::OutgoingMessage::sent()
{
    if(adopted)
    {
        delete stream;
    }
    stream = 0;

    if(out)
    {
        out->sent();
    }
    else if(outAsync)
    {
#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
        invokeSent = outAsync->sent();
        return invokeSent || receivedReply;
#else
        return outAsync->sent();
#endif
    }
    return false;
}

void
Ice::ConnectionI::OutgoingMessage::completed(const Ice::LocalException& ex)
{
    if(out)
    {
        out->completed(ex);
    }
    else if(outAsync)
    {
        if(outAsync->completed(ex))
        {
            outAsync->invokeCompleted();
        }
    }

    if(adopted)
    {
        delete stream;
    }
    stream = 0;
}

void
Ice::ConnectionI::start(const StartCallbackPtr& callback)
{
    try
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        if(_state >= StateClosed) // The connection might already be closed if the communicator was destroyed.
        {
            assert(_exception.get());
            _exception->ice_throw();
        }

        if(!initialize() || !validate())
        {
            if(callback)
            {
                _startCallback = callback;
                return;
            }

            //
            // Wait for the connection to be validated.
            //
            while(_state <= StateNotValidated)
            {
                wait();
            }

            if(_state >= StateClosing)
            {
                assert(_exception.get());
                _exception->ice_throw();
            }
        }

        //
        // We start out in holding state.
        //
        setState(StateHolding);
    }
    catch(const Ice::LocalException& ex)
    {
        exception(ex);
        if(callback)
        {
            callback->connectionStartFailed(this, *_exception.get());
            return;
        }
        else
        {
            waitUntilFinished();
            throw;
        }
    }

    if(callback)
    {
        callback->connectionStartCompleted(this);
    }
}

void
Ice::ConnectionI::activate()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state <= StateNotValidated)
    {
        return;
    }
    if(_acmLastActivity != IceUtil::Time())
    {
        _acmLastActivity = IceUtil::Time::now(IceUtil::Time::Monotonic);
    }
    setState(StateActive);
}

void
Ice::ConnectionI::hold()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state <= StateNotValidated)
    {
        return;
    }

    setState(StateHolding);
}

void
Ice::ConnectionI::destroy(DestructionReason reason)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    switch(reason)
    {
        case ObjectAdapterDeactivated:
        {
            setState(StateClosing, ObjectAdapterDeactivatedException(__FILE__, __LINE__));
            break;
        }

        case CommunicatorDestroyed:
        {
            setState(StateClosing, CommunicatorDestroyedException(__FILE__, __LINE__));
            break;
        }
    }
}

void
Ice::ConnectionI::close(bool force)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(force)
    {
        setState(StateClosed, ForcedCloseConnectionException(__FILE__, __LINE__));
    }
    else
    {
        //
        // If we do a graceful shutdown, then we wait until all
        // outstanding requests have been completed. Otherwise, the
        // CloseConnectionException will cause all outstanding
        // requests to be retried, regardless of whether the server
        // has processed them or not.
        //
        while(!_requests.empty() || !_asyncRequests.empty())
        {
            wait();
        }

        setState(StateClosing, CloseConnectionException(__FILE__, __LINE__));
    }
}

bool
Ice::ConnectionI::isActiveOrHolding() const
{
    //
    // We can not use trylock here, otherwise the outgoing connection
    // factory might return destroyed (closing or closed) connections,
    // resulting in connection retry exhaustion.
    //
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    return _state > StateNotValidated && _state < StateClosing;
}

bool
Ice::ConnectionI::isFinished() const
{
    //
    // We can use trylock here, because as long as there are still
    // threads operating in this connection object, connection
    // destruction is considered as not yet finished.
    //
    IceUtil::Monitor<IceUtil::Mutex>::TryLock sync(*this);

    if(!sync.acquired())
    {
        return false;
    }

    if(_state != StateFinished || _dispatchCount != 0)
    {
        return false;
    }

    assert(_state == StateFinished);
    return true;
}

void
Ice::ConnectionI::throwException() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_exception.get())
    {
        assert(_state >= StateClosing);
        _exception->ice_throw();
    }
}

void
Ice::ConnectionI::waitUntilHolding() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    while(_state < StateHolding || _dispatchCount > 0)
    {
        wait();
    }
}

void
Ice::ConnectionI::waitUntilFinished()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    //
    // We wait indefinitely until the connection is finished and all
    // outstanding requests are completed. Otherwise we couldn't
    // guarantee that there are no outstanding calls when deactivate()
    // is called on the servant locators.
    //
    while(_state < StateFinished || _dispatchCount > 0)
    {
        wait();
    }

    assert(_state == StateFinished);

    //
    // Clear the OA. See bug 1673 for the details of why this is necessary.
    //
    _adapter = 0;
}

void
Ice::ConnectionI::updateObserver()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state < StateNotValidated || _state > StateClosed)
    {
        return;
    }

    assert(_instance->initializationData().observer);
    _observer.attach(_instance->initializationData().observer->getConnectionObserver(initConnectionInfo(),
                                                                                     _endpoint,
                                                                                     toConnectionState(_state),
                                                                                     _observer.get()));
}

void
Ice::ConnectionI::monitor(const IceUtil::Time& now, const ACMConfig& acm)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state != StateActive)
    {
        return;
    }
    assert(acm.timeout != IceUtil::Time());

    //
    // We send a heartbeat if there was no activity in the last
    // (timeout / 4) period. Sending a heartbeat sooner than really
    // needed is safer to ensure that the receiver will receive in
    // time the heartbeat. Sending the heartbeat if there was no
    // activity in the last (timeout / 2) period isn't enough since
    // monitor() is called only every (timeout / 2) period.
    //
    // Note that this doesn't imply that we are sending 4 heartbeats
    // per timeout period because the monitor() method is sill only
    // called every (timeout / 2) period.
    //
    if(acm.heartbeat == HeartbeatAlways ||
       (acm.heartbeat != HeartbeatOff && _writeStream.b.empty() && now >= (_acmLastActivity + acm.timeout / 4)))
    {
        if(acm.heartbeat != HeartbeatOnInvocation || _dispatchCount > 0)
        {
            heartbeat();
        }
    }

    if(static_cast<Int>(_readStream.b.size()) > headerSize || !_writeStream.b.empty())
    {
        //
        // If writing or reading, nothing to do, the connection
        // timeout will kick-in if writes or reads don't progress.
        // This check is necessary because the actitivy timer is
        // only set when a message is fully read/written.
        //
        return;
    }

    if(acm.close != CloseOff && now >= (_acmLastActivity + acm.timeout))
    {
        if(acm.close == CloseOnIdleForceful ||
           (acm.close != CloseOnIdle && (!_requests.empty() || !_asyncRequests.empty())))
        {
            //
            // Close the connection if we didn't receive a heartbeat in
            // the last period.
            //
            setState(StateClosed, ConnectionTimeoutException(__FILE__, __LINE__));
        }
        else if(acm.close != CloseOnInvocation &&
                _dispatchCount == 0 && _batchRequestQueue->isEmpty() && _requests.empty() && _asyncRequests.empty())
        {
            //
            // The connection is idle, close it.
            //
            setState(StateClosing, ConnectionTimeoutException(__FILE__, __LINE__));
        }
    }
}

bool
Ice::ConnectionI::sendRequest(OutgoingBase* out, bool compress, bool response, int batchRequestNum)
{
    BasicStream* os = out->os();

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_exception.get())
    {
        //
        // If the connection is closed before we even have a chance
        // to send our request, we always try to send the request
        // again.
        //
        throw RetryException(*_exception.get());
    }

    assert(_state > StateNotValidated);
    assert(_state < StateClosing);

    //
    // Ensure the message isn't bigger than what we can send with the
    // transport.
    //
    _transceiver->checkSendSize(*os);

    Int requestId = 0;
    if(response)
    {
        //
        // Create a new unique request ID.
        //
        requestId = _nextRequestId++;
        if(requestId <= 0)
        {
            _nextRequestId = 1;
            requestId = _nextRequestId++;
        }

        //
        // Fill in the request ID.
        //
        const Byte* p = reinterpret_cast<const Byte*>(&requestId);
#ifdef ICE_BIG_ENDIAN
        reverse_copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#else
        copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#endif
    }
    else if(batchRequestNum > 0)
    {
        const Byte* p = reinterpret_cast<const Byte*>(&batchRequestNum);
#ifdef ICE_BIG_ENDIAN
        reverse_copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#else
        copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#endif
    }

    out->attachRemoteObserver(initConnectionInfo(), _endpoint, requestId);

    //
    // Send the message. If it can't be sent without blocking the message is added
    // to _sendStreams and it will be sent by the selector thread.
    //
    bool sent = false;
    try
    {
        OutgoingMessage message(out, os, compress, requestId);
        sent = sendMessage(message) & AsyncStatusSent;
    }
    catch(const LocalException& ex)
    {
        setState(StateClosed, ex);
        assert(_exception.get());
        _exception->ice_throw();
    }

    if(response)
    {
        //
        // Add to the requests map.
        //
        _requestsHint = _requests.insert(_requests.end(), pair<const Int, OutgoingBase*>(requestId, out));
    }

    return sent;
}

AsyncStatus
Ice::ConnectionI::sendAsyncRequest(const OutgoingAsyncBasePtr& out, bool compress, bool response, int batchRequestNum)
{
    BasicStream* os = out->getOs();

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_exception.get())
    {
        //
        // If the exception is closed before we even have a chance
        // to send our request, we always try to send the request
        // again.
        //
        throw RetryException(*_exception.get());
    }

    assert(_state > StateNotValidated);
    assert(_state < StateClosing);

    //
    // Ensure the message isn't bigger than what we can send with the
    // transport.
    //
    _transceiver->checkSendSize(*os);

    //
    // Notify the request that it's cancelable with this connection.
    // This will throw if the request is canceled.
    //
    out->cancelable(this);

    Int requestId = 0;
    if(response)
    {
        //
        // Create a new unique request ID.
        //
        requestId = _nextRequestId++;
        if(requestId <= 0)
        {
            _nextRequestId = 1;
            requestId = _nextRequestId++;
        }

        //
        // Fill in the request ID.
        //
        const Byte* p = reinterpret_cast<const Byte*>(&requestId);
#ifdef ICE_BIG_ENDIAN
        reverse_copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#else
        copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#endif
    }
    else if(batchRequestNum > 0)
    {
        const Byte* p = reinterpret_cast<const Byte*>(&batchRequestNum);
#ifdef ICE_BIG_ENDIAN
        reverse_copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#else
        copy(p, p + sizeof(Int), os->b.begin() + headerSize);
#endif
    }

    out->attachRemoteObserver(initConnectionInfo(), _endpoint, requestId);

    AsyncStatus status = AsyncStatusQueued;
    try
    {
        OutgoingMessage message(out, os, compress, requestId);
        status = sendMessage(message);
    }
    catch(const LocalException& ex)
    {
        setState(StateClosed, ex);
        assert(_exception.get());
        _exception->ice_throw();
    }

    if(response)
    {
        //
        // Add to the async requests map.
        //
        _asyncRequestsHint = _asyncRequests.insert(_asyncRequests.end(),
                                                   pair<const Int, OutgoingAsyncBasePtr>(requestId, out));
    }
    return status;
}

BatchRequestQueuePtr
Ice::ConnectionI::getBatchRequestQueue() const
{
    return _batchRequestQueue;
}

void
Ice::ConnectionI::flushBatchRequests()
{
    ConnectionFlushBatch out(this, _instance.get(), __flushBatchRequests_name);
    out.invoke();
}

AsyncResultPtr
Ice::ConnectionI::begin_flushBatchRequests()
{
    return __begin_flushBatchRequests(__dummyCallback, 0);
}

AsyncResultPtr
Ice::ConnectionI::begin_flushBatchRequests(const CallbackPtr& cb, const LocalObjectPtr& cookie)
{
    return __begin_flushBatchRequests(cb, cookie);
}

AsyncResultPtr
Ice::ConnectionI::begin_flushBatchRequests(const Callback_Connection_flushBatchRequestsPtr& cb,
                                           const LocalObjectPtr& cookie)
{
    return __begin_flushBatchRequests(cb, cookie);
}

AsyncResultPtr
Ice::ConnectionI::begin_flushBatchRequests(const IceInternal::Function<void (const Exception&)>& exception,
                                           const IceInternal::Function<void (bool)>& sent)
{
#ifdef ICE_CPP11
    class Cpp11CB : public IceInternal::Cpp11FnCallbackNC
    {
    public:

        Cpp11CB(const IceInternal::Function<void (const Exception&)>& excb,
                const IceInternal::Function<void (bool)>& sentcb) :
            IceInternal::Cpp11FnCallbackNC(excb, sentcb)
        {
            CallbackBase::checkCallback(true, excb != nullptr);
        }

        virtual void
        completed(const AsyncResultPtr& __result) const
        {
            ConnectionPtr __con = __result->getConnection();
            assert(__con);
            try
            {
                __con->end_flushBatchRequests(__result);
                assert(false);
            }
            catch(const Exception& ex)
            {
                IceInternal::Cpp11FnCallbackNC::exception(__result, ex);
            }
        }
    };

    return __begin_flushBatchRequests(new Cpp11CB(exception, sent), 0);
#else
    assert(false); // Ice not built with C++11 support.
    return 0;
#endif
}

AsyncResultPtr
Ice::ConnectionI::__begin_flushBatchRequests(const CallbackBasePtr& cb, const LocalObjectPtr& cookie)
{
    ConnectionFlushBatchAsyncPtr result = new ConnectionFlushBatchAsync(this,
                                                                        _communicator,
                                                                        _instance,
                                                                        __flushBatchRequests_name,
                                                                        cb,
                                                                        cookie);
    result->invoke();
    return result;
}

void
Ice::ConnectionI::end_flushBatchRequests(const AsyncResultPtr& r)
{
    AsyncResult::__check(r, this, __flushBatchRequests_name);
    r->__wait();
}

void
Ice::ConnectionI::setCallback(const ConnectionCallbackPtr& callback)
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        if(_state >= StateClosed)
        {
            if(callback)
            {
                class CallbackWorkItem : public DispatchWorkItem
                {
                public:

                    CallbackWorkItem(const ConnectionIPtr& connection, const ConnectionCallbackPtr& callback) :
                        _connection(connection),
                        _callback(callback)
                    {
                    }

                    virtual void run()
                    {
                        _connection->closeCallback(_callback);
                    }

                private:

                    const ConnectionIPtr _connection;
                    const ConnectionCallbackPtr _callback;
                };
                _threadPool->dispatch(new CallbackWorkItem(this, callback));
            }
        }
        else
        {
            _callback = callback;
        }
    }
}

void
Ice::ConnectionI::closeCallback(const ConnectionCallbackPtr& callback)
{
    try
    {
        callback->closed(this);
    }
    catch(const std::exception& ex)
    {
        Error out(_instance->initializationData().logger);
        out << "connection callback exception:\n" << ex << '\n' << _desc;
    }
    catch(...)
    {
        Error out(_instance->initializationData().logger);
        out << "connection callback exception:\nunknown c++ exception" << '\n' << _desc;
    }
}

void
Ice::ConnectionI::setACM(const IceUtil::Optional<int>& timeout,
                         const IceUtil::Optional<Ice::ACMClose>& close,
                         const IceUtil::Optional<Ice::ACMHeartbeat>& heartbeat)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(!_monitor || _state >= StateClosed)
    {
        return;
    }

    if(_state == StateActive)
    {
        _monitor->remove(this);
    }
    _monitor = _monitor->acm(timeout, close, heartbeat);

    if(_monitor->getACM().timeout <= 0)
    {
        _acmLastActivity = IceUtil::Time(); // Disable the recording of last activity.
    }
    else if(_acmLastActivity == IceUtil::Time() && _state == StateActive)
    {
        _acmLastActivity = IceUtil::Time::now(IceUtil::Time::Monotonic);
    }

    if(_state == StateActive)
    {
        _monitor->add(this);
    }
}

ACM
Ice::ConnectionI::getACM()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    ACM acm;
    acm.timeout = 0;
    acm.close = CloseOff;
    acm.heartbeat = HeartbeatOff;
    return _monitor ? _monitor->getACM() : acm;
}

void
Ice::ConnectionI::requestCanceled(OutgoingBase* out, const Ice::LocalException& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state >= StateClosed)
    {
        return; // The request has already been or will be shortly notified of the failure.
    }

    for(deque<OutgoingMessage>::iterator o = _sendStreams.begin(); o != _sendStreams.end(); ++o)
    {
        if(o->out == out)
        {
            if(o->requestId)
            {
                if(_requestsHint != _requests.end() && _requestsHint->second == out)
                {
                    _requests.erase(_requestsHint);
                    _requestsHint = _requests.end();
                }
                else
                {
                    _requests.erase(o->requestId);
                }
            }

            if(dynamic_cast<const Ice::ConnectionTimeoutException*>(&ex))
            {
                setState(StateClosed, ex);
            }
            else
            {
                //
                // If the request is being sent, don't remove it from the send streams,
                // it will be removed once the sending is finished.
                //
                if(o == _sendStreams.begin())
                {
                    o->canceled(true); // true = adopt the stream.
                }
                else
                {
                    o->canceled(false);
                    _sendStreams.erase(o);
                }
                out->completed(ex);
            }
            return;
        }
    }

    if(dynamic_cast<Outgoing*>(out))
    {
        if(_requestsHint != _requests.end() && _requestsHint->second == out)
        {
            if(dynamic_cast<const Ice::ConnectionTimeoutException*>(&ex))
            {
                setState(StateClosed, ex);
            }
            else
            {
                out->completed(ex);
                _requests.erase(_requestsHint);
                _requestsHint = _requests.end();
            }
            return;
        }
        else
        {
            for(map<Int, OutgoingBase*>::iterator p = _requests.begin(); p != _requests.end(); ++p)
            {
                if(p->second == out)
                {
                    if(dynamic_cast<const Ice::ConnectionTimeoutException*>(&ex))
                    {
                        setState(StateClosed, ex);
                    }
                    else
                    {
                        p->second->completed(ex);
                        assert(p != _requestsHint);
                        _requests.erase(p);
                    }
                    return; // We're done.
                }
            }
        }
    }
}

void
Ice::ConnectionI::asyncRequestCanceled(const OutgoingAsyncBasePtr& outAsync, const LocalException& ex)
{
    //
    // NOTE: This isn't called from a thread pool thread.
    //

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state >= StateClosed)
    {
        return; // The request has already been or will be shortly notified of the failure.
    }

    for(deque<OutgoingMessage>::iterator o = _sendStreams.begin(); o != _sendStreams.end(); ++o)
    {
        if(o->outAsync.get() == outAsync.get())
        {
            if(o->requestId)
            {
                if(_asyncRequestsHint != _asyncRequests.end() &&
                   _asyncRequestsHint->second == OutgoingAsyncPtr::dynamicCast(outAsync))
                {
                    _asyncRequests.erase(_asyncRequestsHint);
                    _asyncRequestsHint = _asyncRequests.end();
                }
                else
                {
                    _asyncRequests.erase(o->requestId);
                }
            }

            if(dynamic_cast<const Ice::ConnectionTimeoutException*>(&ex))
            {
                setState(StateClosed, ex);
            }
            else
            {
                //
                // If the request is being sent, don't remove it from the send streams,
                // it will be removed once the sending is finished.
                //
                if(o == _sendStreams.begin())
                {
                    o->canceled(true); // true = adopt the stream
                }
                else
                {
                    o->canceled(false);
                    _sendStreams.erase(o);
                }
                if(outAsync->completed(ex))
                {
                    outAsync->invokeCompletedAsync();
                }
            }
            return;
        }
    }

    if(OutgoingAsyncPtr::dynamicCast(outAsync))
    {
        if(_asyncRequestsHint != _asyncRequests.end())
        {
            if(_asyncRequestsHint->second == outAsync)
            {
                if(dynamic_cast<const Ice::ConnectionTimeoutException*>(&ex))
                {
                    setState(StateClosed, ex);
                }
                else
                {
                    _asyncRequests.erase(_asyncRequestsHint);
                    _asyncRequestsHint = _asyncRequests.end();
                    if(outAsync->completed(ex))
                    {
                        outAsync->invokeCompletedAsync();
                    }
                }
                return;
            }
        }

        for(map<Int, OutgoingAsyncBasePtr>::iterator p = _asyncRequests.begin(); p != _asyncRequests.end(); ++p)
        {
            if(p->second.get() == outAsync.get())
            {
                if(dynamic_cast<const Ice::ConnectionTimeoutException*>(&ex))
                {
                    setState(StateClosed, ex);
                }
                else
                {
                    assert(p != _asyncRequestsHint);
                    _asyncRequests.erase(p);
                    if(outAsync->completed(ex))
                    {
                        outAsync->invokeCompletedAsync();
                    }
                }
                return;
            }
        }
    }
}

void
Ice::ConnectionI::sendResponse(Int, BasicStream* os, Byte compressFlag, bool /*amd*/)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(_state > StateNotValidated);

    try
    {
        if(--_dispatchCount == 0)
        {
            if(_state == StateFinished)
            {
                reap();
            }
            notifyAll();
        }

        if(_state >= StateClosed)
        {
            assert(_exception.get());
            _exception->ice_throw();
        }

        OutgoingMessage message(os, compressFlag > 0);
        sendMessage(message);

        if(_state == StateClosing && _dispatchCount == 0)
        {
            initiateShutdown();
        }

        return;
    }
    catch(const LocalException& ex)
    {
        setState(StateClosed, ex);
    }
}

void
Ice::ConnectionI::sendNoResponse()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(_state > StateNotValidated);

    try
    {
        if(--_dispatchCount == 0)
        {
            if(_state == StateFinished)
            {
                reap();
            }
            notifyAll();
        }

        if(_state >= StateClosed)
        {
            assert(_exception.get());
            _exception->ice_throw();
        }

        if(_state == StateClosing && _dispatchCount == 0)
        {
            initiateShutdown();
        }
    }
    catch(const LocalException& ex)
    {
        setState(StateClosed, ex);
    }
}

bool
Ice::ConnectionI::systemException(Int, const SystemException&, bool /*amd*/)
{
    return false; // System exceptions aren't marshalled.
}

void
Ice::ConnectionI::invokeException(Ice::Int, const LocalException& ex, int invokeNum, bool /*amd*/)
{
    //
    // Fatal exception while invoking a request. Since sendResponse/sendNoResponse isn't
    // called in case of a fatal exception we decrement _dispatchCount here.
    //

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateClosed, ex);

    if(invokeNum > 0)
    {
        assert(_dispatchCount >= invokeNum);
        _dispatchCount -= invokeNum;
        if(_dispatchCount == 0)
        {
            if(_state == StateFinished)
            {
                reap();
            }
            notifyAll();
        }
    }
}

EndpointIPtr
Ice::ConnectionI::endpoint() const
{
    return _endpoint; // No mutex protection necessary, _endpoint is immutable.
}

ConnectorPtr
Ice::ConnectionI::connector() const
{
    return _connector; // No mutex protection necessary, _connector is immutable.
}

void
Ice::ConnectionI::setAdapter(const ObjectAdapterPtr& adapter)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_state <= StateNotValidated || _state >= StateClosing)
    {
        return;
    }

    _adapter = adapter;

    if(_adapter)
    {
        _servantManager = dynamic_cast<ObjectAdapterI*>(_adapter.get())->getServantManager();
        if(!_servantManager)
        {
            _adapter = 0;
        }
    }
    else
    {
        _servantManager = 0;
    }

    //
    // We never change the thread pool with which we were initially
    // registered, even if we add or remove an object adapter.
    //
}

ObjectAdapterPtr
Ice::ConnectionI::getAdapter() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    return _adapter;
}

EndpointPtr
Ice::ConnectionI::getEndpoint() const
{
    return _endpoint; // No mutex protection necessary, _endpoint is immutable.
}

ObjectPrx
Ice::ConnectionI::createProxy(const Identity& ident) const
{
    //
    // Create a reference and return a reverse proxy for this
    // reference.
    //
    ConnectionIPtr self = const_cast<ConnectionI*>(this);
    return _instance->proxyFactory()->referenceToProxy(_instance->referenceFactory()->create(ident, self));
}

#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
bool
Ice::ConnectionI::startAsync(SocketOperation operation)
{
    if(_state >= StateClosed)
    {
        return false;
    }

    try
    {
        if(operation & SocketOperationWrite)
        {
            if(_observer)
            {
                _observer.startWrite(_writeStream);
            }

            if(_transceiver->startWrite(_writeStream) && !_sendStreams.empty())
            {
                // The whole message is written, assume it's sent now for at-most-once semantics.
                _sendStreams.front().isSent = true;
            }
        }
        else if(operation & SocketOperationRead)
        {
            if(!_hasMoreData)
            {
                if(_observer && !_readHeader)
                {
                    _observer.startRead(_readStream);
                }

                _transceiver->startRead(_readStream);
            }
            else
            {
                _transceiver->getNativeInfo()->completed(IceInternal::SocketOperationRead);
            }
        }
    }
    catch(const Ice::LocalException& ex)
    {
        setState(StateClosed, ex);
        return false;
    }
    return true;
}

bool
Ice::ConnectionI::finishAsync(SocketOperation operation)
{
    try
    {
        if(operation & SocketOperationWrite)
        {
            Buffer::Container::iterator start = _writeStream.i;
            _transceiver->finishWrite(_writeStream);
            if(_instance->traceLevels()->network >= 3 && _writeStream.i != start)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                out << "sent " << (_writeStream.i - start);
                if(!_endpoint->datagram())
                {
                    out << " of " << (_writeStream.b.end() - start);
                }
                out << " bytes via " << _endpoint->protocol() << "\n" << toString();
            }

            if(_observer)
            {
                _observer.finishWrite(_writeStream);
            }
        }
        else if(operation & SocketOperationRead)
        {
            if(!_hasMoreData)
            {
                Buffer::Container::iterator start = _readStream.i;
                _transceiver->finishRead(_readStream, _hasMoreData);
                if(_instance->traceLevels()->network >= 3 && _readStream.i != start)
                {
                    Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                    out << "received ";
                    if(_endpoint->datagram())
                    {
                        out << _readStream.b.size();
                    }
                    else
                    {
                        out << (_readStream.i - start) << " of " << (_readStream.b.end() - start);
                    }
                    out << " bytes via " << _endpoint->protocol() << "\n" << toString();
                }

                if(_observer && !_readHeader)
                {
                    _observer.finishRead(_readStream);
                }
            }
        }
    }
    catch(const Ice::LocalException& ex)
    {
        setState(StateClosed, ex);
    }
    return _state < StateClosed;
}
#endif

void
Ice::ConnectionI::message(ThreadPoolCurrent& current)
{
    StartCallbackPtr startCB;
    vector<OutgoingMessage> sentCBs;
    Byte compress = 0;
    Int requestId = 0;
    Int invokeNum = 0;
    ServantManagerPtr servantManager;
    ObjectAdapterPtr adapter;
    OutgoingAsyncBasePtr outAsync;
    ConnectionCallbackPtr heartbeatCallback;
    int dispatchCount = 0;

    ThreadPoolMessage<ConnectionI> msg(current, *this);

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        ThreadPoolMessage<ConnectionI>::IOScope io(msg);
        if(!io)
        {
            return;
        }

        if(_state >= StateClosed)
        {
            return;
        }

        SocketOperation readyOp = current.operation;
        try
        {
            unscheduleTimeout(current.operation);

            SocketOperation writeOp = SocketOperationNone;
            SocketOperation readOp = SocketOperationNone;
            if(readyOp & SocketOperationWrite)
            {
                if(_observer)
                {
                    _observer.startWrite(_writeStream);
                }
                writeOp = write(_writeStream);
                if(_observer && !(writeOp & SocketOperationWrite))
                {
                    _observer.finishWrite(_writeStream);
                }
            }

            while(readyOp & SocketOperationRead)
            {
                if(_observer && !_readHeader)
                {
                    _observer.startRead(_readStream);
                }

                readOp = read(_readStream);
                if(readOp & SocketOperationRead)
                {
                    break;
                }
                if(_observer && !_readHeader)
                {
                    assert(_readStream.i == _readStream.b.end());
                    _observer.finishRead(_readStream);
                }

                if(_readHeader) // Read header if necessary.
                {
                    _readHeader = false;

                    if(_observer)
                    {
                        _observer->receivedBytes(static_cast<int>(headerSize));
                    }

                    ptrdiff_t pos = _readStream.i - _readStream.b.begin();
                    if(pos < headerSize)
                    {
                        //
                        // This situation is possible for small UDP packets.
                        //
                        throw IllegalMessageSizeException(__FILE__, __LINE__);
                    }

                    _readStream.i = _readStream.b.begin();
                    const Byte* m;
                    _readStream.readBlob(m, static_cast<Int>(sizeof(magic)));
                    if(m[0] != magic[0] || m[1] != magic[1] || m[2] != magic[2] || m[3] != magic[3])
                    {
                        BadMagicException ex(__FILE__, __LINE__);
                        ex.badMagic = Ice::ByteSeq(&m[0], &m[0] + sizeof(magic));
                        throw ex;
                    }
                    ProtocolVersion pv;
                    _readStream.read(pv);
                    checkSupportedProtocol(pv);
                    EncodingVersion ev;
                    _readStream.read(ev);
                    checkSupportedProtocolEncoding(ev);

                    Byte messageType;
                    _readStream.read(messageType);
                    Byte compress;
                    _readStream.read(compress);
                    Int size;
                    _readStream.read(size);
                    if(size < headerSize)
                    {
                        throw IllegalMessageSizeException(__FILE__, __LINE__);
                    }
                    if(size > static_cast<Int>(_messageSizeMax))
                    {
                        Ex::throwMemoryLimitException(__FILE__, __LINE__, size, _messageSizeMax);
                    }
                    if(size > static_cast<Int>(_readStream.b.size()))
                    {
                        _readStream.b.resize(size);
                    }
                    _readStream.i = _readStream.b.begin() + pos;
                }

                if(_readStream.i != _readStream.b.end())
                {
                    if(_endpoint->datagram())
                    {
                        throw DatagramLimitException(__FILE__, __LINE__); // The message was truncated.
                    }
                    continue;
                }
                break;
            }

            SocketOperation newOp = static_cast<SocketOperation>(readOp | writeOp);
            readyOp = static_cast<SocketOperation>(readyOp & ~newOp);
            assert(readyOp || newOp);

            if(_state <= StateNotValidated)
            {
                if(newOp)
                {
                    //
                    // Wait for all the transceiver conditions to be
                    // satisfied before continuing.
                    //
                    scheduleTimeout(newOp);
                    _threadPool->update(this, current.operation, newOp);
                    return;
                }

                if(_state == StateNotInitialized && !initialize(current.operation))
                {
                    return;
                }

                if(_state <= StateNotValidated && !validate(current.operation))
                {
                    return;
                }

                _threadPool->unregister(this, current.operation);

                //
                // We start out in holding state.
                //
                setState(StateHolding);
                if(_startCallback)
                {
                    swap(_startCallback, startCB);
                    if(startCB)
                    {
                        ++dispatchCount;
                    }
                }
            }
            else
            {
                assert(_state <= StateClosingPending);

                //
                // We parse messages first, if we receive a close
                // connection message we won't send more messages.
                //
                if(readyOp & SocketOperationRead)
                {
                    newOp = static_cast<SocketOperation>(newOp | parseMessage(current.stream,
                                                                              invokeNum,
                                                                              requestId,
                                                                              compress,
                                                                              servantManager,
                                                                              adapter,
                                                                              outAsync,
                                                                              heartbeatCallback,
                                                                              dispatchCount));
                }

                if(readyOp & SocketOperationWrite)
                {
                    newOp = static_cast<SocketOperation>(newOp | sendNextMessage(sentCBs));
                    if(!sentCBs.empty())
                    {
                        ++dispatchCount;
                    }
                }

                if(_state < StateClosed)
                {
                    scheduleTimeout(newOp);
                    _threadPool->update(this, current.operation, newOp);
                }
            }

            if(_acmLastActivity != IceUtil::Time())
            {
                _acmLastActivity = IceUtil::Time::now(IceUtil::Time::Monotonic);
            }

            if(dispatchCount == 0)
            {
                return; // Nothing to dispatch we're done!
            }

            _dispatchCount += dispatchCount;
            io.completed();
        }
        catch(const DatagramLimitException&) // Expected.
        {
            if(_warnUdp)
            {
                Warning out(_instance->initializationData().logger);
                out << "maximum datagram size of " << _readStream.i - _readStream.b.begin() << " exceeded";
            }
            _readStream.resize(headerSize);
            _readStream.i = _readStream.b.begin();
            _readHeader = true;
            return;
        }
        catch(const SocketException& ex)
        {
            setState(StateClosed, ex);
            return;
        }
        catch(const LocalException& ex)
        {
            if(_endpoint->datagram())
            {
                if(_warn)
                {
                    Warning out(_instance->initializationData().logger);
                    out << "datagram connection exception:\n" << ex << '\n' << _desc;
                }
                _readStream.resize(headerSize);
                _readStream.i = _readStream.b.begin();
                _readHeader = true;
            }
            else
            {
                setState(StateClosed, ex);
            }
            return;
        }
    }

    if(!_dispatcher) // Optimization, call dispatch() directly if there's no dispatcher.
    {
        dispatch(startCB, sentCBs, compress, requestId, invokeNum, servantManager, adapter, outAsync, heartbeatCallback,
                 current.stream);
    }
    else
    {
        _threadPool->dispatchFromThisThread(new DispatchCall(this, startCB, sentCBs, compress, requestId, invokeNum,
                                                             servantManager, adapter, outAsync, heartbeatCallback,
                                                             current.stream));
    }
}

void
ConnectionI::dispatch(const StartCallbackPtr& startCB, const vector<OutgoingMessage>& sentCBs,
                      Byte compress, Int requestId, Int invokeNum, const ServantManagerPtr& servantManager,
                      const ObjectAdapterPtr& adapter, const OutgoingAsyncBasePtr& outAsync,
                      const ConnectionCallbackPtr& heartbeatCallback, BasicStream& stream)
{
    int dispatchedCount = 0;

    //
    // Notify the factory that the connection establishment and
    // validation has completed.
    //
    if(startCB)
    {
        startCB->connectionStartCompleted(this);
        ++dispatchedCount;
    }

    //
    // Notify AMI calls that the message was sent.
    //
    if(!sentCBs.empty())
    {
        for(vector<OutgoingMessage>::const_iterator p = sentCBs.begin(); p != sentCBs.end(); ++p)
        {
#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
            if(p->invokeSent)
            {
                p->outAsync->invokeSent();
            }
            if(p->receivedReply)
            {
                OutgoingAsyncPtr outAsync = OutgoingAsyncPtr::dynamicCast(p->outAsync);
                if(outAsync->completed())
                {
                    outAsync->invokeCompleted();
                }
            }
#else
            p->outAsync->invokeSent();
#endif
        }
        ++dispatchedCount;
    }

    //
    // Asynchronous replies must be handled outside the thread
    // synchronization, so that nested calls are possible.
    //
    if(outAsync)
    {
        outAsync->invokeCompleted();
        ++dispatchedCount;
    }

    if(heartbeatCallback)
    {
        try
        {
            heartbeatCallback->heartbeat(this);
        }
        catch(const std::exception& ex)
        {
            Error out(_instance->initializationData().logger);
            out << "connection callback exception:\n" << ex << '\n' << _desc;
        }
        catch(...)
        {
            Error out(_instance->initializationData().logger);
            out << "connection callback exception:\nunknown c++ exception" << '\n' << _desc;
        }
        ++dispatchedCount;
    }

    //
    // Method invocation (or multiple invocations for batch messages)
    // must be done outside the thread synchronization, so that nested
    // calls are possible.
    //
    if(invokeNum)
    {
        invokeAll(stream, invokeNum, requestId, compress, servantManager, adapter);

        //
        // Don't increase count, the dispatch count is
        // decreased when the incoming reply is sent.
        //
    }

    //
    // Decrease dispatch count.
    //
    if(dispatchedCount > 0)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        _dispatchCount -= dispatchedCount;
        if(_dispatchCount == 0)
        {
            //
            // Only initiate shutdown if not already done. It might
            // have already been done if the sent callback or AMI
            // callback was dispatched when the connection was already
            // in the closing state.
            //
            if(_state == StateClosing)
            {
                try
                {
                    initiateShutdown();
                }
                catch(const LocalException& ex)
                {
                    setState(StateClosed, ex);
                }
            }
            else if(_state == StateFinished)
            {
                reap();
            }
            notifyAll();
        }
    }
}

void
Ice::ConnectionI::finished(ThreadPoolCurrent& current, bool close)
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        assert(_state == StateClosed);
        unscheduleTimeout(static_cast<SocketOperation>(SocketOperationRead | SocketOperationWrite));
    }

    //
    // If there are no callbacks to call, we don't call ioCompleted() since we're not going
    // to call code that will potentially block (this avoids promoting a new leader and
    // unecessary thread creation, especially if this is called on shutdown).
    //
    if(!_startCallback && _sendStreams.empty() && _asyncRequests.empty() && !_callback)
    {
        finish(close);
        return;
    }

    current.ioCompleted();
    if(!_dispatcher) // Optimization, call finish() directly if there's no dispatcher.
    {
        finish(close);
    }
    else
    {
        _threadPool->dispatchFromThisThread(new FinishCall(this, close));
    }
}

void
Ice::ConnectionI::finish(bool close)
{
    if(!_initialized)
    {
        if(_instance->traceLevels()->network >= 2)
        {
            string verb = _connector ? "establish" : "accept";
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
            out << "failed to " << verb << " " << _endpoint->protocol() << " connection\n" << toString()
                << "\n" << *_exception.get();
        }
    }
    else
    {
        if(_instance->traceLevels()->network >= 1)
        {
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
            out << "closed " << _endpoint->protocol() << " connection\n" << toString();

            //
            // Trace the cause of unexpected connection closures
            //
            if(!(dynamic_cast<const CloseConnectionException*>(_exception.get()) ||
                 dynamic_cast<const ForcedCloseConnectionException*>(_exception.get()) ||
                 dynamic_cast<const ConnectionTimeoutException*>(_exception.get()) ||
                 dynamic_cast<const CommunicatorDestroyedException*>(_exception.get()) ||
                 dynamic_cast<const ObjectAdapterDeactivatedException*>(_exception.get())))
            {
                out << "\n" << *_exception.get();
            }
        }
    }

    if(close)
    {
        _transceiver->close();
    }

    if(_startCallback)
    {
        _startCallback->connectionStartFailed(this, *_exception.get());
        _startCallback = 0;
    }

    if(!_sendStreams.empty())
    {
        if(!_writeStream.b.empty())
        {
            //
            // Return the stream to the outgoing call. This is important for
            // retriable AMI calls which are not marshalled again.
            //
            OutgoingMessage* message = &_sendStreams.front();
            _writeStream.swap(*message->stream);

#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
            //
            // The current message might be sent but not yet removed from _sendStreams. If
            // the response has been received in the meantime, we remove the message from
            // _sendStreams to not call finished on a message which is already done.
            //
            if(message->isSent || message->receivedReply)
            {
                if(message->sent() && message->invokeSent)
                {
                    message->outAsync->invokeSent();
                }
                if(message->receivedReply)
                {
                    OutgoingAsyncPtr outAsync = OutgoingAsyncPtr::dynamicCast(message->outAsync);
                    if(outAsync->completed())
                    {
                        outAsync->invokeCompleted();
                    }
                }
                _sendStreams.pop_front();
            }
#endif
        }

        for(deque<OutgoingMessage>::iterator o = _sendStreams.begin(); o != _sendStreams.end(); ++o)
        {
            o->completed(*_exception.get());
            if(o->requestId) // Make sure finished isn't called twice.
            {
                if(o->out)
                {
                    _requests.erase(o->requestId);
                }
                else
                {
                    _asyncRequests.erase(o->requestId);
                }
            }
        }
        _sendStreams.clear(); // Must be cleared before _requests because of Outgoing* references in OutgoingMessage
    }

    for(map<Int, OutgoingBase*>::const_iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
        p->second->completed(*_exception.get());
    }
    _requests.clear();

    for(map<Int, OutgoingAsyncBasePtr>::const_iterator q = _asyncRequests.begin(); q != _asyncRequests.end(); ++q)
    {
        if(q->second->completed(*_exception.get()))
        {
            q->second->invokeCompleted();
        }
    }
    _asyncRequests.clear();

    //
    // Don't wait to be reaped to reclaim memory allocated by read/write streams.
    //
    _writeStream.clear();
    _writeStream.b.clear();
    _readStream.clear();
    _readStream.b.clear();

    if(_callback)
    {
        closeCallback(_callback);
        _callback = 0;
    }

    //
    // This must be done last as this will cause waitUntilFinished() to return (and communicator
    // objects such as the timer might be destroyed too).
    //
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        setState(StateFinished);

        if(_dispatchCount == 0)
        {
            reap();
        }
    }
}

string
Ice::ConnectionI::toString() const
{
    return _desc; // No mutex lock, _desc is immutable.
}

NativeInfoPtr
Ice::ConnectionI::getNativeInfo()
{
    return _transceiver->getNativeInfo();
}

void
Ice::ConnectionI::timedOut()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state <= StateNotValidated)
    {
        setState(StateClosed, ConnectTimeoutException(__FILE__, __LINE__));
    }
    else if(_state < StateClosing)
    {
        setState(StateClosed, TimeoutException(__FILE__, __LINE__));
    }
    else if(_state < StateClosed)
    {
        setState(StateClosed, CloseTimeoutException(__FILE__, __LINE__));
    }
}

string
Ice::ConnectionI::type() const
{
    return _type; // No mutex lock, _type is immutable.
}

Ice::Int
Ice::ConnectionI::timeout() const
{
    return _endpoint->timeout(); // No mutex lock, _endpoint is immutable.
}

ConnectionInfoPtr
Ice::ConnectionI::getInfo() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state >= StateClosed)
    {
        _exception->ice_throw();
    }
    return initConnectionInfo();
}

void
Ice::ConnectionI::setBufferSize(Ice::Int rcvSize, Ice::Int sndSize)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state >= StateClosed)
    {
        _exception->ice_throw();
    }
    _transceiver->setBufferSize(rcvSize, sndSize);
    _info = 0; // Invalidate the cached connection info
}

void
Ice::ConnectionI::exception(const LocalException& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateClosed, ex);
}

Ice::ConnectionI::ConnectionI(const CommunicatorPtr& communicator,
                              const InstancePtr& instance,
                              const ACMMonitorPtr& monitor,
                              const TransceiverPtr& transceiver,
                              const ConnectorPtr& connector,
                              const EndpointIPtr& endpoint,
                              const ObjectAdapterIPtr& adapter) :
    _communicator(communicator),
    _instance(instance),
    _monitor(monitor),
    _transceiver(transceiver),
    _desc(transceiver->toString()),
    _type(transceiver->protocol()),
    _connector(connector),
    _endpoint(endpoint),
    _adapter(adapter),
    _dispatcher(_instance->initializationData().dispatcher), // Cached for better performance.
    _logger(_instance->initializationData().logger), // Cached for better performance.
    _traceLevels(_instance->traceLevels()), // Cached for better performance.
    _timer(_instance->timer()), // Cached for better performance.
    _writeTimeout(new TimeoutCallback(this)),
    _writeTimeoutScheduled(false),
    _readTimeout(new TimeoutCallback(this)),
    _readTimeoutScheduled(false),
    _warn(_instance->initializationData().properties->getPropertyAsInt("Ice.Warn.Connections") > 0),
    _warnUdp(_instance->initializationData().properties->getPropertyAsInt("Ice.Warn.Datagrams") > 0),
    _compressionLevel(1),
    _nextRequestId(1),
    _requestsHint(_requests.end()),
    _asyncRequestsHint(_asyncRequests.end()),
    _messageSizeMax(adapter ? adapter->messageSizeMax() : _instance->messageSizeMax()),
    _batchRequestQueue(new BatchRequestQueue(instance, endpoint->datagram())),
    _readStream(_instance.get(), Ice::currentProtocolEncoding),
    _readHeader(false),
    _writeStream(_instance.get(), Ice::currentProtocolEncoding),
    _dispatchCount(0),
    _state(StateNotInitialized),
    _shutdownInitiated(false),
    _initialized(false),
    _validated(false)
{
    const Ice::PropertiesPtr& properties = _instance->initializationData().properties;

    int& compressionLevel = const_cast<int&>(_compressionLevel);
    compressionLevel = properties->getPropertyAsIntWithDefault("Ice.Compression.Level", 1);
    if(compressionLevel < 1)
    {
        compressionLevel = 1;
    }
    else if(compressionLevel > 9)
    {
        compressionLevel = 9;
    }

    if(adapter)
    {
        _servantManager = adapter->getServantManager();
    }

    if(_monitor && _monitor->getACM().timeout > 0)
    {
        _acmLastActivity = IceUtil::Time::now(IceUtil::Time::Monotonic);
    }

    __setNoDelete(true);
    try
    {
        if(adapter)
        {
            const_cast<ThreadPoolPtr&>(_threadPool) = adapter->getThreadPool();
        }
        else
        {
            const_cast<ThreadPoolPtr&>(_threadPool) = _instance->clientThreadPool();
        }
        _threadPool->initialize(this);
    }
    catch(const IceUtil::Exception&)
    {
        __setNoDelete(false);
        throw;
    }
    __setNoDelete(false);
}

Ice::ConnectionI::~ConnectionI()
{
    assert(!_startCallback);
    assert(!_callback);
    assert(_state == StateFinished);
    assert(_dispatchCount == 0);
    assert(_sendStreams.empty());
    assert(_requests.empty());
    assert(_asyncRequests.empty());
}

void
Ice::ConnectionI::setState(State state, const LocalException& ex)
{
    //
    // If setState() is called with an exception, then only closed and
    // closing states are permissible.
    //
    assert(state >= StateClosing);

    if(_state == state) // Don't switch twice.
    {
        return;
    }

    if(!_exception.get())
    {
        //
        // If we are in closed state, an exception must be set.
        //
        assert(_state != StateClosed);

        _exception.reset(ex.ice_clone());

        //
        // We don't warn if we are not validated.
        //
        if(_warn && _validated)
        {
            //
            // Don't warn about certain expected exceptions.
            //
            if(!(dynamic_cast<const CloseConnectionException*>(_exception.get()) ||
                 dynamic_cast<const ForcedCloseConnectionException*>(_exception.get()) ||
                 dynamic_cast<const ConnectionTimeoutException*>(_exception.get()) ||
                 dynamic_cast<const CommunicatorDestroyedException*>(_exception.get()) ||
                 dynamic_cast<const ObjectAdapterDeactivatedException*>(_exception.get()) ||
                 (dynamic_cast<const ConnectionLostException*>(_exception.get()) && _state >= StateClosing)))
            {
                Warning out(_logger);
                out << "connection exception:\n" << *_exception.get() << '\n' << _desc;
            }
        }
    }

    //
    // We must set the new state before we notify requests of any
    // exceptions. Otherwise new requests may retry on a connection
    // that is not yet marked as closed or closing.
    //
    setState(state);
}

void
Ice::ConnectionI::setState(State state)
{
    //
    // We don't want to send close connection messages if the endpoint
    // only supports oneway transmission from client to server.
    //
    if(_endpoint->datagram() && state == StateClosing)
    {
        state = StateClosed;
    }

    //
    // Skip graceful shutdown if we are destroyed before validation.
    //
    if(_state <= StateNotValidated && state == StateClosing)
    {
        state = StateClosed;
    }

    if(_state == state) // Don't switch twice.
    {
        return;
    }

    try
    {
        switch(state)
        {
        case StateNotInitialized:
        {
            assert(false);
            break;
        }

        case StateNotValidated:
        {
            if(_state != StateNotInitialized)
            {
                assert(_state == StateClosed);
                return;
            }
            break;
        }

        case StateActive:
        {
            //
            // Can only switch from holding or not validated to
            // active.
            //
            if(_state != StateHolding && _state != StateNotValidated)
            {
                return;
            }
            _threadPool->_register(this, SocketOperationRead);
            break;
        }

        case StateHolding:
        {
            //
            // Can only switch from active or not validated to
            // holding.
            //
            if(_state != StateActive && _state != StateNotValidated)
            {
                return;
            }
            if(_state == StateActive)
            {
                _threadPool->unregister(this, SocketOperationRead);
            }
            break;
        }

        case StateClosing:
        case StateClosingPending:
        {
            //
            // Can't change back from closing pending.
            //
            if(_state >= StateClosingPending)
            {
                return;
            }
            break;
        }

        case StateClosed:
        {
            if(_state == StateFinished)
            {
                return;
            }

            _batchRequestQueue->destroy(*_exception.get());

            //
            // Don't need to close now for connections so only close the transceiver
            // if the selector request it.
            //
            if(_threadPool->finish(this, false))
            {
                _transceiver->close();
            }
            break;
        }

        case StateFinished:
        {
            assert(_state == StateClosed);
            _communicator = 0;
            break;
        }
        }
    }
    catch(const Ice::LocalException& ex)
    {
        Error out(_logger);
        out << "unexpected connection exception:\n" << ex << '\n' << _desc;
    }

    //
    // We only register with the connection monitor if our new state
    // is StateActive. Otherwise we unregister with the connection
    // monitor, but only if we were registered before, i.e., if our
    // old state was StateActive.
    //
    if(_monitor)
    {
        if(state == StateActive)
        {
            if(_acmLastActivity != IceUtil::Time())
            {
                _acmLastActivity = IceUtil::Time::now(IceUtil::Time::Monotonic);
            }
            _monitor->add(this);
        }
        else if(_state == StateActive)
        {
            _monitor->remove(this);
        }
    }

    if(_instance->initializationData().observer)
    {
        ConnectionState oldState = toConnectionState(_state);
        ConnectionState newState = toConnectionState(state);
        if(oldState != newState)
        {
            _observer.attach(_instance->initializationData().observer->getConnectionObserver(initConnectionInfo(),
                                                                                             _endpoint,
                                                                                             newState,
                                                                                             _observer.get()));
        }
        if(_observer && state == StateClosed && _exception.get())
        {
            if(!(dynamic_cast<const CloseConnectionException*>(_exception.get()) ||
                 dynamic_cast<const ForcedCloseConnectionException*>(_exception.get()) ||
                 dynamic_cast<const ConnectionTimeoutException*>(_exception.get()) ||
                 dynamic_cast<const CommunicatorDestroyedException*>(_exception.get()) ||
                 dynamic_cast<const ObjectAdapterDeactivatedException*>(_exception.get()) ||
                 (dynamic_cast<const ConnectionLostException*>(_exception.get()) && _state >= StateClosing)))
            {
                _observer->failed(_exception->ice_name());
            }
        }
    }
    _state = state;

    notifyAll();

    if(_state == StateClosing && _dispatchCount == 0)
    {
        try
        {
            initiateShutdown();
        }
        catch(const LocalException& ex)
        {
            setState(StateClosed, ex);
        }
    }
}

void
Ice::ConnectionI::initiateShutdown()
{
    assert(_state == StateClosing);
    assert(_dispatchCount == 0);

    if(_shutdownInitiated)
    {
        return;
    }
    _shutdownInitiated = true;

    if(!_endpoint->datagram())
    {
        //
        // Before we shut down, we send a close connection message.
        //
        BasicStream os(_instance.get(), Ice::currentProtocolEncoding);
        os.write(magic[0]);
        os.write(magic[1]);
        os.write(magic[2]);
        os.write(magic[3]);
        os.write(currentProtocol);
        os.write(currentProtocolEncoding);
        os.write(closeConnectionMsg);
        os.write(static_cast<Byte>(1)); // compression status: compression supported but not used.
        os.write(headerSize); // Message size.

        OutgoingMessage message(&os, false);
        if(sendMessage(message) & AsyncStatusSent)
        {
            setState(StateClosingPending);

            //
            // Notify the the transceiver of the graceful connection closure.
            //
            SocketOperation op = _transceiver->closing(true, *_exception.get());
            if(op)
            {
                scheduleTimeout(op);
                _threadPool->_register(this, op);
            }
        }
    }
}

void
Ice::ConnectionI::heartbeat()
{
    assert(_state == StateActive);

    if(!_endpoint->datagram())
    {
        BasicStream os(_instance.get(), Ice::currentProtocolEncoding);
        os.write(magic[0]);
        os.write(magic[1]);
        os.write(magic[2]);
        os.write(magic[3]);
        os.write(currentProtocol);
        os.write(currentProtocolEncoding);
        os.write(validateConnectionMsg);
        os.write(static_cast<Byte>(0)); // Compression status (always zero for validate connection).
        os.write(headerSize); // Message size.
        os.i = os.b.begin();
        try
        {
            OutgoingMessage message(&os, false);
            sendMessage(message);
        }
        catch(const LocalException& ex)
        {
            setState(StateClosed, ex);
            assert(_exception.get());
        }
    }
}

bool
Ice::ConnectionI::initialize(SocketOperation operation)
{
    SocketOperation s = _transceiver->initialize(_readStream, _writeStream, _hasMoreData);
    if(s != SocketOperationNone)
    {
        scheduleTimeout(s);
        _threadPool->update(this, operation, s);
        return false;
    }

    //
    // Update the connection description once the transceiver is initialized.
    //
    const_cast<string&>(_desc) = _transceiver->toString();
    _initialized = true;
    setState(StateNotValidated);
    return true;
}

bool
Ice::ConnectionI::validate(SocketOperation operation)
{
    if(!_endpoint->datagram()) // Datagram connections are always implicitly validated.
    {
        if(_adapter) // The server side has the active role for connection validation.
        {
            if(_writeStream.b.empty())
            {
                _writeStream.write(magic[0]);
                _writeStream.write(magic[1]);
                _writeStream.write(magic[2]);
                _writeStream.write(magic[3]);
                _writeStream.write(currentProtocol);
                _writeStream.write(currentProtocolEncoding);
                _writeStream.write(validateConnectionMsg);
                _writeStream.write(static_cast<Byte>(0)); // Compression status (always zero for validate connection).
                _writeStream.write(headerSize); // Message size.
                _writeStream.i = _writeStream.b.begin();
                traceSend(_writeStream, _logger, _traceLevels);
            }

            if(_observer)
            {
                _observer.startWrite(_writeStream);
            }

            if(_writeStream.i != _writeStream.b.end())
            {
                SocketOperation op = write(_writeStream);
                if(op)
                {
                    scheduleTimeout(op);
                    _threadPool->update(this, operation, op);
                    return false;
                }
            }

            if(_observer)
            {
                _observer.finishWrite(_writeStream);
            }
        }
        else // The client side has the passive role for connection validation.
        {
            if(_readStream.b.empty())
            {
                _readStream.b.resize(headerSize);
                _readStream.i = _readStream.b.begin();
            }

            if(_observer)
            {
                _observer.startRead(_readStream);
            }

            if(_readStream.i != _readStream.b.end())
            {
                SocketOperation op = read(_readStream);
                if(op)
                {
                    scheduleTimeout(op);
                    _threadPool->update(this, operation, op);
                    return false;
                }
            }

            if(_observer)
            {
                _observer.finishRead(_readStream);
            }

            assert(_readStream.i == _readStream.b.end());
            _readStream.i = _readStream.b.begin();
            Byte m[4];
            _readStream.read(m[0]);
            _readStream.read(m[1]);
            _readStream.read(m[2]);
            _readStream.read(m[3]);
            if(m[0] != magic[0] || m[1] != magic[1] || m[2] != magic[2] || m[3] != magic[3])
            {
                BadMagicException ex(__FILE__, __LINE__);
                ex.badMagic = Ice::ByteSeq(&m[0], &m[0] + sizeof(magic));
                throw ex;
            }
            ProtocolVersion pv;
            _readStream.read(pv);
            checkSupportedProtocol(pv);
            EncodingVersion ev;
            _readStream.read(ev);
            checkSupportedProtocolEncoding(ev);
            Byte messageType;
            _readStream.read(messageType);
            if(messageType != validateConnectionMsg)
            {
                throw ConnectionNotValidatedException(__FILE__, __LINE__);
            }
            Byte compress;
            _readStream.read(compress); // Ignore compression status for validate connection.
            Int size;
            _readStream.read(size);
            if(size != headerSize)
            {
                throw IllegalMessageSizeException(__FILE__, __LINE__);
            }
            traceRecv(_readStream, _logger, _traceLevels);

            _validated = true;
        }
    }

    _writeStream.resize(0);
    _writeStream.i = _writeStream.b.begin();

    _readStream.resize(headerSize);
    _readStream.i = _readStream.b.begin();
    _readHeader = true;

    if(_instance->traceLevels()->network >= 1)
    {
        Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
        if(_endpoint->datagram())
        {
            out << "starting to " << (_connector ? "send" : "receive") << " " << _endpoint->protocol() << " messages\n";
            out << _transceiver->toDetailedString();
        }
        else
        {
            out << (_connector ? "established" : "accepted") << " " << _endpoint->protocol() << " connection\n";
            out << toString();
        }
    }

    return true;
}

SocketOperation
Ice::ConnectionI::sendNextMessage(vector<OutgoingMessage>& callbacks)
{
    if(_sendStreams.empty())
    {
        return SocketOperationNone;
    }
    else if(_state == StateClosingPending && _writeStream.i == _writeStream.b.begin())
    {
        // Message wasn't sent, empty the _writeStream, we're not going to send more data.
        OutgoingMessage* message = &_sendStreams.front();
        _writeStream.swap(*message->stream);
        return SocketOperationNone;
    }

    assert(!_writeStream.b.empty() && _writeStream.i == _writeStream.b.end());
    try
    {
        while(true)
        {
            //
            // Notify the message that it was sent.
            //
            OutgoingMessage* message = &_sendStreams.front();
            if(message->stream)
            {
                _writeStream.swap(*message->stream);
                if(message->sent())
                {
                    callbacks.push_back(*message);
                }
            }
            _sendStreams.pop_front();

            //
            // If there's nothing left to send, we're done.
            //
            if(_sendStreams.empty())
            {
                break;
            }

            //
            // If we are in the closed state or if the close is
            // pending, don't continue sending.
            //
            // This can occur if parseMessage (called before
            // sendNextMessage by message()) closes the connection.
            //
            if(_state >= StateClosingPending)
            {
                return SocketOperationNone;
            }

            //
            // Otherwise, prepare the next message stream for writing.
            //
            message = &_sendStreams.front();
            assert(!message->stream->i);
#ifdef ICE_HAS_BZIP2
            if(message->compress && message->stream->b.size() >= 100) // Only compress messages > 100 bytes.
            {
                //
                // Message compressed. Request compressed response, if any.
                //
                message->stream->b[9] = 2;

                //
                // Do compression.
                //
                BasicStream stream(_instance.get(), Ice::currentProtocolEncoding);
                doCompress(*message->stream, stream);

                if(message->outAsync)
                {
                    trace("sending asynchronous request", *message->stream, _logger, _traceLevels);
                }
                else
                {
                    traceSend(*message->stream, _logger, _traceLevels);
                }

                message->adopt(&stream); // Adopt the compressed stream.
                message->stream->i = message->stream->b.begin();
            }
            else
            {
#endif
                if(message->compress)
                {
                    //
                    // Message not compressed. Request compressed response, if any.
                    //
                    message->stream->b[9] = 1;
                }

                //
                // No compression, just fill in the message size.
                //
                Int sz = static_cast<Int>(message->stream->b.size());
                const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIG_ENDIAN
                reverse_copy(p, p + sizeof(Int), message->stream->b.begin() + 10);
#else
                copy(p, p + sizeof(Int), message->stream->b.begin() + 10);
#endif
                message->stream->i = message->stream->b.begin();
                if(message->outAsync)
                {
                    trace("sending asynchronous request", *message->stream, _logger, _traceLevels);
                }
                else
                {
                    traceSend(*message->stream, _logger, _traceLevels);
                }
#ifdef ICE_HAS_BZIP2
            }
#endif
            _writeStream.swap(*message->stream);

            //
            // Send the message.
            //
            if(_observer)
            {
                _observer.startWrite(_writeStream);
            }
            assert(_writeStream.i);
            if(_writeStream.i != _writeStream.b.end())
            {
                SocketOperation op = write(_writeStream);
                if(op)
                {
                    return op;
                }
            }
            if(_observer)
            {
                _observer.finishWrite(_writeStream);
            }
        }

        //
        // If all the messages were sent and we are in the closing state, we schedule
        // the close timeout to wait for the peer to close the connection.
        //
        if(_state == StateClosing && _shutdownInitiated)
        {
            setState(StateClosingPending);
            SocketOperation op = _transceiver->closing(true, *_exception.get());
            if(op)
            {
                return op;
            }
        }
    }
    catch(const Ice::LocalException& ex)
    {
        setState(StateClosed, ex);
    }
    return SocketOperationNone;
}

AsyncStatus
Ice::ConnectionI::sendMessage(OutgoingMessage& message)
{
    assert(_state < StateClosed);

    message.stream->i = 0; // Reset the message stream iterator before starting sending the message.

    if(!_sendStreams.empty())
    {
        _sendStreams.push_back(message);
        _sendStreams.back().adopt(0);
        return AsyncStatusQueued;
    }

    //
    // Attempt to send the message without blocking. If the send blocks, we register
    // the connection with the selector thread.
    //

    message.stream->i = message.stream->b.begin();
    SocketOperation op;
#ifdef ICE_HAS_BZIP2
    if(message.compress && message.stream->b.size() >= 100) // Only compress messages larger than 100 bytes.
    {
        //
        // Message compressed. Request compressed response, if any.
        //
        message.stream->b[9] = 2;

        //
        // Do compression.
        //
        BasicStream stream(_instance.get(), Ice::currentProtocolEncoding);
        doCompress(*message.stream, stream);
        stream.i = stream.b.begin();

        if(message.outAsync)
        {
            trace("sending asynchronous request", *message.stream, _logger, _traceLevels);
        }
        else
        {
            traceSend(*message.stream, _logger, _traceLevels);
        }

        //
        // Send the message without blocking.
        //
        if(_observer)
        {
            _observer.startWrite(stream);
        }
        op = write(stream);
        if(!op)
        {
            if(_observer)
            {
                _observer.finishWrite(stream);
            }

            AsyncStatus status = AsyncStatusSent;
            if(message.sent())
            {
                status = static_cast<AsyncStatus>(status | AsyncStatusInvokeSentCallback);
            }
            if(_acmLastActivity != IceUtil::Time())
            {
                _acmLastActivity = IceUtil::Time::now(IceUtil::Time::Monotonic);
            }
            return status;
        }

        _sendStreams.push_back(message);
        _sendStreams.back().adopt(&stream);
    }
    else
    {
#endif
        if(message.compress)
        {
            //
            // Message not compressed. Request compressed response, if any.
            //
            message.stream->b[9] = 1;
        }

        //
        // No compression, just fill in the message size.
        //
        Int sz = static_cast<Int>(message.stream->b.size());
        const Byte* p = reinterpret_cast<const Byte*>(&sz);
#ifdef ICE_BIG_ENDIAN
        reverse_copy(p, p + sizeof(Int), message.stream->b.begin() + 10);
#else
        copy(p, p + sizeof(Int), message.stream->b.begin() + 10);
#endif
        message.stream->i = message.stream->b.begin();

        if(message.outAsync)
        {
            trace("sending asynchronous request", *message.stream, _logger, _traceLevels);
        }
        else
        {
            traceSend(*message.stream, _logger, _traceLevels);
        }

        //
        // Send the message without blocking.
        //
        if(_observer)
        {
            _observer.startWrite(*message.stream);
        }
        op = write(*message.stream);
        if(!op)
        {
            if(_observer)
            {
                _observer.finishWrite(*message.stream);
            }
            AsyncStatus status = AsyncStatusSent;
            if(message.sent())
            {
                status = static_cast<AsyncStatus>(status | AsyncStatusInvokeSentCallback);
            }
            if(_acmLastActivity != IceUtil::Time())
            {
                _acmLastActivity = IceUtil::Time::now(IceUtil::Time::Monotonic);
            }
            return status;
        }

        _sendStreams.push_back(message);
        _sendStreams.back().adopt(0); // Adopt the stream.
#ifdef ICE_HAS_BZIP2
    }
#endif

    _writeStream.swap(*_sendStreams.back().stream);
    scheduleTimeout(op);
    _threadPool->_register(this, op);
    return AsyncStatusQueued;
}

#ifdef ICE_HAS_BZIP2
static string
getBZ2Error(int bzError)
{
    if(bzError == BZ_RUN_OK)
    {
        return ": BZ_RUN_OK";
    }
    else if(bzError == BZ_FLUSH_OK)
    {
        return ": BZ_FLUSH_OK";
    }
    else if(bzError == BZ_FINISH_OK)
    {
        return ": BZ_FINISH_OK";
    }
    else if(bzError == BZ_STREAM_END)
    {
        return ": BZ_STREAM_END";
    }
    else if(bzError == BZ_CONFIG_ERROR)
    {
        return ": BZ_CONFIG_ERROR";
    }
    else if(bzError == BZ_SEQUENCE_ERROR)
    {
        return ": BZ_SEQUENCE_ERROR";
    }
    else if(bzError == BZ_PARAM_ERROR)
    {
        return ": BZ_PARAM_ERROR";
    }
    else if(bzError == BZ_MEM_ERROR)
    {
        return ": BZ_MEM_ERROR";
    }
    else if(bzError == BZ_DATA_ERROR)
    {
        return ": BZ_DATA_ERROR";
    }
    else if(bzError == BZ_DATA_ERROR_MAGIC)
    {
        return ": BZ_DATA_ERROR_MAGIC";
    }
    else if(bzError == BZ_IO_ERROR)
    {
        return ": BZ_IO_ERROR";
    }
    else if(bzError == BZ_UNEXPECTED_EOF)
    {
        return ": BZ_UNEXPECTED_EOF";
    }
    else if(bzError == BZ_OUTBUFF_FULL)
    {
        return ": BZ_OUTBUFF_FULL";
    }
    else
    {
        return "";
    }
}

void
Ice::ConnectionI::doCompress(BasicStream& uncompressed, BasicStream& compressed)
{
    const Byte* p;

    //
    // Compress the message body, but not the header.
    //
    unsigned int uncompressedLen = static_cast<unsigned int>(uncompressed.b.size() - headerSize);
    unsigned int compressedLen = static_cast<unsigned int>(uncompressedLen * 1.01 + 600);
    compressed.b.resize(headerSize + sizeof(Int) + compressedLen);
    int bzError = BZ2_bzBuffToBuffCompress(reinterpret_cast<char*>(&compressed.b[0]) + headerSize + sizeof(Int),
                                           &compressedLen,
                                           reinterpret_cast<char*>(&uncompressed.b[0]) + headerSize,
                                           uncompressedLen,
                                           _compressionLevel, 0, 0);
    if(bzError != BZ_OK)
    {
        CompressionException ex(__FILE__, __LINE__);
        ex.reason = "BZ2_bzBuffToBuffCompress failed" + getBZ2Error(bzError);
        throw ex;
    }
    compressed.b.resize(headerSize + sizeof(Int) + compressedLen);

    //
    // Write the size of the compressed stream into the header of the
    // uncompressed stream. Since the header will be copied, this size
    // will also be in the header of the compressed stream.
    //
    Int compressedSize = static_cast<Int>(compressed.b.size());
    p = reinterpret_cast<const Byte*>(&compressedSize);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Int), uncompressed.b.begin() + 10);
#else
    copy(p, p + sizeof(Int), uncompressed.b.begin() + 10);
#endif

    //
    // Add the size of the uncompressed stream before the message body
    // of the compressed stream.
    //
    Int uncompressedSize = static_cast<Int>(uncompressed.b.size());
    p = reinterpret_cast<const Byte*>(&uncompressedSize);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Int), compressed.b.begin() + headerSize);
#else
    copy(p, p + sizeof(Int), compressed.b.begin() + headerSize);
#endif

    //
    // Copy the header from the uncompressed stream to the compressed one.
    //
    copy(uncompressed.b.begin(), uncompressed.b.begin() + headerSize, compressed.b.begin());
}

void
Ice::ConnectionI::doUncompress(BasicStream& compressed, BasicStream& uncompressed)
{
    Int uncompressedSize;
    compressed.i = compressed.b.begin() + headerSize;
    compressed.read(uncompressedSize);
    if(uncompressedSize <= headerSize)
    {
        throw IllegalMessageSizeException(__FILE__, __LINE__);
    }

    if(uncompressedSize > static_cast<Int>(_messageSizeMax))
    {
        Ex::throwMemoryLimitException(__FILE__, __LINE__, uncompressedSize, _messageSizeMax);
    }
    uncompressed.resize(uncompressedSize);

    unsigned int uncompressedLen = uncompressedSize - headerSize;
    unsigned int compressedLen = static_cast<unsigned int>(compressed.b.size() - headerSize - sizeof(Int));
    int bzError = BZ2_bzBuffToBuffDecompress(reinterpret_cast<char*>(&uncompressed.b[0]) + headerSize,
                                             &uncompressedLen,
                                             reinterpret_cast<char*>(&compressed.b[0]) + headerSize + sizeof(Int),
                                             compressedLen,
                                             0, 0);
    if(bzError != BZ_OK)
    {
        CompressionException ex(__FILE__, __LINE__);
        ex.reason = "BZ2_bzBuffToBuffCompress failed" + getBZ2Error(bzError);
        throw ex;
    }

    copy(compressed.b.begin(), compressed.b.begin() + headerSize, uncompressed.b.begin());
}
#endif

SocketOperation
Ice::ConnectionI::parseMessage(BasicStream& stream, Int& invokeNum, Int& requestId, Byte& compress,
                               ServantManagerPtr& servantManager, ObjectAdapterPtr& adapter,
                               OutgoingAsyncBasePtr& outAsync, ConnectionCallbackPtr& heartbeatCallback,
                               int& dispatchCount)
{
    assert(_state > StateNotValidated && _state < StateClosed);

    _readStream.swap(stream);
    _readStream.resize(headerSize);
    _readStream.i = _readStream.b.begin();
    _readHeader = true;

    assert(stream.i == stream.b.end());

    //
    // Connection is validated on first message. This is only used by
    // setState() to check wether or not we can print a connection
    // warning (a client might close the connection forcefully if the
    // connection isn't validated).
    //
    _validated = true;

    try
    {
        //
        // We don't need to check magic and version here. This has
        // already been done by the ThreadPool, which provides us
        // with the stream.
        //
        assert(stream.i == stream.b.end());
        stream.i = stream.b.begin() + 8;
        Byte messageType;
        stream.read(messageType);
        stream.read(compress);

        if(compress == 2)
        {
#ifdef ICE_HAS_BZIP2
            BasicStream ustream(_instance.get(), Ice::currentProtocolEncoding);
            doUncompress(stream, ustream);
            stream.b.swap(ustream.b);
#else
            FeatureNotSupportedException ex(__FILE__, __LINE__);
            ex.unsupportedFeature = "Cannot uncompress compressed message";
            throw ex;
#endif
        }
        stream.i = stream.b.begin() + headerSize;

        switch(messageType)
        {
            case closeConnectionMsg:
            {
                traceRecv(stream, _logger, _traceLevels);
                if(_endpoint->datagram())
                {
                    if(_warn)
                    {
                        Warning out(_logger);
                        out << "ignoring close connection message for datagram connection:\n" << _desc;
                    }
                }
                else
                {
                    setState(StateClosingPending, CloseConnectionException(__FILE__, __LINE__));

                    //
                    // Notify the the transceiver of the graceful connection closure.
                    //
                    SocketOperation op = _transceiver->closing(false, *_exception.get());
                    if(op)
                    {
                        return op;
                    }
                    setState(StateClosed);
                }
                break;
            }

            case requestMsg:
            {
                if(_state >= StateClosing)
                {
                    trace("received request during closing\n(ignored by server, client will retry)", stream, _logger,
                          _traceLevels);
                }
                else
                {
                    traceRecv(stream, _logger, _traceLevels);
                    stream.read(requestId);
                    invokeNum = 1;
                    servantManager = _servantManager;
                    adapter = _adapter;
                    ++dispatchCount;
                }
                break;
            }

            case requestBatchMsg:
            {
                if(_state >= StateClosing)
                {
                    trace("received batch request during closing\n(ignored by server, client will retry)", stream,
                          _logger, _traceLevels);
                }
                else
                {
                    traceRecv(stream, _logger, _traceLevels);
                    stream.read(invokeNum);
                    if(invokeNum < 0)
                    {
                        invokeNum = 0;
                        throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
                    }
                    servantManager = _servantManager;
                    adapter = _adapter;
                    dispatchCount += invokeNum;
                }
                break;
            }

            case replyMsg:
            {
                traceRecv(stream, _logger, _traceLevels);

                stream.read(requestId);

                map<Int, OutgoingBase*>::iterator p = _requests.end();
                map<Int, OutgoingAsyncBasePtr>::iterator q = _asyncRequests.end();

                if(_requestsHint != _requests.end())
                {
                    if(_requestsHint->first == requestId)
                    {
                        p = _requestsHint;
                    }
                }

                if(p == _requests.end())
                {
                    if(_asyncRequestsHint != _asyncRequests.end())
                    {
                        if(_asyncRequestsHint->first == requestId)
                        {
                            q = _asyncRequestsHint;
                        }
                    }
                }

                if(p == _requests.end() && q == _asyncRequests.end())
                {
                    p = _requests.find(requestId);
                }

                if(p == _requests.end() && q == _asyncRequests.end())
                {
                    q = _asyncRequests.find(requestId);
                }

                if(p != _requests.end())
                {
                    p->second->completed(stream);

                    if(p == _requestsHint)
                    {
                        _requests.erase(p++);
                        _requestsHint = p;
                    }
                    else
                    {
                        _requests.erase(p);
                    }
                    notifyAll(); // Notify threads blocked in close(false)
                }
                else if(q != _asyncRequests.end())
                {
                    outAsync = q->second;

                    if(q == _asyncRequestsHint)
                    {
                        _asyncRequests.erase(q++);
                        _asyncRequestsHint = q;
                    }
                    else
                    {
                        _asyncRequests.erase(q);
                    }

                    stream.swap(*outAsync->getIs());

#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
                    //
                    // If we just received the reply of a request which isn't acknowledge as
                    // sent yet, we queue the reply instead of processing it right away. It
                    // will be processed once the write callback is invoked for the message.
                    //
                    OutgoingMessage* message = _sendStreams.empty() ? 0 : &_sendStreams.front();
                    if(message && message->outAsync.get() == outAsync.get())
                    {
                        message->receivedReply = true;
                        outAsync = 0;
                    }
                    else if(outAsync->completed())
                    {
                        ++dispatchCount;
                    }
                    else
                    {
                        outAsync = 0;
                    }
#else
                    if(outAsync->completed())
                    {
                        ++dispatchCount;
                    }
                    else
                    {
                        outAsync = 0;
                    }
#endif
                    notifyAll(); // Notify threads blocked in close(false)
                }

                break;
            }

            case validateConnectionMsg:
            {
                traceRecv(stream, _logger, _traceLevels);
                if(_callback)
                {
                    heartbeatCallback = _callback;
                    ++dispatchCount;
                }
                break;
            }

            default:
            {
                trace("received unknown message\n(invalid, closing connection)", stream, _logger, _traceLevels);
                throw UnknownMessageException(__FILE__, __LINE__);
            }
        }
    }
    catch(const LocalException& ex)
    {
        if(_endpoint->datagram())
        {
            if(_warn)
            {
                Warning out(_logger);
                out << "datagram connection exception:\n" << ex << '\n' << _desc;
            }
        }
        else
        {
            setState(StateClosed, ex);
        }
    }

    return _state == StateHolding ? SocketOperationNone : SocketOperationRead;
}

void
Ice::ConnectionI::invokeAll(BasicStream& stream, Int invokeNum, Int requestId, Byte compress,
                            const ServantManagerPtr& servantManager, const ObjectAdapterPtr& adapter)
{
    //
    // Note: In contrast to other private or protected methods, this
    // operation must be called *without* the mutex locked.
    //

    try
    {
        while(invokeNum > 0)
        {
            //
            // Prepare the invocation.
            //
            bool response = !_endpoint->datagram() && requestId != 0;
            assert(!response || invokeNum == 1);

            Incoming in(_instance.get(), this, this, adapter, response, compress, requestId);

            //
            // Dispatch the invocation.
            //
            in.invoke(servantManager, &stream);

            --invokeNum;
        }

        stream.clear();
    }
    catch(const LocalException& ex)
    {
        invokeException(requestId, ex, invokeNum, false);  // Fatal invocation exception
    }
}

void
Ice::ConnectionI::scheduleTimeout(SocketOperation status)
{
    int timeout;
    if(_state < StateActive)
    {
        DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
        if(defaultsAndOverrides->overrideConnectTimeout)
        {
            timeout = defaultsAndOverrides->overrideConnectTimeoutValue;
        }
        else
        {
            timeout = _endpoint->timeout();
        }
    }
    else if(_state < StateClosingPending)
    {
        if(_readHeader) // No timeout for reading the header.
        {
            status = static_cast<SocketOperation>(status & ~SocketOperationRead);
        }
        timeout = _endpoint->timeout();
    }
    else
    {
        DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
        if(defaultsAndOverrides->overrideCloseTimeout)
        {
            timeout = defaultsAndOverrides->overrideCloseTimeoutValue;
        }
        else
        {
            timeout = _endpoint->timeout();
        }
    }

    if(timeout < 0)
    {
        return;
    }

    try
    {
        if(status & IceInternal::SocketOperationRead)
        {
            if(_readTimeoutScheduled)
            {
                _timer->cancel(_readTimeout);
            }
            _timer->schedule(_readTimeout, IceUtil::Time::milliSeconds(timeout));
            _readTimeoutScheduled = true;
        }
        if(status & (IceInternal::SocketOperationWrite | IceInternal::SocketOperationConnect))
        {
            if(_writeTimeoutScheduled)
            {
                _timer->cancel(_writeTimeout);
            }
            _timer->schedule(_writeTimeout, IceUtil::Time::milliSeconds(timeout));
            _writeTimeoutScheduled = true;
        }
    }
    catch(const IceUtil::Exception&)
    {
        assert(false);
    }
}

void
Ice::ConnectionI::unscheduleTimeout(SocketOperation status)
{
    if((status & IceInternal::SocketOperationRead) && _readTimeoutScheduled)
    {
        _timer->cancel(_readTimeout);
        _readTimeoutScheduled = false;
    }
    if((status & (IceInternal::SocketOperationWrite | IceInternal::SocketOperationConnect)) &&
       _writeTimeoutScheduled)
    {
        _timer->cancel(_writeTimeout);
        _writeTimeoutScheduled = false;
    }
}

Ice::ConnectionInfoPtr
Ice::ConnectionI::initConnectionInfo() const
{
    if(_state > StateNotInitialized && _info) // Update the connection information until it's initialized
    {
        return _info;
    }

    try
    {
        _info = _transceiver->getInfo();
    }
    catch(const Ice::LocalException&)
    {
        _info = new ConnectionInfo();
    }
    _info->connectionId = _endpoint->connectionId();
    _info->incoming = _connector == 0;
    _info->adapterName = _adapter ? _adapter->getName() : string();
    return _info;
}

ConnectionState
ConnectionI::toConnectionState(State state) const
{
    return connectionStateMap[static_cast<int>(state)];
}

SocketOperation
ConnectionI::read(Buffer& buf)
{
    Buffer::Container::iterator start = buf.i;
    SocketOperation op = _transceiver->read(buf, _hasMoreData);
    if(_instance->traceLevels()->network >= 3 && buf.i != start)
    {
        Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
        out << "received ";
        if(_endpoint->datagram())
        {
            out << buf.b.size();
        }
        else
        {
            out << (buf.i - start) << " of " << (buf.b.end() - start);
        }
        out << " bytes via " << _endpoint->protocol() << "\n" << toString();
    }
    return op;
}

SocketOperation
ConnectionI::write(Buffer& buf)
{
    Buffer::Container::iterator start = buf.i;
    SocketOperation op = _transceiver->write(buf);
    if(_instance->traceLevels()->network >= 3 && buf.i != start)
    {
        Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
        out << "sent " << (buf.i - start);
        if(!_endpoint->datagram())
        {
            out << " of " << (buf.b.end() - start);
        }
        out << " bytes via " << _endpoint->protocol() << "\n" << toString();
    }
    return op;
}

void
ConnectionI::reap()
{
    if(_monitor)
    {
        _monitor->reap(this);
    }
    if(_observer)
    {
        _observer.detach();
    }
}
