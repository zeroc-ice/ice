// Copyright (c) ZeroC, Inc.

#include "ConnectionI.h"
#include "BatchRequestQueue.h"
#include "CheckIdentity.h"
#include "Endian.h"
#include "EndpointI.h"
#include "Ice/IncomingRequest.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "Ice/OutgoingResponse.h"
#include "Ice/Properties.h"
#include "IdleTimeoutTransceiverDecorator.h"
#include "Instance.h"
#include "ObjectAdapterI.h"   // For getThreadPool()
#include "ReferenceFactory.h" // For createProxy().
#include "RequestHandler.h"   // For RetryException
#include "ThreadPool.h"
#include "TraceLevels.h"
#include "TraceUtil.h"
#include "Transceiver.h"

#include "DisableWarnings.h"

#include <algorithm>
#include <iomanip>
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
    class ConnectTimerTask final : public TimerTask
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

    class CloseTimerTask final : public TimerTask
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

    class InactivityTimerTask final : public TimerTask
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
        ConnectionFlushBatchAsync(Ice::ConnectionIPtr, const InstancePtr&);

        [[nodiscard]] virtual Ice::ConnectionPtr getConnection() const;

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

    string createBadMagicMessage(const byte m[])
    {
        ostringstream os;
        os << "bag magic in message header: ";
        for (size_t i = 0; i < sizeof(magic); ++i)
        {
            os << hex << setw(2) << setfill('0') << static_cast<int>(m[i]) << ' ';
        }
        return os.str();
    }
}

ConnectionFlushBatchAsync::ConnectionFlushBatchAsync(ConnectionIPtr connection, const InstancePtr& instance)
    : OutgoingAsyncBase(instance),
      _connection(std::move(connection))
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

Ice::ConnectionI::Observer::Observer() = default;

void
Ice::ConnectionI::Observer::startRead(const Buffer& buf)
{
    if (_readStreamPos)
    {
        assert(!buf.b.empty());
        _observer->receivedBytes(static_cast<int>(buf.i - _readStreamPos));
    }
    _readStreamPos = buf.b.empty() ? nullptr : buf.i;
}

void
Ice::ConnectionI::Observer::finishRead(const Buffer& buf)
{
    if (_readStreamPos == nullptr)
    {
        return;
    }
    assert(buf.i >= _readStreamPos);
    _observer->receivedBytes(static_cast<int>(buf.i - _readStreamPos));
    _readStreamPos = nullptr;
}

void
Ice::ConnectionI::Observer::startWrite(const Buffer& buf)
{
    if (_writeStreamPos)
    {
        assert(!buf.b.empty());
        _observer->sentBytes(static_cast<int>(buf.i - _writeStreamPos));
    }
    _writeStreamPos = buf.b.empty() ? nullptr : buf.i;
}

void
Ice::ConnectionI::Observer::finishWrite(const Buffer& buf)
{
    if (_writeStreamPos == nullptr)
    {
        return;
    }
    if (buf.i > _writeStreamPos)
    {
        _observer->sentBytes(static_cast<int>(buf.i - _writeStreamPos));
    }
    _writeStreamPos = nullptr;
}

void
Ice::ConnectionI::Observer::attach(const Ice::Instrumentation::ConnectionObserverPtr& observer)
{
    ObserverHelperT<Ice::Instrumentation::ConnectionObserver>::attach(observer);
    if (!observer)
    {
        _writeStreamPos = nullptr;
        _readStreamPos = nullptr;
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
    stream = nullptr;
}

void
Ice::ConnectionI::startAsync(
    function<void(const ConnectionIPtr&)> connectionStartCompleted,
    function<void(const ConnectionIPtr&, exception_ptr)> connectionStartFailed)
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
            setState(
                StateClosing,
                make_exception_ptr(
                    ObjectAdapterDeactivatedException{__FILE__, __LINE__, _adapter ? _adapter->getName() : ""}));
            break;
        }

        case CommunicatorDestroyed:
        {
            setState(StateClosing, make_exception_ptr(CommunicatorDestroyedException{__FILE__, __LINE__}));
            break;
        }
    }
}

void
Ice::ConnectionI::abort() noexcept
{
    std::lock_guard lock(_mutex);
    setState(
        StateClosed,
        make_exception_ptr(
            ConnectionAbortedException{__FILE__, __LINE__, "connection aborted by the application", true}));
}

void
Ice::ConnectionI::close(function<void()> response, function<void(std::exception_ptr)> exception) noexcept
{
    std::exception_ptr closeException = nullptr;
    {
        std::lock_guard lock(_mutex);
        if (_state >= StateClosed)
        {
            closeException = _exception;
            assert(closeException);
        }
        else
        {
            if (response || exception)
            {
                _onClosedList.emplace_back(std::move(response), std::move(exception));
            }

            if (_state < StateClosing)
            {
                if (_asyncRequests.empty())
                {
                    doApplicationClose();
                }
                else
                {
                    // We'll close the connection when we get the last reply message.
                    _closeRequested = true;
                    scheduleCloseTimerTask(); // we don't want to wait forever
                }
            }
            // else nothing to do
        }
    }

    if ((response || exception) && closeException) // already closed
    {
        bool success = false;
        try
        {
            rethrow_exception(closeException);
        }
        catch (const ConnectionClosedException&)
        {
            success = true;
        }
        catch (const CloseConnectionException&)
        {
            success = true;
        }
        catch (const CommunicatorDestroyedException&)
        {
            success = true;
        }
        catch (const ObjectAdapterDeactivatedException&)
        {
            success = true;
        }
        catch (const ObjectAdapterDestroyedException&)
        {
            success = true;
        }
        catch (...)
        {
        }

        if (success)
        {
            if (response)
            {
                response();
            }
        }
        else
        {
            if (exception)
            {
                exception(closeException);
            }
        }
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

    // We're just about to send a request, so we are not inactive anymore.
    cancelInactivityTimerTask();

    AsyncStatus status = AsyncStatusQueued;
    try
    {
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
    std::function<void(std::exception_ptr)> ex,
    std::function<void(bool)> sent)
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

    for (auto o = _sendStreams.begin(); o != _sendStreams.end(); ++o)
    {
        if (o->outAsync.get() == outAsync.get())
        {
            if (o->requestId)
            {
                if (_asyncRequestsHint != _asyncRequests.end() &&
                    _asyncRequestsHint->second == dynamic_pointer_cast<OutgoingAsync>(outAsync))
                {
                    _asyncRequests.erase(_asyncRequestsHint);
                    _asyncRequestsHint = _asyncRequests.end();
                }
                else
                {
                    _asyncRequests.erase(o->requestId);
                }
            }

            try
            {
                rethrow_exception(ex);
            }
            catch (const ConnectionAbortedException&)
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

            if (_closeRequested && _state < StateClosing && _asyncRequests.empty())
            {
                doApplicationClose();
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
                catch (const ConnectionAbortedException&)
                {
                    setState(StateClosed, ex);
                }
                catch (const std::exception&)
                {
                    _asyncRequests.erase(_asyncRequestsHint);
                    _asyncRequestsHint = _asyncRequests.end();

                    if (outAsync->exception(ex))
                    {
                        outAsync->invokeExceptionAsync();
                    }
                }

                if (_closeRequested && _state < StateClosing && _asyncRequests.empty())
                {
                    doApplicationClose();
                }
                return;
            }
        }

        for (auto p = _asyncRequests.begin(); p != _asyncRequests.end(); ++p)
        {
            if (p->second.get() == outAsync.get())
            {
                try
                {
                    rethrow_exception(ex);
                }
                catch (const ConnectionAbortedException&)
                {
                    setState(StateClosed, ex);
                }
                catch (const std::exception&)
                {
                    assert(p != _asyncRequestsHint);
                    _asyncRequests.erase(p);

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
    if (!_connector) // server connection
    {
        throw std::logic_error{"setAdapter can only be called on a client connection"};
    }

    if (adapter)
    {
        // Go through the adapter to set the adapter on this connection
        // to ensure the object adapter is still active.
        dynamic_pointer_cast<ObjectAdapterI>(adapter)->setAdapterOnConnection(shared_from_this());
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
Ice::ConnectionI::_createProxy(Identity ident) const
{
    checkIdentity(ident, __FILE__, __LINE__);
    return ObjectPrx::_fromReference(
        _instance->referenceFactory()->create(std::move(ident), const_cast<ConnectionI*>(this)->shared_from_this()));
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

    // Clear cached connection info (if any) as it's no longer accurate.
    _info = nullptr;
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
    InputStream messageStream{_instance.get(), currentProtocolEncoding};

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
                        // setState() to check whether or not we can print a connection
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
                            throw MarshalException{
                                __FILE__,
                                __LINE__,
                                "received Ice message with too few bytes in header"};
                        }

                        // Decode the header.
                        _readStream.i = _readStream.b.begin();
                        const byte* m;
                        _readStream.readBlob(m, static_cast<int32_t>(sizeof(magic)));
                        if (m[0] != magic[0] || m[1] != magic[1] || m[2] != magic[2] || m[3] != magic[3])
                        {
                            throw ProtocolException{__FILE__, __LINE__, createBadMagicMessage(m)};
                        }
                        ProtocolVersion pv;
                        _readStream.read(pv);
                        if (pv != currentProtocol)
                        {
                            throw ProtocolException{
                                __FILE__,
                                __LINE__,
                                "invalid protocol version in message header: " + Ice::protocolVersionToString(pv)};
                        }
                        EncodingVersion ev;
                        _readStream.read(ev);
                        if (ev != currentProtocolEncoding)
                        {
                            throw ProtocolException{
                                __FILE__,
                                __LINE__,
                                "invalid protocol encoding version in message header: " +
                                    Ice::encodingVersionToString(ev)};
                        }
                        uint8_t messageType;
                        _readStream.read(messageType);
                        uint8_t compressByte;
                        _readStream.read(compressByte);
                        int32_t size;
                        _readStream.read(size);
                        if (size < headerSize)
                        {
                            throw MarshalException{
                                __FILE__,
                                __LINE__,
                                "received Ice message with unexpected size " + to_string(size)};
                        }

                        // Resize the read buffer to the message size.
                        if (size > _messageSizeMax)
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
            auto newOp = static_cast<SocketOperation>(readOp | writeOp);

            // Operations that are ready. For example, if message was called with SocketOperationRead and the transport
            // read returned SocketOperationNone, reads are considered done: there's no additional data to read.
            auto readyOp = static_cast<SocketOperation>(current.operation & ~newOp);

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

    if (!_hasExecutor) // Optimization, call upcall() directly if there's no executor.
    {
        upcall(connectionStartCompleted, sentCBs, messageUpcall, messageStream);
    }
    else
    {
        auto stream = make_shared<InputStream>(_instance.get(), currentProtocolEncoding);
        stream->swap(messageStream);

        auto self = shared_from_this();
        _threadPool->executeFromThisThread(
            [self,
             connectionStartCompleted = std::move(connectionStartCompleted),
             sentCBs = std::move(sentCBs),
             messageUpcall = std::move(messageUpcall),
             stream]() { self->upcall(connectionStartCompleted, sentCBs, messageUpcall, *stream); },
            self);
    }
}

void
ConnectionI::upcall(
    const function<void(ConnectionIPtr)>& connectionStartCompleted,
    const vector<OutgoingMessage>& sentCBs,
    const function<bool(InputStream&)>& messageUpcall,
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
        for (const auto& sentCB : sentCBs)
        {
#if defined(ICE_USE_IOCP)
            if (sentCB.invokeSent)
            {
                sentCB.outAsync->invokeSent();
            }
            if (sentCB.receivedReply)
            {
                auto o = dynamic_pointer_cast<OutgoingAsync>(sentCB.outAsync);
                if (o->response())
                {
                    o->invokeResponse();
                }
            }
#else
            sentCB.outAsync->invokeSent();
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
    // Lock the connection here to ensure setState() completes before the code below is executed. This method can be
    // called by the thread pool as soon as setState() calls _threadPool->finish(...). There's no need to lock the mutex
    // for the remainder of the code because the data members accessed by finish() are immutable once _state ==
    // StateClosed (and we don't want to hold the mutex when calling upcalls).
    {
        std::lock_guard lock(_mutex);
        assert(_state == StateClosed);
    }

    // If there are no callbacks to call, we don't call ioCompleted() since we're not going to call code that will
    // potentially block (this avoids promoting a new leader and unecessary thread creation, especially if this is
    // called on shutdown).
    if (!_connectionStartCompleted && !_connectionStartFailed && _sendStreams.empty() && _asyncRequests.empty() &&
        !_closeCallback)
    {
        finish(close);
        return;
    }

    current.ioCompleted();

    if (!_hasExecutor) // Optimization, call finish() directly if there's no executor.
    {
        finish(close);
    }
    else
    {
        auto self = shared_from_this();
        _threadPool->executeFromThisThread([self, close]() { self->finish(close); }, self);
    }
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
            catch (const ConnectionAbortedException&)
            {
            }
            catch (const ConnectionClosedException&)
            {
            }
            catch (const CommunicatorDestroyedException&)
            {
            }
            catch (const ObjectAdapterDeactivatedException&)
            {
            }
            catch (const ObjectAdapterDestroyedException&)
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

        for (auto& sendStream : _sendStreams)
        {
            sendStream.completed(_exception);
            if (sendStream.requestId) // Make sure finished isn't called twice.
            {
                _asyncRequests.erase(sendStream.requestId);
            }
        }

        _sendStreams.clear();
    }

    for (const auto& asyncRequest : _asyncRequests)
    {
        if (asyncRequest.second->exception(_exception))
        {
            asyncRequest.second->invokeException();
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

    if (!_onClosedList.empty())
    {
        bool success;
        try
        {
            rethrow_exception(_exception);
        }
        catch (const ConnectionClosedException&)
        {
            success = true;
        }
        catch (const CloseConnectionException&)
        {
            success = true;
        }
        catch (const CommunicatorDestroyedException&)
        {
            success = true;
        }
        catch (const ObjectAdapterDeactivatedException&)
        {
            success = true;
        }
        catch (const ObjectAdapterDestroyedException&)
        {
            success = true;
        }
        catch (...)
        {
            success = false;
        }

        for (const auto& pair : _onClosedList)
        {
            if (success)
            {
                if (pair.first)
                {
                    pair.first();
                }
            }
            else
            {
                if (pair.second)
                {
                    pair.second(_exception);
                }
            }
        }
        _onClosedList.clear(); // break potential cycles
    }

    if (_closeCallback)
    {
        closeCallback(_closeCallback);
        _closeCallback = nullptr;
    }

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
Ice::ConnectionI::toString() const
{
    return _desc; // No mutex lock, _desc is immutable.
}

NativeInfoPtr
Ice::ConnectionI::getNativeInfo()
{
    return _transceiver->getNativeInfo();
}

const string&
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
    _info = nullptr; // Invalidate the cached connection info
}

void
Ice::ConnectionI::exception(std::exception_ptr ex)
{
    std::lock_guard lock(_mutex);
    setState(StateClosed, ex);
}

Ice::ConnectionI::ConnectionI(
    CommunicatorPtr communicator,
    const InstancePtr& instance,
    const TransceiverPtr& transceiver,
    const ConnectorPtr& connector,
    const EndpointIPtr& endpoint,
    const shared_ptr<ObjectAdapterI>& adapter,
    std::function<void(const ConnectionIPtr&)> removeFromFactory,
    const ConnectionOptions& options) noexcept
    : _communicator(std::move(communicator)),
      _instance(instance),
      _transceiver(transceiver),
      _idleTimeoutTransceiver(dynamic_pointer_cast<IdleTimeoutTransceiverDecorator>(transceiver)),
      _desc(transceiver->toString()),
      _type(transceiver->protocol()),
      _connector(connector),
      _endpoint(endpoint),
      _adapter(adapter),
      _hasExecutor(_instance->initializationData().executor != nullptr), // Cached for better performance.
      _logger(_instance->initializationData().logger),                   // Cached for better performance.
      _traceLevels(_instance->traceLevels()),                            // Cached for better performance.
      _timer(_instance->timer()),                                        // Cached for better performance.
      _connectTimeout(options.connectTimeout),
      _closeTimeout(options.closeTimeout), // not used for datagram connections
      // suppress inactivity timeout for datagram connections
      _inactivityTimeout(endpoint->datagram() ? chrono::seconds::zero() : options.inactivityTimeout),
      _removeFromFactory(std::move(removeFromFactory)),
      _warn(_instance->initializationData().properties->getIcePropertyAsInt("Ice.Warn.Connections") > 0),
      _warnUdp(_instance->initializationData().properties->getIcePropertyAsInt("Ice.Warn.Datagrams") > 0),
      _asyncRequestsHint(_asyncRequests.end()),
      _messageSizeMax(connector ? _instance->messageSizeMax() : adapter->messageSizeMax()),
      _batchRequestQueue(new BatchRequestQueue(instance, endpoint->datagram())),
      _readStream{instance.get(), currentProtocolEncoding},
      _maxDispatches(options.maxDispatches)
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
        decoratedTransceiver =
            make_shared<IdleTimeoutTransceiverDecorator>(transceiver, options.idleTimeout, instance->timer());
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
        decoratedTransceiver->decoratorInit(connection, options.enableIdleCheck);
    }

    if (connector) // client connection
    {
        const_cast<ThreadPoolPtr&>(connection->_threadPool) = connection->_instance->clientThreadPool();
    }
    else
    {
        // server connection
        assert(adapter);
        const_cast<ThreadPoolPtr&>(connection->_threadPool) = adapter->getThreadPool();
    }
    connection->_threadPool->initialize(connection);
    return connection;
}

Ice::ConnectionI::~ConnectionI()
{
    assert(!_connectionStartCompleted);
    assert(!_connectionStartFailed);
    assert(!_closeCallback);
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
            catch (const ConnectionAbortedException&)
            {
            }
            catch (const ConnectionClosedException&)
            {
            }
            catch (const CommunicatorDestroyedException&)
            {
            }
            catch (const ObjectAdapterDeactivatedException&)
            {
            }
            catch (const ObjectAdapterDestroyedException&)
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

    if (state > StateActive)
    {
        cancelInactivityTimerTask();
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
                // Can only switch from holding or not validated to active.
                if (_state != StateHolding && _state != StateNotValidated)
                {
                    return;
                }

                if (_maxDispatches <= 0 || _dispatchCount < _maxDispatches)
                {
                    _threadPool->_register(shared_from_this(), SocketOperationRead);
                    if (_idleTimeoutTransceiver)
                    {
                        _idleTimeoutTransceiver->enableIdleCheck();
                    }
                }
                // else don't resume reading since we're at or over the _maxDispatches limit.

                break;
            }

            case StateHolding:
            {
                // Can only switch from active or not validated to holding.
                if (_state != StateActive && _state != StateNotValidated)
                {
                    return;
                }

                if (_state == StateActive && (_maxDispatches <= 0 || _dispatchCount < _maxDispatches))
                {
                    _threadPool->unregister(shared_from_this(), SocketOperationRead);
                    if (_idleTimeoutTransceiver)
                    {
                        _idleTimeoutTransceiver->disableIdleCheck();
                    }
                }
                // else reads are already disabled because the _maxDispatches limit is reached or exceeded.
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
                _communicator = nullptr;
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
            catch (const ConnectionAbortedException&)
            {
            }
            catch (const ConnectionClosedException&)
            {
            }
            catch (const CommunicatorDestroyedException&)
            {
            }
            catch (const ObjectAdapterDeactivatedException&)
            {
            }
            catch (const ObjectAdapterDestroyedException&)
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
        OutputStream os{Ice::currentProtocolEncoding};
        os.writeBlob(magic, sizeof(magic));
        os.write(currentProtocol);
        os.write(currentProtocolEncoding);
        os.write(closeConnectionMsg);
        os.write(static_cast<uint8_t>(0)); // compression status: always zero for close connection.
        os.write(headerSize);              // Message size.

        scheduleCloseTimerTask();

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
Ice::ConnectionI::idleCheck(const chrono::seconds& idleTimeout) noexcept
{
    std::lock_guard lock(_mutex);
    if (_state == StateActive && _idleTimeoutTransceiver->idleCheckEnabled())
    {
        if (_instance->traceLevels()->network >= 1)
        {
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
            out << "connection aborted by the idle check because it did not receive any bytes for "
                << idleTimeout.count() << "s\n";
            out << _transceiver->toDetailedString();
        }

        setState(
            StateClosed,
            make_exception_ptr(ConnectionAbortedException{
                __FILE__,
                __LINE__,
                "connection aborted by the idle check because it did not receive any bytes for " +
                    to_string(idleTimeout.count()) + "s",
                false})); // closedByApplication: false
    }
    // else, nothing to do
}

void
Ice::ConnectionI::inactivityCheck() noexcept
{
    // Called by the InactivityTimerTask.
    std::lock_guard lock(_mutex);

    // Make sure this timer task was not rescheduled for later while we were waiting for _mutex.
    if (_inactivityTimerTaskScheduled && !_timer->isScheduled(_inactivityTimerTask))
    {
        // Clear flag - the task is no longer scheduled.
        _inactivityTimerTaskScheduled = false;

        if (_state == StateActive)
        {
            setState(
                StateClosing,
                make_exception_ptr(ConnectionClosedException{
                    __FILE__,
                    __LINE__,
                    "connection closed because it remained inactive for longer than the inactivity timeout",
                    false}));
        }
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
        // We don't use setState(state, exception) because we want to overwrite the exception set by a
        // graceful closure.
        _exception = make_exception_ptr(CloseTimeoutException{__FILE__, __LINE__});
        setState(StateClosed);
    }
    // else ignore since we're already closed.
}

void
Ice::ConnectionI::sendHeartbeat() noexcept
{
    assert(!_endpoint->datagram());

    lock_guard lock(_mutex);
    if (_state == StateActive || _state == StateHolding || _state == StateClosing)
    {
        // We check if the connection has become inactive.
        if (_inactivityTimerTask &&           // null when the inactivity timeout is infinite
            !_inactivityTimerTaskScheduled && // we never reschedule this task
            _state == StateActive &&          // only schedule the task if the connection is active
            _dispatchCount == 0 &&            // no pending dispatch
            _asyncRequests.empty() &&         // no pending invocation
            _readHeader &&                    // we're not waiting for the remainder of an incoming message
            _sendStreams.size() <= 1)         // there is at most one pending outgoing message
        {
            // We may become inactive while the peer is back-pressuring us. In this case, we only schedule the
            // inactivity timer if there is no pending outgoing message or the pending outgoing message is a
            // heartbeat.

            // The stream of the first _sendStreams message is in _writeStream.
            if (_sendStreams.empty() || static_cast<uint8_t>(_writeStream.b[8]) == validateConnectionMsg)
            {
                scheduleInactivityTimerTask();
            }
        }

        // We send a heartbeat to the peer to generate a "write" on the connection. This write in turns creates
        // a read on the peer, and resets the peer's idle check timer. When _sendStream is not empty, there is
        // already an outstanding write, so we don't need to send a heartbeat. It's possible the first message
        // of _sendStreams was already sent but not yet removed from _sendStreams: it means the last write
        // occurred very recently, which is good enough with respect to the idle check.
        // As a result of this optimization, the only possible heartbeat in _sendStreams is the first
        // _sendStreams message.
        if (_sendStreams.empty())
        {
            OutputStream os{Ice::currentProtocolEncoding};
            os.writeBlob(magic, sizeof(magic));
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

            if (_state == StateActive && _maxDispatches > 0 && _dispatchCount == _maxDispatches)
            {
                // Resume reading if the connection is active and the dispatch count is about to be less than
                // _maxDispatches.
                _threadPool->update(shared_from_this(), SocketOperationNone, SocketOperationRead);
                if (_idleTimeoutTransceiver)
                {
                    _idleTimeoutTransceiver->enableIdleCheck();
                }
            }

            --_dispatchCount;

            if (_state == StateClosing && _upcallCount == 0)
            {
                initiateShutdown();
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
        if (!_connector) // The server side has the active role for connection validation.
        {
            if (_writeStream.b.empty())
            {
                _writeStream.writeBlob(magic, sizeof(magic));
                _writeStream.write(currentProtocol);
                _writeStream.write(currentProtocolEncoding);
                _writeStream.write(validateConnectionMsg);
                _writeStream.write(
                    static_cast<uint8_t>(0));   // Compression status (always zero for validate connection).
                _writeStream.write(headerSize); // Message size.
                _writeStream.i = _writeStream.b.begin();
                traceSend(_writeStream, _instance, this, _logger, _traceLevels);
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
            const std::byte* m;
            _readStream.readBlob(m, sizeof(magic));
            if (m[0] != magic[0] || m[1] != magic[1] || m[2] != magic[2] || m[3] != magic[3])
            {
                throw ProtocolException{__FILE__, __LINE__, createBadMagicMessage(m)};
            }
            ProtocolVersion pv;
            _readStream.read(pv);
            if (pv != currentProtocol)
            {
                throw ProtocolException{
                    __FILE__,
                    __LINE__,
                    "invalid protocol version in message header: " + Ice::protocolVersionToString(pv)};
            }
            EncodingVersion ev;
            _readStream.read(ev);
            if (ev != currentProtocolEncoding)
            {
                throw ProtocolException{
                    __FILE__,
                    __LINE__,
                    "invalid protocol encoding version in message header: " + Ice::encodingVersionToString(ev)};
            }
            uint8_t messageType;
            _readStream.read(messageType);
            if (messageType != validateConnectionMsg)
            {
                throw ProtocolException{
                    __FILE__,
                    __LINE__,
                    "received message of type " + to_string(messageType) +
                        " over a connection that is not yet validated"};
            }
            uint8_t compress;
            _readStream.read(compress); // Ignore compression status for validate connection.
            int32_t size;
            _readStream.read(size);
            if (size != headerSize)
            {
                throw MarshalException{
                    __FILE__,
                    __LINE__,
                    "received ValidateConnection message with unexpected size " + to_string(size)};
            }
            traceRecv(_readStream, this, _logger, _traceLevels);

            // Client connection starts sending heartbeats once it has received the ValidateConnection message.
            if (_idleTimeoutTransceiver)
            {
                _idleTimeoutTransceiver->scheduleHeartbeat();
            }
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
        // This can occur if no message was being written and the socket write operation was registered with the
        // thread pool (a transceiver read method can request writing data).
        return SocketOperationNone;
    }
    else if (_state == StateClosingPending && _writeStream.i == _writeStream.b.begin())
    {
        // Message wasn't sent, empty the _writeStream, we're not going to send more data.
        OutgoingMessage* message = &_sendStreams.front();
        _writeStream.swap(*message->stream);
        return SocketOperationNone;
    }

    // Assert that the message was fully written.
    assert(!_writeStream.b.empty() && _writeStream.i == _writeStream.b.end());

    try
    {
        while (true)
        {
            //
            // The message that was being sent is sent. We can swap back the write stream buffer to the outgoing message
            // (required for retry) and queue its sent callback (if any).
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
            // If we are in the closed state or if the close is pending, don't continue sending.
            //
            // This can occur if parseMessage (called before sendNextMessages by message()) closes the connection.
            //
            if (_state >= StateClosingPending)
            {
                return SocketOperationNone;
            }

            //
            // Otherwise, prepare the next message.
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
                OutputStream stream{currentProtocolEncoding};
                doCompress(*message->stream, stream);

                traceSend(*message->stream, _instance, this, _logger, _traceLevels);

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
                auto sz = static_cast<int32_t>(message->stream->b.size());
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
                traceSend(*message->stream, _instance, this, _logger, _traceLevels);

#ifdef ICE_HAS_BZIP2
            }
#endif

            //
            // Send the message.
            //
            _writeStream.swap(*message->stream);
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

            // If the message was sent right away, loop to send the next queued message.
        }

        // Once the CloseConnection message is sent, we transition to the StateClosingPending state.
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

    message.stream->i = nullptr; // Reset the message stream iterator before starting sending the message.

    // Some messages are queued for sending. Just adds the message to the send queue and tell the caller that the
    // message was queued.
    if (!_sendStreams.empty())
    {
        _sendStreams.push_back(message);
        _sendStreams.back().adopt(nullptr);
        return AsyncStatusQueued;
    }

    // Prepare and send the message.
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
        OutputStream stream{currentProtocolEncoding};
        doCompress(*message.stream, stream);
        stream.i = stream.b.begin();

        traceSend(*message.stream, _instance, this, _logger, _traceLevels);

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
        auto sz = static_cast<int32_t>(message.stream->b.size());
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

        traceSend(*message.stream, _instance, this, _logger, _traceLevels);

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
        _sendStreams.back().adopt(nullptr); // Adopt the stream.
#ifdef ICE_HAS_BZIP2
    }
#endif

    // The message couldn't be sent right away so we add it to the send stream queue (which is empty) and swap its
    // stream with `_writeStream`. The socket operation returned by the transceiver write is registered with the thread
    // pool. At this point the message() method will take care of sending the whole message (held by _writeStream) when
    // the transceiver is ready to write more of the message buffer.

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
    auto uncompressedLen = static_cast<unsigned int>(uncompressed.b.size() - headerSize);
    auto compressedLen = static_cast<unsigned int>(uncompressedLen * 1.01 + 600);
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
        throw ProtocolException{
            __FILE__,
            __LINE__,
            "cannot compress message - BZ2_bzBuffToBuffCompress failed" + getBZ2Error(bzError)};
    }
    compressed.b.resize(headerSize + sizeof(int32_t) + compressedLen);

    //
    // Write the size of the compressed stream into the header of the
    // uncompressed stream. Since the header will be copied, this size
    // will also be in the header of the compressed stream.
    //
    auto compressedSize = static_cast<int32_t>(compressed.b.size());
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
    auto uncompressedSize = static_cast<int32_t>(uncompressed.b.size());
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
        throw MarshalException{
            __FILE__,
            __LINE__,
            "unexpected message size after uncompress: " + to_string(uncompressedSize)};
    }

    if (uncompressedSize > _messageSizeMax)
    {
        Ex::throwMemoryLimitException(__FILE__, __LINE__, static_cast<size_t>(uncompressedSize), _messageSizeMax);
    }
    uncompressed.resize(static_cast<size_t>(uncompressedSize));

    auto uncompressedLen = static_cast<unsigned int>(uncompressedSize - headerSize);
    auto compressedLen = static_cast<unsigned int>(compressed.b.size() - headerSize - sizeof(int32_t));
    int bzError = BZ2_bzBuffToBuffDecompress(
        reinterpret_cast<char*>(&uncompressed.b[0]) + headerSize,
        &uncompressedLen,
        reinterpret_cast<char*>(&compressed.b[0]) + headerSize + sizeof(int32_t),
        compressedLen,
        0,
        0);
    if (bzError != BZ_OK)
    {
        throw ProtocolException{
            __FILE__,
            __LINE__,
            "cannot decompress message - BZ2_bzBuffToBuffDecompress failed" + getBZ2Error(bzError)};
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
            InputStream ustream{_instance.get(), currentProtocolEncoding};
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
                traceRecv(stream, this, _logger, _traceLevels);
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
                        scheduleCloseTimerTask();
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
                        this,
                        _logger,
                        _traceLevels);
                }
                else
                {
                    traceRecv(stream, this, _logger, _traceLevels);

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

                    cancelInactivityTimerTask();
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
                        this,
                        _logger,
                        _traceLevels);
                }
                else
                {
                    traceRecv(stream, this, _logger, _traceLevels);

                    auto adapter = _adapter;
                    const int32_t requestId = 0;
                    int32_t requestCount;

                    stream.read(requestCount);
                    if (requestCount < 0)
                    {
                        requestCount = 0;
                        throw MarshalException{
                            __FILE__,
                            __LINE__,
                            "received batch request with " + to_string(requestCount) + " batches"};
                    }

                    upcall = [self = shared_from_this(), requestCount, adapter, compress](InputStream& messageStream)
                    {
                        self->dispatchAll(messageStream, requestCount, requestId, compress, adapter);
                        return false; // the upcall will be completed once the servant dispatch is done.
                    };
                    upcallCount += requestCount;

                    cancelInactivityTimerTask();
                    _dispatchCount += requestCount;
                }
                break;
            }

            case replyMsg:
            {
                traceRecv(stream, this, _logger, _traceLevels);

                int32_t requestId;
                stream.read(requestId);

                auto q = _asyncRequests.end();

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
                    if (_closeRequested && _state < StateClosing && _asyncRequests.empty())
                    {
                        doApplicationClose();
                    }
                }

                break;
            }

            case validateConnectionMsg:
            {
                traceRecv(stream, this, _logger, _traceLevels);
                // a heartbeat has no effect on the dispatch count or the inactivity timer task.
                break;
            }

            default:
            {
                trace("received unknown message\n(invalid, closing connection)", stream, this, _logger, _traceLevels);
                throw ProtocolException{
                    __FILE__,
                    __LINE__,
                    "received Ice protocol message with unknown type: " + to_string(messageType)};
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

    if (_state == StateHolding)
    {
        // Don't continue reading if the connection is in the holding state.
        return SocketOperationNone;
    }
    else if (_maxDispatches > 0 && _dispatchCount >= _maxDispatches)
    {
        // Don't continue reading if the _maxDispatches limit is reached or exceeded.
        if (_idleTimeoutTransceiver)
        {
            _idleTimeoutTransceiver->disableIdleCheck();
        }
        return SocketOperationNone;
    }
    else
    {
        // Continue reading.
        return SocketOperationRead;
    }
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

            IncomingRequest request{requestId, shared_from_this(), adapter, stream, requestCount};

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
    // Called with _mutex locked.

    if (_state > StateNotInitialized && _info) // Update the connection information until it's initialized
    {
        return _info;
    }

    bool incoming = !_connector;
    _info = _transceiver->getInfo(incoming, _adapter ? _adapter->getName() : string{}, _endpoint->connectionId());
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
    assert(!_inactivityTimerTaskScheduled);
    assert(_inactivityTimerTask);

    _inactivityTimerTaskScheduled = true;
    _timer->schedule(_inactivityTimerTask, _inactivityTimeout);
}

void
ConnectionI::cancelInactivityTimerTask()
{
    // Called with the ConnectionI mutex locked.
    if (_inactivityTimerTaskScheduled && _inactivityTimerTask)
    {
        _inactivityTimerTaskScheduled = false;
        _timer->cancel(_inactivityTimerTask);
    }
}

void
ConnectionI::scheduleCloseTimerTask()
{
    // Called with the ConnectionI mutex locked.

    if (_closeTimeout > chrono::seconds::zero())
    {
        // We schedule a new task every time this function is called.
        _timer->schedule(make_shared<CloseTimerTask>(shared_from_this()), _closeTimeout);
    }
}

void
ConnectionI::doApplicationClose() noexcept
{
    // Called with the ConnectionI mutex locked.
    assert(_state < StateClosing);

    setState(
        StateClosing,
        make_exception_ptr(
            ConnectionClosedException{__FILE__, __LINE__, "connection closed gracefully by the application", true}));
}
