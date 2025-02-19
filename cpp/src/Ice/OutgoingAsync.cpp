// Copyright (c) ZeroC, Inc.

#include "Ice/OutgoingAsync.h"
#include "CollocatedRequestHandler.h"
#include "ConnectionFactory.h"
#include "ConnectionI.h"
#include "Ice/ImplicitContext.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "Ice/ReplyStatus.h"
#include "Instance.h"
#include "LocatorInfo.h"
#include "ObjectAdapterFactory.h"
#include "Reference.h"
#include "RequestHandlerCache.h"
#include "RetryQueue.h"
#include "RouterInfo.h"
#include "ThreadPool.h"
#include "TraceLevels.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

const unsigned char OutgoingAsyncBase::OK = 0x1;
const unsigned char OutgoingAsyncBase::Sent = 0x2;

OutgoingAsyncCompletionCallback::~OutgoingAsyncCompletionCallback() = default; // Out of line to avoid weak vtable

bool
OutgoingAsyncBase::sent()
{
    return sentImpl(true);
}

bool
OutgoingAsyncBase::exception(std::exception_ptr ex)
{
    return exceptionImpl(ex);
}

bool
OutgoingAsyncBase::response()
{
    assert(false); // Must be overridden by request that can handle responses
    return false;
}

void
OutgoingAsyncBase::invokeSentAsync()
{
    //
    // This is called when it's not safe to call the sent callback
    // synchronously from this thread. Instead the exception callback
    // is called asynchronously from the client thread pool.
    //
    try
    {
        _instance->clientThreadPool()->execute(
            [self = shared_from_this()]() { self->invokeSent(); },
            _cachedConnection);
    }
    catch (const CommunicatorDestroyedException&)
    {
    }
}

void
OutgoingAsyncBase::invokeExceptionAsync()
{
    //
    // CommunicatorDestroyedException is the only exception that can propagate directly from this method.
    //
    _instance->clientThreadPool()->execute(
        [self = shared_from_this()]() { self->invokeException(); },
        _cachedConnection);
}

void
OutgoingAsyncBase::invokeResponseAsync()
{
    //
    // CommunicatorDestroyedException is the only exception that can propagate directly from this method.
    //
    _instance->clientThreadPool()->execute(
        [self = shared_from_this()]() { self->invokeResponse(); },
        _cachedConnection);
}

void
OutgoingAsyncBase::invokeSent()
{
    try
    {
        handleInvokeSent(_sentSynchronously, this);
    }
    catch (...)
    {
        warning("sent", current_exception());
    }

    if (_observer && _doneInSent)
    {
        _observer.detach();
    }
}

void
OutgoingAsyncBase::invokeException()
{
    try
    {
        handleInvokeException(_ex, this);
    }
    catch (...)
    {
        warning("exception", current_exception());
    }

    _observer.detach();
}

void
OutgoingAsyncBase::invokeResponse()
{
    if (_ex)
    {
        invokeException();
        return;
    }

    try
    {
        handleInvokeResponse(_state & OK, this);
    }
    catch (...)
    {
        // With the lambda async API, lambdaInvokeResponse throws _before_ reaching the application's response when the
        // unmarshaling fails or when the response contains a user exception. We want to call handleInvokeException
        // in this situation.
        if (handleException(current_exception()))
        {
            try
            {
                handleInvokeException(current_exception(), this);
            }
            catch (...)
            {
                warning("exception", current_exception());
            }
        }
    }

    _observer.detach();
}

void
OutgoingAsyncBase::cancelable(const CancellationHandlerPtr& handler)
{
    Lock sync(_m);
    if (_cancellationException)
    {
        try
        {
            rethrow_exception(_cancellationException);
        }
        catch (const LocalException&)
        {
            _cancellationException = nullptr;
            throw;
        }
    }
    _cancellationHandler = handler;
}

void
OutgoingAsyncBase::cancel()
{
    cancel(make_exception_ptr(InvocationCanceledException(__FILE__, __LINE__)));
}

void
OutgoingAsyncBase::attachRemoteObserver(const ConnectionInfoPtr& c, const EndpointPtr& endpt, std::int32_t requestId)
{
    const auto size = static_cast<std::int32_t>(_os.b.size() - headerSize - 4);
    _childObserver.attach(getObserver().getRemoteObserver(c, endpt, requestId, size));
}

void
OutgoingAsyncBase::attachCollocatedObserver(const ObjectAdapterPtr& adapter, std::int32_t requestId)
{
    const auto size = static_cast<std::int32_t>(_os.b.size() - headerSize - 4);
    _childObserver.attach(getObserver().getCollocatedObserver(adapter, requestId, size));
}

OutgoingAsyncBase::OutgoingAsyncBase(const InstancePtr& instance)
    : _instance(instance),
      _os(instance.get(), currentProtocolEncoding),
      _is{instance.get(), currentProtocolEncoding}
{
}

bool
OutgoingAsyncBase::sentImpl(bool done)
{
    Lock sync(_m);
    bool alreadySent = (_state & Sent) > 0;
    _state |= Sent;
    if (done)
    {
        _doneInSent = true;
        _childObserver.detach();
        _cancellationHandler = nullptr;
    }

    bool invoke = handleSent(done, alreadySent);
    if (!invoke && _doneInSent)
    {
        _observer.detach();
    }
    return invoke;
}

bool
OutgoingAsyncBase::exceptionImpl(std::exception_ptr ex)
{
    Lock sync(_m);
    _ex = ex;
    if (_childObserver)
    {
        _childObserver.failed(getExceptionId(ex));
        _childObserver.detach();
    }
    _cancellationHandler = nullptr;
    _observer.failed(getExceptionId(ex));

    bool invoke = handleException(ex);
    if (!invoke)
    {
        _observer.detach();
    }
    return invoke;
}

bool
OutgoingAsyncBase::responseImpl(bool ok, bool invoke)
{
    Lock sync(_m);
    if (ok)
    {
        _state |= OK;
    }

    _cancellationHandler = nullptr;

    try
    {
        invoke &= handleResponse(ok);
    }
    catch (const Exception&)
    {
        _ex = current_exception();
        invoke = handleException(_ex);
    }
    if (!invoke)
    {
        _observer.detach();
    }
    return invoke;
}

void
OutgoingAsyncBase::cancel(std::exception_ptr ex)
{
    CancellationHandlerPtr handler;
    {
        Lock sync(_m);
        if (!_cancellationHandler)
        {
            _cancellationException = ex;
            return;
        }
        handler = _cancellationHandler;
    }
    handler->asyncRequestCanceled(shared_from_this(), ex);
}

void
OutgoingAsyncBase::warning(string_view callbackName, std::exception_ptr eptr) const
{
    if (_instance->initializationData().properties->getIcePropertyAsInt("Ice.Warn.AMICallback") > 0)
    {
        Warning out(_instance->initializationData().logger);
        try
        {
            rethrow_exception(eptr);
        }
        catch (const Exception& ex)
        {
            out << "Ice::Exception raised by " << callbackName << " callback:\n" << ex;
        }
        catch (const std::exception& ex)
        {
            out << "std::exception raised by " << callbackName << " callback:\n" << ex.what();
        }
        catch (...)
        {
            out << "unknown exception raised by " << callbackName << " callback";
        }
    }
}

bool
ProxyOutgoingAsyncBase::exception(std::exception_ptr exc)
{
    if (_childObserver)
    {
        _childObserver.failed(getExceptionId(exc));
        _childObserver.detach();
    }

    _cachedConnection = nullptr;

    //
    // NOTE: at this point, synchronization isn't needed, no other threads should be
    // calling on the callback.
    //
    try
    {
        //
        // It's important to let the retry queue do the retry even if
        // the retry interval is 0. This method can be called with the
        // connection locked so we can't just retry here.
        //
        _instance->retryQueue()->add(shared_from_this(), handleRetryAfterException(exc));

        return false;
    }
    catch (const Exception&)
    {
        return exceptionImpl(current_exception()); // No retries, we're done
    }
}

void
ProxyOutgoingAsyncBase::retryException()
{
    try
    {
        // It's important to let the retry queue do the retry. This is
        // called from the connect request handler and the retry might
        // require could end up waiting for the flush of the
        // connection to be done.

        // Clear cached request handler and always retry.
        _proxy._getRequestHandlerCache()->clearCachedRequestHandler(_handler);
        _instance->retryQueue()->add(shared_from_this(), 0);
    }
    catch (const Exception&)
    {
        if (exception(current_exception()))
        {
            invokeExceptionAsync();
        }
    }
}

void
ProxyOutgoingAsyncBase::retry()
{
    invokeImpl(false);
}

void
ProxyOutgoingAsyncBase::abort(std::exception_ptr ex)
{
    assert(!_childObserver);

    if (exceptionImpl(ex))
    {
        invokeExceptionAsync();
    }
    else
    {
        try
        {
            rethrow_exception(ex);
        }
        catch (const CommunicatorDestroyedException&)
        {
            //
            // If it's a communicator destroyed exception, don't swallow
            // it but instead notify the user thread. Even if no callback
            // was provided.
            //
            throw;
        }
        catch (...)
        {
            // ignored.
        }
    }
}

ProxyOutgoingAsyncBase::ProxyOutgoingAsyncBase(ObjectPrx proxy)
    : OutgoingAsyncBase(proxy->_getReference()->getInstance()),
      _proxy(std::move(proxy))
{
}

ProxyOutgoingAsyncBase::~ProxyOutgoingAsyncBase() = default;

void
ProxyOutgoingAsyncBase::invokeImpl(bool userThread)
{
    try
    {
        if (userThread)
        {
            chrono::milliseconds invocationTimeout = _proxy._getReference()->getInvocationTimeout();
            if (invocationTimeout > 0ms)
            {
                _instance->timer()->schedule(shared_from_this(), invocationTimeout);
            }
        }
        else
        {
            _observer.retried();
        }

        while (true)
        {
            try
            {
                _sent = false;
                _handler = _proxy._getRequestHandlerCache()->getRequestHandler();
                AsyncStatus status = _handler->sendAsyncRequest(shared_from_this());
                if (status & AsyncStatusSent)
                {
                    if (userThread)
                    {
                        _sentSynchronously = true;
                        if (status & AsyncStatusInvokeSentCallback)
                        {
                            invokeSent(); // Call the sent callback from the user thread.
                        }
                    }
                    else
                    {
                        if (status & AsyncStatusInvokeSentCallback)
                        {
                            invokeSentAsync(); // Call the sent callback from a client thread pool thread.
                        }
                    }
                }
                return; // We're done!
            }
            catch (const RetryException&)
            {
                // Clear request handler and always retry.
                _proxy._getRequestHandlerCache()->clearCachedRequestHandler(_handler);
            }
            catch (const Exception& ex)
            {
                if (_childObserver)
                {
                    _childObserver.failed(ex.ice_id());
                    _childObserver.detach();
                }
                int interval = handleRetryAfterException(current_exception());

                if (interval > 0)
                {
                    _instance->retryQueue()->add(shared_from_this(), interval);
                    return;
                }
                else
                {
                    _observer.retried();
                }
            }
        }
    }
    catch (const Exception&)
    {
        // If called from the user thread we re-throw, the exception will be caught by the caller and handled using
        // abort.
        if (userThread)
        {
            throw;
        }
        else if (exceptionImpl(current_exception())) // No retries, we're done
        {
            invokeExceptionAsync();
        }
    }
}

bool
ProxyOutgoingAsyncBase::sentImpl(bool done)
{
    _sent = true;
    if (done)
    {
        if (_proxy._getReference()->getInvocationTimeout() > 0ms)
        {
            _instance->timer()->cancel(shared_from_this());
        }
    }
    return OutgoingAsyncBase::sentImpl(done);
}

bool
ProxyOutgoingAsyncBase::exceptionImpl(std::exception_ptr ex)
{
    if (_proxy._getReference()->getInvocationTimeout() > 0ms)
    {
        _instance->timer()->cancel(shared_from_this());
    }
    return OutgoingAsyncBase::exceptionImpl(ex);
}

bool
ProxyOutgoingAsyncBase::responseImpl(bool ok, bool invoke)
{
    if (_proxy._getReference()->getInvocationTimeout() > 0ms)
    {
        _instance->timer()->cancel(shared_from_this());
    }
    return OutgoingAsyncBase::responseImpl(ok, invoke);
}

void
ProxyOutgoingAsyncBase::runTimerTask()
{
    cancel(make_exception_ptr(InvocationTimeoutException(__FILE__, __LINE__)));
}

int
ProxyOutgoingAsyncBase::handleRetryAfterException(std::exception_ptr ex)
{
    // Clear the request handler
    _proxy->_getRequestHandlerCache()->clearCachedRequestHandler(_handler);

    // We only retry local exceptions, system exceptions aren't retried.
    //
    // A CloseConnectionException indicates graceful server shutdown, and is therefore
    // always repeatable without violating "at-most-once". That's because by sending a
    // close connection message, the server guarantees that all outstanding requests
    // can safely be repeated.
    //
    // An ObjectNotExistException can always be retried as well without violating
    // "at-most-once".
    //
    // If the request didn't get sent or if it's non-mutating or idempotent it can
    // also always be retried if the retry count isn't reached.
    try
    {
        rethrow_exception(ex);
    }
    catch (const Ice::LocalException& localEx)
    {
        if (!_sent || _mode != OperationMode::Normal || dynamic_cast<const CloseConnectionException*>(&localEx) ||
            dynamic_cast<const ObjectNotExistException*>(&localEx))
        {
            try
            {
                return checkRetryAfterException(ex);
            }
            catch (const CommunicatorDestroyedException&)
            {
                rethrow_exception(ex); // The communicator is already destroyed, so we cannot retry.
            }
        }
        else
        {
            throw; // Retry could break at-most-once semantics, don't retry.
        }

        // gcc complains without this return statement.
        assert(false);
        return 0;
    }
}

int
ProxyOutgoingAsyncBase::checkRetryAfterException(std::exception_ptr ex)
{
    const ReferencePtr& ref = _proxy._getReference();
    const InstancePtr& instance = ref->getInstance();

    TraceLevelsPtr traceLevels = instance->traceLevels();
    LoggerPtr logger = instance->initializationData().logger;

    // We don't retry batch requests because the exception might have
    // caused all the requests batched with the connection to be
    // aborted and we want the application to be notified.
    if (ref->isBatch())
    {
        rethrow_exception(ex);
    }

    // If it's a fixed proxy, retrying isn't useful as the proxy is tied to
    // the connection and the request will fail with the exception.
    if (dynamic_pointer_cast<FixedReference>(ref))
    {
        rethrow_exception(ex);
    }

    bool isCloseConnectionException = false;
    string errorMessage;
    try
    {
        rethrow_exception(ex);
    }
    catch (const ObjectNotExistException& one)
    {
        if (ref->getRouterInfo() && one.operation() == "ice_add_proxy")
        {
            // If we have a router, an ObjectNotExistException with an
            // operation name "ice_add_proxy" indicates to the client
            // that the router isn't aware of the proxy (for example,
            // because it was evicted by the router). In this case, we
            // must *always* retry, so that the missing proxy is added
            // to the router.

            ref->getRouterInfo()->clearCache(ref);

            if (traceLevels->retry >= 1)
            {
                Trace out(logger, traceLevels->retryCat);
                out << "retrying operation call to add proxy to router\n" << one;
            }

            return 0; // We must always retry, so we don't look at the retry count.
        }
        else if (ref->isIndirect())
        {
            // We retry ObjectNotExistException if the reference is indirect.

            if (ref->isWellKnown())
            {
                LocatorInfoPtr li = ref->getLocatorInfo();
                if (li)
                {
                    li->clearCache(ref);
                }
            }
        }
        else
        {
            // For all other cases, we don't retry ObjectNotExistException.
            throw;
        }
    }
    catch (const RequestFailedException&)
    {
        // We don't retry other *NotExistException, which are all
        // derived from RequestFailedException.
        throw;
    }
    catch (const MarshalException&)
    {
        // There is no point in retrying an operation that resulted in a
        // MarshalException. This must have been raised locally (because
        // if it happened in a server it would result in an
        // UnknownLocalException instead), which means there was a problem
        // in this process that will not change if we try again.
        //
        // A likely cause for a MarshalException is exceeding the
        // maximum message size. For example, a client can attempt to send
        // a message that exceeds the maximum memory size, or accumulate
        // enough batch requests without flushing that the maximum size is
        // reached.
        //
        // This latter case is especially problematic, because if we were
        // to retry a batch request after a MarshalException, we would in
        // fact silently discard the accumulated requests and allow new
        // batch requests to accumulate. If the subsequent batched
        // requests do not exceed the maximum message size, it appears to
        // the client that all of the batched requests were accepted, when
        // in reality only the last few are actually sent.
        throw;
    }
    catch (const CommunicatorDestroyedException&)
    {
        throw;
    }
    catch (const ObjectAdapterDeactivatedException&)
    {
        throw;
    }
    catch (const ObjectAdapterDestroyedException&)
    {
        throw;
    }
    catch (const ConnectionAbortedException& connectionAbortedException)
    {
        if (connectionAbortedException.closedByApplication())
        {
            throw; // do not retry
        }
        errorMessage = connectionAbortedException.what();
        // and retry
    }
    catch (const ConnectionClosedException& connectionClosedException)
    {
        if (connectionClosedException.closedByApplication())
        {
            throw; // do not retry
        }
        errorMessage = connectionClosedException.what();
        // and retry
    }
    catch (const InvocationTimeoutException&)
    {
        throw;
    }
    catch (const InvocationCanceledException&)
    {
        throw;
    }
    catch (const CloseConnectionException& e)
    {
        isCloseConnectionException = true;
        errorMessage = e.what();
        // and retry
    }
    catch (const std::exception& e)
    {
        errorMessage = e.what();
        // We retry on all other exceptions!
    }

    ++_cnt;
    assert(_cnt > 0);

    const auto& retryIntervals = instance->retryIntervals();
    int interval = -1;
    if (_cnt == static_cast<int>(retryIntervals.size() + 1) && isCloseConnectionException)
    {
        // A close connection exception is always retried at least once, even if the retry
        // limit is reached.
        interval = 0;
    }
    else if (_cnt > static_cast<int>(retryIntervals.size()))
    {
        if (traceLevels->retry >= 1)
        {
            Trace out(logger, traceLevels->retryCat);
            out << "cannot retry operation call because retry limit has been exceeded\n" << errorMessage;
        }
        rethrow_exception(ex);
    }
    else
    {
        interval = retryIntervals[static_cast<size_t>(_cnt - 1)];
    }

    if (traceLevels->retry >= 1)
    {
        Trace out(logger, traceLevels->retryCat);
        out << "retrying operation call";
        if (interval > 0)
        {
            out << " in " << interval << "ms";
        }
        out << " because of exception\n" << errorMessage;
    }
    return interval;
}

OutgoingAsync::OutgoingAsync(ObjectPrx proxy, bool synchronous)
    : ProxyOutgoingAsyncBase(std::move(proxy)),
      _encoding(_proxy->_getReference()->getEncoding()),
      _synchronous(synchronous)
{
}

void
OutgoingAsync::prepare(string_view operation, OperationMode mode, const Context& context)
{
    if (_proxy._getReference()->getProtocol().major != currentProtocol.major)
    {
        throw FeatureNotSupportedException{
            __FILE__,
            __LINE__,
            "cannot send request using protocol version " +
                Ice::protocolVersionToString(_proxy._getReference()->getProtocol())};
    }

    _mode = mode;

    _observer.attach(_proxy, operation, context);

    // We need to check isBatch() and not if getBatchRequestQueue() is not null: for a fixed proxy,
    // getBatchRequestQueue() always returns a non null value.
    if (_proxy._getReference()->isBatch())
    {
        _proxy._getReference()->getBatchRequestQueue()->prepareBatchRequest(&_os);
    }
    else
    {
        _os.writeBlob(requestHdr, sizeof(requestHdr));
    }

    Reference* ref = _proxy._getReference().get();

    _os.write(ref->getIdentity());

    //
    // For compatibility with the old FacetPath.
    //
    if (ref->getFacet().empty())
    {
        _os.write(static_cast<string*>(nullptr), static_cast<string*>(nullptr));
    }
    else
    {
        string facet = ref->getFacet();
        _os.write(&facet, &facet + 1);
    }

    _os.write(operation, false);

    _os.write(static_cast<uint8_t>(_mode));

    if (&context != &noExplicitContext)
    {
        //
        // Explicit context
        //
        _os.write(context);
    }
    else
    {
        //
        // Implicit context
        //
        const ImplicitContextPtr& implicitContext = ref->getInstance()->getImplicitContext();
        const Context& prxContext = ref->getContext()->getValue();
        if (implicitContext)
        {
            implicitContext->write(prxContext, &_os);
        }
        else
        {
            _os.write(prxContext);
        }
    }
}

bool
OutgoingAsync::sent()
{
    return ProxyOutgoingAsyncBase::sentImpl(
        !_proxy._getReference()->isTwoway()); // done = true if it's not a two-way proxy
}

bool
OutgoingAsync::response()
{
    //
    // NOTE: this method is called from ConnectionI.parseMessage
    // with the connection locked. Therefore, it must not invoke
    // any user callbacks.
    //
    assert(_proxy._getReference()->isTwoway()); // Can only be called for twoways.

    if (_childObserver)
    {
        _childObserver->reply(static_cast<int32_t>(_is.b.size() - headerSize - 4));
        _childObserver.detach();
    }

    try
    {
        // We can't use the generated code to unmarshal a possibly unknown reply status enumerator.
        uint8_t replyStatusByte;
        _is.read(replyStatusByte);
        ReplyStatus replyStatus{replyStatusByte};

        switch (replyStatus)
        {
            case ReplyStatus::Ok:
                break;
            case ReplyStatus::UserException:
                _observer.userException();
                break;

            case ReplyStatus::ObjectNotExist:
            case ReplyStatus::FacetNotExist:
            case ReplyStatus::OperationNotExist:
            {
                Identity ident;
                _is.read(ident);

                //
                // For compatibility with the old FacetPath.
                //
                vector<string> facetPath;
                _is.read(facetPath);
                string facet;
                if (!facetPath.empty())
                {
                    if (facetPath.size() > 1)
                    {
                        throw MarshalException{__FILE__, __LINE__, "received facet path with more than one element"};
                    }
                    facet.swap(facetPath[0]);
                }

                string operation;
                _is.read(operation, false);
                switch (replyStatus)
                {
                    case ReplyStatus::ObjectNotExist:
                        throw ObjectNotExistException{
                            __FILE__,
                            __LINE__,
                            std::move(ident),
                            std::move(facet),
                            std::move(operation)};
                        break;

                    case ReplyStatus::FacetNotExist:
                        throw FacetNotExistException{
                            __FILE__,
                            __LINE__,
                            std::move(ident),
                            std::move(facet),
                            std::move(operation)};
                        break;

                    default:
                        throw OperationNotExistException{
                            __FILE__,
                            __LINE__,
                            std::move(ident),
                            std::move(facet),
                            std::move(operation)};
                        break;
                }
                break;
            }

            default:
            {
                string message;
                _is.read(message, false);

                switch (replyStatus)
                {
                    case ReplyStatus::UnknownException:
                        throw UnknownException{__FILE__, __LINE__, std::move(message)};
                        break;

                    case ReplyStatus::UnknownLocalException:
                        throw UnknownLocalException{__FILE__, __LINE__, std::move(message)};
                        break;

                    case ReplyStatus::UnknownUserException:
                        throw UnknownUserException{__FILE__, __LINE__, std::move(message)};
                        break;

                    default:
                        throw DispatchException{__FILE__, __LINE__, replyStatus, std::move(message)};
                }
            }
        }

        return responseImpl(replyStatus == ReplyStatus::Ok, true);
    }
    catch (const Exception&)
    {
        return exception(current_exception());
    }
}

AsyncStatus
OutgoingAsync::invokeRemote(const ConnectionIPtr& connection, bool compress, bool response)
{
    _cachedConnection = connection;
    return connection->sendAsyncRequest(shared_from_this(), compress, response, 0);
}

AsyncStatus
OutgoingAsync::invokeCollocated(CollocatedRequestHandler* handler)
{
    return handler->invokeAsyncRequest(this, 0, _synchronous);
}

void
OutgoingAsync::abort(std::exception_ptr ex)
{
    if (_proxy._getReference()->isBatch())
    {
        //
        // If we didn't finish a batch oneway or datagram request, we
        // must notify the connection about that we give up ownership
        // of the batch stream.
        //
        _proxy._getReference()->getBatchRequestQueue()->abortBatchRequest(&_os);
    }

    ProxyOutgoingAsyncBase::abort(ex);
}

void
OutgoingAsync::invoke(string_view operation)
{
    if (_proxy._getReference()->isBatch())
    {
        _sentSynchronously = true;
        _proxy._getReference()->getBatchRequestQueue()->finishBatchRequest(&_os, _proxy, operation);
        responseImpl(true, false); // Don't call sent/completed callback for batch AMI requests
        return;
    }

    // invokeImpl can throw. The exception should be handled by calling abort (in the caller).
    invokeImpl(true); // userThread = true
}

void
OutgoingAsync::invoke(
    string_view operation,
    OperationMode mode,
    optional<FormatType> format,
    const Context& context,
    const function<void(OutputStream*)>& write)
{
    try
    {
        prepare(operation, mode, context);
        if (write)
        {
            _os.startEncapsulation(_encoding, format);
            write(&_os);
            _os.endEncapsulation();
        }
        else
        {
            _os.writeEmptyEncapsulation(_encoding);
        }
        invoke(operation);
    }
    catch (const Exception&)
    {
        abort(current_exception());
    }
}

void
OutgoingAsync::throwUserException()
{
    try
    {
        _is.startEncapsulation();
        _is.throwException();
    }
    catch (const UserException& ex)
    {
        _is.endEncapsulation();
        if (_userException)
        {
            _userException(ex);
        }
        throw UnknownUserException::fromTypeId(__FILE__, __LINE__, ex.ice_id());
    }
}

bool
LambdaInvoke::handleSent(bool, bool alreadySent) noexcept
{
    return _sent != nullptr && !alreadySent; // Invoke the sent callback only if not already invoked.
}

bool
LambdaInvoke::handleException(std::exception_ptr) noexcept
{
    return _exception != nullptr; // Invoke the callback
}

bool
LambdaInvoke::handleResponse(bool)
{
    return _response != nullptr;
}

void
LambdaInvoke::handleInvokeSent(bool sentSynchronously, OutgoingAsyncBase*) const
{
    _sent(sentSynchronously);
}

void
LambdaInvoke::handleInvokeException(std::exception_ptr ex, OutgoingAsyncBase*) const
{
    _exception(ex);
}

void
LambdaInvoke::handleInvokeResponse(bool ok, OutgoingAsyncBase*) const
{
    _response(ok);
}
