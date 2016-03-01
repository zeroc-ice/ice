// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ASYNC_RESULT_H
#define ICE_ASYNC_RESULT_H

#include <IceUtil/Monitor.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/UniquePtr.h>
#include <Ice/LocalObject.h>
#include <Ice/CommunicatorF.h>
#include <Ice/ConnectionF.h>
#include <Ice/ProxyF.h>
#include <Ice/InstanceF.h>
#include <Ice/RequestHandlerF.h>
#include <Ice/AsyncResultF.h>
#include <Ice/ObserverHelper.h>
#include <Ice/BasicStream.h>

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

class ICE_API AsyncResult : public Ice::LocalObject, private IceUtil::noncopyable
{
public:

    void cancel();

    Int getHash() const;

    CommunicatorPtr getCommunicator() const;
    virtual ConnectionPtr getConnection() const;
    virtual ObjectPrx getProxy() const;

    bool isCompleted() const;
    void waitForCompleted();

    bool isSent() const;
    void waitForSent();

    void throwLocalException() const;

    bool sentSynchronously() const;
    LocalObjectPtr getCookie() const;
    const std::string& getOperation() const;

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
    void __throwUserException();

    bool __wait();

    static void __check(const AsyncResultPtr&, const ::IceProxy::Ice::Object*, const ::std::string&);
    static void __check(const AsyncResultPtr&, const Connection*, const ::std::string&);
    static void __check(const AsyncResultPtr&, const Communicator*, const ::std::string&);

protected:

    static void __check(const AsyncResultPtr&, const ::std::string&);

    AsyncResult(const CommunicatorPtr&, const IceInternal::InstancePtr&, const std::string&,
                const IceInternal::CallbackBasePtr&, const LocalObjectPtr&);
    virtual ~AsyncResult(); // Must be heap-allocated

    bool sent(bool);
    bool finished(bool);
    bool finished(const Exception&);

    void invokeSentAsync();
    void invokeCompletedAsync();

    void invokeSent();
    void invokeCompleted();

    void cancel(const LocalException&);
    virtual void cancelable(const IceInternal::CancellationHandlerPtr&);
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

    const IceInternal::InstancePtr _instance;
    IceInternal::InvocationObserver _observer;
    Ice::ConnectionPtr _cachedConnection;
    bool _sentSynchronously;

    IceInternal::BasicStream _is;

    IceUtil::Monitor<IceUtil::Mutex> _monitor;

private:

    const CommunicatorPtr _communicator;
    const std::string& _operation;
    const IceInternal::CallbackBasePtr _callback;
    const LocalObjectPtr _cookie;
    IceUtil::UniquePtr<Exception> _exception;

    IceInternal::CancellationHandlerPtr _cancellationHandler;
    IceUtil::UniquePtr<Ice::LocalException> _cancellationException;

    static const unsigned char OK;
    static const unsigned char Done;
    static const unsigned char Sent;
    static const unsigned char EndCalled;
    static const unsigned char Canceled;
    unsigned char _state;
};

}

namespace IceInternal
{

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

template<class Callable> struct callable_type<Callable, typename ::std::enable_if< 
                                                            ::std::is_class<Callable>::value &&
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

//
// COMPILERFIX: we have to use this function specialization to workaround an issue where 
// VS2012 and GCC 4.6 can't resolve the begin_xxx overloads if we just use std::function.
// We use some SNIFAE here to help the compiler with the overload resolution.
//
template<class S> class Function : public std::function<S>
{

public:

    template<typename T> Function(T f, typename ::std::enable_if<is_callable<T, S>::value>::type* = 0) 
        : std::function<S>(f)
    {
    }

    Function()
    {
    }

    Function(::std::nullptr_t) : ::std::function<S>(nullptr)
    {
    }
};

#else

template<class S> class Function
{
public:
    Function()
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
}

#endif
