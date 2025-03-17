// Copyright (c) ZeroC, Inc.

#ifndef ICE_CONNECTION_FACTORY_H
#define ICE_CONNECTION_FACTORY_H

#include "AcceptorF.h"
#include "ConnectionFactoryF.h"
#include "ConnectionI.h"
#include "ConnectorF.h"
#include "EndpointI.h"
#include "EndpointIF.h"
#include "EventHandler.h"
#include "Ice/CommunicatorF.h"
#include "Ice/Endpoint.h"
#include "Ice/InstanceF.h"
#include "Ice/Instrumentation.h"
#include "Ice/ObjectAdapterF.h"
#include "RouterInfoF.h"
#include "TargetCompare.h"
#include "TransceiverF.h"

#include <condition_variable>
#include <list>
#include <mutex>
#include <set>

namespace Ice
{
    class ObjectAdapterI;
}

namespace IceInternal
{
    class ThreadPoolCurrent;

    class CommunicatorFlushBatchAsync;
    using CommunicatorFlushBatchAsyncPtr = std::shared_ptr<CommunicatorFlushBatchAsync>;

    template<typename T> class ThreadPoolMessage;

    class OutgoingConnectionFactory final : public std::enable_shared_from_this<OutgoingConnectionFactory>
    {
    public:
        void destroy();

        void updateConnectionObservers();

        void waitUntilFinished();

        void createAsync(
            std::vector<EndpointIPtr>,
            bool,
            std::function<void(Ice::ConnectionIPtr, bool)>,
            std::function<void(std::exception_ptr)>);

        void setRouterInfo(const RouterInfoPtr&);
        void removeAdapter(const Ice::ObjectAdapterPtr&);
        void flushAsyncBatchRequests(const CommunicatorFlushBatchAsyncPtr&, Ice::CompressBatch);

        void removeConnection(const Ice::ConnectionIPtr&) noexcept;

        void setDefaultObjectAdapter(Ice::ObjectAdapterPtr adapter) noexcept;
        [[nodiscard]] Ice::ObjectAdapterPtr getDefaultObjectAdapter() const noexcept;

        OutgoingConnectionFactory(Ice::CommunicatorPtr, const InstancePtr&);
        ~OutgoingConnectionFactory();
        friend class Instance;

    private:
        struct ConnectorInfo
        {
            ConnectorInfo(ConnectorPtr c, EndpointIPtr e) : connector(std::move(c)), endpoint(std::move(e)) {}

            bool operator==(const ConnectorInfo& other) const;

            ConnectorPtr connector;
            EndpointIPtr endpoint;
        };

        class ConnectCallback final : public std::enable_shared_from_this<ConnectCallback>
        {
        public:
            ConnectCallback(
                InstancePtr,
                OutgoingConnectionFactoryPtr,
                std::vector<EndpointIPtr>,
                bool,
                std::function<void(Ice::ConnectionIPtr, bool)>,
                std::function<void(std::exception_ptr)>);

            virtual void connectionStartCompleted(const Ice::ConnectionIPtr&);
            virtual void connectionStartFailed(const Ice::ConnectionIPtr&, std::exception_ptr);

            virtual void connectors(const std::vector<ConnectorPtr>&);
            virtual void exception(std::exception_ptr);

            void getConnectors();
            void nextEndpoint();

            void getConnection();
            void nextConnector();

            void setConnection(const Ice::ConnectionIPtr&, bool);
            void setException(std::exception_ptr);

            bool hasConnector(const ConnectorInfo&);
            bool removeConnectors(const std::vector<ConnectorInfo>&);
            void removeFromPending();

        private:
            bool connectionStartFailedImpl(std::exception_ptr);

            const InstancePtr _instance;
            const OutgoingConnectionFactoryPtr _factory;
            const std::vector<EndpointIPtr> _endpoints;
            const bool _hasMore;
            const std::function<void(Ice::ConnectionIPtr, bool)> _createConnectionResponse;
            const std::function<void(std::exception_ptr)> _createConnectionException;
            Ice::Instrumentation::ObserverPtr _observer;
            std::vector<EndpointIPtr>::const_iterator _endpointsIter;
            std::vector<ConnectorInfo> _connectors;
            std::vector<ConnectorInfo>::const_iterator _iter;
        };
        using ConnectCallbackPtr = std::shared_ptr<ConnectCallback>;
        friend class ConnectCallback;

        Ice::ConnectionIPtr findConnection(const std::vector<EndpointIPtr>&, bool&);
        void incPendingConnectCount();
        void decPendingConnectCount();
        Ice::ConnectionIPtr getConnection(const std::vector<ConnectorInfo>&, const ConnectCallbackPtr&, bool&);
        void finishGetConnection(
            const std::vector<ConnectorInfo>&,
            const ConnectorInfo&,
            const Ice::ConnectionIPtr&,
            const ConnectCallbackPtr&);
        void finishGetConnection(const std::vector<ConnectorInfo>&, std::exception_ptr, const ConnectCallbackPtr&);

        bool addToPending(const ConnectCallbackPtr&, const std::vector<ConnectorInfo>&);
        void removeFromPending(const ConnectCallbackPtr&, const std::vector<ConnectorInfo>&);

        Ice::ConnectionIPtr findConnection(const std::vector<ConnectorInfo>&, bool&);
        Ice::ConnectionIPtr createConnection(const TransceiverPtr&, const ConnectorInfo&);

        void handleException(std::exception_ptr, bool);
        void handleConnectionException(std::exception_ptr, bool);

        Ice::CommunicatorPtr _communicator;
        const InstancePtr _instance;
        const Ice::ConnectionOptions _connectionOptions;

        bool _destroyed{false};

        using ConnectCallbackSet = std::set<ConnectCallbackPtr>;

        std::multimap<ConnectorPtr, Ice::ConnectionIPtr, Ice::TargetCompare<ConnectorPtr, std::less>> _connections;
        std::map<ConnectorPtr, ConnectCallbackSet, Ice::TargetCompare<ConnectorPtr, std::less>> _pending;

        std::multimap<EndpointIPtr, Ice::ConnectionIPtr, Ice::TargetCompare<EndpointIPtr, std::less>>
            _connectionsByEndpoint;
        int _pendingConnectCount{0};
        Ice::ObjectAdapterIPtr _defaultObjectAdapter;
        mutable std::mutex _mutex;
        std::condition_variable _conditionVariable;
    };

    class IncomingConnectionFactory final : public EventHandler
    {
    public:
        IncomingConnectionFactory(const InstancePtr&, const EndpointIPtr&, const std::shared_ptr<Ice::ObjectAdapterI>&);
        void activate();
        void hold();
        void destroy();

        void startAcceptor();
        void stopAcceptor();

        void updateConnectionObservers();

        void waitUntilHolding() const;
        void waitUntilFinished();

        [[nodiscard]] EndpointIPtr endpoint() const;
        [[nodiscard]] std::list<Ice::ConnectionIPtr> connections() const;
        void removeConnection(const Ice::ConnectionIPtr&) noexcept;

        void flushAsyncBatchRequests(const CommunicatorFlushBatchAsyncPtr&, Ice::CompressBatch);

        //
        // Operations from EventHandler
        //

#if defined(ICE_USE_IOCP)
        virtual bool startAsync(SocketOperation);
        virtual bool finishAsync(SocketOperation);
#endif

        void message(ThreadPoolCurrent&) override;
        void finished(ThreadPoolCurrent&, bool) override;
#if TARGET_OS_IPHONE != 0
        void finish();
#endif
        [[nodiscard]] std::string toString() const override;
        NativeInfoPtr getNativeInfo() override;

        virtual void connectionStartCompleted(const Ice::ConnectionIPtr&);
        virtual void connectionStartFailed(const Ice::ConnectionIPtr&, std::exception_ptr);
        void initialize();
        ~IncomingConnectionFactory() override;

        std::shared_ptr<IncomingConnectionFactory> shared_from_this()
        {
            return std::static_pointer_cast<IncomingConnectionFactory>(EventHandler::shared_from_this());
        }

    private:
        friend class Ice::ObjectAdapterI;

        enum State
        {
            StateActive,
            StateHolding,
            StateClosed,
            StateFinished
        };

        void setState(State);

        void createAcceptor();
        void closeAcceptor();

        const InstancePtr _instance;
        const Ice::ConnectionOptions _connectionOptions;

        const int _maxConnections;

        AcceptorPtr _acceptor;
        const TransceiverPtr _transceiver;
        EndpointIPtr _endpoint;

        bool _acceptorStarted{false};
        bool _acceptorStopped{false};

        std::shared_ptr<Ice::ObjectAdapterI> _adapter;
        const bool _warn;
        std::set<Ice::ConnectionIPtr> _connections;
        State _state{StateHolding};

#if defined(ICE_USE_IOCP)
        std::exception_ptr _acceptorException;
#endif
        mutable std::mutex _mutex;
        mutable std::condition_variable _conditionVariable;

        // For locking the _mutex
        template<typename T> friend class IceInternal::ThreadPoolMessage;
    };
}

#endif
