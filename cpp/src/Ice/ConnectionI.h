// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
#include <Ice/Dispatcher.h>
#include <Ice/ObserverHelper.h>
#include <Ice/ConnectionAsync.h>

#include <deque>
#include <IceUtil/UniquePtr.h>

namespace IceInternal
{

class Outgoing;
class BatchOutgoing;
class OutgoingMessageCallback;

class ConnectionReaper : public IceUtil::Mutex, public IceUtil::Shared
{
public:

    void add(const Ice::ConnectionIPtr&);
    void swapConnections(std::vector<Ice::ConnectionIPtr>&);

private:

    std::vector<Ice::ConnectionIPtr> _connections;
};
typedef IceUtil::Handle<ConnectionReaper> ConnectionReaperPtr;

}

namespace Ice
{

class LocalException;

class ICE_API ConnectionI : public Connection, public IceInternal::EventHandler, public IceUtil::Monitor<IceUtil::Mutex>
{
    class Observer : public IceInternal::ObserverHelperT<Ice::Instrumentation::ConnectionObserver>
    {
    public:

        Observer();

        void startRead(Ice::Byte*);
        void finishRead(Ice::Byte*);
        void startWrite(Ice::Byte*);
        void finishWrite(Ice::Byte*);

        void attach(const Ice::Instrumentation::ConnectionObserverPtr&);

    private:

        Ice::Byte* _readStreamPos;
        Ice::Byte* _writeStreamPos;
    };

public:

    class StartCallback : virtual public IceUtil::Shared
    {
    public:

        virtual void connectionStartCompleted(const ConnectionIPtr&) = 0;
        virtual void connectionStartFailed(const ConnectionIPtr&, const Ice::LocalException&) = 0;
    };
    typedef IceUtil::Handle<StartCallback> StartCallbackPtr;

    struct SentCallback
    {
#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
        SentCallback(const IceInternal::OutgoingAsyncMessageCallbackPtr& outAsync,
                     const IceInternal::OutgoingAsyncPtr& replyOutAsync) : 
            outAsync(outAsync), replyOutAsync(replyOutAsync)
        {
        }
#else
        SentCallback(const IceInternal::OutgoingAsyncMessageCallbackPtr& outAsync) : outAsync(outAsync)
        {
        }
#endif

        IceInternal::OutgoingAsyncMessageCallbackPtr outAsync;
#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
        IceInternal::OutgoingAsyncPtr replyOutAsync;
#endif
    };

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

    void monitor(const IceUtil::Time&);

    bool sendRequest(IceInternal::Outgoing*, bool, bool);
    IceInternal::AsyncStatus sendAsyncRequest(const IceInternal::OutgoingAsyncPtr&, bool, bool);

    void prepareBatchRequest(IceInternal::BasicStream*);
    void finishBatchRequest(IceInternal::BasicStream*, bool);
    void abortBatchRequest();

    virtual void flushBatchRequests(); // From Connection.
    
#ifdef ICE_CPP11
    virtual ::Ice::AsyncResultPtr begin_flushBatchRequests(
                            const ::IceInternal::Function<void (const ::Ice::Exception&)>& exception, 
                            const ::IceInternal::Function<void (bool)>& sent = ::IceInternal::Function<void (bool)>())
    {
        return __begin_flushBatchRequests(new Cpp11FnCallbackNC_Connection_flushBatchRequests(exception, sent), 0);
    }
#endif

    virtual AsyncResultPtr begin_flushBatchRequests();
    virtual AsyncResultPtr begin_flushBatchRequests(const CallbackPtr&, const LocalObjectPtr& = 0);
    virtual AsyncResultPtr begin_flushBatchRequests(const Callback_Connection_flushBatchRequestsPtr&,
                                                    const LocalObjectPtr& = 0);
    virtual void end_flushBatchRequests(const AsyncResultPtr&);

    bool flushBatchRequests(IceInternal::BatchOutgoing*);
    IceInternal::AsyncStatus flushAsyncBatchRequests(const IceInternal::BatchOutgoingAsyncPtr&);

    void sendResponse(IceInternal::BasicStream*, Byte);
    void sendNoResponse();

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
    virtual void finished(IceInternal::ThreadPoolCurrent&);
    virtual std::string toString() const; // From Connection and EvantHandler.
    virtual IceInternal::NativeInfoPtr getNativeInfo();

    void timedOut();

    virtual std::string type() const; // From Connection.
    virtual Ice::Int timeout() const; // From Connection.
    virtual ConnectionInfoPtr getInfo() const; // From Connection

    void exception(const LocalException&);
    void invokeException(const LocalException&, int);

    void dispatch(const StartCallbackPtr&, const std::vector<SentCallback>&, Byte, Int, Int,
                  const IceInternal::ServantManagerPtr&, const ObjectAdapterPtr&, const IceInternal::OutgoingAsyncPtr&, 
                  IceInternal::BasicStream&);
    void finish();

private:

    friend class IceInternal::ConnectionReaper;

    enum State
    {
        StateNotInitialized,
        StateNotValidated,
        StateActive,
        StateHolding,
        StateClosing,
        StateClosed,
        StateFinished
    };

    struct OutgoingMessage
    {
        OutgoingMessage(IceInternal::BasicStream* str, bool comp) :
            stream(str), out(0), compress(comp), requestId(0), adopted(false), isSent(false)
        {
        }

        OutgoingMessage(IceInternal::OutgoingMessageCallback* o, IceInternal::BasicStream* str, bool comp, int rid) :
            stream(str), out(o), compress(comp), requestId(rid), adopted(false), isSent(false)
        {
        }

        OutgoingMessage(const IceInternal::OutgoingAsyncMessageCallbackPtr& o, IceInternal::BasicStream* str,
                        bool comp, int rid) :
            stream(str), out(0), outAsync(o), compress(comp), requestId(rid), adopted(false), isSent(false)
        {
        }

        void adopt(IceInternal::BasicStream*);
        bool sent(ConnectionI*, bool);
        void finished(const Ice::LocalException&);

        IceInternal::BasicStream* stream;
        IceInternal::OutgoingMessageCallback* out;
        IceInternal::OutgoingAsyncMessageCallbackPtr outAsync;
#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
        IceInternal::OutgoingAsyncPtr replyOutAsync;
#endif
        bool compress;
        int requestId;
        bool adopted;
        bool isSent;
    };

    ConnectionI(const Ice::CommunicatorPtr&, const IceInternal::InstancePtr&, const IceInternal::ConnectionReaperPtr&,
                const IceInternal::TransceiverPtr&, const IceInternal::ConnectorPtr&,
                const IceInternal::EndpointIPtr&, const ObjectAdapterPtr&);
    virtual ~ConnectionI();

    friend class IceInternal::IncomingConnectionFactory;
    friend class IceInternal::OutgoingConnectionFactory;

    void setState(State, const LocalException&);
    void setState(State);

    void initiateShutdown();

    bool initialize(IceInternal::SocketOperation = IceInternal::SocketOperationNone);
    bool validate(IceInternal::SocketOperation = IceInternal::SocketOperationNone);
    void sendNextMessage(std::vector<SentCallback>&);
    IceInternal::AsyncStatus sendMessage(OutgoingMessage&);

#ifndef ICE_OS_WINRT
    void doCompress(IceInternal::BasicStream&, IceInternal::BasicStream&);
    void doUncompress(IceInternal::BasicStream&, IceInternal::BasicStream&);
#endif
    void parseMessage(IceInternal::BasicStream&, Int&, Int&, Byte&,
                      IceInternal::ServantManagerPtr&, ObjectAdapterPtr&, IceInternal::OutgoingAsyncPtr&);
    void invokeAll(IceInternal::BasicStream&, Int, Int, Byte,
                   const IceInternal::ServantManagerPtr&, const ObjectAdapterPtr&);

    void scheduleTimeout(IceInternal::SocketOperation status, int timeout)
    {
        if(timeout < 0)
        {
            return;
        }

        try
        {
            if(status & IceInternal::SocketOperationRead)
            {
                _timer->schedule(_readTimeout, IceUtil::Time::milliSeconds(timeout));
                _readTimeoutScheduled = true;
            }
            if(status & (IceInternal::SocketOperationWrite | IceInternal::SocketOperationConnect))
            {
                _timer->schedule(_writeTimeout, IceUtil::Time::milliSeconds(timeout));
                _writeTimeoutScheduled = true;
            }
        }
        catch(const IceUtil::Exception&)
        {
            assert(false);
        }
    }

    void unscheduleTimeout(IceInternal::SocketOperation status)
    {
        if((status & IceInternal::SocketOperationRead) && _readTimeoutScheduled)
        {
            _timer->cancel(_readTimeout);
            _readTimeoutScheduled = false;
        }
        if((status & (IceInternal::SocketOperationWrite | IceInternal::SocketOperationConnect)) &&
           _writeTimeoutScheduled)
        {
            _timer->cancel(_writeTimeout);
            _writeTimeoutScheduled = false;
        }
    }

    int connectTimeout();
    int closeTimeout();

    Ice::ConnectionInfoPtr initConnectionInfo() const;
    Ice::Instrumentation::ConnectionState toConnectionState(State) const;
        
    AsyncResultPtr __begin_flushBatchRequests(const IceInternal::CallbackBasePtr&, const LocalObjectPtr&);

    Ice::CommunicatorPtr _communicator;
    const IceInternal::InstancePtr _instance;
    const IceInternal::ConnectionReaperPtr _reaper;
    const IceInternal::TransceiverPtr _transceiver;
    const std::string _desc;
    const std::string _type;
    const IceInternal::ConnectorPtr _connector;
    const IceInternal::EndpointIPtr _endpoint;

    mutable Ice::ConnectionInfoPtr _info;
    
    ObjectAdapterPtr _adapter;
    IceInternal::ServantManagerPtr _servantManager;

    const DispatcherPtr _dispatcher;
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
    const int _acmTimeout;
    IceUtil::Time _acmAbsoluteTimeout;

    const int _compressionLevel;

    Int _nextRequestId;

    std::map<Int, IceInternal::Outgoing*> _requests;
    std::map<Int, IceInternal::Outgoing*>::iterator _requestsHint;

    std::map<Int, IceInternal::OutgoingAsyncPtr> _asyncRequests;
    std::map<Int, IceInternal::OutgoingAsyncPtr>::iterator _asyncRequestsHint;

    IceUtil::UniquePtr<LocalException> _exception;

    const bool _batchAutoFlush;
    IceInternal::BasicStream _batchStream;
    bool _batchStreamInUse;
    int _batchRequestNum;
    bool _batchRequestCompress;
    size_t _batchMarker;

    std::deque<OutgoingMessage> _sendStreams;

    IceInternal::BasicStream _readStream;
    bool _readHeader;
    IceInternal::BasicStream _writeStream;

    Observer _observer;

    int _dispatchCount;

    State _state; // The current state.
    bool _shutdownInitiated;
    bool _validated;
};

}

#endif
