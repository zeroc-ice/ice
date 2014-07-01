// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

class ICE_API OutgoingMessageCallback : private IceUtil::noncopyable
{
public:

    virtual ~OutgoingMessageCallback() { }
 
    virtual bool send(const Ice::ConnectionIPtr&, bool, bool) = 0;
    virtual void invokeCollocated(CollocatedRequestHandler*) = 0;

    virtual void sent() = 0;
    virtual void finished(const Ice::Exception&, bool) = 0;
};

class ICE_API Outgoing : public OutgoingMessageCallback
{
public:

    Outgoing(IceProxy::Ice::Object*, const std::string&, Ice::OperationMode, const Ice::Context*);
    ~Outgoing();

    bool invoke(); // Returns true if ok, false if user exception.
    void abort(const Ice::LocalException&);

    virtual bool send(const Ice::ConnectionIPtr&, bool, bool);
    virtual void invokeCollocated(CollocatedRequestHandler*);
    virtual void sent();
    virtual void finished(const Ice::Exception&, bool);

    void finished(BasicStream&);

    // Inlined for speed optimization.
    BasicStream* os() { return &_os; }
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

    void attachRemoteObserver(const Ice::ConnectionInfoPtr& c, const Ice::EndpointPtr& endpt,
                              Ice::Int requestId, Ice::Int size)
    {
        _childObserver.attach(_observer.getRemoteObserver(c, endpt, requestId, size));
    }

    void attachCollocatedObserver(const Ice::ObjectAdapterPtr& adapter, Ice::Int requestId)
    {
        _childObserver.attach(_observer.getCollocatedObserver(adapter, 
                                                               requestId, 
                                                               static_cast<Ice::Int>(_os.b.size() - 
                                                                                     IceInternal::headerSize - 4)));
    }

private:

    //
    // Optimization. The request handler and the reference may not be
    // deleted while a stack-allocated Outgoing still holds it.
    //
    IceProxy::Ice::Object* _proxy;
    Ice::OperationMode _mode;
    RequestHandlerPtr _handler;
    IceUtil::UniquePtr<Ice::Exception> _exception;
    InvocationObserver _observer;
    ObserverHelperT<Ice::Instrumentation::ChildInvocationObserver> _childObserver;

    enum
    {
        StateUnsent,
        StateInProgress,
        StateOK,
        StateUserException,
        StateLocalException,
        StateFailed
    } _state;

    Ice::EncodingVersion _encoding;
    BasicStream _is;
    BasicStream _os;
    bool _sent;

    //
    // NOTE: we use an attribute for the monitor instead of inheriting
    // from the monitor template.  Otherwise, the template would be
    // exported from the DLL on Windows and could cause linker errors
    // because of multiple definition of IceUtil::Monitor<IceUtil::Mutex>, 
    // see bug 1541.
    //
    IceUtil::Monitor<IceUtil::Mutex> _monitor;
};

class BatchOutgoing : public OutgoingMessageCallback
{
public:

    BatchOutgoing(IceProxy::Ice::Object*, const std::string&);
    BatchOutgoing(Ice::ConnectionI*, Instance*, const std::string&);
    
    void invoke();

    virtual bool send(const Ice::ConnectionIPtr&, bool, bool);
    virtual void invokeCollocated(CollocatedRequestHandler*);
    virtual void sent();
    virtual void finished(const Ice::Exception&, bool);
    
    BasicStream* os() { return &_os; }

    void attachRemoteObserver(const Ice::ConnectionInfoPtr& connection, const Ice::EndpointPtr& endpt, Ice::Int sz)
    {
        _childObserver.attach(_observer.getRemoteObserver(connection, endpt, 0, sz));
    }

    void attachCollocatedObserver(const Ice::ObjectAdapterPtr& adapter, Ice::Int requestId)
    {
        _childObserver.attach(_observer.getCollocatedObserver(adapter, 
                                                               requestId, 
                                                               static_cast<Ice::Int>(_os.b.size() - 
                                                                                     IceInternal::headerSize - 4)));
    }

private:

    IceUtil::Monitor<IceUtil::Mutex> _monitor;
    IceProxy::Ice::Object* _proxy;
    Ice::ConnectionI* _connection;
    bool _sent;
    IceUtil::UniquePtr<Ice::Exception> _exception;

    BasicStream _os;

    InvocationObserver _observer;
    ObserverHelperT<Ice::Instrumentation::ChildInvocationObserver> _childObserver;
};

}

#endif
