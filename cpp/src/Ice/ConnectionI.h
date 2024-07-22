//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_CONNECTION_I_H
#define ICE_CONNECTION_I_H

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
#include "Ice/Timer.h"
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
                assert(stream);
                // The outgoing message can be sent asynchronously. If the stream doesn't own its memory, the memory
                // owner could release it before the message is sent.
                assert(stream->b.ownsMemory());
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
                assert(stream);
                assert(stream->b.ownsMemory());
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

        /// Sends a request.
        /// @param out The outgoing request.
        /// @param compress True to compress the request; otherwise, false.
        /// @param response True for a two-way request; false for a one-way request.
        /// @param batchRequestCount The number of requests in the batch (for one-way requests); ignored for two-way
        /// requests.
        /// @return The sent status.
        IceInternal::AsyncStatus sendAsyncRequest(
            const IceInternal::OutgoingAsyncBasePtr& out,
            bool compress,
            bool response,
            int batchRequestCount);

        const IceInternal::BatchRequestQueuePtr& getBatchRequestQueue() const;

        std::function<void()> flushBatchRequestsAsync(
            CompressBatch,
            std::function<void(std::exception_ptr)>,
            std::function<void(bool)> = nullptr) final;

        void setCloseCallback(CloseCallback) final;

        void asyncRequestCanceled(const IceInternal::OutgoingAsyncBasePtr&, std::exception_ptr) final;

        IceInternal::EndpointIPtr endpoint() const;
        IceInternal::ConnectorPtr connector() const;

        void setAdapter(const ObjectAdapterPtr&) final;            // From Connection.
        ObjectAdapterPtr getAdapter() const noexcept final;        // From Connection.
        EndpointPtr getEndpoint() const noexcept final;            // From Connection.
        ObjectPrx _createProxy(const Identity& ident) const final; // From Connection.

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

        std::string type() const noexcept final; // From Connection.
        ConnectionInfoPtr getInfo() const final; // From Connection

        void setBufferSize(std::int32_t rcvSize, std::int32_t sndSize) final; // From Connection

        void exception(std::exception_ptr);

        // This method is called to execute user code (connection start completion callback, invocation sent callbacks,
        // or an upcall issued from an incoming message). The invocation sent callbacks and the message upcall might
        // both be set.
        void upcall(
            std::function<void(ConnectionIPtr)> connectionStartCompleted,
            const std::vector<OutgoingMessage>& sentMessages,
            std::function<bool(InputStream&)> messageUpcall,
            InputStream& messageStream);
        void finish(bool);

        void closeCallback(const CloseCallback&);

        /// Aborts the connection with a ConnectionAbortedException unless any of the following is true:
        /// - the connection is no longer active
        /// - its transceiver is waiting to be read
        /// - the idle check timer task has been rescheduled by a concurrent read
        /// In the two latter cases, this function reschedules the idle check timer task in idle timeout.
        void idleCheck(const Ice::TimerTaskPtr& idleCheckTimerTask, const std::chrono::seconds& idleTimeout) noexcept;

        /// Shuts down the connection gracefully if it's at rest when this function is called.
        void inactivityCheck() noexcept;

        /// Aborts the connection if its state is < StateActive.
        void connectTimedOut() noexcept;

        /// Aborts the connection if its state is < StateClosed.
        void closeTimedOut() noexcept;

        /// Sends a heartbeat.
        void sendHeartbeat() noexcept;

        ~ConnectionI() final;

    private:
        ConnectionI(
            const Ice::CommunicatorPtr&,
            const IceInternal::InstancePtr&,
            const IceInternal::TransceiverPtr&,
            const IceInternal::ConnectorPtr&,
            const IceInternal::EndpointIPtr&,
            const std::shared_ptr<ObjectAdapterI>&,
            std::function<void(const ConnectionIPtr&)>,
            const ConnectionOptions&) noexcept;

        static ConnectionIPtr create(
            const Ice::CommunicatorPtr&,
            const IceInternal::InstancePtr&,
            const IceInternal::TransceiverPtr&,
            const IceInternal::ConnectorPtr&,
            const IceInternal::EndpointIPtr&,
            const std::shared_ptr<ObjectAdapterI>&,
            std::function<void(const ConnectionIPtr&)>,
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

        void sendResponse(OutgoingResponse, std::uint8_t compress);
        void dispatchException(std::exception_ptr, int);

        bool initialize(IceInternal::SocketOperation = IceInternal::SocketOperationNone);
        bool validate(IceInternal::SocketOperation = IceInternal::SocketOperationNone);

        /// Sends the next queued messages. This method is called by message() once the message which is being sent
        /// (_sendStreams.First) is fully sent. Before sending the next message, this message is removed from
        /// _sendsStream. If any, its sent callback is also queued in given callback queue.
        ///
        /// @param callbacks The sent callbacks to call for the messages that were sent.
        /// @return The socket operation to register with the thread pool's selector to send the remainder of the
        /// pending message being sent (_sendStreams.First).
        IceInternal::SocketOperation sendNextMessages(std::vector<OutgoingMessage>& callbacks);

        /// Sends or queues the given message.
        ///
        /// @param message The message to send.
        /// @return The send status.
        IceInternal::AsyncStatus sendMessage(OutgoingMessage& message);

#ifdef ICE_HAS_BZIP2
        void doCompress(Ice::OutputStream&, Ice::OutputStream&);
        void doUncompress(Ice::InputStream&, Ice::InputStream&);
#endif

        IceInternal::SocketOperation parseMessage(
            std::int32_t& upcallCount,
            std::function<bool(InputStream&)>& messageUpcall,
            Ice::InputStream& messageStream);

        void dispatchAll(Ice::InputStream&, std::int32_t, std::int32_t, std::uint8_t, const ObjectAdapterIPtr&);

        Ice::ConnectionInfoPtr initConnectionInfo() const;
        Ice::Instrumentation::ConnectionState toConnectionState(State) const;

        IceInternal::SocketOperation read(IceInternal::Buffer&);
        IceInternal::SocketOperation write(IceInternal::Buffer&);

        void scheduleInactivityTimerTask();
        void cancelInactivityTimerTask();

        Ice::CommunicatorPtr _communicator;
        const IceInternal::InstancePtr _instance;
        const IceInternal::TransceiverPtr _transceiver;
        const std::string _desc;
        const std::string _type;
        const IceInternal::ConnectorPtr _connector;
        const IceInternal::EndpointIPtr _endpoint;

        mutable Ice::ConnectionInfoPtr _info;

        ObjectAdapterIPtr _adapter;

        // The application configured a custom executor or a dispatch queue executor in InitializationData
        const bool _hasExecutor;

        const LoggerPtr _logger;
        const IceInternal::TraceLevelsPtr _traceLevels;
        const IceInternal::ThreadPoolPtr _threadPool;

        const Ice::TimerPtr _timer;

        const std::chrono::seconds _connectTimeout;
        const std::chrono::seconds _closeTimeout;
        const std::chrono::seconds _inactivityTimeout;

        Ice::TimerTaskPtr _inactivityTimerTask;
        bool _inactivityTimerTaskScheduled;

        std::function<void(ConnectionIPtr)> _connectionStartCompleted;
        std::function<void(ConnectionIPtr, std::exception_ptr)> _connectionStartFailed;

        // This function must be called outside the ConnectionI lock to avoid lock acquisition deadlocks.
        const std::function<void(const ConnectionIPtr&)> _removeFromFactory;

        const bool _warn;
        const bool _warnUdp;

        const int _compressionLevel;

        std::int32_t _nextRequestId;

        std::map<std::int32_t, IceInternal::OutgoingAsyncBasePtr> _asyncRequests;
        std::map<std::int32_t, IceInternal::OutgoingAsyncBasePtr>::iterator _asyncRequestsHint;

        std::exception_ptr _exception;

        const size_t _messageSizeMax;
        IceInternal::BatchRequestQueuePtr _batchRequestQueue;

        std::deque<OutgoingMessage> _sendStreams;

        // Contains the message which is being received. If the connection is waiting to receive a message (_readHeader
        // == true), its size is Protocol.headerSize. Otherwise, its size is the message size specified in the received
        // message header.
        Ice::InputStream _readStream;

        // When _readHeader is true, the next bytes we'll read are the header of a new message. When false, we're
        // reading next the remainder of a message that was already partially received.
        bool _readHeader;

        // Contains the message which is being sent. The write stream buffer is empty if no message is being sent.
        Ice::OutputStream _writeStream;

        Observer _observer;

        // The number of user calls currently executed by the thread-pool (servant dispatch, invocation response, ...)
        int _upcallCount;

        // The number of outstanding dispatches. Maintained only while state is StateActive or StateHolding.
        int _dispatchCount = 0;

        State _state; // The current state.
        bool _shutdownInitiated;
        bool _initialized;
        bool _validated;

        CloseCallback _closeCallback;

        mutable std::mutex _mutex;
        mutable std::condition_variable _conditionVariable;

        // For locking the _mutex
        template<typename T> friend class IceInternal::ThreadPoolMessage;
    };
}

#endif
