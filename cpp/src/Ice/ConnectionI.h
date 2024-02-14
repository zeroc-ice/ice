//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_CONNECTION_I_H
#define ICE_CONNECTION_I_H

#include <IceUtil/Time.h>
#include <IceUtil/StopWatch.h>
#include <IceUtil/Timer.h>

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
#include <Ice/OutgoingAsync.h>
#include <Ice/EventHandler.h>
#include <Ice/RequestHandler.h>
#include <Ice/ResponseHandler.h>
#include <Ice/Dispatcher.h>
#include <Ice/ObserverHelper.h>
#include <Ice/BatchRequestQueueF.h>
#include <Ice/ACM.h>
#include <Ice/OutputStream.h>
#include <Ice/InputStream.h>

#include <condition_variable>
#include <deque>
#include <mutex>

#ifndef ICE_HAS_BZIP2
#   define ICE_HAS_BZIP2
#endif

namespace IceInternal
{

template<typename T> class ThreadPoolMessage;

}

namespace Ice
{

class LocalException;
class ObjectAdapterI;

class ConnectionI : public Connection,
                    public IceInternal::EventHandler,
                    public IceInternal::ResponseHandler,
                    public IceInternal::CancellationHandler
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

    std::shared_ptr<ConnectionI> shared_from_this()
    {
        return std::dynamic_pointer_cast<ConnectionI>(VirtualEnableSharedFromThisBase::shared_from_this());
    }

    struct OutgoingMessage
    {
        OutgoingMessage(Ice::OutputStream* str, bool comp) :
            stream(str), compress(comp), requestId(0), adopted(false)
#if defined(ICE_USE_IOCP)
            , isSent(false), invokeSent(false), receivedReply(false)
#endif
        {
        }

        OutgoingMessage(const IceInternal::OutgoingAsyncBasePtr& o, Ice::OutputStream* str,
                        bool comp, int rid) :
            stream(str), outAsync(o), compress(comp), requestId(rid), adopted(false)
#if defined(ICE_USE_IOCP)
            , isSent(false), invokeSent(false), receivedReply(false)
#endif
        {
        }

        void adopt(Ice::OutputStream*);
        void canceled(bool);
        bool sent();
        void completed(std::exception_ptr);

        Ice::OutputStream* stream;
        IceInternal::OutgoingAsyncBasePtr outAsync;
        bool compress;
        int requestId;
        bool adopted;
#if defined(ICE_USE_IOCP)
        bool isSent;
        bool invokeSent;
        bool receivedReply;
#endif
    };

    enum DestructionReason
    {
        ObjectAdapterDeactivated,
        CommunicatorDestroyed
    };

    void startAsync(
        std::function<void(ConnectionIPtr)>,
        std::function<void(Ice::ConnectionIPtr, std::exception_ptr)>);
    void activate();
    void hold();
    void destroy(DestructionReason);
    virtual void close(ConnectionClose) noexcept; // From Connection.

    bool isActiveOrHolding() const;
    bool isFinished() const;

    virtual void throwException() const; // From Connection. Throws the connection exception if destroyed.

    void waitUntilHolding() const;
    void waitUntilFinished(); // Not const, as this might close the connection upon timeout.

    void updateObserver();

    void monitor(const IceUtil::Time&, const IceInternal::ACMConfig&);

    IceInternal::AsyncStatus sendAsyncRequest(const IceInternal::OutgoingAsyncBasePtr&, bool, bool, int);

    IceInternal::BatchRequestQueuePtr getBatchRequestQueue() const;

    virtual std::function<void()>
    flushBatchRequestsAsync(CompressBatch,
                            ::std::function<void(::std::exception_ptr)>,
                            ::std::function<void(bool)> = nullptr);

    virtual void setCloseCallback(CloseCallback);
    virtual void setHeartbeatCallback(HeartbeatCallback);

    virtual void heartbeat();

    virtual std::function<void()>
    heartbeatAsync(::std::function<void(::std::exception_ptr)>, ::std::function<void(bool)> = nullptr);

    virtual void setACM(const std::optional<int>&,
                        const std::optional<ACMClose>&,
                        const std::optional<ACMHeartbeat>&);
    virtual ACM getACM() noexcept;

    virtual void asyncRequestCanceled(const IceInternal::OutgoingAsyncBasePtr&, std::exception_ptr);

    virtual void sendResponse(Int, Ice::OutputStream*, Byte, bool);
    virtual void sendNoResponse();
    virtual bool systemException(Int, std::exception_ptr, bool);
    virtual void invokeException(Ice::Int, std::exception_ptr, int, bool);

    IceInternal::EndpointIPtr endpoint() const;
    IceInternal::ConnectorPtr connector() const;

    virtual void setAdapter(const ObjectAdapterPtr&); // From Connection.
    virtual ObjectAdapterPtr getAdapter() const noexcept; // From Connection.
    virtual EndpointPtr getEndpoint() const noexcept; // From Connection.
    virtual ObjectPrxPtr createProxy(const Identity& ident) const; // From Connection.

    void setAdapterAndServantManager(const ObjectAdapterPtr&, const IceInternal::ServantManagerPtr&);

    //
    // Operations from EventHandler
    //
#if defined(ICE_USE_IOCP)
    bool startAsync(IceInternal::SocketOperation);
    bool finishAsync(IceInternal::SocketOperation);
#endif

    virtual void message(IceInternal::ThreadPoolCurrent&);
    virtual void finished(IceInternal::ThreadPoolCurrent&, bool);
    virtual std::string toString() const noexcept; // From Connection and EvantHandler.
    virtual IceInternal::NativeInfoPtr getNativeInfo();

    void timedOut();

    virtual std::string type() const noexcept; // From Connection.
    virtual Ice::Int timeout() const noexcept; // From Connection.
    virtual ConnectionInfoPtr getInfo() const; // From Connection

    virtual void setBufferSize(Ice::Int rcvSize, Ice::Int sndSize); // From Connection

    void exception(std::exception_ptr);

    void dispatch(std::function<void(ConnectionIPtr)>,
                  const std::vector<OutgoingMessage>&,
                  Byte,
                  Int,
                  Int,
                  const IceInternal::ServantManagerPtr&,
                  const ObjectAdapterPtr&,
                  const IceInternal::OutgoingAsyncBasePtr&,
                  const HeartbeatCallback&,
                  Ice::InputStream&);
    void finish(bool);

    void closeCallback(const CloseCallback&);

    virtual ~ConnectionI();

private:

    ConnectionI(const Ice::CommunicatorPtr&, const IceInternal::InstancePtr&, const IceInternal::ACMMonitorPtr&,
                const IceInternal::TransceiverPtr&, const IceInternal::ConnectorPtr&,
                const IceInternal::EndpointIPtr&, const std::shared_ptr<ObjectAdapterI>&);

    static ConnectionIPtr
    create(const Ice::CommunicatorPtr&, const IceInternal::InstancePtr&, const IceInternal::ACMMonitorPtr&,
           const IceInternal::TransceiverPtr&, const IceInternal::ConnectorPtr&,
           const IceInternal::EndpointIPtr&, const std::shared_ptr<ObjectAdapterI>&);

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

    friend class IceInternal::IncomingConnectionFactory;
    friend class IceInternal::OutgoingConnectionFactory;

    void setState(State, std::exception_ptr);
    void setState(State);

    void initiateShutdown();
    void sendHeartbeatNow();

    bool initialize(IceInternal::SocketOperation = IceInternal::SocketOperationNone);
    bool validate(IceInternal::SocketOperation = IceInternal::SocketOperationNone);
    IceInternal::SocketOperation sendNextMessage(std::vector<OutgoingMessage>&);
    IceInternal::AsyncStatus sendMessage(OutgoingMessage&);

#ifdef ICE_HAS_BZIP2
    void doCompress(Ice::OutputStream&, Ice::OutputStream&);
    void doUncompress(Ice::InputStream&, Ice::InputStream&);
#endif

    IceInternal::SocketOperation parseMessage(Ice::InputStream&, Int&, Int&, Byte&,
                                              IceInternal::ServantManagerPtr&, ObjectAdapterPtr&,
                                              IceInternal::OutgoingAsyncBasePtr&, HeartbeatCallback&, int&);

    void invokeAll(Ice::InputStream&, Int, Int, Byte,
                   const IceInternal::ServantManagerPtr&, const ObjectAdapterPtr&);

    void scheduleTimeout(IceInternal::SocketOperation status);
    void unscheduleTimeout(IceInternal::SocketOperation status);

    Ice::ConnectionInfoPtr initConnectionInfo() const;
    Ice::Instrumentation::ConnectionState toConnectionState(State) const;

    IceInternal::SocketOperation read(IceInternal::Buffer&);
    IceInternal::SocketOperation write(IceInternal::Buffer&);

    void reap();

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

    std::function<void(ConnectionIPtr)> _connectionStartCompleted;
    std::function<void(ConnectionIPtr, std::exception_ptr)> _connectionStartFailed;

    const bool _warn;
    const bool _warnUdp;

    IceUtil::Time _acmLastActivity;

    const int _compressionLevel;

    Int _nextRequestId;

    std::map<Int, IceInternal::OutgoingAsyncBasePtr> _asyncRequests;
    std::map<Int, IceInternal::OutgoingAsyncBasePtr>::iterator _asyncRequestsHint;

    std::exception_ptr _exception;

    const size_t _messageSizeMax;
    IceInternal::BatchRequestQueuePtr _batchRequestQueue;

    std::deque<OutgoingMessage> _sendStreams;

    Ice::InputStream _readStream;
    bool _readHeader;
    Ice::OutputStream _writeStream;

    Observer _observer;

    int _dispatchCount;

    State _state; // The current state.
    bool _shutdownInitiated;
    bool _initialized;
    bool _validated;

    CloseCallback _closeCallback;
    HeartbeatCallback _heartbeatCallback;

    mutable std::mutex _mutex;
    mutable std::condition_variable _conditionVariable;

    // For locking the _mutex
    template<typename T> friend class IceInternal::ThreadPoolMessage;
};

}

#endif
