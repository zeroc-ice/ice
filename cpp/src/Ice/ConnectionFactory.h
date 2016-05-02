// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_CONNECTION_FACTORY_H
#define ICE_CONNECTION_FACTORY_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <Ice/CommunicatorF.h>
#include <Ice/ConnectionFactoryF.h>
#include <Ice/ConnectionI.h>
#include <Ice/InstanceF.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/EndpointIF.h>
#include <Ice/Endpoint.h>
#include <Ice/ConnectorF.h>
#include <Ice/AcceptorF.h>
#include <Ice/TransceiverF.h>
#include <Ice/RouterInfoF.h>
#include <Ice/EventHandler.h>
#include <Ice/EndpointI.h>
#include <Ice/InstrumentationF.h>
#include <Ice/ACMF.h>

#include <list>
#include <set>

namespace Ice
{

class LocalException;
class ObjectAdapterI;
typedef IceUtil::Handle<ObjectAdapterI> ObjectAdapterIPtr;

}

namespace IceInternal
{

class OutgoingConnectionFactory : virtual public IceUtil::Shared, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    class CreateConnectionCallback : virtual public IceUtil::Shared
    {
    public:

        virtual void setConnection(const Ice::ConnectionIPtr&, bool) = 0;
        virtual void setException(const Ice::LocalException&) = 0;
    };
    typedef IceUtil::Handle<CreateConnectionCallback> CreateConnectionCallbackPtr;

    void destroy();

    void updateConnectionObservers();

    void waitUntilFinished();

    void create(const std::vector<EndpointIPtr>&, bool, Ice::EndpointSelectionType,
                const CreateConnectionCallbackPtr&);
    void setRouterInfo(const RouterInfoPtr&);
    void removeAdapter(const Ice::ObjectAdapterPtr&);
    void flushAsyncBatchRequests(const CommunicatorFlushBatchAsyncPtr&);

private:

    OutgoingConnectionFactory(const Ice::CommunicatorPtr&, const InstancePtr&);
    virtual ~OutgoingConnectionFactory();
    friend class Instance;

    struct ConnectorInfo
    {
        ConnectorInfo(const ConnectorPtr& c, const EndpointIPtr& e) : connector(c), endpoint(e)
        {
        }

        bool operator==(const ConnectorInfo& other) const;

        ConnectorPtr connector;
        EndpointIPtr endpoint;
    };

    class ConnectCallback : public Ice::ConnectionI::StartCallback, public IceInternal::EndpointI_connectors
    {
    public:

        ConnectCallback(const InstancePtr&, const OutgoingConnectionFactoryPtr&, const std::vector<EndpointIPtr>&, bool,
                        const CreateConnectionCallbackPtr&, Ice::EndpointSelectionType);

        virtual void connectionStartCompleted(const Ice::ConnectionIPtr&);
        virtual void connectionStartFailed(const Ice::ConnectionIPtr&, const Ice::LocalException&);

        virtual void connectors(const std::vector<ConnectorPtr>&);
        virtual void exception(const Ice::LocalException&);

        void getConnectors();
        void nextEndpoint();

        void getConnection();
        void nextConnector();

        void setConnection(const Ice::ConnectionIPtr&, bool);
        void setException(const Ice::LocalException&);

        bool hasConnector(const ConnectorInfo&);
        bool removeConnectors(const std::vector<ConnectorInfo>&);
        void removeFromPending();

        bool operator<(const ConnectCallback&) const;

    private:

        bool connectionStartFailedImpl(const Ice::LocalException&);

        const InstancePtr _instance;
        const OutgoingConnectionFactoryPtr _factory;
        const std::vector<EndpointIPtr> _endpoints;
        const bool _hasMore;
        const CreateConnectionCallbackPtr _callback;
        const Ice::EndpointSelectionType _selType;
        Ice::Instrumentation::ObserverPtr _observer;
        std::vector<EndpointIPtr>::const_iterator _endpointsIter;
        std::vector<ConnectorInfo> _connectors;
        std::vector<ConnectorInfo>::const_iterator _iter;
    };
    typedef IceUtil::Handle<ConnectCallback> ConnectCallbackPtr;
    friend class ConnectCallback;

    std::vector<EndpointIPtr> applyOverrides(const std::vector<EndpointIPtr>&);
    Ice::ConnectionIPtr findConnection(const std::vector<EndpointIPtr>&, bool&);
    void incPendingConnectCount();
    void decPendingConnectCount();
    Ice::ConnectionIPtr getConnection(const std::vector<ConnectorInfo>&, const ConnectCallbackPtr&, bool&);
    void finishGetConnection(const std::vector<ConnectorInfo>&, const ConnectorInfo&, const Ice::ConnectionIPtr&,
                             const ConnectCallbackPtr&);
    void finishGetConnection(const std::vector<ConnectorInfo>&, const Ice::LocalException&, const ConnectCallbackPtr&);

    bool addToPending(const ConnectCallbackPtr&, const std::vector<ConnectorInfo>&);
    void removeFromPending(const ConnectCallbackPtr&, const std::vector<ConnectorInfo>&);

    Ice::ConnectionIPtr findConnection(const std::vector<ConnectorInfo>&, bool&);
    Ice::ConnectionIPtr createConnection(const TransceiverPtr&, const ConnectorInfo&);

    void handleException(const Ice::LocalException&, bool);
    void handleConnectionException(const Ice::LocalException&, bool);

    Ice::CommunicatorPtr _communicator;
    const InstancePtr _instance;
    const FactoryACMMonitorPtr _monitor;
    bool _destroyed;

    std::multimap<ConnectorPtr, Ice::ConnectionIPtr> _connections;
    std::map<ConnectorPtr, std::set<ConnectCallbackPtr> > _pending;

    std::multimap<EndpointIPtr, Ice::ConnectionIPtr> _connectionsByEndpoint;
    int _pendingConnectCount;
};

class IncomingConnectionFactory : public EventHandler,
                                  public Ice::ConnectionI::StartCallback,
                                  public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    void activate();
    void hold();
    void destroy();

#if TARGET_OS_IPHONE != 0
    void startAcceptor();
    void stopAcceptor();
#endif

    void updateConnectionObservers();

    void waitUntilHolding() const;
    void waitUntilFinished();

    EndpointIPtr endpoint() const;
    std::list<Ice::ConnectionIPtr> connections() const;
    void flushAsyncBatchRequests(const CommunicatorFlushBatchAsyncPtr&);

    //
    // Operations from EventHandler
    //

#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
    virtual bool startAsync(SocketOperation);
    virtual bool finishAsync(SocketOperation);
#endif

    virtual void message(ThreadPoolCurrent&);
    virtual void finished(ThreadPoolCurrent&, bool);
    virtual std::string toString() const;
    virtual NativeInfoPtr getNativeInfo();

    virtual void connectionStartCompleted(const Ice::ConnectionIPtr&);
    virtual void connectionStartFailed(const Ice::ConnectionIPtr&, const Ice::LocalException&);

private:

    IncomingConnectionFactory(const InstancePtr&, const EndpointIPtr&, const Ice::ObjectAdapterIPtr&);
    void initialize();
    virtual ~IncomingConnectionFactory();
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
    const FactoryACMMonitorPtr _monitor;

    AcceptorPtr _acceptor;
    const TransceiverPtr _transceiver;
    EndpointIPtr _endpoint;

#if TARGET_OS_IPHONE != 0
    bool _acceptorStarted;
#endif

    Ice::ObjectAdapterIPtr _adapter;

    const bool _warn;

    std::set<Ice::ConnectionIPtr> _connections;

    State _state;
};

}

#endif
