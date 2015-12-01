// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/OutgoingAsync.h>
#include <Ice/ConnectionI.h>
#include <Ice/CollocatedRequestHandler.h>
#include <Ice/Reference.h>
#include <Ice/Instance.h>
#include <Ice/LocalException.h>
#include <Ice/ReplyStatus.h>
#include <Ice/ImplicitContextI.h>
#include <Ice/ThreadPool.h>
#include <Ice/RetryQueue.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

#ifndef ICE_CPP11_MAPPING
IceUtil::Shared* IceInternal::upCast(OutgoingAsyncBase* p) { return p; }
IceUtil::Shared* IceInternal::upCast(ProxyOutgoingAsyncBase* p) { return p; }
IceUtil::Shared* IceInternal::upCast(OutgoingAsync* p) { return p; }
IceUtil::Shared* IceInternal::upCast(CommunicatorFlushBatchAsync* p) { return p; }
#endif

bool
OutgoingAsyncBase::sent()
{
    return sent(true);
}

bool
OutgoingAsyncBase::completed(const Exception& ex)
{
    return finished(ex);
}

bool
OutgoingAsyncBase::completed()
{
    assert(false); // Must be overriden by request that can handle responses
    return false;
}

BasicStream*
OutgoingAsyncBase::getIs()
{
    return 0; // Must be overriden by request that can handle responses
}

#ifdef ICE_CPP11_MAPPING
OutgoingAsyncBase::OutgoingAsyncBase(const CommunicatorPtr& communicator,
                                     const InstancePtr& instance,
                                     const string& operation,
                                     const CallbackBasePtr& delegate) :
    AsyncResult(communicator, instance, operation, delegate),
#else
OutgoingAsyncBase::OutgoingAsyncBase(const CommunicatorPtr& communicator,
                                     const InstancePtr& instance,
                                     const string& operation,
                                     const CallbackBasePtr& delegate,
                                     const LocalObjectPtr& cookie) :
    AsyncResult(communicator, instance, operation, delegate, cookie),
#endif
    _os(instance.get(), Ice::currentProtocolEncoding)
{
}

bool
OutgoingAsyncBase::sent(bool done)
{
    if(done)
    {
        _childObserver.detach();
    }
    return AsyncResult::sent(done);
}

bool
OutgoingAsyncBase::finished(const Exception& ex)
{
    if(_childObserver)
    {
        _childObserver.failed(ex.ice_name());
        _childObserver.detach();
    }
    return AsyncResult::finished(ex);
}

Ice::ObjectPrxPtr
ProxyOutgoingAsyncBase::getProxy() const
{
    return _proxy;
}

bool
ProxyOutgoingAsyncBase::completed(const Exception& exc)
{
    if(_childObserver)
    {
        _childObserver.failed(exc.ice_name());
        _childObserver.detach();
    }

    _cachedConnection = 0;
    if(_proxy->__reference()->getInvocationTimeout() == -2)
    {
#ifdef ICE_CPP11_MAPPING
        _instance->timer()->cancel(dynamic_pointer_cast<TimerTask>(shared_from_this()));
#else
        _instance->timer()->cancel(this);
#endif
    }

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
#ifdef ICE_CPP11_MAPPING
        _instance->retryQueue()->add(dynamic_pointer_cast<ProxyOutgoingAsyncBase>(shared_from_this()),
                                     handleException(exc));
#else
        _instance->retryQueue()->add(this, handleException(exc));
#endif
        return false;
    }
    catch(const Exception& ex)
    {
        return finished(ex); // No retries, we're done
    }
}

void
ProxyOutgoingAsyncBase::retryException(const Exception& ex)
{
    try
    {
        //
        // It's important to let the retry queue do the retry. This is
        // called from the connect request handler and the retry might
        // require could end up waiting for the flush of the
        // connection to be done.
        //
        _proxy->__updateRequestHandler(_handler, 0); // Clear request handler and always retry.
#ifdef ICE_CPP11_MAPPING
        _instance->retryQueue()->add(dynamic_pointer_cast<ProxyOutgoingAsyncBase>(shared_from_this()), 0);
#else
        _instance->retryQueue()->add(this, 0);
#endif
    }
    catch(const Ice::Exception& exc)
    {
        if(completed(exc))
        {
            invokeCompletedAsync();
        }
    }
}

void
ProxyOutgoingAsyncBase::cancelable(const CancellationHandlerPtr& handler)
{
    if(_proxy->__reference()->getInvocationTimeout() == -2 && _cachedConnection)
    {
        const int timeout = _cachedConnection->timeout();
        if(timeout > 0)
        {
#ifdef ICE_CPP11_MAPPING
            _instance->timer()->schedule(dynamic_pointer_cast<TimerTask>(shared_from_this()),
                                         IceUtil::Time::milliSeconds(timeout));
#else
            _instance->timer()->schedule(this, IceUtil::Time::milliSeconds(timeout));
#endif
        }
    }
    AsyncResult::cancelable(handler);
}

void
ProxyOutgoingAsyncBase::retry()
{
    invokeImpl(false);
}

void
ProxyOutgoingAsyncBase::abort(const Ice::Exception& ex)
{
    assert(!_childObserver);

    if(finished(ex))
    {
        invokeCompletedAsync();
    }
    else if(dynamic_cast<const Ice::CommunicatorDestroyedException*>(&ex))
    {
        //
        // If it's a communicator destroyed exception, don't swallow
        // it but instead notify the user thread. Even if no callback
        // was provided.
        //
        ex.ice_throw();
    }
}

#ifdef ICE_CPP11_MAPPING
ProxyOutgoingAsyncBase::ProxyOutgoingAsyncBase(const ObjectPrxPtr& prx,
                                               const string& operation,
                                               const CallbackBasePtr& delegate) :
    OutgoingAsyncBase(prx->ice_getCommunicator(), prx->__reference()->getInstance(), operation, delegate),
#else
ProxyOutgoingAsyncBase::ProxyOutgoingAsyncBase(const ObjectPrxPtr& prx,
                                               const string& operation,
                                               const CallbackBasePtr& delegate,
                                               const LocalObjectPtr& cookie) :
    OutgoingAsyncBase(prx->ice_getCommunicator(), prx->__reference()->getInstance(), operation, delegate, cookie),
#endif
    _proxy(prx),
    _mode(ICE_ENUM(OperationMode, Normal)),
    _cnt(0),
    _sent(false)
{
}

void
ProxyOutgoingAsyncBase::invokeImpl(bool userThread)
{
    try
    {
        if(userThread)
        {
            int invocationTimeout = _proxy->__reference()->getInvocationTimeout();
            if(invocationTimeout > 0)
            {
#ifdef ICE_CPP11_MAPPING
                _instance->timer()->schedule(dynamic_pointer_cast<TimerTask>(shared_from_this()),
                                             IceUtil::Time::milliSeconds(invocationTimeout));
#else
                _instance->timer()->schedule(this, IceUtil::Time::milliSeconds(invocationTimeout));
#endif
            }
        }
        else
        {
            _observer.retried();
        }

        while(true)
        {
            try
            {
                _sent = false;
                _handler = _proxy->__getRequestHandler();
#ifdef ICE_CPP11_MAPPING
                AsyncStatus status = _handler->sendAsyncRequest(
                                                    dynamic_pointer_cast<ProxyOutgoingAsyncBase>(shared_from_this()));
#else
                AsyncStatus status = _handler->sendAsyncRequest(this);
#endif
                if(status & AsyncStatusSent)
                {
                    if(userThread)
                    {
                        _sentSynchronously = true;
                        if(status & AsyncStatusInvokeSentCallback)
                        {
                            invokeSent(); // Call the sent callback from the user thread.
                        }
                    }
                    else
                    {
                        if(status & AsyncStatusInvokeSentCallback)
                        {
                            invokeSentAsync(); // Call the sent callback from a client thread pool thread.
                        }
                    }
                }
                return; // We're done!
            }
            catch(const RetryException&)
            {
                _proxy->__updateRequestHandler(_handler, 0); // Clear request handler and always retry.
            }
            catch(const Exception& ex)
            {
                if(_childObserver)
                {
                    _childObserver.failed(ex.ice_name());
                    _childObserver.detach();
                }
                int interval = handleException(ex);
                if(interval > 0)
                {
#ifdef ICE_CPP11_MAPPING
                    _instance->retryQueue()->add(dynamic_pointer_cast<ProxyOutgoingAsyncBase>(shared_from_this()),
                                                 interval);
#else
                    _instance->retryQueue()->add(this, interval);
#endif
                    return;
                }
                else
                {
                    _observer.retried();
                }
            }
        }
    }
    catch(const Exception& ex)
    {
        //
        // If called from the user thread we re-throw, the exception
        // will be catch by the caller and abort() will be called.
        //
        if(userThread)
        {
            throw;
        }
        else if(finished(ex)) // No retries, we're done
        {
            invokeCompletedAsync();
        }
    }
}

bool
ProxyOutgoingAsyncBase::sent(bool done)
{
    _sent = true;
    if(done)
    {
        if(_proxy->__reference()->getInvocationTimeout() != -1)
        {
#ifdef ICE_CPP11_MAPPING
            _instance->timer()->cancel(dynamic_pointer_cast<TimerTask>(shared_from_this()));
#else
            _instance->timer()->cancel(this);      
#endif
        }
    }
    return OutgoingAsyncBase::sent(done);
}

bool
ProxyOutgoingAsyncBase::finished(const Exception& ex)
{
    if(_proxy->__reference()->getInvocationTimeout() != -1)
    {
#ifdef ICE_CPP11_MAPPING
        _instance->timer()->cancel(dynamic_pointer_cast<TimerTask>(shared_from_this()));
#else
        _instance->timer()->cancel(this);      
#endif
    }
    return OutgoingAsyncBase::finished(ex);
}

bool
ProxyOutgoingAsyncBase::finished(bool ok)
{
    if(_proxy->__reference()->getInvocationTimeout() != -1)
    {
#ifdef ICE_CPP11_MAPPING
        _instance->timer()->cancel(dynamic_pointer_cast<TimerTask>(shared_from_this()));
#else
        _instance->timer()->cancel(this);      
#endif
    }
    return AsyncResult::finished(ok);
}

int
ProxyOutgoingAsyncBase::handleException(const Exception& exc)
{
    return _proxy->__handleException(exc, _handler, _mode, _sent, _cnt);
}

void
ProxyOutgoingAsyncBase::runTimerTask()
{
    if(_proxy->__reference()->getInvocationTimeout() == -2)
    {
        cancel(ConnectionTimeoutException(__FILE__, __LINE__));
    }
    else
    {
        cancel(InvocationTimeoutException(__FILE__, __LINE__));
    }
}

#ifdef ICE_CPP11_MAPPING
OutgoingAsync::OutgoingAsync(const ObjectPrxPtr& prx,
                             const string& operation,
                             const CallbackBasePtr& delegate) :
    ProxyOutgoingAsyncBase(prx, operation, delegate),
#else
OutgoingAsync::OutgoingAsync(const ObjectPrxPtr& prx,
                             const string& operation,
                             const CallbackBasePtr& delegate,
                             const LocalObjectPtr& cookie) :
    ProxyOutgoingAsyncBase(prx, operation, delegate, cookie),
#endif
    _encoding(getCompatibleEncoding(prx->__reference()->getEncoding()))
{
}

void
OutgoingAsync::prepare(const string& operation, OperationMode mode, const Context* context)
{
    checkSupportedProtocol(getCompatibleProtocol(_proxy->__reference()->getProtocol()));

    _mode = mode;
    _observer.attach(_proxy, operation, context);

    switch(_proxy->__reference()->getMode())
    {
        case Reference::ModeTwoway:
        case Reference::ModeOneway:
        case Reference::ModeDatagram:
        {
            _os.writeBlob(requestHdr, sizeof(requestHdr));
            break;
        }

        case Reference::ModeBatchOneway:
        case Reference::ModeBatchDatagram:
        {
            _proxy->__getBatchRequestQueue()->prepareBatchRequest(&_os);
            break;
        }
    }

    Reference* ref = _proxy->__reference().get();

    _os.write(ref->getIdentity());

    //
    // For compatibility with the old FacetPath.
    //
    if(ref->getFacet().empty())
    {
        _os.write(static_cast<string*>(0), static_cast<string*>(0));
    }
    else
    {
        string facet = ref->getFacet();
        _os.write(&facet, &facet + 1);
    }

    _os.write(operation, false);

    _os.write(static_cast<Byte>(_mode));

    if(context != &Ice::noExplicitContext)
    {
        //
        // Explicit context
        //
        _os.write(*context);
    }
    else
    {
        //
        // Implicit context
        //
        const ImplicitContextIPtr& implicitContext = ref->getInstance()->getImplicitContext();
        const Context& prxContext = ref->getContext()->getValue();
        if(implicitContext == 0)
        {
            _os.write(prxContext);
        }
        else
        {
            implicitContext->write(prxContext, &_os);
        }
    }
}

bool
OutgoingAsync::sent()
{
    return ProxyOutgoingAsyncBase::sent(!_proxy->ice_isTwoway()); // done = true if it's not a two-way proxy
}

AsyncStatus
OutgoingAsync::invokeRemote(const ConnectionIPtr& connection, bool compress, bool response)
{
    _cachedConnection = connection;
#ifdef ICE_CPP11_MAPPING
    return connection->sendAsyncRequest(dynamic_pointer_cast<OutgoingAsyncBase>(shared_from_this()), compress, response, 0);
#else
    return connection->sendAsyncRequest(this, compress, response, 0);
#endif
}

AsyncStatus
OutgoingAsync::invokeCollocated(CollocatedRequestHandler* handler)
{
    return handler->invokeAsyncRequest(this, 0);
}

void
OutgoingAsync::abort(const Exception& ex)
{
    const Reference::Mode mode = _proxy->__reference()->getMode();
    if(mode == Reference::ModeBatchOneway || mode == Reference::ModeBatchDatagram)
    {
        //
        // If we didn't finish a batch oneway or datagram request, we
        // must notify the connection about that we give up ownership
        // of the batch stream.
        //
        _proxy->__getBatchRequestQueue()->abortBatchRequest(&_os);
    }

    ProxyOutgoingAsyncBase::abort(ex);
}

void
OutgoingAsync::invoke()
{
    const Reference::Mode mode = _proxy->__reference()->getMode();
    if(mode == Reference::ModeBatchOneway || mode == Reference::ModeBatchDatagram)
    {
        _sentSynchronously = true;
        _proxy->__getBatchRequestQueue()->finishBatchRequest(&_os, _proxy, getOperation());
        finished(true);
        return; // Don't call sent/completed callback for batch AMI requests
    }

    //
    // NOTE: invokeImpl doesn't throw so this can be called from the
    // try block with the catch block calling abort() in case of an
    // exception.
    //
    invokeImpl(true); // userThread = true
}

bool
OutgoingAsync::completed()
{
    //
    // NOTE: this method is called from ConnectionI.parseMessage
    // with the connection locked. Therefore, it must not invoke
    // any user callbacks.
    //
    assert(_proxy->ice_isTwoway()); // Can only be called for twoways.

    if(_childObserver)
    {
        _childObserver->reply(static_cast<Int>(_is.b.size() - headerSize - 4));
        _childObserver.detach();
    }

    Byte replyStatus;
    try
    {
        _is.read(replyStatus);

        switch(replyStatus)
        {
            case replyOK:
            {
                break;
            }
            case replyUserException:
            {
                _observer.userException();
                break;
            }

            case replyObjectNotExist:
            case replyFacetNotExist:
            case replyOperationNotExist:
            {
                Identity ident;
                _is.read(ident);

                //
                // For compatibility with the old FacetPath.
                //
                vector<string> facetPath;
                _is.read(facetPath);
                string facet;
                if(!facetPath.empty())
                {
                    if(facetPath.size() > 1)
                    {
                        throw MarshalException(__FILE__, __LINE__);
                    }
                    facet.swap(facetPath[0]);
                }

                string operation;
                _is.read(operation, false);

                IceUtil::UniquePtr<RequestFailedException> ex;
                switch(replyStatus)
                {
                    case replyObjectNotExist:
                    {
                        ex.reset(new ObjectNotExistException(__FILE__, __LINE__));
                        break;
                    }

                    case replyFacetNotExist:
                    {
                        ex.reset(new FacetNotExistException(__FILE__, __LINE__));
                        break;
                    }

                    case replyOperationNotExist:
                    {
                        ex.reset(new OperationNotExistException(__FILE__, __LINE__));
                        break;
                    }

                    default:
                    {
                        assert(false);
                        break;
                    }
                }

                ex->id = ident;
                ex->facet = facet;
                ex->operation = operation;
                ex->ice_throw();
            }

            case replyUnknownException:
            case replyUnknownLocalException:
            case replyUnknownUserException:
            {
                string unknown;
                _is.read(unknown, false);

                IceUtil::UniquePtr<UnknownException> ex;
                switch(replyStatus)
                {
                    case replyUnknownException:
                    {
                        ex.reset(new UnknownException(__FILE__, __LINE__));
                        break;
                    }

                    case replyUnknownLocalException:
                    {
                        ex.reset(new UnknownLocalException(__FILE__, __LINE__));
                        break;
                    }

                    case replyUnknownUserException:
                    {
                        ex.reset(new UnknownUserException(__FILE__, __LINE__));
                        break;
                    }

                    default:
                    {
                        assert(false);
                        break;
                    }
                }

                ex->unknown = unknown;
                ex->ice_throw();
            }

            default:
            {
                throw UnknownReplyStatusException(__FILE__, __LINE__);
            }
        }

        return finished(replyStatus == replyOK);
    }
    catch(const Exception& ex)
    {
        return completed(ex);
    }
}

#ifdef ICE_CPP11_MAPPING
ProxyFlushBatchAsync::ProxyFlushBatchAsync(const ObjectPrxPtr& proxy,
                                           const string& operation,
                                           const CallbackBasePtr& delegate) :
    ProxyOutgoingAsyncBase(proxy, operation, delegate)
#else
ProxyFlushBatchAsync::ProxyFlushBatchAsync(const ObjectPrxPtr& proxy,
                                           const string& operation,
                                           const CallbackBasePtr& delegate,
                                           const LocalObjectPtr& cookie) :
    ProxyOutgoingAsyncBase(proxy, operation, delegate, cookie)
#endif
{
    _observer.attach(proxy, operation, 0);
    _batchRequestNum = proxy->__getBatchRequestQueue()->swap(&_os);
}

AsyncStatus
ProxyFlushBatchAsync::invokeRemote(const ConnectionIPtr& connection, bool compress, bool)
{
    if(_batchRequestNum == 0)
    {
        if(sent())
        {
            return static_cast<AsyncStatus>(AsyncStatusSent | AsyncStatusInvokeSentCallback);
        }
        else
        {
            return AsyncStatusSent;
        }
    }
    _cachedConnection = connection;
#ifdef ICE_CPP11_MAPPING
    return connection->sendAsyncRequest(dynamic_pointer_cast<OutgoingAsyncBase>(shared_from_this()),
                                        compress, false, _batchRequestNum);
#else
    return connection->sendAsyncRequest(this, compress, false, _batchRequestNum);
#endif
}

AsyncStatus
ProxyFlushBatchAsync::invokeCollocated(CollocatedRequestHandler* handler)
{
    if(_batchRequestNum == 0)
    {
        if(sent())
        {
            return static_cast<AsyncStatus>(AsyncStatusSent | AsyncStatusInvokeSentCallback);
        }
        else
        {
            return AsyncStatusSent;
        }
    }
    return handler->invokeAsyncRequest(this, _batchRequestNum);
}

void
ProxyFlushBatchAsync::invoke()
{
    checkSupportedProtocol(getCompatibleProtocol(_proxy->__reference()->getProtocol()));
    invokeImpl(true); // userThread = true
}

#ifdef ICE_CPP11_MAPPING
ProxyGetConnection::ProxyGetConnection(const ObjectPrxPtr& prx,
                                       const string& operation,
                                       const CallbackBasePtr& delegate) :
    ProxyOutgoingAsyncBase(prx, operation, delegate)
#else
ProxyGetConnection::ProxyGetConnection(const ObjectPrxPtr& prx,
                                       const string& operation,
                                       const CallbackBasePtr& delegate,
                                       const LocalObjectPtr& cookie) :
    ProxyOutgoingAsyncBase(prx, operation, delegate, cookie)
#endif
{
    _observer.attach(prx, operation, 0);
}

AsyncStatus
ProxyGetConnection::invokeRemote(const ConnectionIPtr& connection, bool, bool)
{
    _cachedConnection = connection;
    if(finished(true))
    {
        invokeCompletedAsync();
    }
    return AsyncStatusSent;
}

AsyncStatus
ProxyGetConnection::invokeCollocated(CollocatedRequestHandler*)
{
    if(finished(true))
    {
        invokeCompletedAsync();
    }
    return AsyncStatusSent;
}

void
ProxyGetConnection::invoke()
{
    invokeImpl(true); // userThread = true
}

#ifdef ICE_CPP11_MAPPING
ConnectionFlushBatchAsync::ConnectionFlushBatchAsync(const ConnectionIPtr& connection,
                                                     const CommunicatorPtr& communicator,
                                                     const InstancePtr& instance,
                                                     const string& operation,
                                                     const CallbackBasePtr& delegate) :
    OutgoingAsyncBase(communicator, instance, operation, delegate),
#else
ConnectionFlushBatchAsync::ConnectionFlushBatchAsync(const ConnectionIPtr& connection,
                                                     const CommunicatorPtr& communicator,
                                                     const InstancePtr& instance,
                                                     const string& operation,
                                                     const CallbackBasePtr& delegate,
                                                     const LocalObjectPtr& cookie) :
    OutgoingAsyncBase(communicator, instance, operation, delegate, cookie),
#endif
    _connection(connection)
{
    _observer.attach(instance.get(), operation);
}

ConnectionPtr
ConnectionFlushBatchAsync::getConnection() const
{
    return _connection;
}

void
ConnectionFlushBatchAsync::invoke()
{
    try
    {
        AsyncStatus status;
        int batchRequestNum = _connection->getBatchRequestQueue()->swap(&_os);
        if(batchRequestNum == 0)
        {
            status = AsyncStatusSent;
            if(sent())
            {
                status = static_cast<AsyncStatus>(status | AsyncStatusInvokeSentCallback);
            }
        }
        else
        {
#ifdef ICE_CPP11_MAPPING
            status = _connection->sendAsyncRequest(
                    dynamic_pointer_cast<OutgoingAsyncBase>(shared_from_this()), false, false, batchRequestNum);
#else
            status = _connection->sendAsyncRequest(this, false, false, batchRequestNum);
#endif
        }

        if(status & AsyncStatusSent)
        {
            _sentSynchronously = true;
            if(status & AsyncStatusInvokeSentCallback)
            {
                invokeSent();
            }
        }
    }
    catch(const RetryException& ex)
    {
        if(completed(*ex.get()))
        {
            invokeCompletedAsync();
        }
    }
    catch(const Exception& ex)
    {
        if(completed(ex))
        {
            invokeCompletedAsync();
        }
    }
}

#ifdef ICE_CPP11_MAPPING
CommunicatorFlushBatchAsync::CommunicatorFlushBatchAsync(const CommunicatorPtr& communicator,
                                                         const InstancePtr& instance,
                                                         const string& operation,
                                                         const CallbackBasePtr& cb) :
    AsyncResult(communicator, instance, operation, cb)
#else
CommunicatorFlushBatchAsync::CommunicatorFlushBatchAsync(const CommunicatorPtr& communicator,
                                                         const InstancePtr& instance,
                                                         const string& operation,
                                                         const CallbackBasePtr& cb,
                                                         const LocalObjectPtr& cookie) :
    AsyncResult(communicator, instance, operation, cb, cookie)
#endif
{
    _observer.attach(instance.get(), operation);

    //
    // _useCount is initialized to 1 to prevent premature callbacks.
    // The caller must invoke ready() after all flush requests have
    // been initiated.
    //
    _useCount = 1;
}

void
CommunicatorFlushBatchAsync::flushConnection(const ConnectionIPtr& con)
{
    class FlushBatch : public OutgoingAsyncBase
    {
    public:

        FlushBatch(const CommunicatorFlushBatchAsyncPtr& outAsync,
                   const InstancePtr& instance,
                   InvocationObserver& observer) :
#ifdef ICE_CPP11_MAPPING
            OutgoingAsyncBase(outAsync->getCommunicator(), instance, outAsync->getOperation(), __dummyCallback),
#else
            OutgoingAsyncBase(outAsync->getCommunicator(), instance, outAsync->getOperation(), __dummyCallback, 0),
#endif
            _outAsync(outAsync),
            _observer(observer)
        {
        }

        virtual bool sent()
        {
            _childObserver.detach();
            _outAsync->check(false);
            return false;
        }

        virtual bool completed(const Exception& ex)
        {
            _childObserver.failed(ex.ice_name());
            _childObserver.detach();
            _outAsync->check(false);
            return false;
        }

    private:

        virtual InvocationObserver& getObserver()
        {
            return _observer;
        }

        const CommunicatorFlushBatchAsyncPtr _outAsync;
        InvocationObserver& _observer;
    };

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        ++_useCount;
    }

    try
    {
#ifdef ICE_CPP11_MAPPING
        auto flushBatch = make_shared<FlushBatch>(
            dynamic_pointer_cast<CommunicatorFlushBatchAsync>(shared_from_this()), _instance, _observer);
#else
        OutgoingAsyncBasePtr flushBatch = new FlushBatch(this, _instance, _observer);
#endif
        int batchRequestNum = con->getBatchRequestQueue()->swap(flushBatch->getOs());
        if(batchRequestNum == 0)
        {
            flushBatch->sent();
        }
        else
        {
            con->sendAsyncRequest(flushBatch, false, false, batchRequestNum);
        }
    }
    catch(const LocalException&)
    {
        check(false);
        throw;
    }
}

void
CommunicatorFlushBatchAsync::ready()
{
    check(true);
}

void
CommunicatorFlushBatchAsync::check(bool userThread)
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        assert(_useCount > 0);
        if(--_useCount > 0)
        {
            return;
        }
    }

    if(sent(true))
    {
        if(userThread)
        {
            _sentSynchronously = true;
            invokeSent();
        }
        else
        {
            invokeSentAsync();
        }
    }
}

#ifdef ICE_CPP11_MAPPING
OnewayClosureCallback::OnewayClosureCallback(
    const string& name,
    const shared_ptr<Ice::ObjectPrx>& proxy,
    function<void ()> response,
    function<void (exception_ptr)> exception,
    function<void (bool)> sent) :
    __name(name),
    __proxy(proxy),
    __response(response),
    __exception(exception),
    __sent(sent)
{
}

void
OnewayClosureCallback::sent(const AsyncResultPtr& __result) const
{
    if(__sent)
    {
        __sent(__result->sentSynchronously());
    }
}

bool
OnewayClosureCallback::hasSentCallback() const
{
    return __sent != nullptr;
}

void
OnewayClosureCallback::completed(const AsyncResultPtr& __result) const
{
    try
    {
        AsyncResult::__check(__result, __proxy.get(), __name);
        bool ok = __result->__wait();
        if(__proxy->__reference()->getMode() == Reference::ModeTwoway)
        {
            if(!ok)
            {
                try
                {
                    __result->__throwUserException();
                }
                catch(const UserException& __ex)
                {
                    throw UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
                }
            }
            __result->__readEmptyParams();
            if(__response)
            {
                try
                {
                    __response();
                }
                catch(...)
                {
                    throw current_exception();
                }
            }
        }
    }
    catch(const exception_ptr& ex)
    {
        rethrow_exception(ex);
    }
    catch(const Ice::Exception&)
    {
        if(__exception)
        {
            __exception(current_exception());
        }
    }
}

function<void ()>
OnewayClosureCallback::invoke(
    const string& __name,
    const shared_ptr<Ice::ObjectPrx>& __proxy,
    Ice::OperationMode __mode,
    Ice::FormatType __format,
    function<void (BasicStream*)> __marshal,
    function<void ()> __response,
    function<void (exception_ptr)> __exception,
    function<void (bool)> __sent,
    const Ice::Context& __context)
{
    auto __result = make_shared<OutgoingAsync>(__proxy, __name, 
        make_shared<OnewayClosureCallback>(__name, __proxy, move(__response), move(__exception), move(__sent)));
    try
    {
        __result->prepare(__name, __mode, &__context);
        if(__marshal)
        {
            __marshal(__result->startWriteParams(__format));
            __result->endWriteParams();
        }
        else
        {
            __result->writeEmptyParams();
        }
        __result->invoke();
    }
    catch(const exception_ptr& ex)
    {
        rethrow_exception(ex);
    }
    catch(const Exception& __ex)
    {
        __result->abort(__ex);
    }
    
    return [__result]()
        {
            __result->cancel();
        };
}

TwowayClosureCallback::TwowayClosureCallback(
    const string& name,
    const shared_ptr<Ice::ObjectPrx>& proxy,
    bool readEmptyParams,
    function<void (BasicStream*)> read,
    function<void (const UserException&)> userException,
    function<void (exception_ptr)> exception,
    function<void (bool)> sent) :
    __name(name),
    __proxy(proxy),
    __readEmptyParams(readEmptyParams),
    __read(move(read)),
    __userException(move(userException)),
    __exception(move(exception)),
    __sent(move(sent))
{
}

void
TwowayClosureCallback::sent(const AsyncResultPtr& result) const
{
    if(__sent != nullptr)
    {
        __sent(result->sentSynchronously());
    }
}

bool
TwowayClosureCallback::hasSentCallback() const
{
    return __sent != nullptr;
}

void
TwowayClosureCallback::completed(const AsyncResultPtr& __result) const
{
    try
    {
        AsyncResult::__check(__result, __proxy.get(), __name);
        if(!__result->__wait())
        {
            try
            {
                __result->__throwUserException();
            }
            catch(const Ice::UserException& __ex)
            {
                if(__userException)
                {
                    __userException(__ex);
                }
                throw ::Ice::UnknownUserException(__FILE__, __LINE__, __ex.ice_name());
            }
        }
        else
        {
            if(__readEmptyParams)
            {
                __result->__readEmptyParams();
                __read(0);
            }
            else
            {
                __read(__result->__startReadParams());
            }
        }
    }
    catch(const exception_ptr& ex)
    {
        rethrow_exception(ex);
    }
    catch(const Ice::Exception&)
    {
        if(__exception)
        {
            __exception(current_exception());
        }
    }
}

function<void ()>
TwowayClosureCallback::invoke(
    const string& __name, 
    const shared_ptr<Ice::ObjectPrx>& __proxy,
    OperationMode __mode,
    FormatType __format,
    function<void (BasicStream*)> __write,
    bool __readEmptyParams,
    function<void (BasicStream*)> __read,
    function<void (const UserException&)> __userException,
    function<void (exception_ptr)> __exception,
    function<void (bool)> __sent,
    const Context& __context)
{
    assert(__proxy);
    auto __result = make_shared<OutgoingAsync>(__proxy, __name, 
        make_shared<TwowayClosureCallback>(__name, __proxy, __readEmptyParams, move(__read),
                                           move(__userException), move(__exception), move(__sent)));
    __proxy->__checkAsyncTwowayOnly(__name);
    try
    {
        __result->prepare(__name, __mode, &__context);
        if(__write)
        {
            __write(__result->startWriteParams(__format));
            __result->endWriteParams();
        }
        else
        {
            __result->writeEmptyParams();
        }
        __result->invoke();
    }
    catch(const Exception& __ex)
    {
        __result->abort(__ex);
    }
    
    return [__result]()
        {
            __result->cancel();
        };
}
#endif
