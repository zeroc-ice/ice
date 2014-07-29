// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/OutgoingAsync.h>
#include <Ice/Object.h>
#include <Ice/ConnectionI.h>
#include <Ice/CollocatedRequestHandler.h>
#include <Ice/Reference.h>
#include <Ice/Instance.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocatorInfo.h>
#include <Ice/ProxyFactory.h>
#include <Ice/RouterInfo.h>
#include <Ice/Protocol.h>
#include <Ice/ReplyStatus.h>
#include <Ice/ImplicitContextI.h>
#include <Ice/ThreadPool.h>
#include <Ice/RetryQueue.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* Ice::upCast(AsyncResult* p) { return p; }

IceUtil::Shared* IceInternal::upCast(OutgoingAsyncMessageCallback* p) { return p; }
IceUtil::Shared* IceInternal::upCast(OutgoingAsync* p) { return p; }
IceUtil::Shared* IceInternal::upCast(BatchOutgoingAsync* p) { return p; }
IceUtil::Shared* IceInternal::upCast(ProxyBatchOutgoingAsync* p) { return p; }
IceUtil::Shared* IceInternal::upCast(ConnectionBatchOutgoingAsync* p) { return p; }
IceUtil::Shared* IceInternal::upCast(CommunicatorBatchOutgoingAsync* p) { return p; }

const unsigned char Ice::AsyncResult::OK = 0x1;
const unsigned char Ice::AsyncResult::Done = 0x2;
const unsigned char Ice::AsyncResult::Sent = 0x4;
const unsigned char Ice::AsyncResult::EndCalled = 0x8;

namespace
{

class AsynchronousException : public DispatchWorkItem
{
public:

    AsynchronousException(const Ice::ConnectionPtr& connection, const Ice::AsyncResultPtr& result,
                          const Ice::Exception& ex) :
        DispatchWorkItem(connection), _result(result), _exception(ex.ice_clone())
    {
    }

    virtual void
    run()
    {
        _result->__invokeException(*_exception.get());
    }

private:

    const Ice::AsyncResultPtr _result;
    const IceUtil::UniquePtr<Ice::Exception> _exception;
};

class AsynchronousSent : public DispatchWorkItem
{
public:

    AsynchronousSent(const Ice::ConnectionPtr& connection, const Ice::AsyncResultPtr& result) :
        DispatchWorkItem(connection), _result(result)
    {
    }

    virtual void
    run()
    {
        _result->__invokeSent();
    }

private:

    const Ice::AsyncResultPtr _result;
};

};

Ice::AsyncResult::AsyncResult(const CommunicatorPtr& communicator,
                              const IceInternal::InstancePtr& instance,
                              const string& op,
                              const CallbackBasePtr& del,
                              const LocalObjectPtr& cookie) :
    _communicator(communicator),
    _instance(instance),
    _operation(op),
    _callback(del),
    _cookie(cookie),
    _is(instance.get(), Ice::currentProtocolEncoding),
    _os(instance.get(), Ice::currentProtocolEncoding),
    _state(0),
    _sentSynchronously(false),
    _exception(0)
{
    if(!_callback)
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__);
    }
    const_cast<CallbackBasePtr&>(_callback) = _callback->verify(_cookie);
}

Ice::AsyncResult::~AsyncResult()
{
}

Int
Ice::AsyncResult::getHash() const
{
    return static_cast<Int>(reinterpret_cast<Long>(this) >> 4);
}

bool
Ice::AsyncResult::isCompleted() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    return _state & Done;
}

void
Ice::AsyncResult::waitForCompleted()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    while(!(_state & Done))
    {
        _monitor.wait();
    }
}

bool
Ice::AsyncResult::isSent() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    return _state & Sent;
}

void
Ice::AsyncResult::waitForSent()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    while(!(_state & Sent) && !_exception.get())
    {
        _monitor.wait();
    }
}

void
Ice::AsyncResult::throwLocalException() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    if(_exception.get())
    {
        _exception.get()->ice_throw();
    }
}

bool
Ice::AsyncResult::__wait()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    if(_state & EndCalled)
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "end_ method called more than once");
    }
    _state |= EndCalled;
    while(!(_state & Done))
    {
        _monitor.wait();
    }
    if(_exception.get())
    {
        _exception.get()->ice_throw();
    }
    return _state & OK;
}

void
Ice::AsyncResult::__throwUserException()
{
    try
    {
        _is.startReadEncaps();
        _is.throwException();
    }
    catch(const Ice::UserException&)
    {
        _is.endReadEncaps();
        throw;
    }
}

void
Ice::AsyncResult::__invokeSent()
{
    //
    // Note: no need to change the _state here, specializations are responsible for
    // changing the state.
    //

    if(_callback)
    {
        try
        {
            AsyncResultPtr self(this);
            _callback->sent(self);
        }
        catch(const std::exception& ex)
        {
            __warning(ex);
        }
        catch(...)
        {
            __warning();
        }
    }

    if(_observer)
    {
        Ice::ObjectPrx proxy = getProxy();
        if(!proxy || !proxy->ice_isTwoway())
        {
            _observer.detach();
        }
    }
}

void
Ice::AsyncResult::__invokeSentAsync()
{
    //
    // This is called when it's not safe to call the sent callback synchronously
    // from this thread. Instead the exception callback is called asynchronously from
    // the client thread pool.
    //
    try
    {
        _instance->clientThreadPool()->dispatch(new AsynchronousSent(_cachedConnection, this));
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
    }
}

void
Ice::AsyncResult::__invokeException(const Ice::Exception& ex)
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        _state |= Done;
        _os.resize(0); // Clear buffer now, instead of waiting for AsyncResult deallocation
        _exception.reset(ex.ice_clone());
        _monitor.notifyAll();
    }

    __invokeCompleted();
}

void
Ice::AsyncResult::__invokeExceptionAsync(const Ice::Exception& ex)
{
    //
    // This is called when it's not safe to call the exception callback synchronously
    // from this thread. Instead the exception callback is called asynchronously from
    // the client thread pool.
    //
    // CommunicatorDestroyedException is the only exception that can propagate directly
    // from this method.
    //
    _instance->clientThreadPool()->dispatch(new AsynchronousException(_cachedConnection, this, ex));
}

void
Ice::AsyncResult::__invokeCompleted()
{
    //
    // Note: no need to change the _state here, specializations are responsible for
    // changing the state.
    //

    if(_callback)
    {
        try
        {
            AsyncResultPtr self(this);
            _callback->completed(self);
        }
        catch(const std::exception& ex)
        {
            __warning(ex);
        }
        catch(...)
        {
            __warning();
        }
    }

    _observer.detach();
}

void
Ice::AsyncResult::runTimerTask() // Implementation of TimerTask::runTimerTask()
{
    RequestHandlerPtr handler;
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        swap(handler, _timeoutRequestHandler);
    }

    if(handler)
    {
        handler->asyncRequestTimedOut(OutgoingAsyncMessageCallbackPtr::dynamicCast(this));
    }
}


void
Ice::AsyncResult::__check(const AsyncResultPtr& r, const IceProxy::Ice::Object* prx, const string& operation)
{
    __check(r, operation);
    if(r->getProxy().get() != prx)
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "Proxy for call to end_" + operation +
                                                " does not match proxy that was used to call corresponding begin_" +
                                                operation + " method");
    }
}

void
Ice::AsyncResult::__check(const AsyncResultPtr& r, const Ice::Communicator* com, const string& operation)
{
    __check(r, operation);
    if(r->getCommunicator().get() != com)
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "Communicator for call to end_" + operation +
                                                " does not match communicator that was used to call corresponding " +
                                                "begin_" + operation + " method");
    }
}

void
Ice::AsyncResult::__check(const AsyncResultPtr& r, const Ice::Connection* con, const string& operation)
{
    __check(r, operation);
    if(r->getConnection().get() != con)
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "Connection for call to end_" + operation +
                                                " does not match connection that was used to call corresponding " +
                                                "begin_" + operation + " method");
    }
}

void
Ice::AsyncResult::__check(const AsyncResultPtr& r, const string& operation)
{
    if(!r)
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "AsyncResult == null");
    }
    else if(&r->_operation != &operation)
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "Incorrect operation for end_" + operation +
                                                " method: " + r->_operation);
    }
}


void
Ice::AsyncResult::__warning(const std::exception& exc) const
{
    if(_instance->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
    {
        Warning out(_instance->initializationData().logger);
        const Exception* ex = dynamic_cast<const Exception*>(&exc);
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
Ice::AsyncResult::__warning() const
{
    if(_instance->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
    {
        Warning out(_instance->initializationData().logger);
        out << "unknown exception raised by AMI callback";
    }
}

void
IceInternal::OutgoingAsyncMessageCallback::__dispatchInvocationTimeout(const ThreadPoolPtr& threadPool,
                                                                       const Ice::ConnectionPtr& connection)
{
    class InvocationTimeoutCall : public DispatchWorkItem
    {
    public:

        InvocationTimeoutCall(const OutgoingAsyncMessageCallbackPtr& outAsync, const Ice::ConnectionPtr& connection) : 
            DispatchWorkItem(connection), _outAsync(outAsync)
        {
        }

        virtual void
        run()
        {
            InvocationTimeoutException ex(__FILE__, __LINE__);
            _outAsync->__finished(ex);
        }

    private:

        const OutgoingAsyncMessageCallbackPtr _outAsync;
    };
    threadPool->dispatch(new InvocationTimeoutCall(this, connection));
}

IceInternal::OutgoingAsync::OutgoingAsync(const ObjectPrx& prx,
                                          const std::string& operation,
                                          const CallbackBasePtr& delegate,
                                          const Ice::LocalObjectPtr& cookie) :
    AsyncResult(prx->ice_getCommunicator(), prx->__reference()->getInstance(), operation, delegate, cookie),
    _proxy(prx),
    _encoding(getCompatibleEncoding(prx->__reference()->getEncoding()))
{
}

void
IceInternal::OutgoingAsync::__prepare(const std::string& operation, OperationMode mode, const Context* context)
{
    _handler = 0;
    _cnt = 0;
    _sent = false;
    _mode = mode;
    _sentSynchronously = false;

    checkSupportedProtocol(getCompatibleProtocol(_proxy->__reference()->getProtocol()));

    _observer.attach(_proxy.get(), operation, context);

    //
    // Can't call async via a batch proxy.
    //
    if(_proxy->ice_isBatchOneway() || _proxy->ice_isBatchDatagram())
    {
        throw Ice::FeatureNotSupportedException(__FILE__, __LINE__, "can't send batch requests with AMI");
    }

    _os.writeBlob(requestHdr, sizeof(requestHdr));

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

    if(context != 0)
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

AsyncStatus
IceInternal::OutgoingAsync::__send(const Ice::ConnectionIPtr& connection, bool compress, bool response)
{
    _cachedConnection = connection;
    return connection->sendAsyncRequest(this, compress, response);
}

AsyncStatus
IceInternal::OutgoingAsync::__invokeCollocated(CollocatedRequestHandler* handler)
{
    return handler->invokeAsyncRequest(this);
}

bool
IceInternal::OutgoingAsync::__sent()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);

    bool alreadySent = _state & Sent; // Expected in case of a retry.
    _state |= Sent;
    _sent = true;

    assert(!(_state & Done));
    if(_proxy->__reference()->getMode() != Reference::ModeTwoway)
    {
        _childObserver.detach();
        if(!_callback || !_callback->hasSentCallback())
        {
            _observer.detach();
        }
        if(_timeoutRequestHandler)
        {
            _instance->timer()->cancel(this);
            _timeoutRequestHandler = 0;
        }
        _state |= Done | OK;
        //_os.resize(0); // Don't clear the buffer now, it's needed for collocation optimization.
    }
    _monitor.notifyAll();
    return !alreadySent && _callback && _callback->hasSentCallback();
}

void
IceInternal::OutgoingAsync::__invokeSent()
{
    ::Ice::AsyncResult::__invokeSent();
}

void
IceInternal::OutgoingAsync::__finished(const Ice::Exception& exc)
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        assert(!(_state & Done));
        _childObserver.failed(exc.ice_name());
        _childObserver.detach();
        if(_timeoutRequestHandler)
        {
            _instance->timer()->cancel(this);
            _timeoutRequestHandler = 0;
        }
    }

    //
    // NOTE: at this point, synchronization isn't needed, no other threads should be
    // calling on the callback.
    //
    try
    {
        if(!handleException(exc)) // This will throw if the invocation can't be retried.
        {
            return; // Can't be retried immediately.
        }

        __invoke(false); // Retry the invocation
    }
    catch(const Ice::Exception& ex)
    {
        __invokeException(ex);
    }
}

void
IceInternal::OutgoingAsync::__finished()
{
    assert(_proxy->ice_isTwoway()); // Can only be called for twoways.

    Ice::Byte replyStatus;
    try
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        assert(!_exception.get() && !(_state & Done));
        assert(!_is.b.empty());

        if(_childObserver)
        {
            _childObserver->reply(static_cast<Int>(_is.b.size() - headerSize - 4));
        }
        _childObserver.detach();

        if(_timeoutRequestHandler)
        {
            _instance->timer()->cancel(this);
            _timeoutRequestHandler = 0;
        }

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

        _state |= Done;
        _os.resize(0); // Clear buffer now, instead of waiting for AsyncResult deallocation
        if(replyStatus == replyOK)
        {
            _state |= OK;
        }
        _monitor.notifyAll();
    }
    catch(const LocalException& ex)
    {
        __finished(ex);
        return;
    }

    assert(replyStatus == replyOK || replyStatus == replyUserException);
    __invokeCompleted();
}

bool
IceInternal::OutgoingAsync::__invoke(bool synchronous)
{
    while(true)
    {
        try
        {
            _sent = false;
            _handler = _proxy->__getRequestHandler(true);
            AsyncStatus status = _handler->sendAsyncRequest(this);
            if(status & AsyncStatusSent)
            {
                if(synchronous)
                {
                    _sentSynchronously = true;
                    if(status & AsyncStatusInvokeSentCallback)
                    {
                        __invokeSent(); // Call the sent callback from the user thread.
                    }
                }
                else
                {
                    if(status & AsyncStatusInvokeSentCallback)
                    {
                        __invokeSentAsync(); // Call the sent callback from a client thread pool thread.
                    }
                }
            }

            if(_proxy->ice_isTwoway() || !(status & AsyncStatusSent))
            {
                IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
                if(!(_state & Done))
                {
                    int invocationTimeout = _handler->getReference()->getInvocationTimeout();
                    if(invocationTimeout > 0)
                    {
                        _instance->timer()->schedule(this, IceUtil::Time::milliSeconds(invocationTimeout));
                        _timeoutRequestHandler = _handler;
                    }
                }
            }
            break;
        }
        catch(const RetryException&)
        {
            _proxy->__setRequestHandler(_handler, 0); // Clear request handler and retry.
        }
        catch(const Ice::Exception& ex)
        {
            if(!handleException(ex)) // This will throw if the invocation can't be retried.
            {
                break; // Can't be retried immediately.
            }
        }
    }
    return _sentSynchronously;
}

bool
IceInternal::OutgoingAsync::handleException(const Ice::Exception& exc)
{
    try
    {
        int interval = _proxy->__handleException(exc, _handler, _mode, _sent, _cnt);
        _observer.retried(); // Invocation is being retried.
        if(interval > 0)
        {
            _instance->retryQueue()->add(this, interval);
            return false; // Don't retry immediately, the retry queue will take care of the retry.
        }
        else
        {
            return true; // Retry immediately.
        }
    }
    catch(const Ice::Exception& ex)
    {
        _observer.failed(ex.ice_name());
        throw;
    }
}

IceInternal::BatchOutgoingAsync::BatchOutgoingAsync(const CommunicatorPtr& communicator,
                                                    const InstancePtr& instance,
                                                    const std::string& operation,
                                                    const CallbackBasePtr& delegate,
                                                    const Ice::LocalObjectPtr& cookie) :
    AsyncResult(communicator, instance, operation, delegate, cookie)
{
}

AsyncStatus 
IceInternal::BatchOutgoingAsync::__send(const Ice::ConnectionIPtr& connection, bool, bool)
{
    _cachedConnection = connection;
    return connection->flushAsyncBatchRequests(this);
}

AsyncStatus
IceInternal::BatchOutgoingAsync::__invokeCollocated(CollocatedRequestHandler* handler)
{
    return handler->invokeAsyncBatchRequests(this);
}

bool
IceInternal::BatchOutgoingAsync::__sent()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    assert(!_exception.get());
    _state |= Done | OK | Sent;
    //_os.resize(0); // Don't clear the buffer now, it's needed for collocation optimization.
    _childObserver.detach();
    if(_timeoutRequestHandler)
    {
        _instance->timer()->cancel(this);
        _timeoutRequestHandler = 0;
    }
    _monitor.notifyAll();
    if(!_callback || !_callback->hasSentCallback())
    {
        _observer.detach();
        return false;
    }
    return true;
}

void
IceInternal::BatchOutgoingAsync::__invokeSent()
{
    ::Ice::AsyncResult::__invokeSent();
}

void
IceInternal::BatchOutgoingAsync::__finished(const Ice::Exception& exc)
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        _childObserver.failed(exc.ice_name());
        _childObserver.detach();
        if(_timeoutRequestHandler)
        {
            _instance->timer()->cancel(this);
            _timeoutRequestHandler = 0;
        }
    }
    __invokeException(exc);
}

IceInternal::ProxyBatchOutgoingAsync::ProxyBatchOutgoingAsync(const Ice::ObjectPrx& proxy,
                                                              const std::string& operation,
                                                              const CallbackBasePtr& delegate,
                                                              const Ice::LocalObjectPtr& cookie) :
    BatchOutgoingAsync(proxy->ice_getCommunicator(), proxy->__reference()->getInstance(), operation, delegate, cookie),
    _proxy(proxy)
{
    _observer.attach(proxy.get(), operation, 0);
}

void
IceInternal::ProxyBatchOutgoingAsync::__invoke()
{
    checkSupportedProtocol(_proxy->__reference()->getProtocol());

    RequestHandlerPtr handler;
    try
    {
        handler = _proxy->__getRequestHandler(true);
        AsyncStatus status = handler->sendAsyncRequest(this);
        if(status & AsyncStatusSent)
        {
            _sentSynchronously = true;
            if(status & AsyncStatusInvokeSentCallback)
            {
                __invokeSent();
            }
        }
        else
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
            if(!(_state & Done))
            {
                int invocationTimeout = handler->getReference()->getInvocationTimeout();
                if(invocationTimeout > 0)
                {
                    _instance->timer()->schedule(this, IceUtil::Time::milliSeconds(invocationTimeout));
                    _timeoutRequestHandler = handler;
                }
            }
        }
    }
    catch(const RetryException&)
    {
        //
        // Clear request handler but don't retry or throw. Retrying
        // isn't useful, there were no batch requests associated with
        // the proxy's request handler.
        //
        _proxy->__setRequestHandler(handler, 0);
    }
    catch(const Ice::Exception& ex)
    {
        _observer.failed(ex.ice_name());
        _proxy->__setRequestHandler(handler, 0); // Clear request handler
        throw; // Throw to notify the user that batch requests were potentially lost.
    }
}

IceInternal::ConnectionBatchOutgoingAsync::ConnectionBatchOutgoingAsync(const ConnectionIPtr& con,
                                                                        const CommunicatorPtr& communicator,
                                                                        const InstancePtr& instance,
                                                                        const string& operation,
                                                                        const CallbackBasePtr& delegate,
                                                                        const Ice::LocalObjectPtr& cookie) :
    BatchOutgoingAsync(communicator, instance, operation, delegate, cookie),
    _connection(con)
{
    _observer.attach(instance.get(), operation);
}

void
IceInternal::ConnectionBatchOutgoingAsync::__invoke()
{
    AsyncStatus status = _connection->flushAsyncBatchRequests(this);
    if(status & AsyncStatusSent)
    {
        _sentSynchronously = true;
        if(status & AsyncStatusInvokeSentCallback)
        {
            __invokeSent();
        }
    }
}

Ice::ConnectionPtr
IceInternal::ConnectionBatchOutgoingAsync::getConnection() const
{
    return _connection;
}

IceInternal::CommunicatorBatchOutgoingAsync::CommunicatorBatchOutgoingAsync(const CommunicatorPtr& communicator,
                                                                            const InstancePtr& instance,
                                                                            const string& operation,
                                                                            const CallbackBasePtr& delegate,
                                                                            const Ice::LocalObjectPtr& cookie) :
    AsyncResult(communicator, instance, operation, delegate, cookie)
{
    //
    // _useCount is initialized to 1 to prevent premature callbacks.
    // The caller must invoke ready() after all flush requests have
    // been initiated.
    //
    _useCount = 1;

    //
    // Assume all connections are flushed synchronously.
    //
    _sentSynchronously = true;
    
    //
    // Attach observer
    //
    _observer.attach(instance.get(), operation);
}

void
IceInternal::CommunicatorBatchOutgoingAsync::flushConnection(const ConnectionIPtr& con)
{
    class BatchOutgoingAsyncI : public BatchOutgoingAsync
    {
    public:

        BatchOutgoingAsyncI(const CommunicatorBatchOutgoingAsyncPtr& outAsync,
                            const InstancePtr& instance, 
                            InvocationObserver& observer) :
            BatchOutgoingAsync(outAsync->getCommunicator(), instance, outAsync->getOperation(), __dummyCallback, 0),
            _outAsync(outAsync), _observer(observer)
        {
        }

        virtual bool __sent()
        {
            _childObserver.detach();
            _outAsync->check(false);
            return false;
        }

#ifdef __SUNPRO_CC
        using BatchOutgoingAsync::__sent;
#endif

        virtual void __finished(const Ice::Exception& ex)
        {
            _childObserver.failed(ex.ice_name());
            _childObserver.detach();
            _outAsync->check(false);
        }

        virtual void __attachRemoteObserver(const Ice::ConnectionInfoPtr& connection, const Ice::EndpointPtr& endpt,
                                            Ice::Int requestId, Ice::Int sz)
        {
            _childObserver.attach(_observer.getRemoteObserver(connection, endpt, requestId, sz));
        }

    private:
        
        const CommunicatorBatchOutgoingAsyncPtr _outAsync;
        InvocationObserver& _observer;
    };

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        ++_useCount;
    }

    try
    {
        AsyncStatus status = con->flushAsyncBatchRequests(new BatchOutgoingAsyncI(this, _instance, _observer));
        if(!(status & AsyncStatusSent))
        {
            _sentSynchronously = false;
        }
    }
    catch(const Ice::LocalException&)
    {
        check(false);
        throw;
    }
}

void
IceInternal::CommunicatorBatchOutgoingAsync::ready()
{
    check(true);
}

void
IceInternal::CommunicatorBatchOutgoingAsync::check(bool userThread)
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        assert(_useCount > 0);
        if(--_useCount > 0)
        {
            return;
        }         
        _state |= Done | OK | Sent;
        _os.resize(0); // Clear buffer now, instead of waiting for AsyncResult deallocation
        _monitor.notifyAll();
    }

    if(!_callback || !_callback->hasSentCallback())
    {
        _observer.detach();
    }
    else
    {
        //
        // _sentSynchronously is immutable here.
        //
        if(!_sentSynchronously || !userThread)
        {
            __invokeSentAsync();
        }
        else
        {
            AsyncResult::__invokeSent();
        }
    }
}


namespace
{

//
// Dummy class derived from CallbackBase
// We use this class for the __dummyCallback extern pointer in OutgoingAsync. In turn,
// this allows us to test whether the user supplied a null delegate instance to the
// generated begin_ method without having to generate a separate test to throw IllegalArgumentException
// in the inlined versions of the begin_ method. In other words, this reduces the amount of generated
// object code.
//
class DummyCallback : public CallbackBase
{
public:

    DummyCallback()
    {
    }

    virtual void 
    completed(const Ice::AsyncResultPtr&) const
    {
         assert(false);
    }

    virtual CallbackBasePtr 
    verify(const Ice::LocalObjectPtr&)
    {
        //
        // Called by the AsyncResult constructor to verify the delegate. The dummy
        // delegate is passed when the user used a begin_ method without delegate.
        // By returning 0 here, we tell the AsyncResult that no delegates was
        // provided.
        //
        return 0;
    }

    virtual void 
    sent(const AsyncResultPtr&) const
    {
         assert(false);
    }

    virtual bool 
    hasSentCallback() const
    {
        assert(false);
        return false;
    }
};

}

//
// This gives a pointer value to compare against in the generated
// begin_ method to decide whether the caller passed a null pointer
// versus the generated inline version of the begin_ method having
// passed a pointer to the dummy delegate.
//
CallbackBasePtr IceInternal::__dummyCallback = new DummyCallback;

#ifdef ICE_CPP11

Ice::CallbackPtr
Ice::newCallback(const ::IceInternal::Function<void (const AsyncResultPtr&)>& completed,
                 const ::IceInternal::Function<void (const AsyncResultPtr&)>& sent)
{
    class Cpp11CB : public GenericCallbackBase
    {
    public:

        Cpp11CB(const ::std::function<void (const AsyncResultPtr&)>& completed,
                const ::std::function<void (const AsyncResultPtr&)>& sent) :
            _completed(completed), 
            _sent(sent)
        {
            checkCallback(true, completed != nullptr);
        }
        
        virtual void 
        completed(const AsyncResultPtr& result) const
        {
            _completed(result);
        }
        
        virtual CallbackBasePtr 
        verify(const LocalObjectPtr&)
        {
            return this; // Nothing to do, the cookie is not type-safe.
        }
        
        virtual void 
        sent(const AsyncResultPtr& result) const
        {
            if(_sent != nullptr)
            {
                _sent(result);
             }
        }
    
        virtual bool 
        hasSentCallback() const
        {
            return _sent != nullptr;
        }
        
    private:

        ::std::function< void (const AsyncResultPtr&)> _completed;
        ::std::function< void (const AsyncResultPtr&)> _sent;
    };
    
    return new Cpp11CB(completed, sent);
}
#endif

void
IceInternal::CallbackBase::checkCallback(bool obj, bool cb)
{
    if(!obj)
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "callback object cannot be null");
    }
    if(!cb)
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "callback cannot be null");
    }
}

void
Ice::AMICallbackBase::__exception(const ::Ice::Exception& ex)
{
    ice_exception(ex);
}

void
Ice::AMICallbackBase::__sent(bool sentSynchronously)
{
    if(!sentSynchronously)
    {
        dynamic_cast<AMISentCallback*>(this)->ice_sent();
    }
}


