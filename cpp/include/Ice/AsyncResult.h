// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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
#include <Ice/VirtualShared.h>

namespace IceInternal
{

class CallbackBase;
ICE_DEFINE_PTR(CallbackBasePtr, CallbackBase);

}

namespace Ice
{

class ICE_API AsyncResult : private IceUtil::noncopyable,
#ifdef ICE_CPP11_MAPPING
    public ::std::enable_shared_from_this<::Ice::AsyncResult>
#else
    public Ice::LocalObject
#endif
{
public:
    
#ifdef ICE_CPP11_MAPPING
    AsyncResult(const CommunicatorPtr&, const IceInternal::InstancePtr&, const std::string&,
                const IceInternal::CallbackBasePtr&);
#else
    AsyncResult(const CommunicatorPtr&, const IceInternal::InstancePtr&, const std::string&,
                const IceInternal::CallbackBasePtr&, const LocalObjectPtr&);
#endif
    virtual ~AsyncResult(); // Must be heap-allocated

    void cancel();

    Int getHash() const;

    CommunicatorPtr getCommunicator() const;
    virtual ConnectionPtr getConnection() const;
    virtual ObjectPrxPtr getProxy() const;

    bool isCompleted() const;
    void waitForCompleted();

    bool isSent() const;
    void waitForSent();

    void throwLocalException() const;

    bool sentSynchronously() const;

#ifndef ICE_CPP11_MAPPING
    LocalObjectPtr getCookie() const;
#endif

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
class ICE_API CallbackBase : public Ice::EnableSharedFromThis<CallbackBase>
{
public:

    void checkCallback(bool, bool);

    virtual void completed(const ::Ice::AsyncResultPtr&) const = 0;
#ifndef ICE_CPP11_MAPPING
    virtual CallbackBasePtr verify(const ::Ice::LocalObjectPtr&) = 0;
#endif
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

}

namespace Ice
{

#ifndef ICE_CPP11_MAPPING
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

#endif
}

//
// Operation callbacks are specified in Proxy.h
//

#endif
