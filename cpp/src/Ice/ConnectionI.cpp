// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Transceiver.h>
#include <Ice/ThreadPool.h>
#include <Ice/SelectorThread.h>
#include <Ice/ConnectionMonitor.h>
#include <Ice/ObjectAdapterI.h> // For getThreadPool() and getServantManager().
#include <Ice/EndpointI.h>
#include <Ice/Outgoing.h>
#include <Ice/OutgoingAsync.h>
#include <Ice/Incoming.h>
#include <Ice/LocalException.h>
#include <Ice/ReferenceFactory.h> // For createProxy().
#include <Ice/ProxyFactory.h> // For createProxy().
#include <bzlib.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

Ice::LocalObject* IceInternal::upCast(ConnectionI* p) { return p; }

namespace IceInternal
{

class FlushSentCallbacks : public ThreadPoolWorkItem
{
public:

    FlushSentCallbacks(const Ice::ConnectionIPtr& connection) : _connection(connection)
    {
    }

    void
    execute(const ThreadPoolPtr& threadPool)
    {
        threadPool->promoteFollower();
        _connection->flushSentCallbacks();
    }

private:

    const Ice::ConnectionIPtr _connection;
};

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
    stream = new BasicStream(str->instance());
    stream->swap(*str);
    adopted = true;
}

void
Ice::ConnectionI::OutgoingMessage::sent(ConnectionI* connection, bool notify)
{
    if(out)
    {
        out->sent(notify); // true = notify the waiting thread that the request was sent.
    }
    else if(outAsync)
    {
        outAsync->__sent(connection);
    }

    if(adopted)
    {
        delete stream;
        stream = 0;
    }
}

void
Ice::ConnectionI::OutgoingMessage::finished(const Ice::LocalException& ex)
{
    if(!response)
    {
        //
        // Only notify oneway requests. The connection keeps track of twoway
        // requests in the _requests/_asyncRequests maps and will notify them
        // of the connection exceptions.
        //
        if(out)
        {
            out->finished(ex);
        }
        else if(outAsync)
        {
            outAsync->__finished(ex);
        }
    }

    if(adopted)
    {
        delete stream;
        stream = 0;
    }
}

void
Ice::ConnectionI::start(const StartCallbackPtr& callback)
{
    try
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        if(_state == StateClosed) // The connection might already be closed if the communicator was destroyed.
        {
            assert(_exception.get());
            _exception->ice_throw();
        }

        SocketStatus status = initialize();
        if(status == Finished)
        {
            status = validate();
        }

        if(status != Finished)
        {
            int timeout;
            DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
            if(defaultsAndOverrides->overrideConnectTimeout)
            {
                timeout = defaultsAndOverrides->overrideConnectTimeoutValue;
            }
            else
            {
                timeout = _endpoint->timeout();
            }

            _sendInProgress = true;
            _selectorThread->_register(_transceiver->fd(), this, status, timeout);

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

    if(_acmTimeout > 0)
    {
        _acmAbsoluteTimeout = IceUtil::Time::now(IceUtil::Time::Monotonic) + IceUtil::Time::seconds(_acmTimeout);
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

    if(_transceiver || _dispatchCount != 0)
    {
        return false;
    }

    assert(_state == StateClosed);
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
    // We wait indefinitely until connection closing has been
    // initiated. We also wait indefinitely until all outstanding
    // requests are completed. Otherwise we couldn't guarantee
    // that there are no outstanding calls when deactivate() is
    // called on the servant locators.
    //
    while(_state < StateClosing || _dispatchCount > 0)
    {
        wait();
    }

    //
    // Now we must wait until close() has been called on the
    // transceiver.
    //
    while(_transceiver)
    {
        if(_state != StateClosed && _endpoint->timeout() >= 0)
        {
            IceUtil::Time timeout = IceUtil::Time::milliSeconds(_endpoint->timeout());
            IceUtil::Time waitTime = _stateTime + timeout - IceUtil::Time::now(IceUtil::Time::Monotonic);

            if(waitTime > IceUtil::Time())
            {
                //
                // We must wait a bit longer until we close this
                // connection.
                //
                if(!timedWait(waitTime))
                {
                    setState(StateClosed, CloseTimeoutException(__FILE__, __LINE__));
                }
            }
            else
            {
                //
                // We already waited long enough, so let's close this
                // connection!
                //
                setState(StateClosed, CloseTimeoutException(__FILE__, __LINE__));
            }

            //
            // No return here, we must still wait until close() is
            // called on the _transceiver.
            //
        }
        else
        {
            wait();
        }
    }

    assert(_state == StateClosed);

    //
    // Clear the OA. See bug 1673 for the details of why this is necessary.
    //
    _adapter = 0;
}

void
Ice::ConnectionI::monitor(const IceUtil::Time& now)
{
    IceUtil::Monitor<IceUtil::Mutex>::TryLock sync(*this);
    if(!sync.acquired())
    {
        return;
    }

    if(_state != StateActive)
    {
        return;
    }

    //
    // Active connection management for idle connections.
    //
    if(_acmTimeout <= 0 ||
       !_requests.empty() || !_asyncRequests.empty() ||
       _batchStreamInUse || !_batchStream.b.empty() ||
       _sendInProgress || _dispatchCount > 0)
    {
        return;
    }

    if(now >= _acmAbsoluteTimeout)
    {
        setState(StateClosing, ConnectionTimeoutException(__FILE__, __LINE__));
    }
}

bool
Ice::ConnectionI::sendRequest(Outgoing* out, bool compress, bool response)
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
        throw LocalExceptionWrapper(*_exception.get(), true);
    }

    assert(_state > StateNotValidated);
    assert(_state < StateClosing);

    Int requestId;
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

    //
    // Send the message. If it can't be sent without blocking the message is added
    // to _sendStreams and it will be sent by the selector thread.
    //
    bool sent = false;
    try
    {
        OutgoingMessage message(out, os, compress, response);
        sent = sendMessage(message);
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
        _requestsHint = _requests.insert(_requests.end(), pair<const Int, Outgoing*>(requestId, out));
    }

    return sent;
}

bool
Ice::ConnectionI::sendAsyncRequest(const OutgoingAsyncPtr& out, bool compress, bool response)
{
    BasicStream* os = out->__getOs();

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_exception.get())
    {
        //
        // If the exception is closed before we even have a chance
        // to send our request, we always try to send the request
        // again.
        //
        throw LocalExceptionWrapper(*_exception.get(), true);
    }

    assert(_state > StateNotValidated);
    assert(_state < StateClosing);

    Int requestId;
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

    bool sent = false;
    try
    {
        OutgoingMessage message(out, os, compress, response);
        sent = sendMessage(message);
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
                                                   pair<const Int, OutgoingAsyncPtr>(requestId, out));
    }
    return sent;
}

void
Ice::ConnectionI::prepareBatchRequest(BasicStream* os)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    //
    // Wait if flushing is currently in progress.
    //
    while(_batchStreamInUse && !_exception.get())
    {
        wait();
    }

    if(_exception.get())
    {
        //
        // If there were no batch requests queued when the connection failed, we can safely 
        // retry with a new connection. Otherwise, we must throw to notify the caller that 
        // some previous batch requests were not sent.
        //
        if(_batchStream.b.empty())
        {
            throw LocalExceptionWrapper(*_exception.get(), true);
        }
        else
        {
            _exception->ice_throw();
        }
    }

    assert(_state > StateNotValidated);
    assert(_state < StateClosing);

    if(_batchStream.b.empty())
    {
        try
        {
            _batchStream.writeBlob(requestBatchHdr, sizeof(requestBatchHdr));
        }
        catch(const LocalException& ex)
        {
            setState(StateClosed, ex);
            ex.ice_throw();
        }
    }

    _batchStreamInUse = true;
    _batchMarker = _batchStream.b.size();
    _batchStream.swap(*os);

    //
    // The batch stream now belongs to the caller, until
    // finishBatchRequest() or abortBatchRequest() is called.
    //
}

void
Ice::ConnectionI::finishBatchRequest(BasicStream* os, bool compress)
{
    try
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        //
        // Get the batch stream back.
        //
        _batchStream.swap(*os);

        if(_exception.get())
        {
            _exception->ice_throw();
        }

        bool flush = false;
        if(_batchAutoFlush)
        {
            //
            // Throw memory limit exception if the first message added causes us to
            // go over limit. Otherwise put aside the marshalled message that caused
            // limit to be exceeded and rollback stream to the marker.
            //
            try
            {
                _transceiver->checkSendSize(_batchStream, _instance->messageSizeMax());
            }
            catch(const Ice::Exception&)
            {
                if(_batchRequestNum > 0)
                {
                    flush = true;
                }
                else
                {
                    throw;
                }
            }
        }

        if(flush)
        {
            //
            // Temporarily save the last request.
            //
            vector<Ice::Byte> lastRequest(_batchStream.b.begin() + _batchMarker, _batchStream.b.end());
            _batchStream.b.resize(_batchMarker);

            //
            // Send the batch stream without the last request.
            //
            try
            {
                //
                // Fill in the number of requests in the batch.
                //
                const Byte* p = reinterpret_cast<const Byte*>(&_batchRequestNum);
#ifdef ICE_BIG_ENDIAN
                reverse_copy(p, p + sizeof(Int), _batchStream.b.begin() + headerSize);
#else
                copy(p, p + sizeof(Int), _batchStream.b.begin() + headerSize);
#endif

                OutgoingMessage message(&_batchStream, _batchRequestCompress);
                sendMessage(message);
            }
            catch(const Ice::LocalException& ex)
            {
                setState(StateClosed, ex);
                assert(_exception.get());
                _exception->ice_throw();
            }

            //
            // Reset the batch.
            //
            BasicStream dummy(_instance.get(), _batchAutoFlush);
            _batchStream.swap(dummy);
            _batchRequestNum = 0;
            _batchRequestCompress = false;
            _batchMarker = 0;

            //
            // Check again if the last request doesn't exceed what we can send with the auto flush
            //
            if(sizeof(requestBatchHdr) + lastRequest.size() >  _instance->messageSizeMax())
            {
                Ex::throwMemoryLimitException(__FILE__, __LINE__, sizeof(requestBatchHdr) + lastRequest.size(),
                                              _instance->messageSizeMax());
            }

            //
            // Start a new batch with the last message that caused us to go over the limit.
            //
            _batchStream.writeBlob(requestBatchHdr, sizeof(requestBatchHdr));
            _batchStream.writeBlob(&lastRequest[0], lastRequest.size());
        }

        //
        // Increment the number of requests in the batch.
        //
        ++_batchRequestNum;

        //
        // We compress the whole batch if there is at least one compressed
        // message.
        //
        if(compress)
        {
            _batchRequestCompress = true;
        }

        //
        // Notify about the batch stream not being in use anymore.
        //
        assert(_batchStreamInUse);
        _batchStreamInUse = false;
        notifyAll();
    }
    catch(const Ice::LocalException&)
    {
        abortBatchRequest();
        throw;
    }
}

void
Ice::ConnectionI::abortBatchRequest()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    BasicStream dummy(_instance.get(), _batchAutoFlush);
    _batchStream.swap(dummy);
    _batchRequestNum = 0;
    _batchRequestCompress = false;
    _batchMarker = 0;

    assert(_batchStreamInUse);
    _batchStreamInUse = false;
    notifyAll();
}

void
Ice::ConnectionI::flushBatchRequests()
{
    BatchOutgoing out(this, _instance.get());
    out.invoke();
}

bool
Ice::ConnectionI::flushBatchRequests(BatchOutgoing* out)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    while(_batchStreamInUse && !_exception.get())
    {
        wait();
    }

    if(_exception.get())
    {
        _exception->ice_throw();
    }

    if(_batchRequestNum == 0)
    {
        out->sent(false);
        return true;
    }

    //
    // Fill in the number of requests in the batch.
    //
    const Byte* p = reinterpret_cast<const Byte*>(&_batchRequestNum);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Int), _batchStream.b.begin() + headerSize);
#else
    copy(p, p + sizeof(Int), _batchStream.b.begin() + headerSize);
#endif
    _batchStream.swap(*out->os());

    //
    // Send the batch stream.
    //
    bool sent = false;
    try
    {
        OutgoingMessage message(out, out->os(), _batchRequestCompress, false);
        sent = sendMessage(message);
    }
    catch(const Ice::LocalException& ex)
    {
        setState(StateClosed, ex);
        assert(_exception.get());
        _exception->ice_throw();
    }

    //
    // Reset the batch stream.
    //
    BasicStream dummy(_instance.get(), _batchAutoFlush);
    _batchStream.swap(dummy);
    _batchRequestNum = 0;
    _batchRequestCompress = false;
    _batchMarker = 0;
    return sent;
}

bool
Ice::ConnectionI::flushAsyncBatchRequests(const BatchOutgoingAsyncPtr& outAsync)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    while(_batchStreamInUse && !_exception.get())
    {
        wait();
    }

    if(_exception.get())
    {
        _exception->ice_throw();
    }

    if(_batchRequestNum == 0)
    {
        outAsync->__sent(this);
        return true;
    }

    //
    // Fill in the number of requests in the batch.
    //
    const Byte* p = reinterpret_cast<const Byte*>(&_batchRequestNum);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Int), _batchStream.b.begin() + headerSize);
#else
    copy(p, p + sizeof(Int), _batchStream.b.begin() + headerSize);
#endif
    _batchStream.swap(*outAsync->__getOs());

    //
    // Send the batch stream.
    //
    bool sent = false;
    try
    {
        OutgoingMessage message(outAsync, outAsync->__getOs(), _batchRequestCompress, false);
        sent = sendMessage(message);
    }
    catch(const Ice::LocalException& ex)
    {
        setState(StateClosed, ex);
        assert(_exception.get());
        _exception->ice_throw();
    }

    //
    // Reset the batch stream.
    //
    BasicStream dummy(_instance.get(), _batchAutoFlush);
    _batchStream.swap(dummy);
    _batchRequestNum = 0;
    _batchRequestCompress = false;
    _batchMarker = 0;
    return sent;
}

void
Ice::ConnectionI::sendResponse(BasicStream* os, Byte compressFlag)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(_state > StateNotValidated);

    try
    {
        if(--_dispatchCount == 0)
        {
            notifyAll();
        }

        if(_state == StateClosed)
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

        if(_acmTimeout > 0)
        {
            _acmAbsoluteTimeout = IceUtil::Time::now(IceUtil::Time::Monotonic) +
                IceUtil::Time::seconds(_acmTimeout);
        }
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
            notifyAll();
        }

        if(_state == StateClosed)
        {
            assert(_exception.get());
            _exception->ice_throw();
        }

        if(_state == StateClosing && _dispatchCount == 0)
        {
            initiateShutdown();
        }

        if(_acmTimeout > 0)
        {
            _acmAbsoluteTimeout = IceUtil::Time::now(IceUtil::Time::Monotonic) +
                IceUtil::Time::seconds(_acmTimeout);
        }
    }
    catch(const LocalException& ex)
    {
        setState(StateClosed, ex);
    }
}

EndpointIPtr
Ice::ConnectionI::endpoint() const
{
    return _endpoint; // No mutex protection necessary, _endpoint is immutable.
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

bool
Ice::ConnectionI::datagram() const
{
    return _endpoint->datagram(); // No mutex protection necessary, _endpoint is immutable.
}

bool
Ice::ConnectionI::readable() const
{
    return true;
}

bool
Ice::ConnectionI::read(BasicStream& stream)
{
    return _transceiver->read(stream);

    //
    // Updating _acmAbsoluteTimeout is too expensive here, because we
    // would have to acquire a lock just for this purpose. Instead, we
    // update _acmAbsoluteTimeout in message().
    //
}

void
Ice::ConnectionI::message(BasicStream& stream, const ThreadPoolPtr& threadPool)
{
    Byte compress = 0;
    Int requestId = 0;
    Int invokeNum = 0;
    ServantManagerPtr servantManager;
    ObjectAdapterPtr adapter;
    OutgoingAsyncPtr outAsync;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        //
        // We must promote within the synchronization, otherwise there
        // could be various race conditions with close connection
        // messages and other messages.
        //
        threadPool->promoteFollower(this);

        if(_state != StateClosed)
        {
            parseMessage(stream, invokeNum, requestId, compress, servantManager, adapter, outAsync);
        }

        //
        // parseMessage() can close the connection, so we must check
        // for closed state again.
        //
        if(_state == StateClosed)
        {
            return;
        }
    }

    //
    // Asynchronous replies must be handled outside the thread
    // synchronization, so that nested calls are possible.
    //
    if(outAsync)
    {
        outAsync->__finished(stream);
    }

    //
    // Method invocation (or multiple invocations for batch messages)
    // must be done outside the thread synchronization, so that nested
    // calls are possible.
    //
    invokeAll(stream, invokeNum, requestId, compress, servantManager, adapter);
}

void
Ice::ConnectionI::finished(const ThreadPoolPtr& threadPool)
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        assert(threadPool.get() == _threadPool.get() && _state == StateClosed && !_sendInProgress);

        threadPool->promoteFollower();

        _threadPool->decFdsInUse();
        _selectorThread->decFdsInUse();

        _flushSentCallbacks = 0; // Clear cyclic reference count.
    }

    if(_startCallback)
    {
        _startCallback->connectionStartFailed(this, *_exception.get());
        _startCallback = 0;
    }

    for(deque<OutgoingMessage>::iterator o = _sendStreams.begin(); o != _sendStreams.end(); ++o)
    {
        o->finished(*_exception.get());
    }
    _sendStreams.clear(); // Must be cleared before _requests because of Outgoing* references in OutgoingMessage

    for(map<Int, Outgoing*>::iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
        p->second->finished(*_exception.get());
    }
    _requests.clear();

    for(map<Int, OutgoingAsyncPtr>::iterator q = _asyncRequests.begin(); q != _asyncRequests.end(); ++q)
    {
        q->second->__finished(*_exception.get());
    }
    _asyncRequests.clear();

    //
    // This must be done last as this will cause waitUntilFinished() to return (and communicator
    // objects such as the timer might be destroyed too).
    //
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        try
        {
            _transceiver->close();
            _transceiver = 0;
            notifyAll();
        }
        catch(const Ice::LocalException&)
        {
            _transceiver = 0;
            notifyAll();
            throw;
        }
    }
}

void
Ice::ConnectionI::exception(const LocalException& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateClosed, ex);
}

void
Ice::ConnectionI::invokeException(const LocalException& ex, int invokeNum)
{
    //
    // Fatal exception while invoking a request. Since sendResponse/sendNoResponse isn't
    // called in case of a fatal exception we decrement _dispatchCount here.
    //

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateClosed, ex);

    if(invokeNum > 0)
    {
        assert(_dispatchCount > 0);
        _dispatchCount -= invokeNum;
        assert(_dispatchCount >= 0);
        if(_dispatchCount == 0)
        {
            notifyAll();
        }
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

string
Ice::ConnectionI::toString() const
{
    return _desc; // No mutex lock, _desc is immutable.
}

//
// Operations from SocketReadyCallback
//
SocketStatus
Ice::ConnectionI::socketReady()
{
    StartCallbackPtr callback;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        assert(_sendInProgress);

        if(_state == StateClosed)
        {
            return Finished;
        }

        try
        {
            //
            // First, we check if there's something to send. If that's the case, the connection
            // must be active and the only thing to do is send the queued streams.
            //
            if(!_sendStreams.empty())
            {
                if(!send())
                {
                    return NeedWrite;
                }
                assert(_sendStreams.empty());
            }
            else
            {
                assert(_state == StateClosed || _state <= StateNotValidated);
                if(_state == StateNotInitialized)
                {
                    SocketStatus status = initialize();
                    if(status != Finished)
                    {
                        return status;
                    }
                }

                if(_state <= StateNotValidated)
                {
                    SocketStatus status = validate();
                    if(status != Finished)
                    {
                        return status;
                    }
                }

                swap(_startCallback, callback);
            }
        }
        catch(const Ice::LocalException& ex)
        {
            setState(StateClosed, ex);
            return Finished;
        }

        assert(_sendStreams.empty());
        _selectorThread->unregister(this);
        _sendInProgress = false;
        if(_acmTimeout > 0)
        {
            _acmAbsoluteTimeout = IceUtil::Time::now(IceUtil::Time::Monotonic) + IceUtil::Time::seconds(_acmTimeout);
        }
    }

    if(callback)
    {
        callback->connectionStartCompleted(this);
    }
    return Finished;
}

void
Ice::ConnectionI::socketFinished()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(_sendInProgress && _state == StateClosed);
    _sendInProgress = false;
    _threadPool->finish(this);
}

void
Ice::ConnectionI::socketTimeout()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state <= StateNotValidated)
    {
        setState(StateClosed, ConnectTimeoutException(__FILE__, __LINE__));
    }
    else if(_state <= StateClosing)
    {
        setState(StateClosed, TimeoutException(__FILE__, __LINE__));
    }
}

//
// Only used by the SSL plug-in.
//
// The external party has to synchronize the connection, since the
// connection is the object that protects the transceiver.
//
IceInternal::TransceiverPtr
Ice::ConnectionI::getTransceiver() const
{
    return _transceiver;
}

Ice::ConnectionI::ConnectionI(const InstancePtr& instance,
                              const TransceiverPtr& transceiver,
                              const EndpointIPtr& endpoint,
                              const ObjectAdapterPtr& adapter) :
    EventHandler(instance, transceiver->fd()),
    _transceiver(transceiver),
    _desc(transceiver->toString()),
    _type(transceiver->type()),
    _endpoint(endpoint),
    _adapter(adapter),
    _logger(_instance->initializationData().logger), // Cached for better performance.
    _traceLevels(_instance->traceLevels()), // Cached for better performance.
    _warn(_instance->initializationData().properties->getPropertyAsInt("Ice.Warn.Connections") > 0),
    _acmTimeout(0),
    _compressionLevel(1),
    _nextRequestId(1),
    _requestsHint(_requests.end()),
    _asyncRequestsHint(_asyncRequests.end()),
    _batchAutoFlush(
        _instance->initializationData().properties->getPropertyAsIntWithDefault("Ice.BatchAutoFlush", 1) > 0),
    _batchStream(_instance.get(), _batchAutoFlush),
    _batchStreamInUse(false),
    _batchRequestNum(0),
    _batchRequestCompress(false),
    _batchMarker(0),
    _sendInProgress(false),
    _dispatchCount(0),
    _state(StateNotInitialized),
    _stateTime(IceUtil::Time::now(IceUtil::Time::Monotonic))
{
    Int& acmTimeout = const_cast<Int&>(_acmTimeout);
    if(_endpoint->datagram())
    {
        acmTimeout = 0;
    }
    else
    {
        if(_adapter)
        {
            acmTimeout = _instance->serverACM();
        }
        else
        {
            acmTimeout = _instance->clientACM();
        }
    }

    int& compressionLevel = const_cast<int&>(_compressionLevel);
    compressionLevel = _instance->initializationData().properties->getPropertyAsIntWithDefault(
        "Ice.Compression.Level", 1);
    if(compressionLevel < 1)
    {
        compressionLevel = 1;
    }
    else if(compressionLevel > 9)
    {
        compressionLevel = 9;
    }

    ObjectAdapterI* adapterImpl = _adapter ? dynamic_cast<ObjectAdapterI*>(_adapter.get()) : 0;
    if(adapterImpl)
    {
        _servantManager = adapterImpl->getServantManager();
    }

    __setNoDelete(true);
    try
    {
        if(adapterImpl)
        {
            const_cast<ThreadPoolPtr&>(_threadPool) = adapterImpl->getThreadPool();
        }
        else
        {
            const_cast<ThreadPoolPtr&>(_threadPool) = _instance->clientThreadPool();
        }
        _threadPool->incFdsInUse();

        const_cast<SelectorThreadPtr&>(_selectorThread) = _instance->selectorThread();
        _selectorThread->incFdsInUse();

        _flushSentCallbacks = new FlushSentCallbacks(this);
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
    assert(_state == StateClosed);
    assert(!_transceiver);
    assert(_dispatchCount == 0);
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
    assert(state == StateClosing || state == StateClosed);

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

        _exception.reset(dynamic_cast<LocalException*>(ex.ice_clone()));

        if(_warn)
        {
            //
            // We don't warn if we are not validated.
            //
            if(_state > StateNotValidated)
            {
                //
                // Don't warn about certain expected exceptions.
                //
                if(!(dynamic_cast<const CloseConnectionException*>(_exception.get()) ||
                     dynamic_cast<const ForcedCloseConnectionException*>(_exception.get()) ||
                     dynamic_cast<const ConnectionTimeoutException*>(_exception.get()) ||
                     dynamic_cast<const CommunicatorDestroyedException*>(_exception.get()) ||
                     dynamic_cast<const ObjectAdapterDeactivatedException*>(_exception.get()) ||
                     (dynamic_cast<const ConnectionLostException*>(_exception.get()) && _state == StateClosing)))
                {
                    Warning out(_logger);
                    out << "connection exception:\n" << *_exception.get() << '\n' << _desc;
#ifdef __GNUC__
                    out << "\n" << ex.ice_stackTrace();
#endif
                }
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
        _threadPool->_register(this);
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
        _threadPool->unregister(this);
        break;
    }

    case StateClosing:
    {
        //
        // Can't change back from closed.
        //
        if(_state == StateClosed)
        {
            return;
        }
        _threadPool->_register(this); // We need to continue to read in closing state.
        break;
    }

    case StateClosed:
    {
        if(_sendInProgress)
        {
            //
            // Unregister with both the pool and the selector thread. We unregister with
            // the pool to ensure that it stops reading on the socket (otherwise, if the
            // socket is closed the thread pool would spin always reading 0 from the FD).
            // The selector thread will register again the FD with the pool once it's
            // done.
            //
            _selectorThread->finish(this);
            _threadPool->unregister(this);
        }
        else
        {
            _threadPool->finish(this);
        }
        break;
    }
    }

    //
    // We only register with the connection monitor if our new state
    // is StateActive. Otherwise we unregister with the connection
    // monitor, but only if we were registered before, i.e., if our
    // old state was StateActive.
    //
    ConnectionMonitorPtr connectionMonitor = _instance->connectionMonitor();
    if(connectionMonitor)
    {
        if(state == StateActive)
        {
            connectionMonitor->add(this);
        }
        else if(_state == StateActive)
        {
            connectionMonitor->remove(this);
        }
    }

    _state = state;
    _stateTime = IceUtil::Time::now(IceUtil::Time::Monotonic);

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

    if(!_endpoint->datagram())
    {
        //
        // Before we shut down, we send a close connection message.
        //
        BasicStream os(_instance.get());
        os.write(magic[0]);
        os.write(magic[1]);
        os.write(magic[2]);
        os.write(magic[3]);
        os.write(protocolMajor);
        os.write(protocolMinor);
        os.write(encodingMajor);
        os.write(encodingMinor);
        os.write(closeConnectionMsg);
        os.write((Byte)1); // Compression status: compression supported but not used.
        os.write(headerSize); // Message size.

        OutgoingMessage message(&os, false);
        sendMessage(message);

        //
        // The CloseConnection message should be sufficient. Closing the write
        // end of the socket is probably an artifact of how things were done
        // in IIOP. In fact, shutting down the write end of the socket causes
        // problems on Windows by preventing the peer from using the socket.
        // For example, the peer is no longer able to continue writing a large
        // message after the socket is shutdown.
        //
        //_transceiver->shutdownWrite();
    }
}

SocketStatus
Ice::ConnectionI::initialize()
{
    SocketStatus status = _transceiver->initialize();
    if(status != Finished)
    {
        return status;
    }

    //
    // Update the connection description once the transceiver is initialized.
    //
    const_cast<string&>(_desc) = _transceiver->toString();
    setState(StateNotValidated);
    return Finished;
}

SocketStatus
Ice::ConnectionI::validate()
{
    if(!_endpoint->datagram()) // Datagram connections are always implicitly validated.
    {
        if(_adapter) // The server side has the active role for connection validation.
        {
            BasicStream& os = _stream;
            if(os.b.empty())
            {
                os.write(magic[0]);
                os.write(magic[1]);
                os.write(magic[2]);
                os.write(magic[3]);
                os.write(protocolMajor);
                os.write(protocolMinor);
                os.write(encodingMajor);
                os.write(encodingMinor);
                os.write(validateConnectionMsg);
                os.write(static_cast<Byte>(0)); // Compression status (always zero for validate connection).
                os.write(headerSize); // Message size.
                os.i = os.b.begin();
                traceSend(os, _logger, _traceLevels);
            }

            if(!_transceiver->write(os))
            {
                return NeedWrite;
            }
        }
        else // The client side has the passive role for connection validation.
        {
            BasicStream& is = _stream;
            if(is.b.empty())
            {
                is.b.resize(headerSize);
                is.i = is.b.begin();
            }

            if(!_transceiver->read(is))
            {
                return NeedRead;
            }

            assert(is.i == is.b.end());
            is.i = is.b.begin();
            Byte m[4];
            is.read(m[0]);
            is.read(m[1]);
            is.read(m[2]);
            is.read(m[3]);
            if(m[0] != magic[0] || m[1] != magic[1] || m[2] != magic[2] || m[3] != magic[3])
            {
                BadMagicException ex(__FILE__, __LINE__);
                ex.badMagic = Ice::ByteSeq(&m[0], &m[0] + sizeof(magic));
                throw ex;
            }
            Byte pMajor;
            Byte pMinor;
            is.read(pMajor);
            is.read(pMinor);
            if(pMajor != protocolMajor)
            {
                UnsupportedProtocolException ex(__FILE__, __LINE__);
                ex.badMajor = static_cast<unsigned char>(pMajor);
                ex.badMinor = static_cast<unsigned char>(pMinor);
                ex.major = static_cast<unsigned char>(protocolMajor);
                ex.minor = static_cast<unsigned char>(protocolMinor);
                throw ex;
            }
            Byte eMajor;
            Byte eMinor;
            is.read(eMajor);
            is.read(eMinor);
            if(eMajor != encodingMajor)
            {
                UnsupportedEncodingException ex(__FILE__, __LINE__);
                ex.badMajor = static_cast<unsigned char>(eMajor);
                ex.badMinor = static_cast<unsigned char>(eMinor);
                ex.major = static_cast<unsigned char>(encodingMajor);
                ex.minor = static_cast<unsigned char>(encodingMinor);
                throw ex;
            }
            Byte messageType;
            is.read(messageType);
            if(messageType != validateConnectionMsg)
            {
                throw ConnectionNotValidatedException(__FILE__, __LINE__);
            }
            Byte compress;
            is.read(compress); // Ignore compression status for validate connection.
            Int size;
            is.read(size);
            if(size != headerSize)
            {
                throw IllegalMessageSizeException(__FILE__, __LINE__);
            }
            traceRecv(is, _logger, _traceLevels);
        }
    }

    _stream.resize(0);
    _stream.i = _stream.b.begin();

    //
    // We start out in holding state.
    //
    setState(StateHolding);
    return Finished;
}

bool
Ice::ConnectionI::send()
{
    assert(_transceiver);
    assert(!_sendStreams.empty());
    
    bool flushSentCallbacks = _sentCallbacks.empty();
    try
    {
        while(!_sendStreams.empty())
        {
            OutgoingMessage* message = &_sendStreams.front();

            //
            // Prepare the message stream for writing if necessary.
            //
            if(!message->stream->i)
            {
                message->stream->i = message->stream->b.begin();
                if(message->compress && message->stream->b.size() >= 100) // Only compress messages > 100 bytes.
                {
                    //
                    // Message compressed. Request compressed response, if any.
                    //
                    message->stream->b[9] = 2;

                    //
                    // Do compression.
                    //
                    BasicStream stream(_instance.get());
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
                }
            }

            //
            // Send the first message.
            //
            assert(message->stream->i);
            if(!_transceiver->write(*message->stream))
            {
                if(flushSentCallbacks && !_sentCallbacks.empty())
                {
                    _threadPool->execute(_flushSentCallbacks);
                }
                return false;
            }

            //
            // Notify the message that it was sent.
            //
            message->sent(this, true);
            if(dynamic_cast<Ice::AMISentCallback*>(message->outAsync.get()))
            {
                _sentCallbacks.push_back(message->outAsync);
            }
            _sendStreams.pop_front();
        }
    }
    catch(const Ice::LocalException&)
    {
        if(flushSentCallbacks && !_sentCallbacks.empty())
        {
            _threadPool->execute(_flushSentCallbacks);
        }
        throw;
    }

    if(flushSentCallbacks && !_sentCallbacks.empty())
    {
        _threadPool->execute(_flushSentCallbacks);
    }
    return true;
}

void
Ice::ConnectionI::flushSentCallbacks()
{
    vector<OutgoingAsyncMessageCallbackPtr> callbacks;
    {
        Lock sync(*this);
        assert(!_sentCallbacks.empty());
        _sentCallbacks.swap(callbacks);
    }
    for(vector<OutgoingAsyncMessageCallbackPtr>::const_iterator p = callbacks.begin(); p != callbacks.end(); ++p)
    {
        (*p)->__sentCallback(_instance);
    }
}

bool
Ice::ConnectionI::sendMessage(OutgoingMessage& message)
{
    assert(_state != StateClosed);

    message.stream->i = 0; // Reset the message stream iterator before starting sending the message.

    if(_sendInProgress)
    {
        _sendStreams.push_back(message);
        _sendStreams.back().adopt(0);
        return false;
    }

    assert(!_sendInProgress);

    //
    // Attempt to send the message without blocking. If the send blocks, we register
    // the connection with the selector thread.
    //

    message.stream->i = message.stream->b.begin();

    if(message.compress && message.stream->b.size() >= 100) // Only compress messages larger than 100 bytes.
    {
        //
        // Message compressed. Request compressed response, if any.
        //
        message.stream->b[9] = 2;

        //
        // Do compression.
        //
        BasicStream stream(_instance.get());
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
        if(_transceiver->write(stream))
        {
            message.sent(this, false);
            if(_acmTimeout > 0)
            {
                _acmAbsoluteTimeout =
                    IceUtil::Time::now(IceUtil::Time::Monotonic) + IceUtil::Time::seconds(_acmTimeout);
            }
            return true;
        }

        _sendStreams.push_back(message);
        _sendStreams.back().adopt(&stream);
    }
    else
    {
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
        if(_transceiver->write(*message.stream))
        {
            message.sent(this, false);
            if(_acmTimeout > 0)
            {
                _acmAbsoluteTimeout =
                    IceUtil::Time::now(IceUtil::Time::Monotonic) + IceUtil::Time::seconds(_acmTimeout);
            }
            return true;
        }

        _sendStreams.push_back(message);
        _sendStreams.back().adopt(0); // Adopt the stream.
    }

    _sendInProgress = true;
    _selectorThread->_register(_transceiver->fd(), this, NeedWrite, _endpoint->timeout());
    return false;
}

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

void
Ice::ConnectionI::parseMessage(BasicStream& stream, Int& invokeNum, Int& requestId, Byte& compress,
                               ServantManagerPtr& servantManager, ObjectAdapterPtr& adapter,
                               OutgoingAsyncPtr& outAsync)
{
    assert(_state > StateNotValidated && _state < StateClosed);

    if(_acmTimeout > 0)
    {
        _acmAbsoluteTimeout = IceUtil::Time::now(IceUtil::Time::Monotonic) + IceUtil::Time::seconds(_acmTimeout);
    }

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
            BasicStream ustream(_instance.get());
            doUncompress(stream, ustream);
            stream.b.swap(ustream.b);
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
                    setState(StateClosed, CloseConnectionException(__FILE__, __LINE__));
                }
                break;
            }

            case requestMsg:
            {
                if(_state == StateClosing)
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
                    ++_dispatchCount;
                }
                break;
            }

            case requestBatchMsg:
            {
                if(_state == StateClosing)
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
                    _dispatchCount += invokeNum;
                }
                break;
            }

            case replyMsg:
            {
                traceRecv(stream, _logger, _traceLevels);

                stream.read(requestId);

                map<Int, Outgoing*>::iterator p = _requests.end();
                map<Int, OutgoingAsyncPtr>::iterator q = _asyncRequests.end();

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

                if(p == _requests.end() && q == _asyncRequests.end())
                {
                    throw UnknownRequestIdException(__FILE__, __LINE__);
                }

                if(p != _requests.end())
                {
                    p->second->finished(stream);

                    if(p == _requestsHint)
                    {
                        _requests.erase(p++);
                        _requestsHint = p;
                    }
                    else
                    {
                        _requests.erase(p);
                    }
                }
                else
                {
                    assert(q != _asyncRequests.end());

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
                }
                notifyAll(); // Notify threads blocked in close(false)
                break;
            }

            case validateConnectionMsg:
            {
                traceRecv(stream, _logger, _traceLevels);
                if(_warn)
                {
                    Warning out(_logger);
                    out << "ignoring unexpected validate connection message:\n" << _desc;
                }
                break;
            }

            default:
            {
                trace("received unknown message\n(invalid, closing connection)", stream, _logger, _traceLevels);
                throw UnknownMessageException(__FILE__, __LINE__);
                break;
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
            Incoming in(_instance.get(), this, adapter, response, compress, requestId);
            BasicStream* is = in.is();
            stream.swap(*is);
            BasicStream* os = in.os();

            //
            // Prepare the response if necessary.
            //
            if(response)
            {
                assert(invokeNum == 1); // No further invocations if a response is expected.
                os->writeBlob(replyHdr, sizeof(replyHdr));

                //
                // Add the request ID.
                //
                os->write(requestId);
            }

            in.invoke(servantManager);

            //
            // If there are more invocations, we need the stream back.
            //
            if(--invokeNum > 0)
            {
                stream.swap(*is);
            }
        }
    }
    catch(const LocalException& ex)
    {
        invokeException(ex, invokeNum);  // Fatal invocation exception
    }
}

