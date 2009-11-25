// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
#include <Ice/OutgoingAsyncF.h>
#include <Ice/InstanceF.h>
#include <Ice/ReferenceF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/Current.h>
#include <Ice/BasicStream.h>

#include <memory>

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

    virtual Ice::CommunicatorPtr getCommunicator() const
    {
        return 0;
    }

    virtual Ice::ConnectionPtr getConnection() const
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

    bool sentSynchronously() const
    {
        return _sentSynchronously; // No lock needed, immutable once __send() is called
    }

    LocalObjectPtr getCookie() const
    {
        return _cookie;	// No lock needed, cookie is immutable
    }

    const std::string& getOperation()
    {
        return _operation;
    }

    //
    // The following methods are used by begin_ and end_ methods to start or complete
    // the asynchronous invocation.
    //
    IceInternal::BasicStream*
    __getOs()
    {
        return &_os;
    }

    IceInternal::BasicStream*
    __getIs()
    {
        return &_is;
    }

    bool __wait();
    void __throwUserException();
    void __exceptionAsync(const Ice::Exception&);

    static void __check(const AsyncResultPtr&, const ::IceProxy::Ice::Object*, const ::std::string&);
    static void __check(const AsyncResultPtr&, const ::Ice::Connection*, const ::std::string&);
    static void __check(const AsyncResultPtr&, const ::Ice::Communicator*, const ::std::string&);

    void __exception(const Ice::Exception&); // Required to be public for AsynchronousException
    void __sent(); // Required to be public for AsynchronousSent

protected:

    static void __check(const AsyncResultPtr&, const ::std::string&);

    AsyncResult(const IceInternal::InstancePtr&, const std::string&, const IceInternal::CallbackBasePtr&, 
                const LocalObjectPtr&);

    void __sentAsync();
    void __response();

    void __warning(const std::exception&) const;
    void __warning() const;

    virtual ~AsyncResult(); // Must be heap-allocated.

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
    std::auto_ptr<Exception> _exception;
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
    void __finished(BasicStream&);

    bool __send(bool);

protected:

    Ice::ObjectPrx _proxy;

private:

    int handleException(const Ice::LocalException&, bool);
    int handleException(const LocalExceptionWrapper&);

    void runTimerTask(); // Implementation of TimerTask::runTimerTask()
    Ice::ConnectionIPtr _timerTaskConnection;

    Handle< IceDelegate::Ice::Object> _delegate;
    int _cnt;
    Ice::OperationMode _mode;
};

class ICE_API BatchOutgoingAsync : public OutgoingAsyncMessageCallback, public Ice::AsyncResult
{
public:

    BatchOutgoingAsync(const InstancePtr&, const std::string&, const CallbackBasePtr&, const Ice::LocalObjectPtr&);

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

    virtual CallbackBasePtr __verify(::Ice::LocalObjectPtr& cookie)
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

//
// Generic callback template that can be used when only the exception
// and sent callbacks are needed (even for twoway calls).
//
template<class T>
class ExceptionCallbackNC : public GenericCallbackBase
{
public:

    typedef T callback_type;

    typedef IceUtil::Handle<T> TPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&);
    typedef void (T::*Sent)(bool);

    ExceptionCallbackNC(const TPtr& instance, Exception excb, Sent sentcb) :
        callback(instance), exception(excb), sent(sentcb)
    {
        checkCallback(instance, excb != 0);
    }

    virtual void __completed(const ::Ice::AsyncResultPtr& result) const
    {
        try
        {
            result->__wait();
        }
        catch(const ::Ice::Exception& ex)
        {
            (callback.get()->*exception)(ex);
        }
    }

    virtual CallbackBasePtr __verify(::Ice::LocalObjectPtr& __cookie)
    {
	if(__cookie) // Makes sure begin_ was called without a cookie
	{
	    throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "cookie specified for callback without cookie");
	}
        return this;
    }

    virtual void __sent(const ::Ice::AsyncResultPtr& result) const
    {
	if(sent)
	{
	    (callback.get()->*sent)(result->sentSynchronously());
	}
    }

    virtual bool __hasSentCallback() const
    {
        return sent != 0;
    }

    TPtr callback;
    Exception exception;
    Sent sent;
};

template<class T, class CT>
class ExceptionCallback : public GenericCallbackBase
{
public:

    typedef T callback_type;
    typedef CT cookie_type;

    typedef IceUtil::Handle<T> TPtr;
    typedef IceUtil::Handle<CT> CTPtr;

    typedef void (T::*Exception)(const ::Ice::Exception&, const CTPtr&);
    typedef void (T::*Sent)(bool, const CTPtr&);

    ExceptionCallback(const TPtr& instance, Exception excb, Sent sentcb) :
        callback(instance), exception(excb), sent(sentcb)
    {
        checkCallback(instance, excb != 0);
    }

    virtual void __completed(const ::Ice::AsyncResultPtr& result) const
    {
        try
        {
            result->__wait();
        }
        catch(const ::Ice::Exception& ex)
        {
            (callback.get()->*exception)(ex, CTPtr::dynamicCast(result->getCookie()));
        }
    }

    virtual CallbackBasePtr __verify(::Ice::LocalObjectPtr& __cookie)
    {
	if(__cookie && !CTPtr::dynamicCast(__cookie))
	{
	    throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "unexpected cookie type");
	}
        return this;
    }

    virtual void __sent(const ::Ice::AsyncResultPtr& result) const
    {
	if(sent)
	{
	    (callback.get()->*sent)(result->sentSynchronously(), CTPtr::dynamicCast(result->getCookie()));
	}
    }

    virtual bool __hasSentCallback() const
    {
        return sent != 0;
    }

    TPtr callback;
    Exception exception;
    Sent sent;
};

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
newCallback(const IceUtil::Handle<T>& instance, 
            void (T::*excb)(const ::Ice::Exception&), 
            void (T::*sentcb)(bool) = 0)
{
    return new ::IceInternal::ExceptionCallbackNC<T>(instance, excb, sentcb);
}

template<class T, class CT> CallbackPtr
newCallback(const IceUtil::Handle<T>& instance, 
            void (T::*excb)(const ::Ice::Exception&, const IceUtil::Handle<CT>&),
            void (T::*sentcb)(bool, const IceUtil::Handle<CT>&) = 0)
{
    return new ::IceInternal::ExceptionCallback<T, CT>(instance, excb, sentcb);
}

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

inline CallbackPtr
newAMICallback(const IceUtil::Handle< ::Ice::AMICallbackBase>& instance)
{
    if(dynamic_cast<AMISentCallback*>(instance.get()))
    {
        return new ::IceInternal::ExceptionCallbackNC<AMICallbackBase>(instance, 
                                                                       &AMICallbackBase::__exception,
                                                                       &AMICallbackBase::__sent);
    }
    else
    {
        return new ::IceInternal::ExceptionCallbackNC<AMICallbackBase>(instance, 
                                                                       &AMICallbackBase::__exception, 
                                                                       0);
    }
}

}

#endif
