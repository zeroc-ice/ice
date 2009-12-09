// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/OutgoingAsync.h>
#include <Ice/Object.h>
#include <Ice/ConnectionI.h>
#include <Ice/RequestHandler.h>
#include <Ice/Reference.h>
#include <Ice/Instance.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocatorInfo.h>
#include <Ice/ProxyFactory.h>
#include <Ice/RouterInfo.h>
#include <Ice/Outgoing.h> // For LocalExceptionWrapper.
#include <Ice/Protocol.h>
#include <Ice/ReplyStatus.h>
#include <Ice/ImplicitContextI.h>
#include <Ice/ThreadPool.h>
#include <Ice/RetryQueue.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(AsyncResult* p) { return p; }

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
    
    AsynchronousException(const IceInternal::InstancePtr& instance,
                          const Ice::AsyncResultPtr& result,
                          const Ice::Exception& ex) :
        DispatchWorkItem(instance), _result(result), _exception(dynamic_cast<Ice::LocalException*>(ex.ice_clone()))
    {
    }
    
    virtual void
    run()
    {
        _result->__exception(*_exception.get());
    }
    
private:
    
    const Ice::AsyncResultPtr _result;
    const auto_ptr<Ice::Exception> _exception;
};

class AsynchronousSent : public DispatchWorkItem
{
public:
    
    AsynchronousSent(const IceInternal::InstancePtr& instance, const Ice::AsyncResultPtr& result) : 
        DispatchWorkItem(instance), _result(result)
    {
    }
    
    virtual void
    run()
    {
        _result->__sent();
    }
    
private:
    
    const Ice::AsyncResultPtr _result;
};

};

Ice::AsyncResult::AsyncResult(const IceInternal::InstancePtr& instance,
                              const string& op,
                              const CallbackBasePtr& del,
                              const LocalObjectPtr& cookie) :
    _instance(instance),
    _operation(op), 
    _callback(del),
    _cookie(cookie),
    _is(instance.get()),
    _os(instance.get()),
    _state(0),
    _exception(0)
{
    if(!_callback)
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__);
    }
    const_cast<CallbackBasePtr&>(_callback) = _callback->__verify(const_cast<LocalObjectPtr&>(_cookie));
}

Ice::AsyncResult::~AsyncResult()
{
}

bool
Ice::AsyncResult::operator==(const AsyncResult& r) const
{
    return this == &r;
}

bool
Ice::AsyncResult::operator<(const AsyncResult& r) const
{
    return this < &r;
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
    while(!(_state & (Sent | Done)))
    {
        _monitor.wait();
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
Ice::AsyncResult::__sent()
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
	    _callback->__sent(self);
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
}

void
Ice::AsyncResult::__sentAsync()
{
    //
    // This is called when it's not safe to call the sent callback synchronously
    // from this thread. Instead the exception callback is called asynchronously from
    // the client thread pool.
    //
    try
    {
        _instance->clientThreadPool()->execute(new AsynchronousSent(_instance, this));
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
    }
}

void
Ice::AsyncResult::__exception(const Ice::Exception& ex)
{
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
	_state |= Done;
	_exception.reset(ex.ice_clone());
	_monitor.notifyAll();
    }

    if(_callback)
    {
	try
	{
            AsyncResultPtr self(this);
            _callback->__completed(self);
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
}

void
Ice::AsyncResult::__exceptionAsync(const Ice::Exception& ex)
{
    //
    // This is called when it's not safe to call the exception callback synchronously
    // from this thread. Instead the exception callback is called asynchronously from
    // the client thread pool.
    //
    // CommunicatorDestroyedException is the only exception that can propagate directly
    // from this method.
    //
    _instance->clientThreadPool()->execute(new AsynchronousException(_instance, this, ex));
}

void
Ice::AsyncResult::__response()
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
            _callback->__completed(self);
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

IceInternal::OutgoingAsync::OutgoingAsync(const ObjectPrx& prx, 
                                          const std::string& operation,
                                          const CallbackBasePtr& delegate,
                                          const Ice::LocalObjectPtr& cookie) :
    AsyncResult(prx->__reference()->getInstance(), operation, delegate, cookie),
    _proxy(prx)
{
}

void
IceInternal::OutgoingAsync::__prepare(const std::string& operation, OperationMode mode, const Context* context)
{
    _delegate = 0;
    _cnt = 0;
    _mode = mode;
    _sentSynchronously = false;

    //
    // Can't call async via a batch proxy.
    //
    if(_proxy->ice_isBatchOneway() || _proxy->ice_isBatchDatagram())
    {
        throw Ice::FeatureNotSupportedException(__FILE__, __LINE__, "can't send batch requests with AMI");
    }

    _os.writeBlob(requestHdr, sizeof(requestHdr));

    Reference* ref = _proxy->__reference().get();

    ref->getIdentity().__write(&_os);

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
        __writeContext(&_os, *context);
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
            __writeContext(&_os, prxContext);
        }
        else
        {
            implicitContext->write(prxContext, &_os);
        }
    }
        
    _os.startWriteEncaps();
}

bool
IceInternal::OutgoingAsync::__sent(Ice::ConnectionI* connection)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);

    bool alreadySent = _state & Sent; // Expected in case of a retry.
    _state |= Sent;

    //
    // It's possible for the request to be done already when using IOCP. This 
    // is the case for example if the send callback is dispatched after the 
    // read callback for the response/exception.
    //
    if(!(_state & Done))
    {
        if(!_proxy->ice_isTwoway())
        {
            _state |= Done | OK;
        }
        else if(connection->timeout() > 0)
        {
            assert(!_timerTaskConnection);
            _timerTaskConnection = connection;
            IceUtil::Time timeout = IceUtil::Time::milliSeconds(connection->timeout());
            _instance->timer()->schedule(this, timeout);
        }
    }
    _monitor.notifyAll();
    return !alreadySent && _callback && _callback->__hasSentCallback();
}

void
IceInternal::OutgoingAsync::__sent()
{
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    AsyncResult::__sent();
#else
    ::Ice::AsyncResult::__sent();
#endif
}

void
IceInternal::OutgoingAsync::__finished(const Ice::LocalException& exc, bool sent)
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        assert(!(_state & Done));
        if(_timerTaskConnection)
        {
            _instance->timer()->cancel(this);
            _timerTaskConnection = 0;
        }
    }
     
    //
    // NOTE: at this point, synchronization isn't needed, no other threads should be
    // calling on the callback.
    //

    try
    {
        int interval = handleException(exc, sent); // This will throw if the invocation can't be retried.
        if(interval > 0)
        {
            _instance->retryQueue()->add(this, interval);
        }
        else
        {
            __send(false);
        }
    }
    catch(const Ice::LocalException& ex)
    {
        __exception(ex);
    }
}

void
IceInternal::OutgoingAsync::__finished(const LocalExceptionWrapper& exc)
{
    //
    // NOTE: at this point, synchronization isn't needed, no other threads should be
    // calling on the callback. The LocalExceptionWrapper exception is only called
    // before the invocation is sent.
    //

    try
    {
        int interval = handleException(exc); // This will throw if the invocation can't be retried.
        if(interval > 0)
        {
            _instance->retryQueue()->add(this, interval);
        }
        else
        {
            __send(false);
        }
    }
    catch(const Ice::LocalException& ex)
    {
        __exception(ex);
    }
}

void
IceInternal::OutgoingAsync::__finished(BasicStream& is)
{
    assert(_proxy->ice_isTwoway()); // Can only be called for twoways.

    Ice::Byte replyStatus;
    try
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        assert(!_exception.get() && !(_state & Done));

        if(_timerTaskConnection)
        {
            _instance->timer()->cancel(this);
            _timerTaskConnection = 0;
        }

        _is.swap(is);
        _is.read(replyStatus);
        
        switch(replyStatus)
        {
            case replyOK:
            case replyUserException:
            {
                break;
            }
            
            case replyObjectNotExist:
            case replyFacetNotExist:
            case replyOperationNotExist:
            {
                Identity ident;
                ident.__read(&_is);
                
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
                
                auto_ptr<RequestFailedException> ex;
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
                
                auto_ptr<UnknownException> ex;
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
	if(replyStatus == replyOK)
	{
	    _state |= OK;
	}
	_monitor.notifyAll();
    }
    catch(const LocalException& ex)
    {
        __finished(ex, true);
        return;
    }

    assert(replyStatus == replyOK || replyStatus == replyUserException);
    __response();
}

bool
IceInternal::OutgoingAsync::__send(bool synchronous)
{
    while(true)
    {
        int interval = 0;
        try
        {
            _delegate = _proxy->__getDelegate(true);
            AsyncStatus status = _delegate->__getRequestHandler()->sendAsyncRequest(this);
            if(status & AsyncStatusSent)
            {
                if(synchronous)
                {
                    _sentSynchronously = true;
                    if(status & AsyncStatusInvokeSentCallback)
                    {
                        __sent(); // Call the sent callback from the user thread.
                    }
                }
                else
                {
                    if(status & AsyncStatusInvokeSentCallback)
                    {
                        __sentAsync(); // Call the sent callback from a client thread pool thread.
                    }
                }
            }
            break;
        }
        catch(const LocalExceptionWrapper& ex)
        {
            interval = handleException(ex);
        }
        catch(const Ice::LocalException& ex)
        {
            interval = handleException(ex, false);
        }

        if(interval > 0)
        {
            _instance->retryQueue()->add(this, interval);
            return false;
        }
    }
    return _sentSynchronously;
}

int
IceInternal::OutgoingAsync::handleException(const LocalExceptionWrapper& ex)
{
    if(_mode == Nonmutating || _mode == Idempotent)
    {
        return _proxy->__handleExceptionWrapperRelaxed(_delegate, ex, false, _cnt);
    }
    else
    {
        return _proxy->__handleExceptionWrapper(_delegate, ex);
    }
}

int
IceInternal::OutgoingAsync::handleException(const Ice::LocalException& exc, bool sent)
{
    try
    {
        //
        // A CloseConnectionException indicates graceful server shutdown, and is therefore
        // always repeatable without violating "at-most-once". That's because by sending a
        // close connection message, the server guarantees that all outstanding requests
        // can safely be repeated.
        //
        // An ObjectNotExistException can always be retried as well without violating 
        // "at-most-once" (see the implementation of the checkRetryAfterException method of
        // the ProxyFactory class for the reasons why it can be useful).
        // 
        if(!sent || 
           dynamic_cast<const CloseConnectionException*>(&exc) || 
           dynamic_cast<const ObjectNotExistException*>(&exc))
        {
            exc.ice_throw();
        }

        //
        // Throw the exception wrapped in a LocalExceptionWrapper, to indicate that the 
        // request cannot be resent without potentially violating the "at-most-once" 
        // principle.
        //
        throw LocalExceptionWrapper(exc, false);
    }
    catch(const LocalExceptionWrapper& ex)
    {
        if(_mode == Nonmutating || _mode == Idempotent)
        {
            return _proxy->__handleExceptionWrapperRelaxed(_delegate, ex, false, _cnt);
        }
        else
        {
            return _proxy->__handleExceptionWrapper(_delegate, ex);
        }
    }
    catch(const Ice::LocalException& ex)
    {
        return _proxy->__handleException(_delegate, ex, false, _cnt);
    }
    return 0; // Keep the compiler happy.
}

void
IceInternal::OutgoingAsync::runTimerTask() // Implementation of TimerTask::runTimerTask()
{
    Ice::ConnectionIPtr connection;
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        connection = _timerTaskConnection;
        _timerTaskConnection = 0;
    }

    if(connection)
    {
        connection->exception(Ice::TimeoutException(__FILE__, __LINE__));
    }
}

IceInternal::BatchOutgoingAsync::BatchOutgoingAsync(const InstancePtr& instance, 
                                                    const std::string& operation,
                                                    const CallbackBasePtr& delegate,
                                                    const Ice::LocalObjectPtr& cookie) :
    AsyncResult(instance, operation, delegate, cookie)
{
}

bool
IceInternal::BatchOutgoingAsync::__sent(Ice::ConnectionI* connection)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    assert(!_exception.get());
    _state |= Done | OK | Sent;
    _monitor.notifyAll();
    return _callback && _callback->__hasSentCallback();
}

void
IceInternal::BatchOutgoingAsync::__sent()
{
#if defined(_MSC_VER) && (_MSC_VER < 1300) // VC++ 6 compiler bug
    AsyncResult::__sent();
#else
    ::Ice::AsyncResult::__sent();
#endif
}

void
IceInternal::BatchOutgoingAsync::__finished(const Ice::LocalException& exc, bool)
{
    __exception(exc);
}

IceInternal::ProxyBatchOutgoingAsync::ProxyBatchOutgoingAsync(const Ice::ObjectPrx& proxy, 
                                                              const std::string& operation,
                                                              const CallbackBasePtr& delegate,
                                                              const Ice::LocalObjectPtr& cookie) :
    BatchOutgoingAsync(proxy->__reference()->getInstance(), operation, delegate, cookie),
    _proxy(proxy)
{
}

void
IceInternal::ProxyBatchOutgoingAsync::__send()
{
    //
    // We don't automatically retry if ice_flushBatchRequests fails. Otherwise, if some batch
    // requests were queued with the connection, they would be lost without being noticed.
    //
    Handle<IceDelegate::Ice::Object> delegate;
    int cnt = -1; // Don't retry.
    try
    {
        delegate = _proxy->__getDelegate(true);
        AsyncStatus status = delegate->__getRequestHandler()->flushAsyncBatchRequests(this);
        if(status & AsyncStatusSent)
        {
            _sentSynchronously = true;
            if(status & AsyncStatusInvokeSentCallback)
            {
                __sent();
            }
        }
    }
    catch(const ::Ice::LocalException& ex)
    {
        _proxy->__handleException(delegate, ex, 0, cnt);
    }
}

IceInternal::ConnectionBatchOutgoingAsync::ConnectionBatchOutgoingAsync(const ConnectionIPtr& con,
                                                                        const InstancePtr& instance,
                                                                        const string& operation,
                                                                        const CallbackBasePtr& delegate,
                                                                        const Ice::LocalObjectPtr& cookie) :
    BatchOutgoingAsync(instance, operation, delegate, cookie),
    _connection(con)
{
}

void
IceInternal::ConnectionBatchOutgoingAsync::__send()
{
    AsyncStatus status = _connection->flushAsyncBatchRequests(this);
    if(status & AsyncStatusSent)
    {
        _sentSynchronously = true;
        if(status & AsyncStatusInvokeSentCallback)
        {
            __sent();
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
    BatchOutgoingAsync(instance, operation, delegate, cookie),
    _communicator(communicator)
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
}

void
IceInternal::CommunicatorBatchOutgoingAsync::flushConnection(const ConnectionPtr& con)
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        ++_useCount;
    }
    CallbackPtr cb = newCallback(this, &CommunicatorBatchOutgoingAsync::completed,
                                 &CommunicatorBatchOutgoingAsync::sent);
    con->begin_flushBatchRequests(cb);
}

void
IceInternal::CommunicatorBatchOutgoingAsync::ready()
{
    check(0, 0, true);
}

void
IceInternal::CommunicatorBatchOutgoingAsync::completed(const AsyncResultPtr& r)
{
    ConnectionPtr con = r->getConnection();
    assert(con);

    try
    {
        con->end_flushBatchRequests(r);
        assert(false); // completed() should only be called when an exception occurs.
    }
    catch(const Ice::LocalException& ex)
    {
        check(r, &ex, false);
    }
}

void
IceInternal::CommunicatorBatchOutgoingAsync::sent(const AsyncResultPtr& r)
{
    check(r, 0, r->sentSynchronously());
}

void
IceInternal::CommunicatorBatchOutgoingAsync::check(const AsyncResultPtr& r, const LocalException* ex, bool userThread)
{
    bool done = false;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        assert(_useCount > 0);
        --_useCount;

        //
        // We report that the communicator flush request was sent synchronously
        // if all of the connection flush requests are sent synchronously.
        //
        if((r && !r->sentSynchronously()) || ex)
        {
            _sentSynchronously = false;
        }

        if(_useCount == 0)
        {
            done = true;
            _state |= Done | OK | Sent;
            _monitor.notifyAll();
        }
    }

    if(done)
    {
        //
        // _sentSynchronously is immutable here.
        //
        if(!_sentSynchronously && userThread)
        {
            __sentAsync();
        }
        else
        {
            assert(_sentSynchronously == userThread); // sentSynchronously && !userThread is impossible.
            BatchOutgoingAsync::__sent();
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

    virtual void __completed(const Ice::AsyncResultPtr&) const
    {
         assert(false);
    }

    virtual CallbackBasePtr __verify(Ice::LocalObjectPtr&)
    {
        //
        // Called by the AsyncResult constructor to verify the delegate. The dummy
        // delegate is passed when the user used a begin_ method without delegate. 
        // By returning 0 here, we tell the AsyncResult that no delegates was 
        // provided.
        //
        return 0;
    }

    virtual void __sent(const AsyncResultPtr&) const
    {
         assert(false);
    }

    virtual bool __hasSentCallback() const
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
