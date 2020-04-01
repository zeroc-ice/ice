//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
#include <Ice/ConnectionFactory.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/LoggerUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

const unsigned char OutgoingAsyncBase::OK = 0x1;
const unsigned char OutgoingAsyncBase::Sent = 0x2;

OutgoingAsyncCompletionCallback::~OutgoingAsyncCompletionCallback()
{
    // Out of line to avoid weak vtable
}

bool
OutgoingAsyncBase::sent()
{
    return sentImpl(true);
}

bool
OutgoingAsyncBase::exception(const Exception& ex)
{
    return exceptionImpl(ex);
}

bool
OutgoingAsyncBase::response()
{
    assert(false); // Must be overriden by request that can handle responses
    return false;
}

void
OutgoingAsyncBase::invokeSentAsync()
{
    class AsynchronousSent : public DispatchWorkItem
    {
    public:

        AsynchronousSent(const ConnectionPtr& connection, const OutgoingAsyncBasePtr& outAsync) :
            DispatchWorkItem(connection), _outAsync(outAsync)
        {
        }

        virtual void
        run()
        {
            _outAsync->invokeSent();
        }

    private:

        const OutgoingAsyncBasePtr _outAsync;
    };

    //
    // This is called when it's not safe to call the sent callback
    // synchronously from this thread. Instead the exception callback
    // is called asynchronously from the client thread pool.
    //
    try
    {
        _instance->clientThreadPool()->dispatch(new AsynchronousSent(_cachedConnection, ICE_SHARED_FROM_THIS));
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
    }
}

void
OutgoingAsyncBase::invokeExceptionAsync()
{
    class AsynchronousException : public DispatchWorkItem
    {
    public:

        AsynchronousException(const ConnectionPtr& c, const OutgoingAsyncBasePtr& outAsync) :
            DispatchWorkItem(c), _outAsync(outAsync)
        {
        }

        virtual void
        run()
        {
            _outAsync->invokeException();
        }

    private:

        const OutgoingAsyncBasePtr _outAsync;
    };

    //
    // CommunicatorDestroyedException is the only exception that can propagate directly from this method.
    //
    _instance->clientThreadPool()->dispatch(new AsynchronousException(_cachedConnection, ICE_SHARED_FROM_THIS));
}

void
OutgoingAsyncBase::invokeResponseAsync()
{
    class AsynchronousResponse : public DispatchWorkItem
    {
    public:

        AsynchronousResponse(const ConnectionPtr& connection, const OutgoingAsyncBasePtr& outAsync) :
            DispatchWorkItem(connection), _outAsync(outAsync)
        {
        }

        virtual void
        run()
        {
            _outAsync->invokeResponse();
        }

    private:

        const OutgoingAsyncBasePtr _outAsync;
    };

    //
    // CommunicatorDestroyedException is the only exception that can propagate directly from this method.
    //
    _instance->clientThreadPool()->dispatch(new AsynchronousResponse(_cachedConnection, ICE_SHARED_FROM_THIS));
}

void
OutgoingAsyncBase::invokeSent()
{
    try
    {
        handleInvokeSent(_sentSynchronously, this);
    }
    catch(const std::exception& ex)
    {
        warning(ex);
    }
    catch(...)
    {
        warning();
    }

    if(_observer && _doneInSent)
    {
        _observer.detach();
    }
}

void
OutgoingAsyncBase::invokeException()
{
    try
    {
        handleInvokeException(*_ex, this);
    }
    catch(const std::exception& ex)
    {
        warning(ex);
    }
    catch(...)
    {
        warning();
    }

    _observer.detach();
}

void
OutgoingAsyncBase::invokeResponse()
{
    if(_ex)
    {
        invokeException();
        return;
    }

    try
    {
        try
        {
            handleInvokeResponse(_state & OK, this);
        }
        catch(const Ice::Exception& ex)
        {
            if(handleException(ex))
            {
                handleInvokeException(ex, this);
            }
        }
        catch(const exception_ptr& ex)
        {
            rethrow_exception(ex);
        }
    }
    catch(const std::exception& ex)
    {
        warning(ex);
    }
    catch(...)
    {
        warning();
    }

    _observer.detach();
}

void
OutgoingAsyncBase::cancelable(const CancellationHandlerPtr& handler)
{
    Lock sync(_m);
    if(_cancellationException)
    {
        try
        {
            _cancellationException->ice_throw();
        }
        catch(const Ice::LocalException&)
        {
            _cancellationException.reset();
            throw;
        }
    }
    _cancellationHandler = handler;
}

void
OutgoingAsyncBase::cancel()
{
    cancel(Ice::InvocationCanceledException(__FILE__, __LINE__));
}

OutgoingAsyncBase::OutgoingAsyncBase(const InstancePtr& instance) :
    _instance(instance),
    _sentSynchronously(false),
    _doneInSent(false),
    _state(0),
    _os(instance.get(), Ice::currentProtocolEncoding),
    _is(instance.get(), Ice::currentProtocolEncoding)
{
}

bool
OutgoingAsyncBase::sentImpl(bool done)
{
    Lock sync(_m);
    bool alreadySent = (_state & Sent) > 0;
    _state |= Sent;
    if(done)
    {
        _doneInSent = true;
        _childObserver.detach();
        _cancellationHandler = 0;
    }

    bool invoke = handleSent(done, alreadySent);
    if(!invoke && _doneInSent)
    {
        _observer.detach();
    }
    return invoke;
}

bool
OutgoingAsyncBase::exceptionImpl(const Exception& ex)
{
    Lock sync(_m);
    ICE_SET_EXCEPTION_FROM_CLONE(_ex, ex.ice_clone());
    if(_childObserver)
    {
        _childObserver.failed(ex.ice_id());
        _childObserver.detach();
    }
    _cancellationHandler = 0;
    _observer.failed(ex.ice_id());

    bool invoke = handleException(ex);
    if(!invoke)
    {
        _observer.detach();
    }
    return invoke;
}

bool
OutgoingAsyncBase::responseImpl(bool ok, bool invoke)
{
    Lock sync(_m);
    if(ok)
    {
        _state |= OK;
    }

    _cancellationHandler = 0;

    try
    {
        invoke &= handleResponse(ok);
    }
    catch(const Ice::Exception& ex)
    {
        ICE_SET_EXCEPTION_FROM_CLONE(_ex, ex.ice_clone());
        invoke = handleException(ex);
    }
    if(!invoke)
    {
        _observer.detach();
    }
    return invoke;
}

void
OutgoingAsyncBase::cancel(const Ice::LocalException& ex)
{
    CancellationHandlerPtr handler;
    {
        Lock sync(_m);
        if(!_cancellationHandler)
        {
            ICE_SET_EXCEPTION_FROM_CLONE(_cancellationException, ex.ice_clone());
            return;
        }
        handler = _cancellationHandler;
    }
    handler->asyncRequestCanceled(ICE_SHARED_FROM_THIS, ex);
}

void
OutgoingAsyncBase::warning(const std::exception& exc) const
{
    if(_instance->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
    {
        Ice::Warning out(_instance->initializationData().logger);
        const Ice::Exception* ex = dynamic_cast<const Ice::Exception*>(&exc);
        if(ex)
        {
            out << "Ice::Exception raised by AMI callback:\n" << *ex;
        }
        else
        {
            out << "std::exception raised by AMI callback:\n" << exc.what();
        }
    }
}

void
OutgoingAsyncBase::warning() const
{
    if(_instance->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
    {
        Ice::Warning out(_instance->initializationData().logger);
        out << "unknown exception raised by AMI callback";
    }
}

bool
ProxyOutgoingAsyncBase::exception(const Exception& exc)
{
    if(_childObserver)
    {
        _childObserver.failed(exc.ice_id());
        _childObserver.detach();
    }

    _cachedConnection = 0;
    if(_proxy->_getReference()->getInvocationTimeout() == -2)
    {
        _instance->timer()->cancel(ICE_SHARED_FROM_THIS);
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
        _instance->retryQueue()->add(ICE_SHARED_FROM_THIS, _proxy->_handleException(exc, _handler, _mode, _sent, _cnt));
        return false;
    }
    catch(const Exception& ex)
    {
        return exceptionImpl(ex); // No retries, we're done
    }
}

void
ProxyOutgoingAsyncBase::cancelable(const CancellationHandlerPtr& handler)
{
    if(_proxy->_getReference()->getInvocationTimeout() == -2 && _cachedConnection)
    {
        const int timeout = _cachedConnection->timeout();
        if(timeout > 0)
        {
            _instance->timer()->schedule(ICE_SHARED_FROM_THIS, IceUtil::Time::milliSeconds(timeout));
        }
    }
    OutgoingAsyncBase::cancelable(handler);
}

void
ProxyOutgoingAsyncBase::retryException(const Exception&)
{
    try
    {
        //
        // It's important to let the retry queue do the retry. This is
        // called from the connect request handler and the retry might
        // require could end up waiting for the flush of the
        // connection to be done.
        //
        _proxy->_updateRequestHandler(_handler, 0); // Clear request handler and always retry.
        _instance->retryQueue()->add(ICE_SHARED_FROM_THIS, 0);
    }
    catch(const Ice::Exception& exc)
    {
        if(exception(exc))
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
ProxyOutgoingAsyncBase::abort(const Ice::Exception& ex)
{
    assert(!_childObserver);

    if(exceptionImpl(ex))
    {
        invokeExceptionAsync();
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

ProxyOutgoingAsyncBase::ProxyOutgoingAsyncBase(const ObjectPrxPtr& prx) :
    OutgoingAsyncBase(prx->_getReference()->getInstance()),
    _proxy(prx),
    _mode(ICE_ENUM(OperationMode, Normal)),
    _cnt(0),
    _sent(false)
{
}

ProxyOutgoingAsyncBase::~ProxyOutgoingAsyncBase()
{
}

void
ProxyOutgoingAsyncBase::invokeImpl(bool userThread)
{
    try
    {
        if(userThread)
        {
            int invocationTimeout = _proxy->_getReference()->getInvocationTimeout();
            if(invocationTimeout > 0)
            {
                _instance->timer()->schedule(ICE_SHARED_FROM_THIS, IceUtil::Time::milliSeconds(invocationTimeout));
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
                _handler = _proxy->_getRequestHandler();
                AsyncStatus status = _handler->sendAsyncRequest(ICE_SHARED_FROM_THIS);
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
                _proxy->_updateRequestHandler(_handler, 0); // Clear request handler and always retry.
            }
            catch(const Exception& ex)
            {
                if(_childObserver)
                {
                    _childObserver.failed(ex.ice_id());
                    _childObserver.detach();
                }
                int interval = _proxy->_handleException(ex, _handler, _mode, _sent, _cnt);
                if(interval > 0)
                {
                    _instance->retryQueue()->add(ICE_SHARED_FROM_THIS, interval);
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
        // will be catch by the caller and abort(ex) will be called.
        //
        if(userThread)
        {
            throw;
        }
        else if(exceptionImpl(ex)) // No retries, we're done
        {
            invokeExceptionAsync();
        }
    }
}

bool
ProxyOutgoingAsyncBase::sentImpl(bool done)
{
    _sent = true;
    if(done)
    {
        if(_proxy->_getReference()->getInvocationTimeout() != -1)
        {
            _instance->timer()->cancel(ICE_SHARED_FROM_THIS);
        }
    }
    return OutgoingAsyncBase::sentImpl(done);
}

bool
ProxyOutgoingAsyncBase::exceptionImpl(const Exception& ex)
{
    if(_proxy->_getReference()->getInvocationTimeout() != -1)
    {
        _instance->timer()->cancel(ICE_SHARED_FROM_THIS);
    }
    return OutgoingAsyncBase::exceptionImpl(ex);
}

bool
ProxyOutgoingAsyncBase::responseImpl(bool ok, bool invoke)
{
    if(_proxy->_getReference()->getInvocationTimeout() != -1)
    {
        _instance->timer()->cancel(ICE_SHARED_FROM_THIS);
    }
    return OutgoingAsyncBase::responseImpl(ok, invoke);
}

void
ProxyOutgoingAsyncBase::runTimerTask()
{
    if(_proxy->_getReference()->getInvocationTimeout() == -2)
    {
        cancel(ConnectionTimeoutException(__FILE__, __LINE__));
    }
    else
    {
        cancel(InvocationTimeoutException(__FILE__, __LINE__));
    }
}

OutgoingAsync::OutgoingAsync(const ObjectPrxPtr& prx, bool synchronous) :
    ProxyOutgoingAsyncBase(prx),
    _encoding(getCompatibleEncoding(prx->_getReference()->getEncoding())),
    _synchronous(synchronous)
{
}

void
OutgoingAsync::prepare(const string& operation, OperationMode mode, const Context& context)
{
    checkSupportedProtocol(getCompatibleProtocol(_proxy->_getReference()->getProtocol()));

    _mode = mode;
    _observer.attach(_proxy, operation, context);

    switch(_proxy->_getReference()->getMode())
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
            _proxy->_getBatchRequestQueue()->prepareBatchRequest(&_os);
            break;
        }
    }

    Reference* ref = _proxy->_getReference().get();

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

    if(&context != &Ice::noExplicitContext)
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
    return ProxyOutgoingAsyncBase::sentImpl(!_proxy->ice_isTwoway()); // done = true if it's not a two-way proxy
}

bool
OutgoingAsync::response()
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

                IceInternal::UniquePtr<RequestFailedException> ex;
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
                break;
            }

            case replyUnknownException:
            case replyUnknownLocalException:
            case replyUnknownUserException:
            {
                string unknown;
                _is.read(unknown, false);

                IceInternal::UniquePtr<UnknownException> ex;
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
                break;
            }

            default:
            {
                throw UnknownReplyStatusException(__FILE__, __LINE__);
            }
        }

        return responseImpl(replyStatus == replyOK, true);
    }
    catch(const Exception& ex)
    {
        return exception(ex);
    }
}

AsyncStatus
OutgoingAsync::invokeRemote(const ConnectionIPtr& connection, bool compress, bool response)
{
    _cachedConnection = connection;
    return connection->sendAsyncRequest(ICE_SHARED_FROM_THIS, compress, response, 0);
}

AsyncStatus
OutgoingAsync::invokeCollocated(CollocatedRequestHandler* handler)
{
    return handler->invokeAsyncRequest(this, 0, _synchronous);
}

void
OutgoingAsync::abort(const Exception& ex)
{
    const Reference::Mode mode = _proxy->_getReference()->getMode();
    if(mode == Reference::ModeBatchOneway || mode == Reference::ModeBatchDatagram)
    {
        //
        // If we didn't finish a batch oneway or datagram request, we
        // must notify the connection about that we give up ownership
        // of the batch stream.
        //
        _proxy->_getBatchRequestQueue()->abortBatchRequest(&_os);
    }

    ProxyOutgoingAsyncBase::abort(ex);
}

void
OutgoingAsync::invoke(const string& operation)
{
    const Reference::Mode mode = _proxy->_getReference()->getMode();
    if(mode == Reference::ModeBatchOneway || mode == Reference::ModeBatchDatagram)
    {
        _sentSynchronously = true;
        _proxy->_getBatchRequestQueue()->finishBatchRequest(&_os, _proxy, operation);
        responseImpl(true, false); // Don't call sent/completed callback for batch AMI requests
        return;
    }

    //
    // NOTE: invokeImpl doesn't throw so this can be called from the
    // try block with the catch block calling abort(ex) in case of an
    // exception.
    //
    invokeImpl(true); // userThread = true
}

void
OutgoingAsync::invoke(const string& operation,
                      Ice::OperationMode mode,
                      Ice::FormatType format,
                      const Ice::Context& context,
                      function<void(Ice::OutputStream*)> write)
{
    try
    {
        prepare(operation, mode, context);
        if(write)
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
    catch(const Ice::Exception& ex)
    {
        abort(ex);
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
    catch(const UserException& ex)
    {
        _is.endEncapsulation();
        if(_userException)
        {
            _userException(ex);
        }
        throw UnknownUserException(__FILE__, __LINE__, ex.ice_id());
    }
}

bool
LambdaInvoke::handleSent(bool, bool alreadySent)
{
    return _sent != nullptr && !alreadySent; // Invoke the sent callback only if not already invoked.
}

bool
LambdaInvoke::handleException(const Ice::Exception&)
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
LambdaInvoke::handleInvokeException(const Ice::Exception& ex, OutgoingAsyncBase*) const
{
    try
    {
        ex.ice_throw();
    }
    catch(const Ice::Exception&)
    {
        _exception(current_exception());
    }
}

void
LambdaInvoke::handleInvokeResponse(bool ok, OutgoingAsyncBase*) const
{
    _response(ok);
}
