// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OUTGOING_H
#define ICE_OUTGOING_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/UniquePtr.h>

#include <Ice/RequestHandlerF.h>
#include <Ice/InstanceF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/ReferenceF.h>
#include <Ice/BasicStream.h>
#include <Ice/Current.h>
#include <Ice/ObserverHelper.h>
#include <Ice/ObjectAdapterF.h>

namespace Ice
{

class LocalException;

}

namespace IceInternal
{

class CollocatedRequestHandler; // Forward declaration

class ICE_API OutgoingBase : private IceUtil::noncopyable
{
public:

    virtual ~OutgoingBase() { }
 
    virtual bool send(const Ice::ConnectionIPtr&, bool, bool) = 0;
    virtual void invokeCollocated(CollocatedRequestHandler*) = 0;

    virtual void sent() = 0;
    virtual void completed(const Ice::Exception&) = 0;
    virtual void retryException(const Ice::Exception&) = 0;

    BasicStream* os() { return &_os; }

    void attachRemoteObserver(const Ice::ConnectionInfoPtr& c, const Ice::EndpointPtr& endpt, Ice::Int requestId)
    {
        const Ice::Int size = static_cast<Ice::Int>(_os.b.size() - IceInternal::headerSize - 4);
        _childObserver.attach(_observer.getRemoteObserver(c, endpt, requestId, size));
    }

    void attachCollocatedObserver(const Ice::ObjectAdapterPtr& adapter, Ice::Int requestId)
    {
        const Ice::Int size = static_cast<Ice::Int>(_os.b.size() - IceInternal::headerSize - 4);
        _childObserver.attach(_observer.getCollocatedObserver(adapter, requestId, size));
    }

protected:

    OutgoingBase(Instance*, const std::string&);

    BasicStream _os;
    IceUtil::UniquePtr<Ice::Exception> _exception;
    bool _sent;
    InvocationObserver _observer;
    ObserverHelperT<Ice::Instrumentation::ChildInvocationObserver> _childObserver;

    IceUtil::Monitor<IceUtil::Mutex> _monitor;
};

class ICE_API Outgoing : public OutgoingBase
{
public:

    Outgoing(IceProxy::Ice::Object*, const std::string&, Ice::OperationMode, const Ice::Context*);
    ~Outgoing();

    virtual bool send(const Ice::ConnectionIPtr&, bool, bool);
    virtual void invokeCollocated(CollocatedRequestHandler*);

    virtual void sent();
    virtual void completed(const Ice::Exception&);
    virtual void retryException(const Ice::Exception&);

    bool invoke(); // Returns true if ok, false if user exception.
    void abort(const Ice::LocalException&);
    void completed(BasicStream&);

    // Inlined for speed optimization.
    BasicStream* startReadParams()
    {
        _is.startReadEncaps();
        return &_is;
    }
    void endReadParams()
    {
        _is.endReadEncaps();
    }
    void readEmptyParams()
    {
        _is.skipEmptyEncaps();
    }
    void readParamEncaps(const Ice::Byte*& encaps, Ice::Int& sz)
    {
        _is.readEncaps(encaps, sz);
    }

    BasicStream* startWriteParams(Ice::FormatType format)
    {
        _os.startWriteEncaps(_encoding, format);
        return &_os;
    }
    void endWriteParams()
    {
        _os.endWriteEncaps();
    }
    void writeEmptyParams()
    {
        _os.writeEmptyEncaps(_encoding);
    }
    void writeParamEncaps(const Ice::Byte* encaps, Ice::Int size)
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

    bool hasResponse() 
    {
        return !_is.b.empty();
    }

    void throwUserException();

private:

    //
    // Optimization. The request handler and the reference may not be
    // deleted while a stack-allocated Outgoing still holds it.
    //
    IceProxy::Ice::Object* _proxy;
    Ice::OperationMode _mode;
    RequestHandlerPtr _handler;
    IceUtil::Time _invocationTimeoutDeadline;

    enum
    {
        StateUnsent,
        StateInProgress,
        StateRetry,
        StateOK,
        StateUserException,
        StateLocalException,
        StateFailed
    } _state;

    Ice::EncodingVersion _encoding;
    BasicStream _is;
};

class FlushBatch : public OutgoingBase
{
public:

    FlushBatch(IceProxy::Ice::Object*, const std::string&);
    FlushBatch(Ice::ConnectionI*, Instance*, const std::string&);
    
    void invoke();

    virtual bool send(const Ice::ConnectionIPtr&, bool, bool);
    virtual void invokeCollocated(CollocatedRequestHandler*);

    virtual void sent();
    virtual void completed(const Ice::Exception&);
    virtual void retryException(const Ice::Exception&);

private:

    IceProxy::Ice::Object* _proxy;
    Ice::ConnectionI* _connection;
};

}

#endif
