// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
#include <Ice/RequestHandlerF.h>
#include <Ice/InstanceF.h>
#include <Ice/ReferenceF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/Current.h>
#include <Ice/BasicStream.h>
#include <Ice/ObserverHelper.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/ThreadPoolF.h>

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

class ICE_API AsyncResult : virtual public Ice::LocalObject, protected IceUtil::TimerTask, private IceUtil::noncopyable
{
public:

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
    virtual void __invokeExceptionAsync(const Exception&);
    void __invokeCompleted();

    static void __check(const AsyncResultPtr&, const ::IceProxy::Ice::Object*, const ::std::string&);
    static void __check(const AsyncResultPtr&, const Connection*, const ::std::string&);
    static void __check(const AsyncResultPtr&, const Communicator*, const ::std::string&);

    virtual void __invokeException(const Exception&); // Required to be public for AsynchronousException
    void __invokeSent(); // Required to be public for AsynchronousSent

    void __attachRemoteObserver(const Ice::ConnectionInfoPtr& c, const Ice::EndpointPtr& endpt,
                                Ice::Int requestId, Ice::Int sz)
    {
        _childObserver.attach(_observer.getRemoteObserver(c, endpt, requestId, sz));
    }

    void __attachCollocatedObserver(const Ice::ObjectAdapterPtr& adapter, Ice::Int requestId)
    {
        _childObserver.attach(_observer.getCollocatedObserver(adapter,
                                                               requestId,
                                                               static_cast<Ice::Int>(_os.b.size() -
                                                                                     IceInternal::headerSize - 4)));
    }

protected:

    static void __check(const AsyncResultPtr&, const ::std::string&);

    AsyncResult(const CommunicatorPtr&, const IceInternal::InstancePtr&, const std::string&,
                const IceInternal::CallbackBasePtr&, const LocalObjectPtr&);

    void __invokeSentAsync();

    void runTimerTask(); // Implementation of TimerTask::runTimerTask()

    void __warning(const std::exception&) const;
    void __warning() const;

    virtual ~AsyncResult(); // Must be heap-allocated.

    const CommunicatorPtr _communicator;
    const IceInternal::InstancePtr _instance;
    const std::string& _operation;
    Ice::ConnectionPtr _cachedConnection;
    const IceInternal::CallbackBasePtr _callback;
    const LocalObjectPtr _cookie;

    IceUtil::Monitor<IceUtil::Mutex> _monitor;
    IceInternal::BasicStream _is;
    IceInternal::BasicStream _os;

    IceInternal::RequestHandlerPtr _timeoutRequestHandler;

    static const unsigned char OK;
    static const unsigned char Done;
    static const unsigned char Sent;
    static const unsigned char EndCalled;

    unsigned char _state;
    bool _sentSynchronously;
    IceUtil::UniquePtr<Exception> _exception;
    IceInternal::InvocationObserver _observer;
    IceInternal::ObserverHelperT<Ice::Instrumentation::ChildInvocationObserver> _childObserver;
};

}

namespace IceInternal
{

//
// See comments in OutgoingAsync.cpp
//
extern ICE_API CallbackBasePtr __dummyCallback;

class CollocatedRequestHandler;

//
// This interface is used by the connection to handle OutgoingAsync
// and BatchOutgoingAsync messages.
//
class ICE_API OutgoingAsyncMessageCallback : virtual public Ice::LocalObject
{
public:

    virtual ~OutgoingAsyncMessageCallback()
    {
    }

    //
    // Called by the request handler to send the request over the connection.
    //
    virtual IceInternal::AsyncStatus __send(const Ice::ConnectionIPtr&, bool, bool) = 0;

    //
    // Called by the collocated request handler to invoke the request.
    //
    virtual IceInternal::AsyncStatus __invokeCollocated(CollocatedRequestHandler*) = 0;

    //
    // Called by the connection when the message is confirmed sent. The connection is locked
    // when this is called so this method can't call the sent callback. Instead, this method
    // returns true if there's a sent callback and false otherwise. If true is returned, the
    // connection will call the __invokeSentCallback() method bellow (which in turn should
    // call the sent callback).
    //
    virtual bool __sent() = 0;

    //
    // Called by the connection to call the user sent callback.
    //
    virtual void __invokeSent() = 0;

    //
    // Called by the connection when the request failed.
    //
    virtual void __finished(const Ice::Exception&) = 0;

    //
    // Called by the retry queue to process retry.
    //
    virtual void __processRetry(bool destroyed) = 0;

    //
    // Helper to dispatch invocation timeout.
    //
    void __dispatchInvocationTimeout(const ThreadPoolPtr&, const Ice::ConnectionPtr&);
};

class ICE_API OutgoingAsync : public OutgoingAsyncMessageCallback, public Ice::AsyncResult
{
public:

    OutgoingAsync(const Ice::ObjectPrx&, const std::string&, const CallbackBasePtr&, const Ice::LocalObjectPtr&);

    void __prepare(const std::string&, Ice::OperationMode, const Ice::Context*);

    virtual Ice::ObjectPrx
    getProxy() const
    {
        return _proxy;
    }

    virtual IceInternal::AsyncStatus __send(const Ice::ConnectionIPtr&, bool, bool);
    virtual IceInternal::AsyncStatus __invokeCollocated(CollocatedRequestHandler*);
    virtual bool __sent();
    virtual void __invokeSent();
    virtual void __finished(const Ice::Exception&);
    virtual void __processRetry(bool);
    virtual void __invokeExceptionAsync(const Ice::Exception&);

    bool __finished();
    bool __invoke(bool);

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

    void handleException(const Ice::Exception&);


    Ice::EncodingVersion _encoding;

    RequestHandlerPtr _handler;
    int _cnt;
    bool _sent;
    Ice::OperationMode _mode;
};

class ICE_API BatchOutgoingAsync : public OutgoingAsyncMessageCallback, public Ice::AsyncResult
{
public:

    BatchOutgoingAsync(const Ice::CommunicatorPtr&, const InstancePtr&, const std::string&, const CallbackBasePtr&,
                       const Ice::LocalObjectPtr&);

    virtual IceInternal::AsyncStatus __send(const Ice::ConnectionIPtr&, bool, bool);
    virtual IceInternal::AsyncStatus __invokeCollocated(CollocatedRequestHandler*);
    virtual bool __sent();
    virtual void __invokeSent();
    virtual void __finished(const Ice::Exception&);
    virtual void __processRetry(bool);
};

class ICE_API ProxyBatchOutgoingAsync : public BatchOutgoingAsync
{
public:

    ProxyBatchOutgoingAsync(const Ice::ObjectPrx&, const std::string&, const CallbackBasePtr&,
                            const Ice::LocalObjectPtr&);

    void __invoke();

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

    void __invoke();

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

class ICE_API GetConnectionOutgoingAsync :  public OutgoingAsyncMessageCallback, public Ice::AsyncResult
{
public:

    GetConnectionOutgoingAsync(const Ice::ObjectPrx&, const std::string&, const CallbackBasePtr&,
                               const Ice::LocalObjectPtr&);

    void __invoke();

    virtual Ice::ObjectPrx
    getProxy() const
    {
        return _proxy;
    }

    virtual AsyncStatus __send(const Ice::ConnectionIPtr&, bool, bool);
    virtual AsyncStatus __invokeCollocated(CollocatedRequestHandler*);
    virtual bool __sent();
    virtual void __invokeSent();
    virtual void __finished(const Ice::Exception&);
    virtual void __processRetry(bool);

private:

    void handleException(const Ice::Exception&);

    Ice::ObjectPrx _proxy;
    RequestHandlerPtr _handler;
    int _cnt;
};

//
// Base class for all callbacks.
//
class ICE_API CallbackBase : public IceUtil::Shared
{
public:

    void checkCallback(bool, bool);

    virtual void completed(const ::Ice::AsyncResultPtr&) const = 0;
    virtual CallbackBasePtr verify(const ::Ice::LocalObjectPtr&) = 0;
    virtual void sent(const ::Ice::AsyncResultPtr&) const = 0;
    virtual bool hasSentCallback() const = 0;
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

#endif

}

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

#ifdef ICE_CPP11

ICE_API CallbackPtr
newCallback(const ::IceInternal::Function<void (const AsyncResultPtr&)>&,
            const ::IceInternal::Function<void (const AsyncResultPtr&)>& =
               ::IceInternal::Function<void (const AsyncResultPtr&)>());
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
