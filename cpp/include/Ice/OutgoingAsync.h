// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OUTGOING_ASYNC_H
#define ICE_OUTGOING_ASYNC_H

#include <IceUtil/Timer.h>
#include <Ice/OutgoingAsyncF.h>
#include <Ice/AsyncResult.h>
#include <Ice/CommunicatorF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/ObjectAdapterF.h>

#include <exception>

namespace IceInternal
{

class RetryException;
class CollocatedRequestHandler;

//
// Base class for handling asynchronous invocations. This class is
// responsible for the handling of the output stream and the child
// invocation observer.
//
class ICE_API OutgoingAsyncBase : public Ice::AsyncResult
{
public:

    virtual bool sent();
    virtual bool completed(const Ice::Exception&);
    virtual bool completed();

    // Those methods are public when called from an OutgoingAsyncBase reference.
    using Ice::AsyncResult::cancelable;
    using Ice::AsyncResult::invokeSent;
    using Ice::AsyncResult::invokeSentAsync;
    using Ice::AsyncResult::invokeCompleted;
    using Ice::AsyncResult::invokeCompletedAsync;

    void attachRemoteObserver(const Ice::ConnectionInfoPtr& c, const Ice::EndpointPtr& endpt, Ice::Int requestId)
    {
        const Ice::Int size = static_cast<Ice::Int>(_os.b.size() - headerSize - 4);
        _childObserver.attach(getObserver().getRemoteObserver(c, endpt, requestId, size));
    }

    void attachCollocatedObserver(const Ice::ObjectAdapterPtr& adapter, Ice::Int requestId)
    {
        const Ice::Int size = static_cast<Ice::Int>(_os.b.size() - headerSize - 4);
        _childObserver.attach(getObserver().getCollocatedObserver(adapter, requestId, size));
    }

    BasicStream* getOs()
    {
        return &_os;
    }

    virtual BasicStream* getIs();

protected:

#ifdef ICE_CPP11_MAPPING
    OutgoingAsyncBase(const Ice::CommunicatorPtr&, const InstancePtr&, const std::string&, const CallbackBasePtr&);
#else
    OutgoingAsyncBase(const Ice::CommunicatorPtr&, const InstancePtr&, const std::string&, const CallbackBasePtr&,
                      const Ice::LocalObjectPtr&);
#endif
    bool sent(bool);
    bool finished(const Ice::Exception&);

    ObserverHelperT<Ice::Instrumentation::ChildInvocationObserver> _childObserver;

    BasicStream _os;
};

//
// Base class for proxy based invocations. This class handles the
// retry for proxy invocations. It also ensures the child observer is
// correct notified of failures and make sure the retry task is
// correctly canceled when the invocation completes.
//
class ICE_API ProxyOutgoingAsyncBase : public OutgoingAsyncBase, public IceUtil::TimerTask
{
public:

    virtual AsyncStatus invokeRemote(const Ice::ConnectionIPtr&, bool, bool) = 0;
    virtual AsyncStatus invokeCollocated(CollocatedRequestHandler*) = 0;

    virtual Ice::ObjectPrxPtr getProxy() const;

    using OutgoingAsyncBase::sent;
    virtual bool completed(const Ice::Exception&);
    void retryException(const Ice::Exception&);
    virtual void cancelable(const CancellationHandlerPtr&);

    void retry();
    void abort(const Ice::Exception&);

protected:

#ifdef ICE_CPP11_MAPPING
    ProxyOutgoingAsyncBase(const Ice::ObjectPrxPtr&, const std::string&, const CallbackBasePtr&);
#else
    ProxyOutgoingAsyncBase(const Ice::ObjectPrxPtr&, const std::string&, const CallbackBasePtr&,
                           const Ice::LocalObjectPtr&);
#endif
    void invokeImpl(bool);

    bool sent(bool);
    bool finished(const Ice::Exception&);
    bool finished(bool);

    int handleException(const Ice::Exception&);
    virtual void runTimerTask();

    const Ice::ObjectPrxPtr _proxy;
    RequestHandlerPtr _handler;
    Ice::OperationMode _mode;

private:

    int _cnt;
    bool _sent;
};

//
// Class for handling Slice operation invocations
//
class ICE_API OutgoingAsync : public ProxyOutgoingAsyncBase
{
public:

#ifdef ICE_CPP11_MAPPING
    OutgoingAsync(const Ice::ObjectPrxPtr&, const std::string&, const CallbackBasePtr&);
#else
    OutgoingAsync(const Ice::ObjectPrxPtr&, const std::string&, const CallbackBasePtr&, const Ice::LocalObjectPtr&);
#endif
    void prepare(const std::string&, Ice::OperationMode, const Ice::Context&);

    virtual bool sent();

    virtual AsyncStatus invokeRemote(const Ice::ConnectionIPtr&, bool, bool);
    virtual AsyncStatus invokeCollocated(CollocatedRequestHandler*);

    using ProxyOutgoingAsyncBase::completed;
    virtual bool completed();

    void abort(const Ice::Exception&);

    void invoke();

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
    void writeParamEncaps(const ::Ice::Byte* encaps, ::Ice::Int size)
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

    virtual BasicStream* getIs()
    {
        return &_is;
    }

private:

    const Ice::EncodingVersion _encoding;
};

//
// Class for handling the proxy's begin_ice_flushBatchRequest request.
//
class ICE_API ProxyFlushBatchAsync : public ProxyOutgoingAsyncBase
{
public:

#ifdef ICE_CPP11_MAPPING
    ProxyFlushBatchAsync(const Ice::ObjectPrxPtr&, const std::string&, const CallbackBasePtr&);
#else
    ProxyFlushBatchAsync(const Ice::ObjectPrxPtr&, const std::string&, const CallbackBasePtr&, const Ice::LocalObjectPtr&);
#endif

    virtual AsyncStatus invokeRemote(const Ice::ConnectionIPtr&, bool, bool);
    virtual AsyncStatus invokeCollocated(CollocatedRequestHandler*);

    void invoke();

private:

    int _batchRequestNum;
};
typedef IceUtil::Handle<ProxyFlushBatchAsync> ProxyFlushBatchAsyncPtr;

//
// Class for handling the proxy's begin_ice_getConnection request.
//
class ICE_API ProxyGetConnection :  public ProxyOutgoingAsyncBase
{
public:

#ifdef ICE_CPP11_MAPPING
    ProxyGetConnection(const Ice::ObjectPrxPtr&, const std::string&, const CallbackBasePtr&);
#else
    ProxyGetConnection(const Ice::ObjectPrxPtr&, const std::string&, const CallbackBasePtr&, const Ice::LocalObjectPtr&);
#endif
    virtual AsyncStatus invokeRemote(const Ice::ConnectionIPtr&, bool, bool);
    virtual AsyncStatus invokeCollocated(CollocatedRequestHandler*);

    void invoke();
};
typedef IceUtil::Handle<ProxyGetConnection> ProxyGetConnectionPtr;

//
// Class for handling Ice::Connection::begin_flushBatchRequests
//
class ICE_API ConnectionFlushBatchAsync : public OutgoingAsyncBase
{
public:

#ifdef ICE_CPP11_MAPPING
    ConnectionFlushBatchAsync(const Ice::ConnectionIPtr&, const Ice::CommunicatorPtr&, const InstancePtr&,
                              const std::string&, const CallbackBasePtr&);
#else
    ConnectionFlushBatchAsync(const Ice::ConnectionIPtr&, const Ice::CommunicatorPtr&, const InstancePtr&,
                              const std::string&, const CallbackBasePtr&, const Ice::LocalObjectPtr&);
#endif
    virtual Ice::ConnectionPtr getConnection() const;

    void invoke();

private:

    const Ice::ConnectionIPtr _connection;
};
typedef IceUtil::Handle<ConnectionFlushBatchAsync> ConnectionFlushBatchAsyncPtr;

//
// Class for handling Ice::Communicator::begin_flushBatchRequests
//
class ICE_API CommunicatorFlushBatchAsync : public Ice::AsyncResult
{
public:

#ifdef ICE_CPP11_MAPPING
    CommunicatorFlushBatchAsync(const Ice::CommunicatorPtr&, const InstancePtr&, const std::string&,
                                const CallbackBasePtr&);
#else
    CommunicatorFlushBatchAsync(const Ice::CommunicatorPtr&, const InstancePtr&, const std::string&,
                                const CallbackBasePtr&, const Ice::LocalObjectPtr&);
#endif

    void flushConnection(const Ice::ConnectionIPtr&);
    void ready();

private:

    void check(bool);

    int _useCount;
};

}

//
// Base callback for C++11 mapping
//
#ifdef ICE_CPP11_MAPPING
namespace IceInternal
{

class ICE_API OnewayClosureCallback : public ::IceInternal::GenericCallbackBase
{
public:

    OnewayClosureCallback(const std::string&,
             const std::shared_ptr<Ice::ObjectPrx>&,
             std::function<void ()>,
             std::function<void (::std::exception_ptr)>,
             std::function<void (bool)>);

    virtual void
    sent(const ::Ice::AsyncResultPtr&) const;

    virtual bool
    hasSentCallback() const;

    virtual void
    completed(const ::Ice::AsyncResultPtr&) const;

    static std::function<void ()>
    invoke(const std::string&,
           const std::shared_ptr<Ice::ObjectPrx>&,
           Ice::OperationMode,
           Ice::FormatType,
           std::function<void (::IceInternal::BasicStream*)>,
           std::function<void ()>,
           std::function<void (::std::exception_ptr)>,
           std::function<void (bool)>,
           const Ice::Context&);

private:

    const std::string& __name;
    std::shared_ptr<Ice::ObjectPrx> __proxy;
    std::function<void ()> __response;
    std::function<void (::std::exception_ptr)> __exception;
    std::function<void (bool)> __sent;
};

class ICE_API TwowayClosureCallback : public ::IceInternal::GenericCallbackBase
{
public:

    TwowayClosureCallback(const std::string&,
             const std::shared_ptr<Ice::ObjectPrx>&,
             bool,
             std::function<void (::IceInternal::BasicStream*)>,
             std::function<void (const ::Ice::UserException&)>,
             std::function<void (::std::exception_ptr)>,
             std::function<void (bool)>);

    virtual void
    sent(const ::Ice::AsyncResultPtr&) const;

    virtual bool
    hasSentCallback() const;

    virtual void
    completed(const ::Ice::AsyncResultPtr&) const;

    static std::function<void ()>
    invoke(const std::string&,
           const std::shared_ptr<Ice::ObjectPrx>&,
           Ice::OperationMode,
           Ice::FormatType,
           std::function<void (::IceInternal::BasicStream*)>,
           bool,
           std::function<void (::IceInternal::BasicStream*)>,
           std::function<void (const ::Ice::UserException&)>,
           std::function<void (::std::exception_ptr)>,
           std::function<void (bool)>,
           const Ice::Context&);

private:

    const std::string& __name;
    std::shared_ptr<Ice::ObjectPrx> __proxy;
    bool __readEmptyParams;
    std::function<void (::IceInternal::BasicStream*)> __read;
    std::function<void (const ::Ice::UserException&)> __userException;
    std::function<void (::std::exception_ptr)> __exception;
    std::function<void (bool)> __sent;
};

}
#endif

#endif
