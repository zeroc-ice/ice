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
#include <Ice/OutgoingAsyncF.h>
#include <Ice/InstanceF.h>
#include <Ice/ReferenceF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/Current.h>

namespace IceInternal
{

class BasicStream;
class LocalExceptionWrapper;
class Outgoing;
class RetryTask;

class ICE_API OutgoingAsyncMessageCallback : virtual public IceUtil::Shared
{
public:

    OutgoingAsyncMessageCallback();    
    virtual ~OutgoingAsyncMessageCallback();

    virtual void __sent(Ice::ConnectionI*) = 0;
    virtual void __finished(const Ice::LocalException&) = 0;

    virtual void ice_exception(const Ice::Exception&) = 0;

    BasicStream*
    __getOs()
    {
        return __os;
    }

    void __sentCallback(const InstancePtr&);
    void __exception(const Ice::Exception&);

protected:

    friend class ::IceInternal::RetryTask;

    void __acquireCallback(const Ice::ObjectPrx&);
    void __releaseCallback(const Ice::LocalException&);
    void __releaseCallback()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(__monitor);
        __releaseCallbackNoSync();
    }
    void __releaseCallbackNoSync();

    void __warning(const InstancePtr&, const std::exception&) const;
    void __warning(const InstancePtr&) const;

    void __warning(const std::exception&) const;
    void __warning() const;

    IceUtil::Monitor<IceUtil::Mutex> __monitor;
    BasicStream* __is;
    BasicStream* __os;
};

//
// We need virtual inheritance from shared, because the user might use
// multiple inheritance from IceUtil::Shared.
//
class ICE_API OutgoingAsync : public OutgoingAsyncMessageCallback, private IceUtil::TimerTask
{
public:

    void __sent(Ice::ConnectionI*);

    void __finished(BasicStream&);
    void __finished(const Ice::LocalException&);
    void __finished(const LocalExceptionWrapper&);

    void __retry(int);
    bool __send();

protected:

    void __prepare(const Ice::ObjectPrx&, const std::string&, Ice::OperationMode, const Ice::Context*);

    virtual void __response(bool) = 0;
    void __throwUserException();

private:

    void handleException(const Ice::LocalException&);
    void handleException(const LocalExceptionWrapper&);

    void runTimerTask(); // Implementation of TimerTask::runTimerTask()
    Ice::ConnectionIPtr _timerTaskConnection;

    bool _sent;
    bool _sentSynchronously;
    bool _response;
    ::Ice::ObjectPrx _proxy;
    Handle< ::IceDelegate::Ice::Object> _delegate;
    int _cnt;
    Ice::OperationMode _mode;
};

class ICE_API BatchOutgoingAsync : public OutgoingAsyncMessageCallback
{
public:

    virtual void __sent(Ice::ConnectionI*);
    virtual void __finished(const Ice::LocalException&);
    
protected:

    void __prepare(const InstancePtr&);
};

}

namespace Ice
{

class ICE_API AMISentCallback 
{
public:

    virtual ~AMISentCallback() { }

    virtual void ice_sent() = 0;
};

class ICE_API AMI_Object_ice_invoke : public IceInternal::OutgoingAsync
{
public:

    virtual void ice_response(bool, const std::vector<Ice::Byte>&) = 0;
    virtual void ice_exception(const Ice::Exception&) = 0;

    bool __invoke(const Ice::ObjectPrx&, const std::string& operation, OperationMode,
                  const std::vector<Ice::Byte>&, const Context*);

protected:

    virtual void __response(bool);
};

class ICE_API AMI_Array_Object_ice_invoke : public IceInternal::OutgoingAsync
{
public:

    virtual void ice_response(bool, const std::pair<const Byte*, const Byte*>&) = 0;
    virtual void ice_exception(const Ice::Exception&) = 0;

    bool __invoke(const Ice::ObjectPrx&, const std::string& operation, OperationMode,
                  const std::pair<const Byte*, const Byte*>&, const Context*);

protected:

    virtual void __response(bool);
};

class ICE_API AMI_Object_ice_flushBatchRequests : public IceInternal::BatchOutgoingAsync
{
public:

    bool __invoke(const Ice::ObjectPrx&);

    virtual void ice_exception(const Ice::Exception&) = 0;
};

}

#endif
