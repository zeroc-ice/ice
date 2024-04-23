//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_CONNECTION_I_H
#define ICE_CONNECTION_I_H

#include "ACM.h"
#include "ConnectionFactoryF.h"
#include "ConnectionOptions.h"
#include "ConnectorF.h"
#include "EndpointIF.h"
#include "EventHandler.h"
#include "Ice/BatchRequestQueueF.h"
#include "Ice/CommunicatorF.h"
#include "Ice/Connection.h"
#include "Ice/ConnectionIF.h"
#include "Ice/InputStream.h"
#include "Ice/InstanceF.h"
#include "Ice/Logger.h"
#include "Ice/ObjectAdapterF.h"
#include "Ice/ObserverHelper.h"
#include "Ice/OutgoingAsync.h"
#include "Ice/OutgoingResponse.h"
#include "Ice/OutputStream.h"
#include "IceUtil/StopWatch.h"
#include "IceUtil/Timer.h"
#include "RequestHandler.h"
#include "TraceLevelsF.h"
#include "TransceiverF.h"

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>

#ifndef ICE_HAS_BZIP2
#    define ICE_HAS_BZIP2
#endif

namespace IceInternal
{
    template<typename T> class ThreadPoolMessage;
}

namespace Ice
{
    class LocalException;
    class ObjectAdapterI;
    using ObjectAdapterIPtr = std::shared_ptr<ObjectAdapterI>;

    class ConnectionI final : public Connection,
                              public IceInternal::EventHandler,
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
            std::byte* _readStreamPos;
            std::byte* _writeStreamPos;
        };

    public:
        std::shared_ptr<ConnectionI> shared_from_this()
        {
            return std::dynamic_pointer_cast<ConnectionI>(IceInternal::EventHandler::shared_from_this());
        }

        struct OutgoingMessage
        {
            OutgoingMessage(Ice::OutputStream* str, bool comp)
                : stream(str),
                  compress(comp),
                  requestId(0),
                  adopted(false)
#if defined(ICE_USE_IOCP)
                  ,
                  isSent(false),
                  invokeSent(false),
                  receivedReply(false)
#endif
            {
            }

            OutgoingMessage(const IceInternal::OutgoingAsyncBasePtr& o, Ice::OutputStream* str, bool comp, int rid)
                : stream(str),
                  outAsync(o),
                  compress(comp),
                  requestId(rid),
                  adopted(false)
#if defined(ICE_USE_IOCP)
                  ,
                  isSent(false),
                  invokeSent(false),
                  receivedReply(false)
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
        void close(ConnectionClose) noexcept final; // From Connection.

        bool isActiveOrHolding() const;
        bool isFinished() const;

        void throwException() const final; // From Connection. Throws the connection exception if destroyed.

        void waitUntilHolding() const;
        void waitUntilFinished(); // Not const, as this might close the connection upon timeout.

        void updateObserver();

        void monitor(const std::chrono::steady_clock::time_point&, const IceInternal::ACMConfig&);

        IceInternal::AsyncStatus sendAsyncRequest(const IceInternal::OutgoingAsyncBasePtr&, bool, bool, int);

        const IceInternal::BatchRequestQueuePtr& getBatchRequestQueue() const;

        std::function<void()> flushBatchRequestsAsync(
            CompressBatch,
            std::function<void(std::exception_ptr)>,
            std::function<void(bool)> = nullptr) final;

        void setCloseCallback(CloseCallback) final;
        void setHeartbeatCallback(HeartbeatCallback) final;

        std::function<void()>
            heartbeatAsync(std::function<void(std::exception_ptr)>, std::function<void(bool)> = nullptr) final;

        void
        setACM(const std::optional<int>&, const std::optional<ACMClose>&, const std::optional<ACMHeartbeat>&) final;
        ACM getACM() noexcept final;

        void asyncRequestCanceled(const IceInternal::OutgoingAsyncBasePtr&, std::exception_ptr) final;

        IceInternal::EndpointIPtr endpoint() const;
        IceInternal::ConnectorPtr connector() const;

        void setAdapter(const ObjectAdapterPtr&) final;           // From Connection.
        ObjectAdapterPtr getAdapter() const noexcept final;       // From Connection.
        EndpointPtr getEndpoint() const noexcept final;           // From Connection.
        ObjectPrx createProxy(const Identity& ident) const final; // From Connection.

        void setAdapterFromAdapter(const ObjectAdapterIPtr&); // From ObjectAdapterI.

        //
        // Operations from EventHandler
        //
#if defined(ICE_USE_IOCP)
        bool startAsync(IceInternal::SocketOperation);
        bool finishAsync(IceInternal::SocketOperation);
#endif

        void message(IceInternal::ThreadPoolCurrent&) final;
        void finished(IceInternal::ThreadPoolCurrent&, bool) final;
        std::string toString() const noexcept final; // From Connection and EventHandler.
        IceInternal::NativeInfoPtr getNativeInfo() final;

        std::string type() const noexcept final;     // From Connection.
        std::int32_t timeout() const noexcept final; // From Connection.
        ConnectionInfoPtr getInfo() const final;     // From Connection

        void setBufferSize(std::int32_t rcvSize, std::int32_t sndSize) final; // From Connection

        void exception(std::exception_ptr);

        // This method is called to execute user code (connection start completion callback, invocation sent callbacks,
        // servant dispatch, invocation response, heartbeat callback). Only the invocation sent callbacks and one of the
        // other callbacks can be set at the same time. TODO: improve this to use separate functions encapsulated with
        // a std::function
        void upcall(
            std::function<void(ConnectionIPtr)> connectionStartCompleted,
            const std::vector<OutgoingMessage>& sentMessages, // for calling invocation sent callbacks
            std::uint8_t compress,
            std::int32_t requestId,
            std::int32_t dispatchCount,
            const ObjectAdapterIPtr& adapter,
            const IceInternal::OutgoingAsyncBasePtr& outAsync, // for callback the invocation response
            const HeartbeatCallback& heartbeatCallback,
            Ice::InputStream& stream); // the incoming request stream
        void finish(bool);

        void closeCallback(const CloseCallback&);

        /// Aborts the connection with a ConnectionIdleException unless any of the following is true:
        /// - the connection is no longer active
        /// - its transceiver is waiting to be read
        /// - the idle check timer task has been rescheduled by a concurrent read
        /// In the two latter cases, this function reschedules the idle check timer task in idle timeout.
        void
        idleCheck(const IceUtil::TimerTaskPtr& idleCheckTimerTask, const std::chrono::seconds& idleTimeout) noexcept;

        /// Aborts the connection if its state is < StateActive.
        void connectTimedOut() noexcept;

        /// Aborts the connection if its state is < StateClosed.
        void closeTimedOut() noexcept;

        // TODO: there are too many functions with similar names. This is the function called by the HeartbeatTimerTask.
        void sendHeartbeat() noexcept;

        ~ConnectionI() final;

    private:
        ConnectionI(
            const Ice::CommunicatorPtr&,
            const IceInternal::InstancePtr&,
            const IceInternal::ACMMonitorPtr&,
            const IceInternal::TransceiverPtr&,
            const IceInternal::ConnectorPtr&,
            const IceInternal::EndpointIPtr&,
            const std::shared_ptr<ObjectAdapterI>&,
            const ConnectionOptions&) noexcept;

        static ConnectionIPtr create(
            const Ice::CommunicatorPtr&,
            const IceInternal::InstancePtr&,
            const IceInternal::ACMMonitorPtr&,
            const IceInternal::TransceiverPtr&,
            const IceInternal::ConnectorPtr&,
            const IceInternal::EndpointIPtr&,
            const std::shared_ptr<ObjectAdapterI>&,
            const ConnectionOptions&);

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

        void sendResponse(OutgoingResponse, std::uint8_t compress);
        void dispatchException(std::exception_ptr, int);

        bool initialize(IceInternal::SocketOperation = IceInternal::SocketOperationNone);
        bool validate(IceInternal::SocketOperation = IceInternal::SocketOperationNone);
        IceInternal::SocketOperation sendNextMessages(std::vector<OutgoingMessage>&);
        IceInternal::AsyncStatus sendMessage(OutgoingMessage&);

#ifdef ICE_HAS_BZIP2
        void doCompress(Ice::OutputStream&, Ice::OutputStream&);
        void doUncompress(Ice::InputStream&, Ice::InputStream&);
#endif

        IceInternal::SocketOperation parseMessage(
            Ice::InputStream&,
            std::int32_t&,
            std::int32_t&,
            std::uint8_t&,
            ObjectAdapterIPtr&,
            IceInternal::OutgoingAsyncBasePtr&,
            HeartbeatCallback&,
            int&);

        void dispatchAll(Ice::InputStream&, std::int32_t, std::int32_t, std::uint8_t, const ObjectAdapterIPtr&);

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

        ObjectAdapterIPtr _adapter;

        const bool _hasExecutor;
        const LoggerPtr _logger;
        const IceInternal::TraceLevelsPtr _traceLevels;
        const IceInternal::ThreadPoolPtr _threadPool;

        const IceUtil::TimerPtr _timer;

        const std::chrono::seconds _connectTimeout;
        const std::chrono::seconds _closeTimeout;
        const std::chrono::seconds _inactivityTimeout;

        std::function<void(ConnectionIPtr)> _connectionStartCompleted;
        std::function<void(ConnectionIPtr, std::exception_ptr)> _connectionStartFailed;

        const bool _warn;
        const bool _warnUdp;

        std::chrono::steady_clock::time_point _acmLastActivity;

        const int _compressionLevel;

        std::int32_t _nextRequestId;

        std::map<std::int32_t, IceInternal::OutgoingAsyncBasePtr> _asyncRequests;
        std::map<std::int32_t, IceInternal::OutgoingAsyncBasePtr>::iterator _asyncRequestsHint;

        std::exception_ptr _exception;

        const size_t _messageSizeMax;
        IceInternal::BatchRequestQueuePtr _batchRequestQueue;

        std::deque<OutgoingMessage> _sendStreams;

        Ice::InputStream _readStream;
        bool _readHeader;
        Ice::OutputStream _writeStream;

        Observer _observer;

        // The number of user calls currently executed by the thread-pool (servant dispatch, invocation response, ...)
        int _upcallCount;

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
