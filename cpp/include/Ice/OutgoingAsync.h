// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OUTGOING_ASYNC_H
#define ICE_OUTGOING_ASYNC_H

#include <IceUtil/Timer.h>
#include <IceUtil/Monitor.h>
#include <Ice/OutgoingAsyncF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/RequestHandlerF.h>
#include <Ice/ConnectionF.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>
#include <Ice/ObserverHelper.h>
#include <Ice/LocalException.h>
#include <IceUtil/UniquePtr.h>

#ifndef ICE_CPP11_MAPPING
#    include <Ice/AsyncResult.h>
#endif

#include <exception>

namespace IceInternal
{

class RetryException;
class CollocatedRequestHandler;

class ICE_API OutgoingAsyncCompletionCallback
{
public:
    virtual ~OutgoingAsyncCompletionCallback();

protected:

    virtual bool handleSent(bool, bool) = 0;
    virtual bool handleException(const Ice::Exception&) = 0;
    virtual bool handleResponse(bool) = 0;

    virtual void handleInvokeSent(bool, OutgoingAsyncBase*) const = 0;
    virtual void handleInvokeException(const Ice::Exception&, OutgoingAsyncBase*) const = 0;
    virtual void handleInvokeResponse(bool, OutgoingAsyncBase*) const = 0;
};

//
// Base class for handling asynchronous invocations. This class is
// responsible for the handling of the output stream and the child
// invocation observer.
//
class ICE_API OutgoingAsyncBase : virtual public OutgoingAsyncCompletionCallback,
#ifdef ICE_CPP11_MAPPING
                                  public std::enable_shared_from_this<OutgoingAsyncBase>
#else
                                  public Ice::AsyncResult
#endif
{
public:

    virtual bool sent();
    virtual bool exception(const Ice::Exception&);
    virtual bool response();

    void invokeSentAsync();
    void invokeExceptionAsync();
    void invokeResponseAsync();

    void invokeSent();
    void invokeException();
    void invokeResponse();

    virtual void cancelable(const IceInternal::CancellationHandlerPtr&);
    void cancel();

#ifndef ICE_CPP11_MAPPING
    virtual Ice::Int getHash() const;

    virtual Ice::CommunicatorPtr getCommunicator() const;
    virtual Ice::ConnectionPtr getConnection() const;
    virtual Ice::ObjectPrx getProxy() const;

    virtual Ice::LocalObjectPtr getCookie() const;
    virtual const std::string& getOperation() const;

    virtual bool isCompleted() const;
    virtual void waitForCompleted();

    virtual bool isSent() const;
    virtual void waitForSent();

    virtual bool sentSynchronously() const;

    virtual void throwLocalException() const;

    virtual bool __wait();
    virtual Ice::InputStream* __startReadParams();
    virtual void __endReadParams();
    virtual void __readEmptyParams();
    virtual void __readParamEncaps(const ::Ice::Byte*&, ::Ice::Int&);
    virtual void __throwUserException();
#endif

    void attachRemoteObserver(const Ice::ConnectionInfoPtr& c, const Ice::EndpointPtr& endpt, Ice::Int requestId)
    {
        const Ice::Int size = static_cast<Ice::Int>(_os.b.size() - headerSize - 4);
        _childObserver.attach(getObserver().getRemoteObserver(c, endpt, requestId, size));
    }

    void attachCollocatedObserver(const Ice::ObjectAdapterPtr& adapter, Ice::Int requestId)
    {
        const Ice::Int size = static_cast<Ice::Int>(_os.b.size() - headerSize - 4);
        _childObserver.attach(getObserver().getCollocatedObserver(adapter, requestId, size));
    }

    Ice::OutputStream* getOs()
    {
        return &_os;
    }

    Ice::InputStream* getIs()
    {
        return &_is;
    }

protected:

    OutgoingAsyncBase(const InstancePtr&);

    bool sentImpl(bool);
    bool exceptionImpl(const Ice::Exception&);
    bool responseImpl(bool);

    void cancel(const Ice::LocalException&);
    void checkCanceled();

    void warning(const std::exception&) const;
    void warning() const;

    //
    // This virtual method is necessary for the communicator flush
    // batch requests implementation.
    //
    virtual IceInternal::InvocationObserver& getObserver()
    {
        return _observer;
    }

    const InstancePtr _instance;
    Ice::ConnectionPtr _cachedConnection;
    bool _sentSynchronously;
    bool _doneInSent;
    unsigned char _state;

#ifdef ICE_CPP11_MAPPING
    std::mutex _m;
    using Lock = std::lock_guard<std::mutex>;
#else
    IceUtil::Monitor<IceUtil::Mutex> _m;
    typedef IceUtil::Monitor<IceUtil::Mutex>::Lock Lock;
    Ice::LocalObjectPtr _cookie;
#endif

    IceUtil::UniquePtr<Ice::Exception> _ex;
    IceUtil::UniquePtr<Ice::LocalException> _cancellationException;

    InvocationObserver _observer;
    ObserverHelperT<Ice::Instrumentation::ChildInvocationObserver> _childObserver;

    Ice::OutputStream _os;
    Ice::InputStream _is;

    CancellationHandlerPtr _cancellationHandler;

    static const unsigned char OK;
    static const unsigned char Sent;
#ifndef ICE_CPP11_MAPPING
    static const unsigned char Done;
    static const unsigned char EndCalled;
#endif
};

//
// Base class for proxy based invocations. This class handles the
// retry for proxy invocations. It also ensures the child observer is
// correct notified of failures and make sure the retry task is
// correctly canceled when the invocation completes.
//
class ICE_API ProxyOutgoingAsyncBase : public OutgoingAsyncBase,
                                       public IceUtil::TimerTask
{
public:

    virtual AsyncStatus invokeRemote(const Ice::ConnectionIPtr&, bool, bool) = 0;
    virtual AsyncStatus invokeCollocated(CollocatedRequestHandler*) = 0;

    virtual bool exception(const Ice::Exception&);
    virtual void cancelable(const CancellationHandlerPtr&);

    void retryException(const Ice::Exception&);
    void retry();
    void abort(const Ice::Exception&);

#ifdef ICE_CPP11_MAPPING
    std::shared_ptr<ProxyOutgoingAsyncBase> shared_from_this()
    {
        return std::static_pointer_cast<ProxyOutgoingAsyncBase>(OutgoingAsyncBase::shared_from_this());
    }
#else
    virtual Ice::ObjectPrx getProxy() const;
    virtual Ice::CommunicatorPtr getCommunicator() const;
#endif

protected:

    ProxyOutgoingAsyncBase(const Ice::ObjectPrxPtr&);
    ~ProxyOutgoingAsyncBase();

    void invokeImpl(bool);
    bool sentImpl(bool);
    bool exceptionImpl(const Ice::Exception&);
    bool responseImpl(bool);

    virtual void runTimerTask();

    const Ice::ObjectPrxPtr _proxy;
    RequestHandlerPtr _handler;
    Ice::OperationMode _mode;

private:

    int _cnt;
    bool _sent;
};

//
// Class for handling Slice operation invocations
//
class ICE_API OutgoingAsync : public ProxyOutgoingAsyncBase
{
public:

    OutgoingAsync(const Ice::ObjectPrxPtr&);

    void prepare(const std::string&, Ice::OperationMode, const Ice::Context&);

    virtual bool sent();
    virtual bool response();

    virtual AsyncStatus invokeRemote(const Ice::ConnectionIPtr&, bool, bool);
    virtual AsyncStatus invokeCollocated(CollocatedRequestHandler*);

    void abort(const Ice::Exception&);
    void invoke(const std::string&);
#ifdef ICE_CPP11_MAPPING
    void invoke(const std::string&, Ice::OperationMode, Ice::FormatType, const Ice::Context&,
                std::function<void(Ice::OutputStream*)>);
    void throwUserException();
#endif

    Ice::OutputStream* startWriteParams(Ice::FormatType format)
    {
        _os.startEncapsulation(_encoding, format);
        return &_os;
    }
    void endWriteParams()
    {
        _os.endEncapsulation();
    }
    void writeEmptyParams()
    {
        _os.writeEmptyEncapsulation(_encoding);
    }
    void writeParamEncaps(const ::Ice::Byte* encaps, ::Ice::Int size)
    {
        if(size == 0)
        {
            _os.writeEmptyEncapsulation(_encoding);
        }
        else
        {
            _os.writeEncapsulation(encaps, size);
        }
    }

protected:

    const Ice::EncodingVersion _encoding;

#ifdef ICE_CPP11_MAPPING
    std::function<void(const ::Ice::UserException&)> _userException;
#endif

    bool _synchronous;
};

//
// Class for handling the proxy's begin_ice_flushBatchRequest request.
//
class ICE_API ProxyFlushBatchAsync : public ProxyOutgoingAsyncBase
{
public:

    ProxyFlushBatchAsync(const Ice::ObjectPrxPtr&);

    virtual AsyncStatus invokeRemote(const Ice::ConnectionIPtr&, bool, bool);
    virtual AsyncStatus invokeCollocated(CollocatedRequestHandler*);

    void invoke(const std::string&);

private:

    int _batchRequestNum;
};
typedef IceUtil::Handle<ProxyFlushBatchAsync> ProxyFlushBatchAsyncPtr;

//
// Class for handling the proxy's begin_ice_getConnection request.
//
class ICE_API ProxyGetConnection :  public ProxyOutgoingAsyncBase
{
public:

    ProxyGetConnection(const Ice::ObjectPrxPtr&);

    virtual AsyncStatus invokeRemote(const Ice::ConnectionIPtr&, bool, bool);
    virtual AsyncStatus invokeCollocated(CollocatedRequestHandler*);

    void invoke(const std::string&);
};
typedef IceUtil::Handle<ProxyGetConnection> ProxyGetConnectionPtr;

//
// Class for handling Ice::Connection::begin_flushBatchRequests
//
class ICE_API ConnectionFlushBatchAsync : public OutgoingAsyncBase
{
public:

    ConnectionFlushBatchAsync(const Ice::ConnectionIPtr&, const InstancePtr&);

    virtual Ice::ConnectionPtr getConnection() const;

    void invoke(const std::string&);

private:

    const Ice::ConnectionIPtr _connection;
};
typedef IceUtil::Handle<ConnectionFlushBatchAsync> ConnectionFlushBatchAsyncPtr;

//
// Class for handling Ice::Communicator::begin_flushBatchRequests
//
class ICE_API CommunicatorFlushBatchAsync : public OutgoingAsyncBase
{
public:

    virtual ~CommunicatorFlushBatchAsync();

    CommunicatorFlushBatchAsync(const InstancePtr&);

    void flushConnection(const Ice::ConnectionIPtr&);
    void invoke(const std::string&);

#ifdef ICE_CPP11_MAPPING
    std::shared_ptr<CommunicatorFlushBatchAsync> shared_from_this()
    {
        return std::static_pointer_cast<CommunicatorFlushBatchAsync>(OutgoingAsyncBase::shared_from_this());
    }
#endif

private:

    void check(bool);

    int _useCount;
    InvocationObserver _observer;
};

}

namespace IceInternal
{

#ifdef ICE_CPP11_MAPPING

class ICE_API LambdaInvoke : virtual public OutgoingAsyncCompletionCallback
{
public:

    LambdaInvoke(std::function<void(::std::exception_ptr)> exception, std::function<void(bool)> sent) :
        _exception(std::move(exception)), _sent(std::move(sent))
    {
    }

protected:

    virtual bool handleSent(bool, bool) override;
    virtual bool handleException(const Ice::Exception&) override;
    virtual bool handleResponse(bool) override;

    virtual void handleInvokeSent(bool, OutgoingAsyncBase*) const override;
    virtual void handleInvokeException(const Ice::Exception&, OutgoingAsyncBase*) const override;
    virtual void handleInvokeResponse(bool, OutgoingAsyncBase*) const override;

    std::function<void(::std::exception_ptr)> _exception;
    std::function<void(bool)> _sent;
    std::function<void(bool)> _response;
};

template<typename Promise>
class PromiseInvoke : virtual public OutgoingAsyncCompletionCallback
{
public:

    auto
    getFuture() -> decltype(std::declval<Promise>().get_future())
    {
        return _promise.get_future();
    }

protected:

    Promise _promise;
    std::function<void(bool)> _response;

private:

    virtual bool handleSent(bool, bool) override
    {
        return false;
    }

    virtual bool handleException(const Ice::Exception& ex) override
    {
        try
        {
            ex.ice_throw();
        }
        catch(const Ice::Exception&)
        {
            _promise.set_exception(std::current_exception());
        }
        return false;
    }

    virtual bool handleResponse(bool ok) override
    {
        _response(ok);
        return false;
    }

    virtual void handleInvokeSent(bool, OutgoingAsyncBase*) const override
    {
        assert(false);
    }

    virtual void handleInvokeException(const Ice::Exception&, OutgoingAsyncBase*) const override
    {
        assert(false);
    }

    virtual void handleInvokeResponse(bool, OutgoingAsyncBase*) const override
    {
        assert(false);
    }
};

template<typename T>
class OutgoingAsyncT : public OutgoingAsync
{
public:

    using OutgoingAsync::OutgoingAsync;

    void
    invoke(const std::string& operation,
           Ice::OperationMode mode,
           Ice::FormatType format,
           const Ice::Context& ctx,
           std::function<void(Ice::OutputStream*)> write,
           std::function<void(const Ice::UserException&)> userException)
    {
        _read = [](Ice::InputStream* stream)
        {
            T v;
            stream->read(v);
            return v;
        };
        _userException = std::move(userException);
        OutgoingAsync::invoke(operation, mode, format, ctx, std::move(write));
    }

    void
    invoke(const std::string& operation,
           Ice::OperationMode mode,
           Ice::FormatType format,
           const Ice::Context& ctx,
           std::function<void(Ice::OutputStream*)> write,
           std::function<void(const Ice::UserException&)> userException,
           std::function<T(Ice::InputStream*)> read)
    {
        _read = std::move(read);
        _userException = std::move(userException);
        OutgoingAsync::invoke(operation, mode, format, ctx, std::move(write));
    }

protected:

    std::function<T(Ice::InputStream*)> _read;
};

template<>
class OutgoingAsyncT<void> : public OutgoingAsync
{
public:

    using OutgoingAsync::OutgoingAsync;

    void
    invoke(const std::string& operation,
           Ice::OperationMode mode,
           Ice::FormatType format,
           const Ice::Context& ctx,
           std::function<void(Ice::OutputStream*)> write,
           std::function<void(const Ice::UserException&)> userException)
    {
        _userException = std::move(userException);
        OutgoingAsync::invoke(operation, mode, format, ctx, std::move(write));
    }
};

template<typename R>
class LambdaOutgoing : public OutgoingAsyncT<R>, public LambdaInvoke
{
public:

    LambdaOutgoing(const std::shared_ptr<Ice::ObjectPrx>& proxy,
                   std::function<void(R)> response,
                   std::function<void(::std::exception_ptr)> ex,
                   std::function<void(bool)> sent) :
        OutgoingAsyncT<R>(proxy), LambdaInvoke(std::move(ex), std::move(sent))
    {
        _response = [this, response](bool ok)
        {
            if(!ok)
            {
                this->throwUserException();
            }
            else if(response)
            {
                assert(this->_read);
                this->_is.startEncapsulation();
                R v = this->_read(&this->_is);
                this->_is.endEncapsulation();
                try
                {
                    response(std::move(v));
                }
                catch(...)
                {
                    throw std::current_exception();
                }
            }
        };
    }
};

template<>
class LambdaOutgoing<void> : public OutgoingAsyncT<void>, public LambdaInvoke
{
public:

    LambdaOutgoing(const std::shared_ptr<Ice::ObjectPrx>& proxy,
                   std::function<void()> response,
                   std::function<void(::std::exception_ptr)> ex,
                   std::function<void(bool)> sent) :
        OutgoingAsyncT<void>(proxy), LambdaInvoke(std::move(ex), std::move(sent))
    {
        _response = [this, response](bool ok)
        {
            if(!ok)
            {
                this->throwUserException();
            }
            else if(response)
            {
                if(!this->_is.b.empty())
                {
                    this->_is.skipEmptyEncapsulation();
                }

                try
                {
                    response();
                }
                catch(...)
                {
                    throw std::current_exception();
                }
            }
        };
    }
};

class CustomLambdaOutgoing : public OutgoingAsync, public LambdaInvoke
{
public:

    CustomLambdaOutgoing(const std::shared_ptr<Ice::ObjectPrx>& proxy,
                         std::function<void(Ice::InputStream*)> read,
                         std::function<void(::std::exception_ptr)> ex,
                         std::function<void(bool)> sent) :
        OutgoingAsync(proxy), LambdaInvoke(std::move(ex), std::move(sent))
    {
        _response = [this, read](bool ok)
        {
            if(!ok)
            {
                this->throwUserException();
            }
            else if(read)
            {
                //
                // Read and respond
                //
                read(&this->_is);
            }
        };
    }

    void
    invoke(const std::string& operation,
           Ice::OperationMode mode,
           Ice::FormatType format,
           const Ice::Context& ctx,
           std::function<void(Ice::OutputStream*)> write,
           std::function<void(const Ice::UserException&)> userException)
    {
        _userException = std::move(userException);
        OutgoingAsync::invoke(operation, mode, format, ctx, std::move(write));
    }
};

template<typename P, typename R>
class PromiseOutgoing : public OutgoingAsyncT<R>, public PromiseInvoke<P>
{
public:

    PromiseOutgoing(const std::shared_ptr<Ice::ObjectPrx>& proxy, bool synchronous) :
        OutgoingAsyncT<R>(proxy)
    {
        this->_synchronous = synchronous;
        this->_response = [this](bool ok)
        {
            if(ok)
            {
                assert(this->_read);
                this->_is.startEncapsulation();
                R v = this->_read(&this->_is);
                this->_is.endEncapsulation();
                this->_promise.set_value(v);
            }
            else
            {
                this->throwUserException();
            }
        };
    }
};

template<typename P>
class PromiseOutgoing<P, void> : public OutgoingAsyncT<void>, public PromiseInvoke<P>
{
public:

    PromiseOutgoing(const std::shared_ptr<Ice::ObjectPrx>& proxy, bool synchronous) :
        OutgoingAsyncT<void>(proxy)
    {
        this->_synchronous = synchronous;
        this->_response = [&](bool ok)
        {
            if(this->_is.b.empty())
            {
                //
                // If there's no response (oneway, batch-oneway proxies), we just set the promise
                // on completion without reading anything from the input stream. This is required for
                // batch invocations.
                //
                this->_promise.set_value();
            }
            else if(ok)
            {
                this->_is.skipEmptyEncapsulation();
                this->_promise.set_value();
            }
            else
            {
                this->throwUserException();
            }
        };
    }

    virtual bool handleSent(bool done, bool) override
    {
        if(done)
        {
            PromiseInvoke<P>::_promise.set_value();
        }
        return false;
    }
};

#else

//
// Base class for all callbacks.
//
class ICE_API CallbackBase : public IceUtil::Shared
{
public:

    virtual ~CallbackBase();

    void checkCallback(bool, bool);

    virtual void completed(const ::Ice::AsyncResultPtr&) const = 0;
    virtual IceUtil::Handle<CallbackBase> verify(const ::Ice::LocalObjectPtr&) = 0;
    virtual void sent(const ::Ice::AsyncResultPtr&) const = 0;
    virtual bool hasSentCallback() const = 0;
};
typedef IceUtil::Handle<CallbackBase> CallbackBasePtr;

//
// Base class for generic callbacks.
//
class ICE_API GenericCallbackBase : public virtual CallbackBase
{
public:

    virtual ~GenericCallbackBase();
};

//
// See comments in OutgoingAsync.cpp
//
extern ICE_API CallbackBasePtr __dummyCallback;

//
// Generic callback template that requires the caller to down-cast the
// proxy and the cookie that are obtained from the AsyncResult.
//
template<class T>
class AsyncCallback : public GenericCallbackBase
{
public:

    typedef T callback_type;
    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Callback)(const ::Ice::AsyncResultPtr&);

    AsyncCallback(const TPtr& instance, Callback cb, Callback sentcb = 0) :
        _callback(instance), _completed(cb), _sent(sentcb)
    {
        checkCallback(instance, cb != 0);
    }

    virtual void completed(const ::Ice::AsyncResultPtr& result) const
    {
        (_callback.get()->*_completed)(result);
    }

    virtual CallbackBasePtr verify(const ::Ice::LocalObjectPtr&)
    {
        return this; // Nothing to do, the cookie is not type-safe.
    }

    virtual void sent(const ::Ice::AsyncResultPtr& result) const
    {
        if(_sent)
        {
            (_callback.get()->*_sent)(result);
        }
    }

    virtual bool hasSentCallback() const
    {
        return _sent != 0;
    }

private:

    TPtr _callback;
    Callback _completed;
    Callback _sent;
};

class CallbackCompletion : virtual public OutgoingAsyncCompletionCallback
{
public:

    CallbackCompletion(const CallbackBasePtr& cb, const Ice::LocalObjectPtr& cookie) : _callback(cb)
    {
        if(!_callback)
        {
            throw IceUtil::IllegalArgumentException(__FILE__, __LINE__);
        }
        const_cast<CallbackBasePtr&>(_callback) = _callback->verify(cookie);
    }

    virtual bool handleSent(bool, bool alreadySent)
    {
        return _callback && _callback->hasSentCallback() && !alreadySent;
    }

    virtual bool handleException(const Ice::Exception&)
    {
        return _callback;
    }

    virtual bool handleResponse(bool)
    {
        return _callback;
    }

    virtual void handleInvokeSent(bool, OutgoingAsyncBase* outAsync) const
    {
        _callback->sent(outAsync);
    }

    virtual void handleInvokeException(const Ice::Exception&, OutgoingAsyncBase* outAsync) const
    {
        _callback->completed(outAsync);
    }

    virtual void handleInvokeResponse(bool, OutgoingAsyncBase* outAsync) const
    {
        _callback->completed(outAsync);
    }

private:

    const CallbackBasePtr _callback;
};

class CallbackOutgoing : public OutgoingAsync, public CallbackCompletion
{
public:

    CallbackOutgoing(const Ice::ObjectPrx& proxy,
                     const std::string& operation,
                     const CallbackBasePtr& cb,
                     const Ice::LocalObjectPtr& cookie) :
        OutgoingAsync(proxy), CallbackCompletion(cb, cookie), _operation(operation)
    {
        _cookie = cookie;
    }

    virtual const std::string&
    getOperation() const
    {
        return _operation;
    }

private:

    const std::string& _operation;
};

#endif

}

#ifndef ICE_CPP11_MAPPING

namespace Ice
{

typedef IceUtil::Handle< ::IceInternal::GenericCallbackBase> CallbackPtr;

template<class T> CallbackPtr
newCallback(const IceUtil::Handle<T>& instance,
            void (T::*cb)(const AsyncResultPtr&),
            void (T::*sentcb)(const AsyncResultPtr&) = 0)
{
    return new ::IceInternal::AsyncCallback<T>(instance, cb, sentcb);
}

template<class T> CallbackPtr
newCallback(T* instance,
            void (T::*cb)(const AsyncResultPtr&),
            void (T::*sentcb)(const AsyncResultPtr&) = 0)
{
    return new ::IceInternal::AsyncCallback<T>(instance, cb, sentcb);
}

}

//
// Operation callbacks are specified in Proxy.h
//

#endif

#endif
