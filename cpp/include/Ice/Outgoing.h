// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

    virtual void sent() = 0;
    virtual void completed(const Ice::Exception&) = 0;
    virtual void completed(BasicStream&) = 0;
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

    OutgoingBase(Instance*);

    BasicStream _os;
    IceUtil::UniquePtr<Ice::Exception> _exception;
    bool _sent;
    InvocationObserver _observer;
    ObserverHelperT<Ice::Instrumentation::ChildInvocationObserver> _childObserver;

    IceUtil::Monitor<IceUtil::Mutex> _monitor;
};

class ICE_API ProxyOutgoingBase : public OutgoingBase
{
public:

    ProxyOutgoingBase(IceProxy::Ice::Object*, Ice::OperationMode);
    ~ProxyOutgoingBase();

    virtual bool invokeRemote(const Ice::ConnectionIPtr&, bool, bool) = 0;
    virtual void invokeCollocated(CollocatedRequestHandler*) = 0;

    virtual void sent();
    virtual void completed(const Ice::Exception&);
    virtual void completed(BasicStream&);
    virtual void retryException(const Ice::Exception&);

protected:

    bool invokeImpl(); // Returns true if ok, false if user exception.

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
};

class ICE_API Outgoing : public ProxyOutgoingBase
{
public:

    Outgoing(IceProxy::Ice::Object*, const std::string&, Ice::OperationMode, const Ice::Context*);
    ~Outgoing();

    virtual bool invokeRemote(const Ice::ConnectionIPtr&, bool, bool);
    virtual void invokeCollocated(CollocatedRequestHandler*);

    bool invoke(); // Returns true if ok, false if user exception.
    void abort(const Ice::LocalException&);

    virtual void completed(BasicStream&);

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

    Ice::EncodingVersion _encoding;
    BasicStream _is;
    const std::string& _operation;
};

class ProxyFlushBatch : public ProxyOutgoingBase
{
public:

    ProxyFlushBatch(IceProxy::Ice::Object*, const std::string&);

    virtual bool invokeRemote(const Ice::ConnectionIPtr&, bool, bool);
    virtual void invokeCollocated(CollocatedRequestHandler*);

    void invoke();

private:

    int _batchRequestNum;
};

class ConnectionFlushBatch : public OutgoingBase
{
public:

    ConnectionFlushBatch(Ice::ConnectionI*, Instance*, const std::string&);

    void invoke();

    virtual void sent();
    virtual void completed(const Ice::Exception&);
    virtual void completed(BasicStream&);
    virtual void retryException(const Ice::Exception&);

private:

    Ice::ConnectionI* _connection;
};

}

#endif
