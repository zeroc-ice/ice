// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OUTGOING_ASYNC_H
#define ICE_OUTGOING_ASYNC_H

#include <IceUtil/Monitor.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Timer.h>
#include <IceUtil/Exception.h>
#include <IceUtil/UniquePtr.h>
#include <Ice/OutgoingAsyncF.h>
#include <Ice/InstanceF.h>
#include <Ice/ReferenceF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/Current.h>
#include <Ice/BasicStream.h>
#include <Ice/ObserverHelper.h>

#ifdef ICE_CPP11
#   include <functional> // for std::function
#endif

namespace IceInternal
{

class CallbackBase;
typedef IceUtil::Handle<CallbackBase> CallbackBasePtr;

}

namespace Ice
{

class ICE_API AsyncResult : virtual public IceUtil::Shared, private IceUtil::noncopyable
{
public:

    virtual bool operator==(const AsyncResult&) const;
    virtual bool operator<(const AsyncResult&) const;

    virtual Int getHash() const;

    virtual CommunicatorPtr getCommunicator() const
    {
        return _communicator;
    }

    virtual ConnectionPtr getConnection() const
    {
        return 0;
    }

    virtual ObjectPrx getProxy() const
    {
        return 0;
    }

    bool isCompleted() const;
    void waitForCompleted();

    bool isSent() const;
    void waitForSent();

    void throwLocalException() const;

    bool sentSynchronously() const
    {
        return _sentSynchronously; // No lock needed, immutable once __send() is called
    }

    LocalObjectPtr getCookie() const
    {
        return _cookie; // No lock needed, cookie is immutable
    }

    const std::string& getOperation() const
    {
        return _operation;
    }

    ::IceInternal::BasicStream*
    __getOs()
    {
        return &_os;
    }

    ::IceInternal::BasicStream* __startReadParams()
    {
        _is.startReadEncaps();
        return &_is;
    }
    void __endReadParams()
    {
        _is.endReadEncaps();
    }
    void __readEmptyParams()
    {
        _is.skipEmptyEncaps();
    }
    void __readParamEncaps(const ::Ice::Byte*& encaps, ::Ice::Int& sz)
    {
        _is.readEncaps(encaps, sz);
    }

    bool __wait();
    void __throwUserException();
    void __exceptionAsync(const Exception&);

    static void __check(const AsyncResultPtr&, const ::IceProxy::Ice::Object*, const ::std::string&);
    static void __check(const AsyncResultPtr&, const Connection*, const ::std::string&);
    static void __check(const AsyncResultPtr&, const Communicator*, const ::std::string&);

    virtual void __exception(const Exception&); // Required to be public for AsynchronousException
    void __sent(); // Required to be public for AsynchronousSent

    virtual void __attachRemoteObserver(const Ice::ConnectionInfoPtr& c, const Ice::EndpointPtr& endpt, 
                                        Ice::Int requestId, Ice::Int sz)
    {
        _remoteObserver.attach(_observer.getRemoteObserver(c, endpt, requestId, sz));
    }

    IceInternal::InvocationObserver& __getObserver()
    {
        return _observer;
    }

protected:

    static void __check(const AsyncResultPtr&, const ::std::string&);

    AsyncResult(const CommunicatorPtr&, const IceInternal::InstancePtr&, const std::string&,
                const IceInternal::CallbackBasePtr&, const LocalObjectPtr&);

    void __sentAsync();
    void __response();

    void __warning(const std::exception&) const;
    void __warning() const;

    virtual ~AsyncResult(); // Must be heap-allocated.

    const CommunicatorPtr _communicator;
    const IceInternal::InstancePtr _instance;
    const std::string& _operation;
    const IceInternal::CallbackBasePtr _callback;
    const LocalObjectPtr _cookie;

    IceUtil::Monitor<IceUtil::Mutex> _monitor;
    IceInternal::BasicStream _is;
    IceInternal::BasicStream _os;

    static const unsigned char OK;
    static const unsigned char Done;
    static const unsigned char Sent;
    static const unsigned char EndCalled;

    unsigned char _state;
    bool _sentSynchronously;
    IceUtil::UniquePtr<Exception> _exception;
    IceInternal::InvocationObserver _observer;
    IceInternal::ObserverHelperT<Ice::Instrumentation::RemoteObserver> _remoteObserver;
};

}

namespace IceInternal
{

//
// See comments in OutgoingAsync.cpp
//
extern ICE_API CallbackBasePtr __dummyCallback;

class LocalExceptionWrapper;

//
// This interface is used by the connection to handle OutgoingAsync
// and BatchOutgoingAsync messages.
//
class ICE_API OutgoingAsyncMessageCallback : virtual public IceUtil::Shared
{
public:

    virtual ~OutgoingAsyncMessageCallback()
    {
    }

    //
    // Called by the connection when the message is confirmed sent. The connection is locked
    // when this is called so this method can call the sent callback. Instead, this method
    // returns true if there's a sent callback and false otherwise. If true is returned, the
    // connection will call the __sent() method bellow (which in turn should call the sent
    // callback).
    //
    virtual bool __sent(Ice::ConnectionI*) = 0;

    //
    // Called by the connection to call the user sent callback.
    //
    virtual void __sent() = 0;

    //
    // Called by the connection when the request failed. The boolean indicates whether or
    // not the message was possibly sent (this is useful for retry to figure out whether
    // or not the request can't be retried without breaking at-most-once semantics.)
    //
    virtual void __finished(const Ice::LocalException&, bool) = 0;
};

class ICE_API OutgoingAsync : public OutgoingAsyncMessageCallback, public Ice::AsyncResult, private IceUtil::TimerTask
{
public:

    OutgoingAsync(const Ice::ObjectPrx&, const std::string&, const CallbackBasePtr&, const Ice::LocalObjectPtr&);

    void __prepare(const std::string&, Ice::OperationMode, const Ice::Context*);

    virtual Ice::ObjectPrx
    getProxy() const
    {
        return _proxy;
    }

    virtual bool __sent(Ice::ConnectionI*);
    virtual void __sent();
    virtual void __finished(const Ice::LocalException&, bool);

    void __finished(const LocalExceptionWrapper&);
    void __finished();

    bool __send(bool);

    BasicStream* __startWriteParams(Ice::FormatType format)
    {
        _os.startWriteEncaps(_encoding, format);
        return &_os;
    }
    void __endWriteParams()
    {
        _os.endWriteEncaps();
    }
    void __writeEmptyParams()
    {
        _os.writeEmptyEncaps(_encoding);
    }
    void __writeParamEncaps(const ::Ice::Byte* encaps, ::Ice::Int size)
    {
        if(size == 0)
        {
            _os.writeEmptyEncaps(_encoding);
        }
        else
        {
            _os.writeEncaps(encaps, size);
        }
    }

    ::IceInternal::BasicStream*
    __getIs()
    {
        return &_is;
    }

protected:

    Ice::ObjectPrx _proxy;

private:

    int handleException(const Ice::LocalException&, bool);
    int handleException(const LocalExceptionWrapper&);

    void runTimerTask(); // Implementation of TimerTask::runTimerTask()
    Ice::ConnectionIPtr _timerTaskConnection;

    Handle< IceDelegate::Ice::Object> _delegate;
    Ice::EncodingVersion _encoding;
    int _cnt;
    Ice::OperationMode _mode;
};

class ICE_API BatchOutgoingAsync : public OutgoingAsyncMessageCallback, public Ice::AsyncResult
{
public:

    BatchOutgoingAsync(const Ice::CommunicatorPtr&, const InstancePtr&, const std::string&, const CallbackBasePtr&,
                       const Ice::LocalObjectPtr&);

    virtual bool __sent(Ice::ConnectionI*);
    virtual void __sent();
    virtual void __finished(const Ice::LocalException&, bool);
};

class ICE_API ProxyBatchOutgoingAsync : public BatchOutgoingAsync
{
public:

    ProxyBatchOutgoingAsync(const Ice::ObjectPrx&, const std::string&, const CallbackBasePtr&,
                            const Ice::LocalObjectPtr&);

    void __send();

    virtual Ice::ObjectPrx
    getProxy() const
    {
        return _proxy;
    }

private:
    
    Ice::ObjectPrx _proxy;
};

class ICE_API ConnectionBatchOutgoingAsync : public BatchOutgoingAsync
{
public:

    ConnectionBatchOutgoingAsync(const Ice::ConnectionIPtr&, const Ice::CommunicatorPtr&, const InstancePtr&,
                                 const std::string&, const CallbackBasePtr&, const Ice::LocalObjectPtr&);

    void __send();

    virtual Ice::ConnectionPtr getConnection() const;

private:

    const Ice::ConnectionIPtr _connection;
};

class ICE_API CommunicatorBatchOutgoingAsync : public Ice::AsyncResult
{
public:

    CommunicatorBatchOutgoingAsync(const Ice::CommunicatorPtr&, const InstancePtr&, const std::string&,
                                   const CallbackBasePtr&, const Ice::LocalObjectPtr&);

    void flushConnection(const Ice::ConnectionIPtr&);
    void ready();

private:

    void check(bool);

    int _useCount;
};

//
// Base class for all callbacks.
//
class ICE_API CallbackBase : public IceUtil::Shared
{
public:

    void checkCallback(bool obj, bool cb)
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

    virtual void __completed(const ::Ice::AsyncResultPtr&) const = 0;
    virtual CallbackBasePtr __verify(::Ice::LocalObjectPtr&) = 0;
    virtual void __sent(const ::Ice::AsyncResultPtr&) const = 0;
    virtual bool __hasSentCallback() const = 0;
};

//
// Base class for generic callbacks.
//
class ICE_API GenericCallbackBase : virtual public CallbackBase
{
};

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
        callback(instance), completed(cb), sent(sentcb)
    {
        checkCallback(instance, cb != 0);
    }

    virtual void __completed(const ::Ice::AsyncResultPtr& result) const
    {
        (callback.get()->*completed)(result);
    }

    virtual CallbackBasePtr __verify(::Ice::LocalObjectPtr&)
    {
        return this; // Nothing to do, the cookie is not type-safe.
    }

    virtual void __sent(const ::Ice::AsyncResultPtr& result) const
    {
        if(sent)
        {
            (callback.get()->*sent)(result);
        }
    }

    virtual bool __hasSentCallback() const
    {
        return sent != 0;
    }

    TPtr callback;
    Callback completed;
    Callback sent;
};

#ifdef ICE_CPP11

template<typename T> struct callback_type
{
    static const int value = 1;
};

template<> struct callback_type<void(const ::Ice::AsyncResultPtr&)>
{
    static const int value = 2;
};

template<> struct callback_type<void(const ::Ice::Exception&)>
{
    static const int value = 3;
};

template<typename Callable, typename = void> struct callable_type
{
    static const int value = 1;
};

template<class Callable> struct callable_type<Callable, typename ::std::enable_if< ::std::is_class<Callable>::value && 
                                                                                   !::std::is_bind_expression<Callable>::value>::type>
{
    template<typename T, T> struct TypeCheck;
    template<typename T> struct AsyncResultCallback
    {
        typedef void (T::*ok)(const ::Ice::AsyncResultPtr&) const;
    };
    template<typename T> struct ExceptionCallback
    {
        typedef void (T::*ok)(const ::Ice::Exception&) const;
    };

    typedef char (&other)[1];
    typedef char (&asyncResult)[2];
    typedef char (&exception)[3];

    template<typename T> static other check(...);
    template<typename T> static asyncResult check(TypeCheck<typename AsyncResultCallback<T>::ok, &T::operator()>*);
    template<typename T> static exception check(TypeCheck<typename ExceptionCallback<T>::ok, &T::operator()>*);

    enum { value = sizeof(check<Callable>(0)) };
};

template<> struct callable_type<void(*)(const ::Ice::AsyncResultPtr&)> 
{
    static const int value = 2;
};

template<> struct callable_type<void(*)(const ::Ice::Exception&)>
{
    static const int value = 3;
};

template<typename Callable, typename Callback> struct is_callable
{
    static const bool value = callable_type<Callable>::value == callback_type<Callback>::value;
};

template<class S> class Function : public std::function<S>
{

public:

    template<typename T> Function(T f, typename ::std::enable_if<is_callable<T, S>::value>::type* = 0) : std::function<S>(f)
    {
    }
    
    Function()
    {
    }
    
    Function(::std::nullptr_t) : ::std::function<S>(nullptr)
    {
    }
};

class Cpp11AsyncCallback : public GenericCallbackBase
{
public:

    Cpp11AsyncCallback(const ::std::function<void (const ::Ice::AsyncResultPtr&)>& completed,
                       const ::std::function<void (const ::Ice::AsyncResultPtr&)>& sent) :
        _completed(completed), 
        _sent(sent)
    {
        checkCallback(true, completed != nullptr);
    }

    virtual void __completed(const ::Ice::AsyncResultPtr& result) const
    {
        _completed(result);
    }

    virtual CallbackBasePtr __verify(::Ice::LocalObjectPtr&)
    {
        return this; // Nothing to do, the cookie is not type-safe.
    }

    virtual void __sent(const ::Ice::AsyncResultPtr& result) const
    {
        if(_sent != nullptr)
        {
            _sent(result);
        }
    }
    
    virtual bool __hasSentCallback() const
    {
        return _sent != nullptr;
    }
    
    ::std::function< void (const ::Ice::AsyncResultPtr&)> _completed;
    ::std::function< void (const ::Ice::AsyncResultPtr&)> _sent;
};
#endif

}

namespace Ice
{

typedef IceUtil::Handle< ::IceInternal::GenericCallbackBase> CallbackPtr;

template<class T> CallbackPtr
newCallback(const IceUtil::Handle<T>& instance,
            void (T::*cb)(const ::Ice::AsyncResultPtr&),
            void (T::*sentcb)(const ::Ice::AsyncResultPtr&) = 0)
{
    return new ::IceInternal::AsyncCallback<T>(instance, cb, sentcb);
}

template<class T> CallbackPtr
newCallback(T* instance,
            void (T::*cb)(const ::Ice::AsyncResultPtr&),
            void (T::*sentcb)(const ::Ice::AsyncResultPtr&) = 0)
{
    return new ::IceInternal::AsyncCallback<T>(instance, cb, sentcb);
}

#ifdef ICE_CPP11
inline CallbackPtr
newCallback(const ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>& completed,
            const ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>& sent = 
                  ::IceInternal::Function<void (const ::Ice::AsyncResultPtr&)>())
{
    return new ::IceInternal::Cpp11AsyncCallback(completed, sent);
}
#endif

//
// Operation callbacks are specified in Proxy.h
//

//
// Interfaces for the deprecated AMI mapping.
//

class ICE_API AMISentCallback 
{
public:

    virtual ~AMISentCallback() { }

    virtual void ice_sent() = 0;
};

class ICE_API AMICallbackBase : virtual public IceUtil::Shared
{
public:

    virtual void ice_exception(const Exception&) = 0;

    void __exception(const Exception&);
    void __sent(bool);
};

}

#endif
