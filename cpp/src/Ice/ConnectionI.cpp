//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "ConnectionI.h"
#include "BatchRequestQueue.h"
#include "CheckIdentity.h"
#include "DefaultsAndOverrides.h"
#include "Endian.h"
#include "EndpointI.h"
#include "Ice/IncomingRequest.h"
#include "Ice/LocalException.h"
#include "Ice/LoggerUtil.h"
#include "Ice/OutgoingResponse.h"
#include "Ice/Properties.h"
#include "IceUtil/DisableWarnings.h"
#include "IdleTimeoutTransceiverDecorator.h"
#include "Instance.h"
#include "ObjectAdapterI.h"   // For getThreadPool()
#include "ReferenceFactory.h" // For createProxy().
#include "RequestHandler.h"   // For RetryException
#include "ThreadPool.h"
#include "TraceLevels.h"
#include "TraceUtil.h"
#include "Transceiver.h"

#include <stdexcept>

#ifdef ICE_HAS_BZIP2
#    include <bzlib.h>
#endif

using namespace std;
using namespace Ice;
using namespace Ice::Instrumentation;
using namespace IceInternal;

namespace
{
    class ConnectTimerTask final : public IceUtil::TimerTask
    {
    public:
        ConnectTimerTask(const Ice::ConnectionIPtr& connection) : _connection(connection) {}

        void runTimerTask() final
        {
            if (auto connection = _connection.lock())
            {
                connection->connectTimedOut();
            }
        }

    private:
        const weak_ptr<Ice::ConnectionI> _connection;
    };

    class CloseTimerTask final : public IceUtil::TimerTask
    {
    public:
        CloseTimerTask(const Ice::ConnectionIPtr& connection) : _connection(connection) {}

        void runTimerTask() final
        {
            if (auto connection = _connection.lock())
            {
                connection->closeTimedOut();
            }
        }

    private:
        const weak_ptr<Ice::ConnectionI> _connection;
    };

    class InactivityTimerTask final : public IceUtil::TimerTask
    {
    public:
        InactivityTimerTask(const Ice::ConnectionIPtr& connection) : _connection(connection) {}

        void runTimerTask() final
        {
            if (auto connection = _connection.lock())
            {
                connection->inactivityCheck();
            }
        }

    private:
        const weak_ptr<Ice::ConnectionI> _connection;
    };

    //
    // Class for handling Ice::Connection::begin_flushBatchRequests
    //
    class ConnectionFlushBatchAsync : public OutgoingAsyncBase
    {
    public:
        ConnectionFlushBatchAsync(const Ice::ConnectionIPtr&, const InstancePtr&);

        virtual Ice::ConnectionPtr getConnection() const;

        void invoke(std::string_view, Ice::CompressBatch);

    private:
        const Ice::ConnectionIPtr _connection;
    };

    ConnectionState connectionStateMap[] = {
        ConnectionState::ConnectionStateValidating, // StateNotInitialized
        ConnectionState::ConnectionStateValidating, // StateNotValidated
        ConnectionState::ConnectionStateActive,     // StateActive
        ConnectionState::ConnectionStateHolding,    // StateHolding
        ConnectionState::ConnectionStateClosing,    // StateClosing
        ConnectionState::ConnectionStateClosing,    // StateClosingPending
        ConnectionState::ConnectionStateClosed,     // StateClosed
        ConnectionState::ConnectionStateClosed,     // StateFinished
    };
}

ConnectionFlushBatchAsync::ConnectionFlushBatchAsync(const ConnectionIPtr& connection, const InstancePtr& instance)
    : OutgoingAsyncBase(instance),
      _connection(connection)
{
}

ConnectionPtr
ConnectionFlushBatchAsync::getConnection() const
{
    return _connection;
}

void
ConnectionFlushBatchAsync::invoke(string_view operation, Ice::CompressBatch compressBatch)
{
    _observer.attach(_instance.get(), operation);
    try
    {
        AsyncStatus status;
        bool compress;
        int batchRequestCount = _connection->getBatchRequestQueue()->swap(&_os, compress);
        if (batchRequestCount == 0)
        {
            status = AsyncStatusSent;
            if (sent())
            {
                status = static_cast<AsyncStatus>(status | AsyncStatusInvokeSentCallback);
            }
        }
        else
        {
            if (compressBatch == CompressBatch::Yes)
            {
                compress = true;
            }
            else if (compressBatch == CompressBatch::No)
            {
                compress = false;
            }
            status = _connection->sendAsyncRequest(shared_from_this(), compress, false, batchRequestCount);
        }

        if (status & AsyncStatusSent)
        {
            _sentSynchronously = true;
            if (status & AsyncStatusInvokeSentCallback)
            {
                invokeSent();
            }
        }
    }
    catch (const RetryException& ex)
    {
        try
        {
            rethrow_exception(ex.get());
        }
        catch (const std::exception&)
        {
            if (exception(current_exception()))
            {
                invokeExceptionAsync();
            }
        }
    }
    catch (const Exception&)
    {
        if (exception(current_exception()))
        {
            invokeExceptionAsync();
        }
    }
}

Ice::ConnectionI::Observer::Observer() : _readStreamPos(nullptr), _writeStreamPos(nullptr) {}

void
Ice::ConnectionI::Observer::startRead(const Buffer& buf)
{
    if (_readStreamPos)
    {
        assert(!buf.b.empty());
        _observer->receivedBytes(static_cast<int>(buf.i - _readStreamPos));
    }
    _readStreamPos = buf.b.empty() ? 0 : buf.i;
}

void
Ice::ConnectionI::Observer::finishRead(const Buffer& buf)
{
    if (_readStreamPos == 0)
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
    if (_writeStreamPos)
    {
        assert(!buf.b.empty());
        _observer->sentBytes(static_cast<int>(buf.i - _writeStreamPos));
    }
    _writeStreamPos = buf.b.empty() ? 0 : buf.i;
}

void
Ice::ConnectionI::Observer::finishWrite(const Buffer& buf)
{
    if (_writeStreamPos == 0)
    {
        return;
    }
    if (buf.i > _writeStreamPos)
    {
        _observer->sentBytes(static_cast<int>(buf.i - _writeStreamPos));
    }
    _writeStreamPos = 0;
}

void
Ice::ConnectionI::Observer::attach(const Ice::Instrumentation::ConnectionObserverPtr& observer)
{
    ObserverHelperT<Ice::Instrumentation::ConnectionObserver>::attach(observer);
    if (!observer)
    {
        _writeStreamPos = 0;
        _readStreamPos = 0;
    }
}

void
Ice::ConnectionI::OutgoingMessage::adopt(OutputStream* str)
{
    if (adopted)
    {
        if (str)
        {
            delete stream;
            stream = nullptr;
            adopted = false;
        }
        else
        {
            return; // Stream is already adopted.
        }
    }
    else if (!str)
    {
        if (outAsync)
        {
            return; // Adopting request stream is not necessary.
        }
        else
        {
            str = stream; // Adopt this stream
            stream = nullptr;
        }
    }

    assert(str);
    assert(str->b.ownsMemory());
    stream = new OutputStream(std::move(*str));
    adopted = true;
}

void
Ice::ConnectionI::OutgoingMessage::canceled(bool adoptStream)
{
    assert(outAsync); // Only requests can timeout.
    outAsync = nullptr;
    if (adoptStream)
    {
        adopt(nullptr); // Adopt the request stream
    }
    else
    {
        assert(!adopted);
    }
}

bool
Ice::ConnectionI::OutgoingMessage::sent()
{
    // This function is called with the connection mutex locked.

    if (adopted)
    {
        delete stream;
    }
    stream = nullptr;

    if (outAsync)
    {
#if defined(ICE_USE_IOCP)
        invokeSent = outAsync->sent();
        return invokeSent || receivedReply;
#else
        return outAsync->sent();
#endif
    }
    return false;
}

void
Ice::ConnectionI::OutgoingMessage::completed(std::exception_ptr ex)
{
    if (outAsync)
    {
        if (outAsync->exception(ex))
        {
            outAsync->invokeException();
        }
    }

    if (adopted)
    {
        delete stream;
    }
    stream = 0;
}

void
Ice::ConnectionI::startAsync(
    function<void(Ice::ConnectionIPtr)> connectionStartCompleted,
    function<void(Ice::ConnectionIPtr, exception_ptr)> connectionStartFailed)
{
    try
    {
        std::unique_lock lock(_mutex);
        if (_state >= StateClosed) // The connection might already be closed if the communicator was destroyed.
        {
            assert(_exception);
            rethrow_exception(_exception);
        }

        if (!initialize() || !validate())
        {
            if (_connectTimeout > chrono::seconds::zero())
            {
                _timer->schedule(make_shared<ConnectTimerTask>(shared_from_this()), _connectTimeout);
            }

            if (connectionStartCompleted && connectionStartFailed)
            {
                _connectionStartCompleted = std::move(connectionStartCompleted);
                _connectionStartFailed = std::move(connectionStartFailed);
                return;
            }

            //
            // Wait for the connection to be validated.
            //
            _conditionVariable.wait(lock, [this] { return _state > StateNotValidated; });

            if (_state >= StateClosing)
            {
                assert(_exception);
                rethrow_exception(_exception);
            }
        }

        //
        // We start out in holding state.
        //
        setState(StateHolding);
    }
    catch (const Ice::LocalException&)
    {
        exception(current_exception());
        if (connectionStartFailed)
        {
            connectionStartFailed(shared_from_this(), current_exception());
            return;
        }
        else
        {
            waitUntilFinished();
            throw;
        }
    }

    if (connectionStartCompleted)
    {
        connectionStartCompleted(shared_from_this());
    }
}

void
Ice::ConnectionI::activate()
{
    std::lock_guard lock(_mutex);
    if (_state <= StateNotValidated)
    {
        return;
    }
    setState(StateActive);
}

void
Ice::ConnectionI::hold()
{
    std::lock_guard lock(_mutex);
    if (_state <= StateNotValidated)
    {
        return;
    }

    setState(StateHolding);
}

void
Ice::ConnectionI::destroy(DestructionReason reason)
{
    std::lock_guard lock(_mutex);

    switch (reason)
    {
        case ObjectAdapterDeactivated:
        {
            setState(StateClosing, make_exception_ptr(ObjectAdapterDeactivatedException(__FILE__, __LINE__)));
            break;
        }

        case CommunicatorDestroyed:
        {
            setState(StateClosing, make_exception_ptr(CommunicatorDestroyedException(__FILE__, __LINE__)));
            break;
        }
    }
}

void
Ice::ConnectionI::close(ConnectionClose mode) noexcept
{
    std::unique_lock lock(_mutex);

    if (mode == ConnectionClose::Forcefully)
    {
        setState(StateClosed, make_exception_ptr(ConnectionManuallyClosedException(__FILE__, __LINE__, false)));
    }
    else if (mode == ConnectionClose::Gracefully)
    {
        setState(StateClosing, make_exception_ptr(ConnectionManuallyClosedException(__FILE__, __LINE__, true)));
    }
    else
    {
        assert(mode == ConnectionClose::GracefullyWithWait);

        //
        // Wait until all outstanding requests have been completed.
        //
        _conditionVariable.wait(lock, [this] { return _asyncRequests.empty(); });

        setState(StateClosing, make_exception_ptr(ConnectionManuallyClosedException(__FILE__, __LINE__, true)));
    }
}

bool
Ice::ConnectionI::isActiveOrHolding() const
{
    // We can not use trylock here, otherwise the outgoing connection
    // factory might return destroyed (closing or closed) connections,
    // resulting in connection retry exhaustion.

    std::lock_guard lock(_mutex);

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
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);

    if (!lock.owns_lock())
    {
        return false;
    }

    if (_state != StateFinished || _upcallCount != 0)
    {
        return false;
    }

    assert(_state == StateFinished);
    return true;
}

void
Ice::ConnectionI::throwException() const
{
    std::lock_guard lock(_mutex);

    if (_exception)
    {
        assert(_state >= StateClosing);
        rethrow_exception(_exception);
    }
}

void
Ice::ConnectionI::waitUntilHolding() const
{
    std::unique_lock lock(_mutex);
    _conditionVariable.wait(lock, [this] { return _state >= StateHolding && _upcallCount == 0; });
}

void
Ice::ConnectionI::waitUntilFinished()
{
    std::unique_lock lock(_mutex);

    //
    // We wait indefinitely until the connection is finished and all
    // outstanding requests are completed. Otherwise we couldn't
    // guarantee that there are no outstanding calls when deactivate()
    // is called on the servant locators.
    //
    _conditionVariable.wait(lock, [this] { return _state >= StateFinished && _upcallCount == 0; });

    assert(_state == StateFinished);

    //
    // Clear the OA. See bug 1673 for the details of why this is necessary.
    //
    _adapter = nullptr;
}

void
Ice::ConnectionI::updateObserver()
{
    std::lock_guard lock(_mutex);
    if (_state < StateNotValidated || _state > StateClosed)
    {
        return;
    }

    assert(_instance->initializationData().observer);

    ConnectionObserverPtr o = _instance->initializationData().observer->getConnectionObserver(
        initConnectionInfo(),
        _endpoint,
        toConnectionState(_state),
        _observer.get());
    _observer.attach(o);
}

AsyncStatus
Ice::ConnectionI::sendAsyncRequest(const OutgoingAsyncBasePtr& out, bool compress, bool response, int batchRequestCount)
{
    OutputStream* os = out->getOs();

    std::lock_guard lock(_mutex);
    //
    // If the exception is closed before we even have a chance
    // to send our request, we always try to send the request
    // again.
    //
    if (_exception)
    {
        throw RetryException(_exception);
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
    out->cancelable(shared_from_this());
    int32_t requestId = 0;
    if (response)
    {
        //
        // Create a new unique request ID.
        //
        requestId = _nextRequestId++;
        if (requestId <= 0)
        {
            _nextRequestId = 1;
            requestId = _nextRequestId++;
        }

        //
        // Fill in the request ID.
        //
        const byte* p = reinterpret_cast<const byte*>(&requestId);
        if constexpr (endian::native == endian::big)
        {
            reverse_copy(p, p + sizeof(int32_t), os->b.begin() + headerSize);
        }
        else
        {
            copy(p, p + sizeof(int32_t), os->b.begin() + headerSize);
        }
    }
    else if (batchRequestCount > 0)
    {
        const byte* p = reinterpret_cast<const byte*>(&batchRequestCount);
        if constexpr (endian::native == endian::big)
        {
            reverse_copy(p, p + sizeof(int32_t), os->b.begin() + headerSize);
        }
        else
        {
            copy(p, p + sizeof(int32_t), os->b.begin() + headerSize);
        }
    }

    out->attachRemoteObserver(initConnectionInfo(), _endpoint, requestId);

    AsyncStatus status = AsyncStatusQueued;
    try
    {
        if (isAtRest())
        {
            // If we were at rest, we're not anymore since we're sending a request.
            cancelInactivityTimerTask();
        }

        OutgoingMessage message(out, os, compress, requestId);
        status = sendMessage(message);
    }
    catch (const LocalException&)
    {
        setState(StateClosed, current_exception());
        assert(_exception);
        rethrow_exception(_exception);
    }

    if (response)
    {
        _asyncRequestsHint =
            _asyncRequests.insert(_asyncRequests.end(), pair<const int32_t, OutgoingAsyncBasePtr>(requestId, out));
    }
    else if (isAtRest())
    {
        // A oneway invocation is considered completed as soon as sendMessage returns.
        scheduleInactivityTimerTask();
    }
    return status;
}

const BatchRequestQueuePtr&
Ice::ConnectionI::getBatchRequestQueue() const
{
    return _batchRequestQueue;
}

std::function<void()>
Ice::ConnectionI::flushBatchRequestsAsync(
    CompressBatch compress,
    ::std::function<void(::std::exception_ptr)> ex,
    ::std::function<void(bool)> sent)
{
    class ConnectionFlushBatchLambda : public ConnectionFlushBatchAsync, public LambdaInvoke
    {
    public:
        ConnectionFlushBatchLambda(
            std::shared_ptr<Ice::ConnectionI>&& connection,
            const InstancePtr& instance,
            std::function<void(std::exception_ptr)> ex,
            std::function<void(bool)> sent)
            : ConnectionFlushBatchAsync(connection, instance),
              LambdaInvoke(std::move(ex), std::move(sent))
        {
        }
    };
    auto outAsync = make_shared<ConnectionFlushBatchLambda>(shared_from_this(), _instance, ex, sent);
    static constexpr string_view operationName = "flushBatchRequests";
    outAsync->invoke(operationName, compress);
    return [outAsync]() { outAsync->cancel(); };
}

namespace
{
    class HeartbeatAsync : public OutgoingAsyncBase
    {
    public:
        HeartbeatAsync(
            const ConnectionIPtr& connection,
            const CommunicatorPtr& communicator,
            const InstancePtr& instance)
            : OutgoingAsyncBase(instance),
              _communicator(communicator),
              _connection(connection)
        {
        }

        virtual CommunicatorPtr getCommunicator() const { return _communicator; }

        virtual ConnectionPtr getConnection() const { return _connection; }

        virtual string_view getOperation() const { return _operationName; }

        void invoke()
        {
            _observer.attach(_instance.get(), _operationName);
            try
            {
                _os.write(magic[0]);
                _os.write(magic[1]);
                _os.write(magic[2]);
                _os.write(magic[3]);
                _os.write(currentProtocol);
                _os.write(currentProtocolEncoding);
                _os.write(validateConnectionMsg);
                _os.write(static_cast<uint8_t>(0)); // Compression status (always zero for validate connection).
                _os.write(headerSize);              // Message size.
                _os.i = _os.b.begin();

                AsyncStatus status = _connection->sendAsyncRequest(shared_from_this(), false, false, 0);
                if (status & AsyncStatusSent)
                {
                    _sentSynchronously = true;
                    if (status & AsyncStatusInvokeSentCallback)
                    {
                        invokeSent();
                    }
                }
            }
            catch (const RetryException& ex)
            {
                if (exception(ex.get()))
                {
                    invokeExceptionAsync();
                }
            }
            catch (const Exception&)
            {
                if (exception(current_exception()))
                {
                    invokeExceptionAsync();
                }
            }
        }

    private:
        CommunicatorPtr _communicator;
        ConnectionIPtr _connection;
        static constexpr string_view _operationName = "heartbeat";
    };
}

std::function<void()>
Ice::ConnectionI::heartbeatAsync(::std::function<void(::std::exception_ptr)> ex, ::std::function<void(bool)> sent)
{
    class HeartbeatLambda : public HeartbeatAsync, public LambdaInvoke
    {
    public:
        HeartbeatLambda(
            std::shared_ptr<Ice::ConnectionI>&& connection,
            Ice::CommunicatorPtr& communicator,
            const InstancePtr& instance,
            std::function<void(std::exception_ptr)> ex,
            std::function<void(bool)> sent)
            : HeartbeatAsync(connection, communicator, instance),
              LambdaInvoke(std::move(ex), std::move(sent))
        {
        }
    };
    auto outAsync = make_shared<HeartbeatLambda>(shared_from_this(), _communicator, _instance, ex, sent);
    outAsync->invoke();
    return [outAsync]() { outAsync->cancel(); };
}

void
Ice::ConnectionI::setHeartbeatCallback(HeartbeatCallback callback)
{
    std::lock_guard lock(_mutex);
    if (_state >= StateClosed)
    {
        return;
    }
    _heartbeatCallback = std::move(callback);
}

void
Ice::ConnectionI::setCloseCallback(CloseCallback callback)
{
    std::lock_guard lock(_mutex);
    if (_state >= StateClosed)
    {
        if (callback)
        {
            auto self = shared_from_this();
            _threadPool->execute([self, callback = std::move(callback)]() { self->closeCallback(callback); }, self);
        }
    }
    else
    {
        _closeCallback = std::move(callback);
    }
}

void
Ice::ConnectionI::closeCallback(const CloseCallback& callback)
{
    try
    {
        callback(shared_from_this());
    }
    catch (const std::exception& ex)
    {
        Error out(_instance->initializationData().logger);
        out << "connection callback exception:\n" << ex << '\n' << _desc;
    }
    catch (...)
    {
        Error out(_instance->initializationData().logger);
        out << "connection callback exception:\nunknown c++ exception" << '\n' << _desc;
    }
}

void
Ice::ConnectionI::asyncRequestCanceled(const OutgoingAsyncBasePtr& outAsync, exception_ptr ex)
{
    //
    // NOTE: This isn't called from a thread pool thread.
    //

    std::lock_guard lock(_mutex);
    if (_state >= StateClosed)
    {
        return; // The request has already been or will be shortly notified of the failure.
    }

    for (deque<OutgoingMessage>::iterator o = _sendStreams.begin(); o != _sendStreams.end(); ++o)
    {
        if (o->outAsync.get() == outAsync.get())
        {
            if (o->requestId)
            {
                bool removed = false;
                if (_asyncRequestsHint != _asyncRequests.end() &&
                    _asyncRequestsHint->second == dynamic_pointer_cast<OutgoingAsync>(outAsync))
                {
                    _asyncRequests.erase(_asyncRequestsHint);
                    _asyncRequestsHint = _asyncRequests.end();
                    removed = true;
                }
                else
                {
                    removed = _asyncRequests.erase(o->requestId) == 1;
                }

                if (removed && isAtRest())
                {
                    scheduleInactivityTimerTask();
                }
            }

            try
            {
                rethrow_exception(ex);
            }
            catch (const ConnectionIdleException&)
            {
                setState(StateClosed, ex);
            }
            catch (const std::exception&)
            {
                //
                // If the request is being sent, don't remove it from the send streams,
                // it will be removed once the sending is finished.
                //
                if (o == _sendStreams.begin())
                {
                    o->canceled(true); // true = adopt the stream
                }
                else
                {
                    o->canceled(false);
                    _sendStreams.erase(o);
                }
                if (outAsync->exception(ex))
                {
                    outAsync->invokeExceptionAsync();
                }
            }
            return;
        }
    }

    if (dynamic_pointer_cast<OutgoingAsync>(outAsync))
    {
        if (_asyncRequestsHint != _asyncRequests.end())
        {
            if (_asyncRequestsHint->second == outAsync)
            {
                try
                {
                    rethrow_exception(ex);
                }
                catch (const ConnectionIdleException&)
                {
                    setState(StateClosed, ex);
                }
                catch (const std::exception&)
                {
                    _asyncRequests.erase(_asyncRequestsHint);
                    _asyncRequestsHint = _asyncRequests.end();

                    if (isAtRest())
                    {
                        scheduleInactivityTimerTask();
                    }

                    if (outAsync->exception(ex))
                    {
                        outAsync->invokeExceptionAsync();
                    }
                }
                return;
            }
        }

        for (map<int32_t, OutgoingAsyncBasePtr>::iterator p = _asyncRequests.begin(); p != _asyncRequests.end(); ++p)
        {
            if (p->second.get() == outAsync.get())
            {
                try
                {
                    rethrow_exception(ex);
                }
                catch (const ConnectionIdleException&)
                {
                    setState(StateClosed, ex);
                }
                catch (const std::exception&)
                {
                    assert(p != _asyncRequestsHint);
                    _asyncRequests.erase(p);

                    if (isAtRest())
                    {
                        scheduleInactivityTimerTask();
                    }

                    if (outAsync->exception(ex))
                    {
                        outAsync->invokeExceptionAsync();
                    }
                }
                return;
            }
        }
    }
}

void
Ice::ConnectionI::dispatchException(exception_ptr ex, int requestCount)
{
    // Fatal exception while dispatching a request. Since sendResponse isn't called in case of a fatal exception we
    // decrement _upcallCount here.

    bool finished = false;
    {
        std::lock_guard lock(_mutex);
        setState(StateClosed, ex);

        if (requestCount > 0)
        {
            assert(_upcallCount >= requestCount);
            _upcallCount -= requestCount;
            if (_upcallCount == 0)
            {
                if (_state == StateFinished)
                {
                    finished = true;
                    if (_observer)
                    {
                        _observer.detach();
                    }
                }
                _conditionVariable.notify_all();
            }

            _dispatchCount -= requestCount;
            if (isAtRest())
            {
                scheduleInactivityTimerTask();
            }
        }
    }
    if (finished && _removeFromFactory)
    {
        _removeFromFactory(shared_from_this());
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
    if (adapter)
    {
        // Go through the adapter to set the adapter on this connection
        // to ensure the object adapter is still active.
        dynamic_cast<ObjectAdapterI*>(adapter.get())->setAdapterOnConnection(shared_from_this());
    }
    else
    {
        std::lock_guard lock(_mutex);
        if (_state <= StateNotValidated || _state >= StateClosing)
        {
            return;
        }

        _adapter = nullptr;
    }

    //
    // We never change the thread pool with which we were initially
    // registered, even if we add or remove an object adapter.
    //
}

ObjectAdapterPtr
Ice::ConnectionI::getAdapter() const noexcept
{
    std::lock_guard lock(_mutex);
    return _adapter;
}

EndpointPtr
Ice::ConnectionI::getEndpoint() const noexcept
{
    return _endpoint; // No mutex protection necessary, _endpoint is immutable.
}

ObjectPrx
Ice::ConnectionI::createProxy(const Identity& ident) const
{
    checkIdentity(ident, __FILE__, __LINE__);
    return ObjectPrx::_fromReference(
        _instance->referenceFactory()->create(ident, const_cast<ConnectionI*>(this)->shared_from_this()));
}

void
Ice::ConnectionI::setAdapterFromAdapter(const ObjectAdapterIPtr& adapter)
{
    std::lock_guard lock(_mutex);
    if (_state <= StateNotValidated || _state >= StateClosing)
    {
        return;
    }
    assert(adapter); // Called by ObjectAdapterI::setAdapterOnConnection
    _adapter = adapter;
}

#if defined(ICE_USE_IOCP)
bool
Ice::ConnectionI::startAsync(SocketOperation operation)
{
    if (_state >= StateClosed)
    {
        return false;
    }

    try
    {
        if (operation & SocketOperationWrite)
        {
            if (_observer)
            {
                _observer.startWrite(_writeStream);
            }

            if (_transceiver->startWrite(_writeStream) && !_sendStreams.empty())
            {
                // The whole message is written, assume it's sent now for at-most-once semantics.
                _sendStreams.front().isSent = true;
            }
        }
        else if (operation & SocketOperationRead)
        {
            if (_observer && !_readHeader)
            {
                _observer.startRead(_readStream);
            }

            _transceiver->startRead(_readStream);
        }
    }
    catch (const Ice::LocalException&)
    {
        setState(StateClosed, current_exception());
        return false;
    }
    return true;
}

bool
Ice::ConnectionI::finishAsync(SocketOperation operation)
{
    try
    {
        if (operation & SocketOperationWrite)
        {
            Buffer::Container::iterator start = _writeStream.i;
            _transceiver->finishWrite(_writeStream);
            if (_instance->traceLevels()->network >= 3 && _writeStream.i != start)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                out << "sent " << (_writeStream.i - start);
                if (!_endpoint->datagram())
                {
                    out << " of " << (_writeStream.b.end() - start);
                }
                out << " bytes via " << _endpoint->protocol() << "\n" << toString();
            }

            if (_observer)
            {
                _observer.finishWrite(_writeStream);
            }
        }
        else if (operation & SocketOperationRead)
        {
            Buffer::Container::iterator start = _readStream.i;
            _transceiver->finishRead(_readStream);
            if (_instance->traceLevels()->network >= 3 && _readStream.i != start)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                out << "received ";
                if (_endpoint->datagram())
                {
                    out << _readStream.b.size();
                }
                else
                {
                    out << (_readStream.i - start) << " of " << (_readStream.b.end() - start);
                }
                out << " bytes via " << _endpoint->protocol() << "\n" << toString();
            }

            if (_observer && !_readHeader)
            {
                _observer.finishRead(_readStream);
            }
        }
    }
    catch (const Ice::LocalException&)
    {
        setState(StateClosed, current_exception());
    }
    return _state < StateClosed;
}
#endif

void
Ice::ConnectionI::message(ThreadPoolCurrent& current)
{
    int upcallCount = 0;

    function<void(ConnectionIPtr)> connectionStartCompleted;
    vector<OutgoingMessage> sentCBs;
    function<bool(InputStream&)> messageUpcall;
    InputStream messageStream(_instance.get(), currentProtocolEncoding);

    ThreadPoolMessage<ConnectionI> msg(current, *this);
    {
        std::lock_guard lock(_mutex);

        ThreadPoolMessage<ConnectionI>::IOScope io(msg);
        if (!io)
        {
            return;
        }

        if (_state >= StateClosed)
        {
            return;
        }

        try
        {
            SocketOperation writeOp = SocketOperationNone;
            SocketOperation readOp = SocketOperationNone;

            // If writes are ready, write the data from the connection's write buffer (_writeStream)
            if (current.operation & SocketOperationWrite)
            {
                if (_observer)
                {
                    _observer.startWrite(_writeStream);
                }
                writeOp = write(_writeStream);
                if (_observer && !(writeOp & SocketOperationWrite))
                {
                    _observer.finishWrite(_writeStream);
                }
            }

            // If reads are ready, read the data into the connection's read buffer (_readStream). The data is read
            // until:
            // - the full message is read (the transport read returns SocketOperationNone) and
            //   the read buffer is fully filled
            // - the read operation on the transport can't continue without blocking
            if (current.operation & SocketOperationRead)
            {
                while (true)
                {
                    if (_observer && !_readHeader)
                    {
                        _observer.startRead(_readStream);
                    }

                    readOp = read(_readStream);
                    if (readOp & SocketOperationRead)
                    {
                        // Can't continue without blocking, exit out of the loop.
                        break;
                    }

                    if (_observer && !_readHeader)
                    {
                        assert(_readStream.i == _readStream.b.end());
                        _observer.finishRead(_readStream);
                    }

                    // If read header is true, we're reading a new Ice protocol message and we need to read
                    // the message header.
                    if (_readHeader)
                    {
                        // The next read will read the remainder of the message.
                        _readHeader = false;

                        if (_observer)
                        {
                            _observer->receivedBytes(static_cast<int>(headerSize));
                        }

                        //
                        // Connection is validated on first message. This is only used by
                        // setState() to check wether or not we can print a connection
                        // warning (a client might close the connection forcefully if the
                        // connection isn't validated, we don't want to print a warning
                        // in this case).
                        //
                        _validated = true;

                        // Full header should be read because the size of _readStream is always headerSize (14) when
                        // reading a new message (see the code that sets _readHeader = true).
                        ptrdiff_t pos = _readStream.i - _readStream.b.begin();
                        if (pos < headerSize)
                        {
                            //
                            // This situation is possible for small UDP packets.
                            //
                            throw IllegalMessageSizeException(__FILE__, __LINE__);
                        }

                        // Decode the header.
                        _readStream.i = _readStream.b.begin();
                        const byte* m;
                        _readStream.readBlob(m, static_cast<int32_t>(sizeof(magic)));
                        if (m[0] != magic[0] || m[1] != magic[1] || m[2] != magic[2] || m[3] != magic[3])
                        {
                            throw BadMagicException(__FILE__, __LINE__, "", Ice::ByteSeq(&m[0], &m[0] + sizeof(magic)));
                        }
                        ProtocolVersion pv;
                        _readStream.read(pv);
                        checkSupportedProtocol(pv);
                        EncodingVersion ev;
                        _readStream.read(ev);
                        checkSupportedProtocolEncoding(ev);

                        uint8_t messageType;
                        _readStream.read(messageType);
                        uint8_t compressByte;
                        _readStream.read(compressByte);
                        int32_t size;
                        _readStream.read(size);
                        if (size < headerSize)
                        {
                            throw IllegalMessageSizeException(__FILE__, __LINE__);
                        }

                        // Resize the read buffer to the message size.
                        if (size > static_cast<int32_t>(_messageSizeMax))
                        {
                            Ex::throwMemoryLimitException(
                                __FILE__,
                                __LINE__,
                                static_cast<size_t>(size),
                                _messageSizeMax);
                        }
                        if (static_cast<size_t>(size) > _readStream.b.size())
                        {
                            _readStream.b.resize(static_cast<size_t>(size));
                        }
                        _readStream.i = _readStream.b.begin() + pos;
                    }

                    if (_readStream.i != _readStream.b.end())
                    {
                        if (_endpoint->datagram())
                        {
                            throw DatagramLimitException(__FILE__, __LINE__); // The message was truncated.
                        }
                        continue;
                    }
                    break;
                }
            }

            // readOp and writeOp are set to the operations that the transport read or write calls from above returned.
            // They indicate which operations will need to be monitored by the thread pool's selector when this method
            // returns.
            SocketOperation newOp = static_cast<SocketOperation>(readOp | writeOp);

            // Operations that are ready. For example, if message was called with SocketOperationRead and the transport
            // read returned SocketOperationNone, reads are considered done: there's no additional data to read.
            SocketOperation readyOp = static_cast<SocketOperation>(current.operation & ~newOp);

            if (_state <= StateNotValidated)
            {
                // If the connection is still not validated and there's still data to read or write, continue waiting
                // for data to read or write.
                if (newOp)
                {
                    _threadPool->update(shared_from_this(), current.operation, newOp);
                    return;
                }

                // Initialize the connection if it's not initialized yet.
                if (_state == StateNotInitialized && !initialize(current.operation))
                {
                    return;
                }

                // Validate the connection if it's not validated yet.
                if (_state <= StateNotValidated && !validate(current.operation))
                {
                    return;
                }

                // The connection is validated and doesn't need additional data to be read or written. So unregister
                // it from the thread pool's selector.
                _threadPool->unregister(shared_from_this(), current.operation);

                // The connection starts in the holding state. It will be activated by the connection factory.
                setState(StateHolding);
                if (_connectionStartCompleted)
                {
                    connectionStartCompleted = std::move(_connectionStartCompleted);
                    ++upcallCount;
                    _connectionStartCompleted = nullptr;
                    _connectionStartFailed = nullptr;
                }
            }
            else // The connection is active or waits for the CloseConnection message.
            {
                assert(_state <= StateClosingPending);

                //
                // We parse messages first, if we receive a close
                // connection message we won't send more messages.
                //
                if (readyOp & SocketOperationRead)
                {
                    // At this point, the protocol message is fully read and can therefore be decoded by parseMessage.
                    // parseMessage returns the operation to wait for readiness next.
                    newOp =
                        static_cast<SocketOperation>(newOp | parseMessage(upcallCount, messageUpcall, messageStream));
                }

                if (readyOp & SocketOperationWrite)
                {
                    // At this point the message from _writeStream is fully written and the next message can be written.

                    newOp = static_cast<SocketOperation>(newOp | sendNextMessages(sentCBs));
                    if (!sentCBs.empty())
                    {
                        ++upcallCount;
                    }
                }

                // If the connection is not closed yet, we update the thread pool selector to wait for readiness of
                // read, write or both operations.
                if (_state < StateClosed)
                {
                    _threadPool->update(shared_from_this(), current.operation, newOp);
                }
            }

            if (upcallCount == 0)
            {
                return; // Nothing to dispatch we're done!
            }

            _upcallCount += upcallCount;

            // There's something to dispatch so we mark IO as completed to elect a new leader thread and let IO be
            // performed on this new leader thread while this thread continues with dispatching the up-calls.
            io.completed();
        }
        catch (const DatagramLimitException&) // Expected.
        {
            if (_warnUdp)
            {
                Warning out(_instance->initializationData().logger);
                out << "maximum datagram size of " << _readStream.i - _readStream.b.begin() << " exceeded";
            }
            _readStream.resize(headerSize);
            _readStream.i = _readStream.b.begin();
            _readHeader = true;
            return;
        }
        catch (const SocketException&)
        {
            setState(StateClosed, current_exception());
            return;
        }
        catch (const LocalException& ex)
        {
            if (_endpoint->datagram())
            {
                if (_warn)
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
                setState(StateClosed, current_exception());
            }
            return;
        }
    }

// executeFromThisThread dispatches to the correct DispatchQueue
#ifdef ICE_SWIFT
    auto stream = make_shared<InputStream>();
    stream->swap(messageStream);

    auto self = shared_from_this();
    _threadPool->executeFromThisThread(
        [self,
         connectionStartCompleted = std::move(connectionStartCompleted),
         sentCBs = std::move(sentCBs),
         messageUpcall = std::move(messageUpcall),
         stream]()
        { self->upcall(std::move(connectionStartCompleted), std::move(sentCBs), std::move(messageUpcall), *stream); },
        self);
#else
    if (!_hasExecutor) // Optimization, call dispatch() directly if there's no executor.
    {
        upcall(std::move(connectionStartCompleted), std::move(sentCBs), std::move(messageUpcall), messageStream);
    }
    else
    {
        auto stream = make_shared<InputStream>();
        stream->swap(messageStream);

        auto self = shared_from_this();
        _threadPool->executeFromThisThread(
            [self,
             connectionStartCompleted = std::move(connectionStartCompleted),
             sentCBs = std::move(sentCBs),
             messageUpcall = std::move(messageUpcall),
             stream]() {
                self->upcall(
                    std::move(connectionStartCompleted),
                    std::move(sentCBs),
                    std::move(messageUpcall),
                    *stream);
            },
            self);
    }
#endif
}

void
ConnectionI::upcall(
    function<void(ConnectionIPtr)> connectionStartCompleted,
    const vector<OutgoingMessage>& sentCBs,
    function<bool(InputStream&)> messageUpcall,
    InputStream& messageStream)
{
    int completedUpcallCount = 0;

    //
    // Notify the factory that the connection establishment and
    // validation has completed.
    //
    if (connectionStartCompleted)
    {
        connectionStartCompleted(shared_from_this());
        ++completedUpcallCount;
    }

    //
    // Notify AMI calls that the message was sent.
    //
    if (!sentCBs.empty())
    {
        for (vector<OutgoingMessage>::const_iterator p = sentCBs.begin(); p != sentCBs.end(); ++p)
        {
#if defined(ICE_USE_IOCP)
            if (p->invokeSent)
            {
                p->outAsync->invokeSent();
            }
            if (p->receivedReply)
            {
                auto o = dynamic_pointer_cast<OutgoingAsync>(p->outAsync);
                if (o->response())
                {
                    o->invokeResponse();
                }
            }
#else
            p->outAsync->invokeSent();
#endif
        }
        ++completedUpcallCount;
    }

    if (messageUpcall && messageUpcall(messageStream))
    {
        ++completedUpcallCount;
    }

    //
    // Decrease the upcall count.
    //
    bool finished = false;
    if (completedUpcallCount > 0)
    {
        std::lock_guard lock(_mutex);
        _upcallCount -= completedUpcallCount;
        if (_upcallCount == 0)
        {
            // Only initiate shutdown if not already initiated. It might have already been initiated if the sent
            // callback or AMI callback was called when the connection was in the closing state.
            if (_state == StateClosing)
            {
                try
                {
                    initiateShutdown();
                }
                catch (const LocalException&)
                {
                    setState(StateClosed, current_exception());
                }
            }
            else if (_state == StateFinished)
            {
                finished = true;
                if (_observer)
                {
                    _observer.detach();
                }
            }
            _conditionVariable.notify_all();
        }
    }
    if (finished && _removeFromFactory)
    {
        _removeFromFactory(shared_from_this());
    }
}

void
Ice::ConnectionI::finished(ThreadPoolCurrent& current, bool close)
{
    // If there are no callbacks to call, we don't call ioCompleted() since we're not going to call code that will
    // potentially block (this avoids promoting a new leader and unecessary thread creation, especially if this is
    // called on shutdown).
    if (!_connectionStartCompleted && !_connectionStartFailed && _sendStreams.empty() && _asyncRequests.empty() &&
        !_closeCallback && !_heartbeatCallback)
    {
        finish(close);
        return;
    }

    current.ioCompleted();

    // executeFromThisThread dispatches to the correct DispatchQueue
#ifdef ICE_SWIFT
    auto self = shared_from_this();
    _threadPool->executeFromThisThread([self, close]() { self->finish(close); }, self);
#else
    if (!_hasExecutor) // Optimization, call finish() directly if there's no executor.
    {
        finish(close);
    }
    else
    {
        auto self = shared_from_this();
        _threadPool->executeFromThisThread([self, close]() { self->finish(close); }, self);
    }
#endif
}

void
Ice::ConnectionI::finish(bool close)
{
    if (!_initialized)
    {
        if (_instance->traceLevels()->network >= 2)
        {
            string verb = _connector ? "establish" : "accept";
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);

            try
            {
                rethrow_exception(_exception);
            }
            catch (const std::exception& ex)
            {
                out << "failed to " << verb << " " << _endpoint->protocol() << " connection\n"
                    << toString() << "\n"
                    << ex;
            }
        }
    }
    else
    {
        if (_instance->traceLevels()->network >= 1)
        {
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
            out << "closed " << _endpoint->protocol() << " connection\n" << toString();

            try
            {
                rethrow_exception(_exception);
            }
            catch (const CloseConnectionException&)
            {
            }
            catch (const ConnectionManuallyClosedException&)
            {
            }
            catch (const ConnectionClosedException&)
            {
            }
            catch (const ConnectionIdleException&)
            {
            }
            catch (const CommunicatorDestroyedException&)
            {
            }
            catch (const ObjectAdapterDeactivatedException&)
            {
            }
            catch (const std::exception& ex)
            {
                out << "\n" << ex;
            }
        }
    }

    if (close)
    {
        try
        {
            _transceiver->close();
        }
        catch (const Ice::LocalException& ex)
        {
            Error out(_logger);
            out << "unexpected connection exception:\n" << ex << '\n' << _desc;
        }
    }

    if (_connectionStartFailed)
    {
        assert(_exception);
        _connectionStartFailed(shared_from_this(), _exception);
        _connectionStartFailed = nullptr;
        _connectionStartCompleted = nullptr;
    }

    if (!_sendStreams.empty())
    {
        if (!_writeStream.b.empty())
        {
            //
            // Return the stream to the outgoing call. This is important for
            // retriable AMI calls which are not marshaled again.
            //
            OutgoingMessage* message = &_sendStreams.front();
            _writeStream.swap(*message->stream);

#if defined(ICE_USE_IOCP)
            //
            // The current message might be sent but not yet removed from _sendStreams. If
            // the response has been received in the meantime, we remove the message from
            // _sendStreams to not call finished on a message which is already done.
            //
            if (message->isSent || message->receivedReply)
            {
                if (message->sent() && message->invokeSent)
                {
                    message->outAsync->invokeSent();
                }
                if (message->receivedReply)
                {
                    OutgoingAsyncPtr outAsync = dynamic_pointer_cast<OutgoingAsync>(message->outAsync);
                    if (outAsync->response())
                    {
                        outAsync->invokeResponse();
                    }
                }
                _sendStreams.pop_front();
            }
#endif
        }

        for (deque<OutgoingMessage>::iterator o = _sendStreams.begin(); o != _sendStreams.end(); ++o)
        {
            o->completed(_exception);
            if (o->requestId) // Make sure finished isn't called twice.
            {
                if (_asyncRequests.erase(o->requestId) == 1 && isAtRest())
                {
                    scheduleInactivityTimerTask();
                }
            }
        }

        _sendStreams.clear();
    }

    for (map<int32_t, OutgoingAsyncBasePtr>::const_iterator q = _asyncRequests.begin(); q != _asyncRequests.end(); ++q)
    {
        if (q->second->exception(_exception))
        {
            q->second->invokeException();
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

    if (_closeCallback)
    {
        closeCallback(_closeCallback);
        _closeCallback = nullptr;
    }

    _heartbeatCallback = nullptr;

    // This must be done last as this will cause waitUntilFinished() to return (and communicator
    // objects such as the timer might be destroyed too).
    bool finished = false;
    {
        std::lock_guard lock(_mutex);
        setState(StateFinished);

        if (_upcallCount == 0)
        {
            finished = true;
            if (_observer)
            {
                _observer.detach();
            }
        }
    }
    if (finished && _removeFromFactory)
    {
        _removeFromFactory(shared_from_this());
    }
}

string
Ice::ConnectionI::toString() const noexcept
{
    return _desc; // No mutex lock, _desc is immutable.
}

NativeInfoPtr
Ice::ConnectionI::getNativeInfo()
{
    return _transceiver->getNativeInfo();
}

string
Ice::ConnectionI::type() const noexcept
{
    return _type; // No mutex lock, _type is immutable.
}

ConnectionInfoPtr
Ice::ConnectionI::getInfo() const
{
    std::lock_guard lock(_mutex);
    if (_state >= StateClosed)
    {
        rethrow_exception(_exception);
    }
    return initConnectionInfo();
}

void
Ice::ConnectionI::setBufferSize(int32_t rcvSize, int32_t sndSize)
{
    std::lock_guard lock(_mutex);
    if (_state >= StateClosed)
    {
        rethrow_exception(_exception);
    }
    _transceiver->setBufferSize(rcvSize, sndSize);
    _info = 0; // Invalidate the cached connection info
}

void
Ice::ConnectionI::exception(std::exception_ptr ex)
{
    std::lock_guard lock(_mutex);
    setState(StateClosed, ex);
}

Ice::ConnectionI::ConnectionI(
    const CommunicatorPtr& communicator,
    const InstancePtr& instance,
    const TransceiverPtr& transceiver,
    const ConnectorPtr& connector,
    const EndpointIPtr& endpoint,
    const shared_ptr<ObjectAdapterI>& adapter,
    std::function<void(const ConnectionIPtr&)> removeFromFactory,
    const ConnectionOptions& options) noexcept
    : _communicator(communicator),
      _instance(instance),
      _transceiver(transceiver),
      _desc(transceiver->toString()),
      _type(transceiver->protocol()),
      _connector(connector),
      _endpoint(endpoint),
      _adapter(adapter),
      _hasExecutor(_instance->initializationData().executor), // Cached for better performance.
      _logger(_instance->initializationData().logger),        // Cached for better performance.
      _traceLevels(_instance->traceLevels()),                 // Cached for better performance.
      _timer(_instance->timer()),                             // Cached for better performance.
      _connectTimeout(options.connectTimeout),
      _closeTimeout(options.closeTimeout), // not used for datagram connections
      // suppress inactivity timeout for datagram connections
      _inactivityTimeout(endpoint->datagram() ? chrono::seconds::zero() : options.inactivityTimeout),
      _removeFromFactory(std::move(removeFromFactory)),
      _warn(_instance->initializationData().properties->getIcePropertyAsInt("Ice.Warn.Connections") > 0),
      _warnUdp(_instance->initializationData().properties->getIcePropertyAsInt("Ice.Warn.Datagrams") > 0),
      _compressionLevel(1),
      _nextRequestId(1),
      _asyncRequestsHint(_asyncRequests.end()),
      _messageSizeMax(adapter ? adapter->messageSizeMax() : _instance->messageSizeMax()),
      _batchRequestQueue(new BatchRequestQueue(instance, endpoint->datagram())),
      _readStream(_instance.get(), Ice::currentProtocolEncoding),
      _readHeader(false),
      _writeStream(_instance.get(), Ice::currentProtocolEncoding),
      _upcallCount(0),
      _state(StateNotInitialized),
      _shutdownInitiated(false),
      _initialized(false),
      _validated(false)
{
    const Ice::PropertiesPtr& properties = _instance->initializationData().properties;

    int& compressionLevel = const_cast<int&>(_compressionLevel);
    compressionLevel = properties->getIcePropertyAsInt("Ice.Compression.Level");
    if (compressionLevel < 1)
    {
        compressionLevel = 1;
    }
    else if (compressionLevel > 9)
    {
        compressionLevel = 9;
    }
}

Ice::ConnectionIPtr
Ice::ConnectionI::create(
    const CommunicatorPtr& communicator,
    const InstancePtr& instance,
    const TransceiverPtr& transceiver,
    const ConnectorPtr& connector,
    const EndpointIPtr& endpoint,
    const shared_ptr<ObjectAdapterI>& adapter,
    std::function<void(const ConnectionIPtr&)> removeFromFactory,
    const ConnectionOptions& options)
{
    shared_ptr<IdleTimeoutTransceiverDecorator> decoratedTransceiver;
    if (options.idleTimeout > chrono::milliseconds::zero() && !endpoint->datagram())
    {
        decoratedTransceiver = make_shared<IdleTimeoutTransceiverDecorator>(
            transceiver,
            options.idleTimeout,
            options.enableIdleCheck,
            instance->timer());
    }

    Ice::ConnectionIPtr connection(new ConnectionI(
        communicator,
        instance,
        decoratedTransceiver ? decoratedTransceiver : transceiver,
        connector,
        endpoint,
        adapter,
        std::move(removeFromFactory),
        options));

    if (connection->_inactivityTimeout > chrono::seconds::zero())
    {
        connection->_inactivityTimerTask = make_shared<InactivityTimerTask>(connection);
    }

    if (decoratedTransceiver)
    {
        decoratedTransceiver->decoratorInit(connection);
    }

    if (adapter)
    {
        const_cast<ThreadPoolPtr&>(connection->_threadPool) = adapter->getThreadPool();
    }
    else
    {
        const_cast<ThreadPoolPtr&>(connection->_threadPool) = connection->_instance->clientThreadPool();
    }
    connection->_threadPool->initialize(connection);
    return connection;
}

Ice::ConnectionI::~ConnectionI()
{
    assert(!_connectionStartCompleted);
    assert(!_connectionStartFailed);
    assert(!_closeCallback);
    assert(!_heartbeatCallback);
    assert(_state == StateFinished);
    assert(_upcallCount == 0);
    assert(_sendStreams.empty());
    assert(_asyncRequests.empty());
}

void
Ice::ConnectionI::setState(State state, exception_ptr ex)
{
    //
    // If setState() is called with an exception, then only closed and
    // closing states are permissible.
    //
    assert(state >= StateClosing);

    if (_state == state) // Don't switch twice.
    {
        return;
    }

    if (!_exception)
    {
        //
        // If we are in closed state, an exception must be set.
        //
        assert(_state != StateClosed);
        _exception = ex;
        //
        // We don't warn if we are not validated.
        //
        if (_warn && _validated)
        {
            //
            // Don't warn about certain expected exceptions.
            //
            try
            {
                rethrow_exception(ex);
            }
            catch (const CloseConnectionException&)
            {
            }
            catch (const ConnectionManuallyClosedException&)
            {
            }
            catch (const ConnectionClosedException&)
            {
            }
            catch (const ConnectionIdleException&)
            {
            }
            catch (const CommunicatorDestroyedException&)
            {
            }
            catch (const ObjectAdapterDeactivatedException&)
            {
            }
            catch (const ConnectionLostException& e)
            {
                if (_state < StateClosing)
                {
                    Warning out(_logger);
                    out << "connection exception:\n" << e << '\n' << _desc;
                }
            }
            catch (const std::exception& e)
            {
                Warning out(_logger);
                out << "connection exception:\n" << e << '\n' << _desc;
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
    if (_endpoint->datagram() && state == StateClosing)
    {
        state = StateClosed;
    }

    //
    // Skip graceful shutdown if we are destroyed before validation.
    //
    if (_state <= StateNotValidated && state == StateClosing)
    {
        state = StateClosed;
    }

    if (_state == state) // Don't switch twice.
    {
        return;
    }

    try
    {
        switch (state)
        {
            case StateNotInitialized:
            {
                assert(false);
                break;
            }

            case StateNotValidated:
            {
                if (_state != StateNotInitialized)
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
                if (_state != StateHolding && _state != StateNotValidated)
                {
                    return;
                }

                _threadPool->_register(shared_from_this(), SocketOperationRead);
                break;
            }

            case StateHolding:
            {
                //
                // Can only switch from active or not validated to
                // holding.
                //
                if (_state != StateActive && _state != StateNotValidated)
                {
                    return;
                }

                // We don't shut down the connection due to inactivity when it's in the Holding state.
                cancelInactivityTimerTask();

                if (_state == StateActive)
                {
                    _threadPool->unregister(shared_from_this(), SocketOperationRead);
                }
                break;
            }

            case StateClosing:
            case StateClosingPending:
            {
                //
                // Can't change back from closing pending.
                //
                if (_state >= StateClosingPending)
                {
                    return;
                }
                break;
            }

            case StateClosed:
            {
                if (_state == StateFinished)
                {
                    return;
                }

                _batchRequestQueue->destroy(_exception);

                //
                // Don't need to close now for connections so only close the transceiver
                // if the selector request it.
                //
                if (_threadPool->finish(shared_from_this(), false))
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
    catch (const Ice::LocalException& ex)
    {
        Error out(_logger);
        out << "unexpected connection exception:\n" << ex << '\n' << _desc;
    }

    if (_instance->initializationData().observer)
    {
        ConnectionState oldState = toConnectionState(_state);
        ConnectionState newState = toConnectionState(state);
        if (oldState != newState)
        {
            _observer.attach(
                _instance->initializationData()
                    .observer->getConnectionObserver(initConnectionInfo(), _endpoint, newState, _observer.get()));
        }
        if (_observer && state == StateClosed && _exception)
        {
            try
            {
                rethrow_exception(_exception);
            }
            catch (const CloseConnectionException&)
            {
            }
            catch (const ConnectionManuallyClosedException&)
            {
            }
            catch (const ConnectionClosedException&)
            {
            }
            catch (const ConnectionIdleException&)
            {
            }
            catch (const CommunicatorDestroyedException&)
            {
            }
            catch (const ObjectAdapterDeactivatedException&)
            {
            }
            catch (const ConnectionLostException& ex)
            {
                if (_state < StateClosing)
                {
                    _observer->failed(ex.ice_id());
                }
            }
            catch (const Ice::Exception& ex)
            {
                _observer->failed(ex.ice_id());
            }
            catch (const std::exception& ex)
            {
                _observer->failed(ex.what());
            }
        }
    }
    _state = state;
    _conditionVariable.notify_all();

    if (_state == StateClosing && _upcallCount == 0)
    {
        try
        {
            initiateShutdown();
        }
        catch (const LocalException&)
        {
            setState(StateClosed, current_exception());
        }
    }

    if (isAtRest())
    {
        // If the connection became active and there is no outstanding invocation or dispatch (very common case), we
        // schedule the inactivity timer task.
        scheduleInactivityTimerTask();
    }
}

void
Ice::ConnectionI::initiateShutdown()
{
    assert(_state == StateClosing && _upcallCount == 0);

    if (_shutdownInitiated)
    {
        return;
    }
    _shutdownInitiated = true;

    if (!_endpoint->datagram())
    {
        //
        // Before we shut down, we send a close connection message.
        //
        OutputStream os(_instance.get(), Ice::currentProtocolEncoding);
        os.write(magic[0]);
        os.write(magic[1]);
        os.write(magic[2]);
        os.write(magic[3]);
        os.write(currentProtocol);
        os.write(currentProtocolEncoding);
        os.write(closeConnectionMsg);
        os.write(static_cast<uint8_t>(1)); // compression status: compression supported but not used.
        os.write(headerSize);              // Message size.

        if (_closeTimeout > chrono::seconds::zero())
        {
            _timer->schedule(make_shared<CloseTimerTask>(shared_from_this()), _closeTimeout);
        }

        OutgoingMessage message(&os, false);
        if (sendMessage(message) & AsyncStatusSent)
        {
            setState(StateClosingPending);

            //
            // Notify the transceiver of the graceful connection closure.
            //
            SocketOperation op = _transceiver->closing(true, _exception);
            if (op)
            {
                _threadPool->_register(shared_from_this(), op);
            }
        }
    }
}

void
Ice::ConnectionI::idleCheck(
    const IceUtil::TimerTaskPtr& idleCheckTimerTask,
    const chrono::seconds& idleTimeout) noexcept
{
    std::lock_guard lock(_mutex);
    if (_state == StateActive || _state == StateHolding)
    {
        // _timer->cancel(task) returns true if a concurrent read rescheduled the timer task.
        if (_transceiver->isWaitingToBeRead() || _timer->cancel(idleCheckTimerTask))
        {
            // Schedule or reschedule timer task. Reschedule in the rare case where a concurrent read scheduled the task
            // already.
            _timer->reschedule(idleCheckTimerTask, idleTimeout);

            if (_instance->traceLevels()->network >= 3)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                out << "the idle check scheduled a new idle check in " << idleTimeout.count()
                    << "s because the connection is waiting to be read\n";
                out << _transceiver->toDetailedString();
            }
        }
        else
        {
            if (_instance->traceLevels()->network >= 1)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                out << "connection aborted by the idle check because it did not receive any byte for "
                    << idleTimeout.count() << "s\n";
                out << _transceiver->toDetailedString();
            }

            setState(StateClosed, make_exception_ptr(ConnectionIdleException(__FILE__, __LINE__)));
        }
    }
    // else, nothing to do
}

void
Ice::ConnectionI::inactivityCheck() noexcept
{
    // Called by the InactivityTimerTask.
    std::lock_guard lock(_mutex);
    if (isAtRest())
    {
        setState(
            StateClosing,
            make_exception_ptr(ConnectionClosedException{
                __FILE__,
                __LINE__,
                "connection closed because it remained inactive for longer than the inactivity timeout"}));
    }
}

void
Ice::ConnectionI::connectTimedOut() noexcept
{
    std::lock_guard lock(_mutex);
    if (_state < StateActive)
    {
        setState(StateClosed, make_exception_ptr(ConnectTimeoutException(__FILE__, __LINE__)));
    }
    // else ignore since we're already connected.
}

void
Ice::ConnectionI::closeTimedOut() noexcept
{
    std::lock_guard lock(_mutex);
    if (_state < StateClosed)
    {
        setState(StateClosed, make_exception_ptr(CloseTimeoutException(__FILE__, __LINE__)));
    }
    // else ignore since we're already closed.
}

void
Ice::ConnectionI::sendHeartbeat() noexcept
{
    assert(!_endpoint->datagram());

    lock_guard lock(_mutex);
    if (_state == StateActive || _state == StateHolding)
    {
        OutputStream os(_instance.get(), Ice::currentProtocolEncoding);
        os.write(magic[0]);
        os.write(magic[1]);
        os.write(magic[2]);
        os.write(magic[3]);
        os.write(currentProtocol);
        os.write(currentProtocolEncoding);
        os.write(validateConnectionMsg);
        os.write(static_cast<uint8_t>(0)); // Compression status (always zero for validate connection).
        os.write(headerSize);              // Message size.
        os.i = os.b.begin();
        try
        {
            OutgoingMessage message(&os, false);
            sendMessage(message);
        }
        catch (...)
        {
            setState(StateClosed, current_exception());
        }
    }
    // else nothing to do
}

void
Ice::ConnectionI::sendResponse(OutgoingResponse response, uint8_t compress)
{
    bool isTwoWay = !_endpoint->datagram() && response.current().requestId != 0;

    bool finished = false;
    try
    {
        std::unique_lock lock(_mutex);
        assert(_state > StateNotValidated);

        try
        {
            if (--_upcallCount == 0)
            {
                if (_state == StateFinished)
                {
                    finished = true;
                    if (_observer)
                    {
                        _observer.detach();
                    }
                }
                _conditionVariable.notify_all();
            }

            if (_state >= StateClosed)
            {
                exception_ptr exception = _exception;
                assert(exception);

                if (finished && _removeFromFactory)
                {
                    lock.unlock();
                    _removeFromFactory(shared_from_this());
                }

                rethrow_exception(exception);
            }
            assert(!finished);

            if (isTwoWay)
            {
                OutgoingMessage message(&response.outputStream(), compress > 0);
                sendMessage(message);
            }

            if (_state == StateClosing && _upcallCount == 0)
            {
                initiateShutdown();
            }

            --_dispatchCount;
            if (isAtRest())
            {
                scheduleInactivityTimerTask();
            }
        }
        catch (const LocalException&)
        {
            setState(StateClosed, current_exception());
        }
    }
    catch (...)
    {
        dispatchException(current_exception(), 1);
    }
}

bool
Ice::ConnectionI::initialize(SocketOperation operation)
{
    SocketOperation s = _transceiver->initialize(_readStream, _writeStream);
    if (s != SocketOperationNone)
    {
        _threadPool->update(shared_from_this(), operation, s);
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
    if (!_endpoint->datagram()) // Datagram connections are always implicitly validated.
    {
        if (_adapter) // The server side has the active role for connection validation.
        {
            if (_writeStream.b.empty())
            {
                _writeStream.write(magic[0]);
                _writeStream.write(magic[1]);
                _writeStream.write(magic[2]);
                _writeStream.write(magic[3]);
                _writeStream.write(currentProtocol);
                _writeStream.write(currentProtocolEncoding);
                _writeStream.write(validateConnectionMsg);
                _writeStream.write(
                    static_cast<uint8_t>(0));   // Compression status (always zero for validate connection).
                _writeStream.write(headerSize); // Message size.
                _writeStream.i = _writeStream.b.begin();
                traceSend(_writeStream, _logger, _traceLevels);
            }

            if (_observer)
            {
                _observer.startWrite(_writeStream);
            }

            if (_writeStream.i != _writeStream.b.end())
            {
                SocketOperation op = write(_writeStream);
                if (op)
                {
                    _threadPool->update(shared_from_this(), operation, op);
                    return false;
                }
            }

            if (_observer)
            {
                _observer.finishWrite(_writeStream);
            }
        }
        else // The client side has the passive role for connection validation.
        {
            if (_readStream.b.empty())
            {
                _readStream.b.resize(headerSize);
                _readStream.i = _readStream.b.begin();
            }

            if (_observer)
            {
                _observer.startRead(_readStream);
            }

            if (_readStream.i != _readStream.b.end())
            {
                SocketOperation op = read(_readStream);
                if (op)
                {
                    _threadPool->update(shared_from_this(), operation, op);
                    return false;
                }
            }

            if (_observer)
            {
                _observer.finishRead(_readStream);
            }

            _validated = true;

            assert(_readStream.i == _readStream.b.end());
            _readStream.i = _readStream.b.begin();
            byte m[4];
            _readStream.read(m[0]);
            _readStream.read(m[1]);
            _readStream.read(m[2]);
            _readStream.read(m[3]);
            if (m[0] != magic[0] || m[1] != magic[1] || m[2] != magic[2] || m[3] != magic[3])
            {
                throw BadMagicException(__FILE__, __LINE__, "", Ice::ByteSeq(&m[0], &m[0] + sizeof(magic)));
            }
            ProtocolVersion pv;
            _readStream.read(pv);
            checkSupportedProtocol(pv);
            EncodingVersion ev;
            _readStream.read(ev);
            checkSupportedProtocolEncoding(ev);
            uint8_t messageType;
            _readStream.read(messageType);
            if (messageType != validateConnectionMsg)
            {
                throw ConnectionNotValidatedException(__FILE__, __LINE__);
            }
            uint8_t compress;
            _readStream.read(compress); // Ignore compression status for validate connection.
            int32_t size;
            _readStream.read(size);
            if (size != headerSize)
            {
                throw IllegalMessageSizeException(__FILE__, __LINE__);
            }
            traceRecv(_readStream, _logger, _traceLevels);
        }
    }

    _writeStream.resize(0);
    _writeStream.i = _writeStream.b.begin();

    _readStream.resize(headerSize);
    _readStream.i = _readStream.b.begin();
    _readHeader = true;

    if (_instance->traceLevels()->network >= 1)
    {
        Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
        if (_endpoint->datagram())
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
Ice::ConnectionI::sendNextMessages(vector<OutgoingMessage>& callbacks)
{
    if (_sendStreams.empty())
    {
        return SocketOperationNone;
    }
    else if (_state == StateClosingPending && _writeStream.i == _writeStream.b.begin())
    {
        // Message wasn't sent, empty the _writeStream, we're not going to send more data.
        OutgoingMessage* message = &_sendStreams.front();
        _writeStream.swap(*message->stream);
        return SocketOperationNone;
    }

    assert(!_writeStream.b.empty() && _writeStream.i == _writeStream.b.end());
    try
    {
        while (true)
        {
            //
            // Notify the message that it was sent.
            //
            OutgoingMessage* message = &_sendStreams.front();
            if (message->stream)
            {
                _writeStream.swap(*message->stream);
                if (message->sent())
                {
                    callbacks.push_back(*message);
                }
            }
            _sendStreams.pop_front();

            //
            // If there's nothing left to send, we're done.
            //
            if (_sendStreams.empty())
            {
                break;
            }

            //
            // If we are in the closed state or if the close is
            // pending, don't continue sending.
            //
            // This can occur if parseMessage (called before
            // sendNextMessages by message()) closes the connection.
            //
            if (_state >= StateClosingPending)
            {
                return SocketOperationNone;
            }

            //
            // Otherwise, prepare the next message stream for writing.
            //
            message = &_sendStreams.front();
            assert(!message->stream->i);
#ifdef ICE_HAS_BZIP2
            if (message->compress && message->stream->b.size() >= 100) // Only compress messages > 100 bytes.
            {
                //
                // Message compressed. Request compressed response, if any.
                //
                message->stream->b[9] = byte{2};

                //
                // Do compression.
                //
                OutputStream stream(_instance.get(), Ice::currentProtocolEncoding);
                doCompress(*message->stream, stream);

                traceSend(*message->stream, _logger, _traceLevels);

                message->adopt(&stream); // Adopt the compressed stream.
                message->stream->i = message->stream->b.begin();
            }
            else
            {
#endif
                if (message->compress)
                {
                    //
                    // Message not compressed. Request compressed response, if any.
                    //
                    message->stream->b[9] = byte{1};
                }

                //
                // No compression, just fill in the message size.
                //
                int32_t sz = static_cast<int32_t>(message->stream->b.size());
                const byte* p = reinterpret_cast<const byte*>(&sz);
                if constexpr (endian::native == endian::big)
                {
                    reverse_copy(p, p + sizeof(int32_t), message->stream->b.begin() + 10);
                }
                else
                {
                    copy(p, p + sizeof(int32_t), message->stream->b.begin() + 10);
                }
                message->stream->i = message->stream->b.begin();
                traceSend(*message->stream, _logger, _traceLevels);

#ifdef ICE_HAS_BZIP2
            }
#endif
            _writeStream.swap(*message->stream);

            //
            // Send the message.
            //
            if (_observer)
            {
                _observer.startWrite(_writeStream);
            }
            assert(_writeStream.i);
            if (_writeStream.i != _writeStream.b.end())
            {
                SocketOperation op = write(_writeStream);
                if (op)
                {
                    return op;
                }
            }
            if (_observer)
            {
                _observer.finishWrite(_writeStream);
            }
        }

        //
        // If all the messages were sent and we are in the closing state, we schedule
        // the close timeout to wait for the peer to close the connection.
        //
        if (_state == StateClosing && _shutdownInitiated)
        {
            setState(StateClosingPending);
            SocketOperation op = _transceiver->closing(true, _exception);
            if (op)
            {
                return op;
            }
        }
    }
    catch (const Ice::LocalException&)
    {
        setState(StateClosed, current_exception());
    }
    return SocketOperationNone;
}

AsyncStatus
Ice::ConnectionI::sendMessage(OutgoingMessage& message)
{
    assert(_state >= StateActive);
    assert(_state < StateClosed);

    message.stream->i = 0; // Reset the message stream iterator before starting sending the message.

    if (!_sendStreams.empty())
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
    if (message.compress && message.stream->b.size() >= 100) // Only compress messages larger than 100 bytes.
    {
        //
        // Message compressed. Request compressed response, if any.
        //
        message.stream->b[9] = byte{2};

        //
        // Do compression.
        //
        OutputStream stream(_instance.get(), Ice::currentProtocolEncoding);
        doCompress(*message.stream, stream);
        stream.i = stream.b.begin();

        traceSend(*message.stream, _logger, _traceLevels);

        //
        // Send the message without blocking.
        //
        if (_observer)
        {
            _observer.startWrite(stream);
        }
        op = write(stream);
        if (!op)
        {
            if (_observer)
            {
                _observer.finishWrite(stream);
            }

            AsyncStatus status = AsyncStatusSent;
            if (message.sent())
            {
                status = static_cast<AsyncStatus>(status | AsyncStatusInvokeSentCallback);
            }
            return status;
        }

        _sendStreams.push_back(message);
        _sendStreams.back().adopt(&stream);
    }
    else
    {
#endif
        if (message.compress)
        {
            //
            // Message not compressed. Request compressed response, if any.
            //
            message.stream->b[9] = byte{1};
        }

        //
        // No compression, just fill in the message size.
        //
        int32_t sz = static_cast<int32_t>(message.stream->b.size());
        const byte* p = reinterpret_cast<const byte*>(&sz);
        if constexpr (endian::native == endian::big)
        {
            reverse_copy(p, p + sizeof(int32_t), message.stream->b.begin() + 10);
        }
        else
        {
            copy(p, p + sizeof(int32_t), message.stream->b.begin() + 10);
        }
        message.stream->i = message.stream->b.begin();

        traceSend(*message.stream, _logger, _traceLevels);

        //
        // Send the message without blocking.
        //
        if (_observer)
        {
            _observer.startWrite(*message.stream);
        }
        op = write(*message.stream);
        if (!op)
        {
            if (_observer)
            {
                _observer.finishWrite(*message.stream);
            }
            AsyncStatus status = AsyncStatusSent;
            if (message.sent())
            {
                status = static_cast<AsyncStatus>(status | AsyncStatusInvokeSentCallback);
            }
            return status;
        }

        _sendStreams.push_back(message);
        _sendStreams.back().adopt(0); // Adopt the stream.
#ifdef ICE_HAS_BZIP2
    }
#endif

    _writeStream.swap(*_sendStreams.back().stream);
    _threadPool->_register(shared_from_this(), op);
    return AsyncStatusQueued;
}

#ifdef ICE_HAS_BZIP2
static string
getBZ2Error(int bzError)
{
    if (bzError == BZ_RUN_OK)
    {
        return ": BZ_RUN_OK";
    }
    else if (bzError == BZ_FLUSH_OK)
    {
        return ": BZ_FLUSH_OK";
    }
    else if (bzError == BZ_FINISH_OK)
    {
        return ": BZ_FINISH_OK";
    }
    else if (bzError == BZ_STREAM_END)
    {
        return ": BZ_STREAM_END";
    }
    else if (bzError == BZ_CONFIG_ERROR)
    {
        return ": BZ_CONFIG_ERROR";
    }
    else if (bzError == BZ_SEQUENCE_ERROR)
    {
        return ": BZ_SEQUENCE_ERROR";
    }
    else if (bzError == BZ_PARAM_ERROR)
    {
        return ": BZ_PARAM_ERROR";
    }
    else if (bzError == BZ_MEM_ERROR)
    {
        return ": BZ_MEM_ERROR";
    }
    else if (bzError == BZ_DATA_ERROR)
    {
        return ": BZ_DATA_ERROR";
    }
    else if (bzError == BZ_DATA_ERROR_MAGIC)
    {
        return ": BZ_DATA_ERROR_MAGIC";
    }
    else if (bzError == BZ_IO_ERROR)
    {
        return ": BZ_IO_ERROR";
    }
    else if (bzError == BZ_UNEXPECTED_EOF)
    {
        return ": BZ_UNEXPECTED_EOF";
    }
    else if (bzError == BZ_OUTBUFF_FULL)
    {
        return ": BZ_OUTBUFF_FULL";
    }
    else
    {
        return "";
    }
}

void
Ice::ConnectionI::doCompress(OutputStream& uncompressed, OutputStream& compressed)
{
    const byte* p;

    //
    // Compress the message body, but not the header.
    //
    unsigned int uncompressedLen = static_cast<unsigned int>(uncompressed.b.size() - headerSize);
    unsigned int compressedLen = static_cast<unsigned int>(uncompressedLen * 1.01 + 600);
    compressed.b.resize(headerSize + sizeof(int32_t) + compressedLen);
    int bzError = BZ2_bzBuffToBuffCompress(
        reinterpret_cast<char*>(&compressed.b[0]) + headerSize + sizeof(int32_t),
        &compressedLen,
        reinterpret_cast<char*>(&uncompressed.b[0]) + headerSize,
        uncompressedLen,
        _compressionLevel,
        0,
        0);
    if (bzError != BZ_OK)
    {
        throw CompressionException(__FILE__, __LINE__, "BZ2_bzBuffToBuffCompress failed" + getBZ2Error(bzError));
    }
    compressed.b.resize(headerSize + sizeof(int32_t) + compressedLen);

    //
    // Write the size of the compressed stream into the header of the
    // uncompressed stream. Since the header will be copied, this size
    // will also be in the header of the compressed stream.
    //
    int32_t compressedSize = static_cast<int32_t>(compressed.b.size());
    p = reinterpret_cast<const byte*>(&compressedSize);
    if constexpr (endian::native == endian::big)
    {
        reverse_copy(p, p + sizeof(int32_t), uncompressed.b.begin() + 10);
    }
    else
    {
        copy(p, p + sizeof(int32_t), uncompressed.b.begin() + 10);
    }

    //
    // Add the size of the uncompressed stream before the message body
    // of the compressed stream.
    //
    int32_t uncompressedSize = static_cast<int32_t>(uncompressed.b.size());
    p = reinterpret_cast<const byte*>(&uncompressedSize);
    if constexpr (endian::native == endian::big)
    {
        reverse_copy(p, p + sizeof(int32_t), compressed.b.begin() + headerSize);
    }
    else
    {
        copy(p, p + sizeof(int32_t), compressed.b.begin() + headerSize);
    }

    //
    // Copy the header from the uncompressed stream to the compressed one.
    //
    copy(uncompressed.b.begin(), uncompressed.b.begin() + headerSize, compressed.b.begin());
}

void
Ice::ConnectionI::doUncompress(InputStream& compressed, InputStream& uncompressed)
{
    int32_t uncompressedSize;
    compressed.i = compressed.b.begin() + headerSize;
    compressed.read(uncompressedSize);
    if (uncompressedSize <= headerSize)
    {
        throw IllegalMessageSizeException(__FILE__, __LINE__);
    }

    if (uncompressedSize > static_cast<int32_t>(_messageSizeMax))
    {
        Ex::throwMemoryLimitException(__FILE__, __LINE__, static_cast<size_t>(uncompressedSize), _messageSizeMax);
    }
    uncompressed.resize(static_cast<size_t>(uncompressedSize));

    unsigned int uncompressedLen = static_cast<unsigned int>(uncompressedSize - headerSize);
    unsigned int compressedLen = static_cast<unsigned int>(compressed.b.size() - headerSize - sizeof(int32_t));
    int bzError = BZ2_bzBuffToBuffDecompress(
        reinterpret_cast<char*>(&uncompressed.b[0]) + headerSize,
        &uncompressedLen,
        reinterpret_cast<char*>(&compressed.b[0]) + headerSize + sizeof(int32_t),
        compressedLen,
        0,
        0);
    if (bzError != BZ_OK)
    {
        throw CompressionException(__FILE__, __LINE__, "BZ2_bzBuffToBuffCompress failed" + getBZ2Error(bzError));
    }

    copy(compressed.b.begin(), compressed.b.begin() + headerSize, uncompressed.b.begin());
}
#endif

SocketOperation
Ice::ConnectionI::parseMessage(int32_t& upcallCount, function<bool(InputStream&)>& upcall, InputStream& stream)
{
    assert(_state > StateNotValidated && _state < StateClosed);

    _readStream.swap(stream);
    _readStream.resize(headerSize);
    _readStream.i = _readStream.b.begin();
    _readHeader = true;

    assert(stream.i == stream.b.end());

    try
    {
        //
        // We don't need to check magic and version here. This has
        // already been done by the ThreadPool, which provides us
        // with the stream.
        //
        assert(stream.i == stream.b.end());
        stream.i = stream.b.begin() + 8;
        uint8_t messageType;
        stream.read(messageType);
        uint8_t compress;
        stream.read(compress);

        if (compress == 2)
        {
#ifdef ICE_HAS_BZIP2
            InputStream ustream(_instance.get(), Ice::currentProtocolEncoding);
            doUncompress(stream, ustream);
            stream.b.swap(ustream.b);
#else
            throw FeatureNotSupportedException(__FILE__, __LINE__, "Cannot uncompress compressed message");
#endif
        }
        stream.i = stream.b.begin() + headerSize;

        switch (messageType)
        {
            case closeConnectionMsg:
            {
                traceRecv(stream, _logger, _traceLevels);
                if (_endpoint->datagram())
                {
                    if (_warn)
                    {
                        Warning out(_logger);
                        out << "ignoring close connection message for datagram connection:\n" << _desc;
                    }
                }
                else
                {
                    setState(StateClosingPending, make_exception_ptr(CloseConnectionException(__FILE__, __LINE__)));

                    //
                    // Notify the transceiver of the graceful connection closure.
                    //
                    SocketOperation op = _transceiver->closing(false, _exception);
                    if (op)
                    {
                        if (_closeTimeout > chrono::seconds::zero())
                        {
                            _timer->schedule(make_shared<CloseTimerTask>(shared_from_this()), _closeTimeout);
                        }
                        return op;
                    }
                    setState(StateClosed);
                }
                break;
            }

            case requestMsg:
            {
                if (_state >= StateClosing)
                {
                    trace(
                        "received request during closing\n(ignored by server, client will retry)",
                        stream,
                        _logger,
                        _traceLevels);
                }
                else
                {
                    traceRecv(stream, _logger, _traceLevels);

                    auto adapter = _adapter;
                    const int32_t requestCount = 1;
                    int32_t requestId;

                    stream.read(requestId);

                    upcall = [self = shared_from_this(), requestId, adapter, compress](InputStream& messageStream)
                    {
                        self->dispatchAll(messageStream, requestCount, requestId, compress, adapter);
                        return false; // the upcall will be completed once the dispatch is done.
                    };
                    ++upcallCount;

                    if (isAtRest())
                    {
                        cancelInactivityTimerTask();
                    }
                    ++_dispatchCount;
                }
                break;
            }

            case requestBatchMsg:
            {
                if (_state >= StateClosing)
                {
                    trace(
                        "received batch request during closing\n(ignored by server, client will retry)",
                        stream,
                        _logger,
                        _traceLevels);
                }
                else
                {
                    traceRecv(stream, _logger, _traceLevels);

                    auto adapter = _adapter;
                    const int32_t requestId = 0;
                    int32_t requestCount;

                    stream.read(requestCount);
                    if (requestCount < 0)
                    {
                        requestCount = 0;
                        throw UnmarshalOutOfBoundsException(__FILE__, __LINE__);
                    }

                    upcall = [self = shared_from_this(), requestCount, adapter, compress](InputStream& messageStream)
                    {
                        self->dispatchAll(messageStream, requestCount, requestId, compress, adapter);
                        return false; // the upcall will be completed once the servant dispatch is done.
                    };
                    upcallCount += requestCount;

                    if (isAtRest())
                    {
                        cancelInactivityTimerTask();
                    }
                    _dispatchCount += requestCount;
                }
                break;
            }

            case replyMsg:
            {
                traceRecv(stream, _logger, _traceLevels);

                int32_t requestId;
                stream.read(requestId);

                map<int32_t, OutgoingAsyncBasePtr>::iterator q = _asyncRequests.end();

                if (_asyncRequestsHint != _asyncRequests.end())
                {
                    if (_asyncRequestsHint->first == requestId)
                    {
                        q = _asyncRequestsHint;
                    }
                }

                if (q == _asyncRequests.end())
                {
                    q = _asyncRequests.find(requestId);
                }

                if (q != _asyncRequests.end())
                {
                    auto outAsync = q->second;

                    if (q == _asyncRequestsHint)
                    {
                        _asyncRequests.erase(q++);
                        _asyncRequestsHint = q;
                    }
                    else
                    {
                        _asyncRequests.erase(q);
                    }

                    if (isAtRest())
                    {
                        scheduleInactivityTimerTask();
                    }

                    // The message stream is adopted by the outgoing.
                    *outAsync->getIs() = std::move(stream);

#if defined(ICE_USE_IOCP)
                    //
                    // If we just received the reply of a request which isn't acknowledge as
                    // sent yet, we queue the reply instead of processing it right away. It
                    // will be processed once the write callback is invoked for the message.
                    //
                    OutgoingMessage* message = _sendStreams.empty() ? 0 : &_sendStreams.front();
                    if (message && message->outAsync.get() == outAsync.get())
                    {
                        message->receivedReply = true;
                        outAsync = 0;
                    }
#endif
                    if (outAsync && outAsync->response())
                    {
                        // The message stream is not used here because it has been adopted above.
                        upcall = [outAsync](InputStream&)
                        {
                            outAsync->invokeResponse();
                            return true; // upcall is done
                        };
                        ++upcallCount;
                    }
                    _conditionVariable.notify_all(); // Notify threads blocked in close(false)
                }

                break;
            }

            case validateConnectionMsg:
            {
                traceRecv(stream, _logger, _traceLevels);
                if (_heartbeatCallback)
                {
                    upcall = [self = shared_from_this(), heartbeatCallback = _heartbeatCallback](InputStream&)
                    {
                        try
                        {
                            heartbeatCallback(self);
                        }
                        catch (const std::exception& ex)
                        {
                            Error out(self->_instance->initializationData().logger);
                            out << "connection callback exception:\n" << ex << '\n' << self->_desc;
                        }
                        catch (...)
                        {
                            Error out(self->_instance->initializationData().logger);
                            out << "connection callback exception:\nunknown c++ exception" << '\n' << self->_desc;
                        }
                        return true; // upcall is done
                    };
                    ++upcallCount;
                }
                // a heartbeat has no effect on the dispatch count or the inactivity timer task.
                break;
            }

            default:
            {
                trace("received unknown message\n(invalid, closing connection)", stream, _logger, _traceLevels);
                throw UnknownMessageException(__FILE__, __LINE__);
            }
        }
    }
    catch (const LocalException& ex)
    {
        if (_endpoint->datagram())
        {
            if (_warn)
            {
                Warning out(_logger);
                out << "datagram connection exception:\n" << ex << '\n' << _desc;
            }
        }
        else
        {
            setState(StateClosed, current_exception());
        }
    }

    return _state == StateHolding ? SocketOperationNone : SocketOperationRead;
}

void
Ice::ConnectionI::dispatchAll(
    InputStream& stream,
    int32_t requestCount,
    int32_t requestId,
    uint8_t compress,
    const ObjectAdapterIPtr& adapter)
{
    // Note: In contrast to other private or protected methods, this operation must be called *without* the mutex
    // locked.

    try
    {
        while (requestCount > 0)
        {
            //
            // Start of the dispatch pipeline.
            //

            IncomingRequest request{requestId, shared_from_this(), adapter, stream};

            if (adapter)
            {
                try
                {
                    adapter->dispatchPipeline()->dispatch(
                        request,
                        [self = shared_from_this(), compress](OutgoingResponse response)
                        { self->sendResponse(std::move(response), compress); });
                }
                catch (...)
                {
                    sendResponse(makeOutgoingResponse(current_exception(), request.current()), 0);
                }
            }
            else
            {
                // Received request on a connection without an object adapter.
                sendResponse(
                    makeOutgoingResponse(
                        make_exception_ptr(ObjectNotExistException{__FILE__, __LINE__}),
                        request.current()),
                    0);
            }

            --requestCount;
        }

        stream.clear();
    }
    catch (...)
    {
        dispatchException(current_exception(), requestCount); // Fatal invocation exception
    }
}

Ice::ConnectionInfoPtr
Ice::ConnectionI::initConnectionInfo() const
{
    if (_state > StateNotInitialized && _info) // Update the connection information until it's initialized
    {
        return _info;
    }

    try
    {
        _info = _transceiver->getInfo();
    }
    catch (const Ice::LocalException&)
    {
        _info = std::make_shared<ConnectionInfo>();
    }

    Ice::ConnectionInfoPtr info = _info;
    while (info)
    {
        info->connectionId = _endpoint->connectionId();
        info->incoming = _connector == 0;
        info->adapterName = _adapter ? _adapter->getName() : string();
        info = info->underlying;
    }
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
    SocketOperation op = _transceiver->read(buf);
    if (_instance->traceLevels()->network >= 3 && buf.i != start)
    {
        Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
        out << "received ";
        if (_endpoint->datagram())
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
    if (_instance->traceLevels()->network >= 3 && buf.i != start)
    {
        Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
        out << "sent " << (buf.i - start);
        if (!_endpoint->datagram())
        {
            out << " of " << (buf.b.end() - start);
        }
        out << " bytes via " << _endpoint->protocol() << "\n" << toString();
    }
    return op;
}

void
ConnectionI::scheduleInactivityTimerTask()
{
    // Called with the ConnectionI mutex locked.
    if (_inactivityTimeout > chrono::seconds::zero())
    {
        assert(_inactivityTimerTask);
        _timer->schedule(_inactivityTimerTask, _inactivityTimeout);
    }
}

void
ConnectionI::cancelInactivityTimerTask()
{
    // Called with the ConnectionI mutex locked.
    if (_inactivityTimeout > chrono::seconds::zero())
    {
        assert(_inactivityTimerTask);
        _timer->cancel(_inactivityTimerTask);
    }
}
