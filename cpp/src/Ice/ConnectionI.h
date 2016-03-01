// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_CONNECTION_I_H
#define ICE_CONNECTION_I_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Time.h>
#include <IceUtil/StopWatch.h>
#include <IceUtil/Timer.h>
#include <IceUtil/UniquePtr.h>

#include <Ice/CommunicatorF.h>
#include <Ice/Connection.h>
#include <Ice/ConnectionIF.h>
#include <Ice/ConnectionFactoryF.h>
#include <Ice/InstanceF.h>
#include <Ice/TransceiverF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/ServantManagerF.h>
#include <Ice/EndpointIF.h>
#include <Ice/ConnectorF.h>
#include <Ice/LoggerF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/OutgoingAsyncF.h>
#include <Ice/EventHandler.h>
#include <Ice/RequestHandler.h>
#include <Ice/ResponseHandler.h>
#include <Ice/Dispatcher.h>
#include <Ice/ObserverHelper.h>
#include <Ice/ConnectionAsync.h>
#include <Ice/BatchRequestQueueF.h>
#include <Ice/ACM.h>

#include <deque>

#if TARGET_OS_IPHONE == 0 && !defined(ICE_OS_WINRT)
#    ifndef ICE_HAS_BZIP2
#        define ICE_HAS_BZIP2
#    endif
#endif

namespace IceInternal
{

class Outgoing;
class OutgoingBase;

}

namespace Ice
{

class LocalException;
class ObjectAdapterI;
typedef IceUtil::Handle<ObjectAdapterI> ObjectAdapterIPtr;

class ConnectionI : public Connection,
                    public IceInternal::EventHandler,
                    public IceInternal::ResponseHandler,
                    public IceInternal::CancellationHandler,
                    public IceUtil::Monitor<IceUtil::Mutex>
{
    class Observer : public IceInternal::ObserverHelperT<Ice::Instrumentation::ConnectionObserver>
    {
    public:

        Observer();

        void startRead(const IceInternal::Buffer&);
        void finishRead(const IceInternal::Buffer&);
        void startWrite(const IceInternal::Buffer&);
        void finishWrite(const IceInternal::Buffer&);

        void attach(const Ice::Instrumentation::ConnectionObserverPtr&);

    private:

        Ice::Byte* _readStreamPos;
        Ice::Byte* _writeStreamPos;
    };

public:

    struct OutgoingMessage
    {
        OutgoingMessage(IceInternal::BasicStream* str, bool comp) :
            stream(str), out(0), compress(comp), requestId(0), adopted(false)
#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
            , isSent(false), invokeSent(false), receivedReply(false)
#endif
        {
        }

        OutgoingMessage(IceInternal::OutgoingBase* o, IceInternal::BasicStream* str, bool comp, int rid) :
            stream(str), out(o), compress(comp), requestId(rid), adopted(false)
#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
            , isSent(false), invokeSent(false), receivedReply(false)
#endif
        {
        }

        OutgoingMessage(const IceInternal::OutgoingAsyncBasePtr& o, IceInternal::BasicStream* str,
                        bool comp, int rid) :
            stream(str), out(0), outAsync(o), compress(comp), requestId(rid), adopted(false)
#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
            , isSent(false), invokeSent(false), receivedReply(false)
#endif
        {
        }

        void adopt(IceInternal::BasicStream*);
        void canceled(bool);
        bool sent();
        void completed(const Ice::LocalException&);

        IceInternal::BasicStream* stream;
        IceInternal::OutgoingBase* out;
        IceInternal::OutgoingAsyncBasePtr outAsync;
        bool compress;
        int requestId;
        bool adopted;
#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
        bool isSent;
        bool invokeSent;
        bool receivedReply;
#endif
    };

    class StartCallback : virtual public IceUtil::Shared
    {
    public:

        virtual void connectionStartCompleted(const ConnectionIPtr&) = 0;
        virtual void connectionStartFailed(const ConnectionIPtr&, const Ice::LocalException&) = 0;
    };
    typedef IceUtil::Handle<StartCallback> StartCallbackPtr;

    enum DestructionReason
    {
        ObjectAdapterDeactivated,
        CommunicatorDestroyed
    };

    void start(const StartCallbackPtr&);
    void activate();
    void hold();
    void destroy(DestructionReason);
    virtual void close(bool); // From Connection.

    bool isActiveOrHolding() const;
    bool isFinished() const;

    void throwException() const; // Throws the connection exception if destroyed.

    void waitUntilHolding() const;
    void waitUntilFinished(); // Not const, as this might close the connection upon timeout.

    void updateObserver();

    void monitor(const IceUtil::Time&, const IceInternal::ACMConfig&);

    bool sendRequest(IceInternal::OutgoingBase*, bool, bool, int);
    IceInternal::AsyncStatus sendAsyncRequest(const IceInternal::OutgoingAsyncBasePtr&, bool, bool, int);

    IceInternal::BatchRequestQueuePtr getBatchRequestQueue() const;

    virtual void flushBatchRequests(); // From Connection.

    virtual AsyncResultPtr begin_flushBatchRequests();
    virtual AsyncResultPtr begin_flushBatchRequests(const CallbackPtr&, const LocalObjectPtr& = 0);
    virtual AsyncResultPtr begin_flushBatchRequests(const Callback_Connection_flushBatchRequestsPtr&,
                                                    const LocalObjectPtr& = 0);

    virtual AsyncResultPtr begin_flushBatchRequests(
        const ::IceInternal::Function<void (const ::Ice::Exception&)>&,
        const ::IceInternal::Function<void (bool)>& = ::IceInternal::Function<void (bool)>());

    virtual void end_flushBatchRequests(const AsyncResultPtr&);

    virtual void setCallback(const ConnectionCallbackPtr&);
    virtual void setACM(const IceUtil::Optional<int>&,
                        const IceUtil::Optional<ACMClose>&,
                        const IceUtil::Optional<ACMHeartbeat>&);
    virtual ACM getACM();

    virtual void requestCanceled(IceInternal::OutgoingBase*, const LocalException&);
    virtual void asyncRequestCanceled(const IceInternal::OutgoingAsyncBasePtr&, const LocalException&);

    virtual void sendResponse(Int, IceInternal::BasicStream*, Byte, bool);
    virtual void sendNoResponse();
    virtual bool systemException(Int, const SystemException&, bool);
    virtual void invokeException(Ice::Int, const LocalException&, int, bool);

    IceInternal::EndpointIPtr endpoint() const;
    IceInternal::ConnectorPtr connector() const;

    virtual void setAdapter(const ObjectAdapterPtr&); // From Connection.
    virtual ObjectAdapterPtr getAdapter() const; // From Connection.
    virtual EndpointPtr getEndpoint() const; // From Connection.
    virtual ObjectPrx createProxy(const Identity& ident) const; // From Connection.

    //
    // Operations from EventHandler
    //
#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
    bool startAsync(IceInternal::SocketOperation);
    bool finishAsync(IceInternal::SocketOperation);
#endif

    virtual void message(IceInternal::ThreadPoolCurrent&);
    virtual void finished(IceInternal::ThreadPoolCurrent&, bool);
    virtual std::string toString() const; // From Connection and EvantHandler.
    virtual IceInternal::NativeInfoPtr getNativeInfo();

    void timedOut();

    virtual std::string type() const; // From Connection.
    virtual Ice::Int timeout() const; // From Connection.
    virtual ConnectionInfoPtr getInfo() const; // From Connection

    virtual void setBufferSize(Ice::Int rcvSize, Ice::Int sndSize); // From Connection

    void exception(const LocalException&);

    void dispatch(const StartCallbackPtr&, const std::vector<OutgoingMessage>&, Byte, Int, Int,
                  const IceInternal::ServantManagerPtr&, const ObjectAdapterPtr&,
                  const IceInternal::OutgoingAsyncBasePtr&,
                  const ConnectionCallbackPtr&, IceInternal::BasicStream&);
    void finish(bool);

    void closeCallback(const ConnectionCallbackPtr&);

private:

    enum State
    {
        StateNotInitialized,
        StateNotValidated,
        StateActive,
        StateHolding,
        StateClosing,
        StateClosingPending,
        StateClosed,
        StateFinished
    };

    ConnectionI(const Ice::CommunicatorPtr&, const IceInternal::InstancePtr&, const IceInternal::ACMMonitorPtr&,
                const IceInternal::TransceiverPtr&, const IceInternal::ConnectorPtr&,
                const IceInternal::EndpointIPtr&, const ObjectAdapterIPtr&);
    virtual ~ConnectionI();

    friend class IceInternal::IncomingConnectionFactory;
    friend class IceInternal::OutgoingConnectionFactory;

    void setState(State, const LocalException&);
    void setState(State);

    void initiateShutdown();
    void heartbeat();

    bool initialize(IceInternal::SocketOperation = IceInternal::SocketOperationNone);
    bool validate(IceInternal::SocketOperation = IceInternal::SocketOperationNone);
    IceInternal::SocketOperation sendNextMessage(std::vector<OutgoingMessage>&);
    IceInternal::AsyncStatus sendMessage(OutgoingMessage&);

#ifdef ICE_HAS_BZIP2
    void doCompress(IceInternal::BasicStream&, IceInternal::BasicStream&);
    void doUncompress(IceInternal::BasicStream&, IceInternal::BasicStream&);
#endif

    IceInternal::SocketOperation parseMessage(IceInternal::BasicStream&, Int&, Int&, Byte&,
                                              IceInternal::ServantManagerPtr&, ObjectAdapterPtr&,
                                              IceInternal::OutgoingAsyncBasePtr&, ConnectionCallbackPtr&, int&);

    void invokeAll(IceInternal::BasicStream&, Int, Int, Byte,
                   const IceInternal::ServantManagerPtr&, const ObjectAdapterPtr&);

    void scheduleTimeout(IceInternal::SocketOperation status);
    void unscheduleTimeout(IceInternal::SocketOperation status);

    Ice::ConnectionInfoPtr initConnectionInfo() const;
    Ice::Instrumentation::ConnectionState toConnectionState(State) const;

    IceInternal::SocketOperation read(IceInternal::Buffer&);
    IceInternal::SocketOperation write(IceInternal::Buffer&);

    void reap();

    AsyncResultPtr __begin_flushBatchRequests(const IceInternal::CallbackBasePtr&, const LocalObjectPtr&);

    Ice::CommunicatorPtr _communicator;
    const IceInternal::InstancePtr _instance;
    IceInternal::ACMMonitorPtr _monitor;
    const IceInternal::TransceiverPtr _transceiver;
    const std::string _desc;
    const std::string _type;
    const IceInternal::ConnectorPtr _connector;
    const IceInternal::EndpointIPtr _endpoint;

    mutable Ice::ConnectionInfoPtr _info;

    ObjectAdapterPtr _adapter;
    IceInternal::ServantManagerPtr _servantManager;

    const bool _dispatcher;
    const LoggerPtr _logger;
    const IceInternal::TraceLevelsPtr _traceLevels;
    const IceInternal::ThreadPoolPtr _threadPool;

    const IceUtil::TimerPtr _timer;
    const IceUtil::TimerTaskPtr _writeTimeout;
    bool _writeTimeoutScheduled;
    const IceUtil::TimerTaskPtr _readTimeout;
    bool _readTimeoutScheduled;

    StartCallbackPtr _startCallback;

    const bool _warn;
    const bool _warnUdp;

    IceUtil::Time _acmLastActivity;

    const int _compressionLevel;

    Int _nextRequestId;

    std::map<Int, IceInternal::OutgoingBase*> _requests;
    std::map<Int, IceInternal::OutgoingBase*>::iterator _requestsHint;

    std::map<Int, IceInternal::OutgoingAsyncBasePtr> _asyncRequests;
    std::map<Int, IceInternal::OutgoingAsyncBasePtr>::iterator _asyncRequestsHint;

    IceUtil::UniquePtr<LocalException> _exception;

    const size_t _messageSizeMax;
    IceInternal::BatchRequestQueuePtr _batchRequestQueue;

    std::deque<OutgoingMessage> _sendStreams;

    IceInternal::BasicStream _readStream;
    bool _readHeader;
    IceInternal::BasicStream _writeStream;

    Observer _observer;

    int _dispatchCount;

    State _state; // The current state.
    bool _shutdownInitiated;
    bool _initialized;
    bool _validated;

    Ice::ConnectionCallbackPtr _callback;
};

}

#endif
