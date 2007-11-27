// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

class ICE_API OutgoingAsyncMessageCallback : virtual public IceUtil::Shared
{
public:

    virtual ~OutgoingAsyncMessageCallback() { }

    virtual void __sent(Ice::ConnectionI*) = 0;
    virtual void __finished(const Ice::LocalException&) = 0;
};

//
// We need virtual inheritance from shared, because the user might use
// multiple inheritance from IceUtil::Shared.
//
class ICE_API OutgoingAsync : public OutgoingAsyncMessageCallback, public IceUtil::TimerTask
{
public:

    OutgoingAsync();
    virtual ~OutgoingAsync();

    void __sent(Ice::ConnectionI*);

    BasicStream*
    __getOs()
    {
        return __os;
    }

    virtual void ice_exception(const Ice::Exception&) = 0;

    void __finished(BasicStream&);
    void __finished(const Ice::LocalException&);
    void __finished(const LocalExceptionWrapper&);

protected:

    void __prepare(const Ice::ObjectPrx&, const std::string&, Ice::OperationMode, const Ice::Context*);
    void __send();

    virtual void __response(bool) = 0;

    BasicStream* __is;
    BasicStream* __os;

private:

    void runTimerTask(); // Implementation of TimerTask::runTimerTask()

    void warning(const std::exception&) const;
    void warning() const;

    void cleanup();

    bool _sent;
    bool _response;
    ::Ice::ObjectPrx _proxy;
    Handle< ::IceDelegate::Ice::Object> _delegate;
    int _cnt;
    Ice::OperationMode _mode;

    Ice::ConnectionIPtr _timerTaskConnection;
    IceUtil::Monitor<IceUtil::Mutex> _monitor;
};

class ICE_API BatchOutgoingAsync : public OutgoingAsyncMessageCallback
{
public:

    BatchOutgoingAsync();

    void __prepare(const InstancePtr&);
    virtual void __sent(Ice::ConnectionI*);
    virtual void __finished(const Ice::LocalException&);
    
    BasicStream*
    __getOs()
    {
        return _os;
    }

    virtual void ice_exception(const Ice::Exception&) = 0;

private:

    void warning(const std::exception&) const;
    void warning() const;

    void cleanup();

    IceUtil::Monitor<IceUtil::Mutex> _monitor;
    BasicStream* _os;
};

}

namespace Ice
{

class ICE_API AMI_Object_ice_invoke : public IceInternal::OutgoingAsync
{
public:

    virtual void ice_response(bool, const std::vector<Ice::Byte>&) = 0;
    virtual void ice_exception(const Ice::Exception&) = 0;

    void __invoke(const Ice::ObjectPrx&, const std::string& operation, OperationMode,
                  const std::vector<Ice::Byte>&, const Context*);

protected:

    virtual void __response(bool);
};

class ICE_API AMI_Array_Object_ice_invoke : public IceInternal::OutgoingAsync
{
public:

    virtual void ice_response(bool, const std::pair<const Byte*, const Byte*>&) = 0;
    virtual void ice_exception(const Ice::Exception&) = 0;

    void __invoke(const Ice::ObjectPrx&, const std::string& operation, OperationMode,
                  const std::pair<const Byte*, const Byte*>&, const Context*);

protected:

    virtual void __response(bool);
};

class ICE_API AMI_Object_ice_flushBatchRequests : public IceInternal::BatchOutgoingAsync
{
public:

    void __invoke(const Ice::ObjectPrx&);

    virtual void ice_exception(const Ice::Exception&) = 0;
};

}

#endif
