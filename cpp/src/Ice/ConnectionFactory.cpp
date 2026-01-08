// Copyright (c) ZeroC, Inc.

#include "ConnectionFactory.h"
#include "Acceptor.h"
#include "CommunicatorFlushBatchAsync.h"
#include "ConnectionI.h"
#include "Connector.h"
#include "DefaultsAndOverrides.h"
#include "EndpointI.h"
#include "Ice/Communicator.h"
#include "Ice/LocalExceptions.h"
#include "Ice/LoggerUtil.h"
#include "Ice/Properties.h"
#include "Instance.h"
#include "ObjectAdapterI.h"
#include "RouterInfo.h"
#include "ThreadPool.h"
#include "TraceLevels.h"
#include "Transceiver.h"

#include <algorithm>
#include <chrono>
#include <iterator>

#if TARGET_OS_IPHONE != 0
namespace IceInternal
{
    bool registerForBackgroundNotification(const IceInternal::IncomingConnectionFactoryPtr&);
    void unregisterForBackgroundNotification(const IceInternal::IncomingConnectionFactoryPtr&);
}
#endif

using namespace std;
using namespace Ice;
using namespace Ice::Instrumentation;
using namespace IceInternal;

namespace
{
    template<typename Map> void remove(Map& m, const typename Map::key_type& k, const typename Map::mapped_type& v)
    {
        auto pr = m.equal_range(k);
        assert(pr.first != pr.second);
        for (auto q = pr.first; q != pr.second; ++q)
        {
            if (q->second.get() == v.get())
            {
                m.erase(q);
                return;
            }
        }
        assert(false); // Nothing was removed which is an error.
    }

    template<typename Map, typename Predicate>
    typename Map::mapped_type find(const Map& m, const typename Map::key_type& k, Predicate predicate)
    {
        auto pr = m.equal_range(k);
        for (auto q = pr.first; q != pr.second; ++q)
        {
            if (predicate(q->second))
            {
                return q->second;
            }
        }
        return nullptr;
    }
}

bool
IceInternal::OutgoingConnectionFactory::ConnectorInfo::operator==(const ConnectorInfo& other) const
{
    return targetEqualTo(connector, other.connector);
}

void
IceInternal::OutgoingConnectionFactory::destroy()
{
    lock_guard lock(_mutex);

    if (_destroyed)
    {
        return;
    }

    for (const auto& [_, connection] : _connections)
    {
        connection->destroy(ConnectionI::CommunicatorDestroyed);
    }
    _destroyed = true;
    _communicator = nullptr;

    _conditionVariable.notify_all();
}

void
IceInternal::OutgoingConnectionFactory::updateConnectionObservers()
{
    lock_guard lock(_mutex);
    for (const auto& [_, connection] : _connections)
    {
        connection->updateObserver();
    }
}

void
IceInternal::OutgoingConnectionFactory::waitUntilFinished()
{
    multimap<ConnectorPtr, ConnectionIPtr, Ice::TargetCompare<ConnectorPtr, std::less>> connections;

    {
        unique_lock lock(_mutex);

        //
        // First we wait until the factory is destroyed. We also wait
        // until there are no pending connections anymore. Only then
        // we can be sure the _connections contains all connections.
        //
        _conditionVariable.wait(lock, [this] { return _destroyed && _pending.empty() && _pendingConnectCount == 0; });

        //
        // We want to wait until all connections are finished outside the
        // thread synchronization.
        //
        connections = _connections;
    }

    for (const auto& p : connections)
    {
        p.second->waitUntilFinished();
    }

    {
        lock_guard lock(_mutex);
        _connections.clear();
        _connectionsByEndpoint.clear();
    }
}

void
IceInternal::OutgoingConnectionFactory::createAsync(
    vector<EndpointIPtr> endpoints,
    bool hasMore,
    function<void(Ice::ConnectionIPtr, bool)> response,
    function<void(std::exception_ptr)> exception)
{
    assert(!endpoints.empty());

    //
    // Try to find a connection to one of the given endpoints.
    //
    try
    {
        bool compress;
        Ice::ConnectionIPtr connection = findConnection(endpoints, compress);
        if (connection)
        {
            response(std::move(connection), compress);
            return;
        }
    }
    catch (...)
    {
        exception(current_exception());
        return;
    }

    auto cb = make_shared<ConnectCallback>(
        _instance,
        shared_from_this(),
        std::move(endpoints),
        hasMore,
        std::move(response),
        std::move(exception));
    cb->getConnectors();
}

void
IceInternal::OutgoingConnectionFactory::setRouterInfo(const RouterInfoPtr& routerInfo)
{
    assert(routerInfo);
    ObjectAdapterPtr adapter = routerInfo->getAdapter();
    vector<EndpointIPtr> endpoints = routerInfo->getClientEndpoints(); // Must be called outside the synchronization

    lock_guard lock(_mutex);

    if (_destroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    //
    // Search for connections to the router's client proxy endpoints,
    // and update the object adapter for such connections, so that
    // callbacks from the router can be received over such
    // connections.
    //
    for (auto endpoint : endpoints)
    {
        //
        // The Connection object does not take the compression flag of
        // endpoints into account, but instead gets the information
        // about whether messages should be compressed or not from
        // other sources. In order to allow connection sharing for
        // endpoints that differ in the value of the compression flag
        // only, we always set the compression flag to false here in
        // this connection factory. We also clear the timeout as it is
        // no longer used for Ice 3.8 or greater.
        //
        endpoint = endpoint->compress(false)->timeout(-1);

        for (const auto& [_, connection] : _connections)
        {
            if (connection->endpoint() == endpoint)
            {
                connection->setAdapter(adapter);
            }
        }
    }
}

void
IceInternal::OutgoingConnectionFactory::removeAdapter(const ObjectAdapterPtr& adapter)
{
    lock_guard lock(_mutex);

    if (_destroyed)
    {
        return;
    }

    for (const auto& [_, connection] : _connections)
    {
        if (connection->getAdapter() == adapter)
        {
            connection->setAdapter(nullptr);
        }
    }
}

void
IceInternal::OutgoingConnectionFactory::flushAsyncBatchRequests(
    const CommunicatorFlushBatchAsyncPtr& outAsync,
    Ice::CompressBatch compress)
{
    list<ConnectionIPtr> c;

    {
        lock_guard lock(_mutex);
        for (const auto& [_, connection] : _connections)
        {
            if (connection->isActiveOrHolding())
            {
                c.push_back(connection);
            }
        }
    }

    for (const auto& p : c)
    {
        try
        {
            outAsync->flushConnection(p, compress);
        }
        catch (const LocalException&)
        {
            // Ignore.
        }
    }
}

void
IceInternal::OutgoingConnectionFactory::removeConnection(const ConnectionIPtr& connection) noexcept
{
    lock_guard lock(_mutex);
    if (!_destroyed)
    {
        remove(_connections, connection->connector(), connection);
        remove(_connectionsByEndpoint, connection->endpoint(), connection);
        remove(_connectionsByEndpoint, connection->endpoint()->compress(true), connection);
    }
}

IceInternal::OutgoingConnectionFactory::OutgoingConnectionFactory(
    CommunicatorPtr communicator,
    const InstancePtr& instance)
    : _communicator(std::move(communicator)),
      _instance(instance),
      _connectionOptions(instance->clientConnectionOptions())
{
}

IceInternal::OutgoingConnectionFactory::~OutgoingConnectionFactory()
{
    assert(_destroyed);
    assert(_connections.empty());
    assert(_connectionsByEndpoint.empty());
    assert(_pending.empty());
    assert(_pendingConnectCount == 0);
}

ObjectAdapterPtr
IceInternal::OutgoingConnectionFactory::getDefaultObjectAdapter() const noexcept
{
    lock_guard lock(_mutex);
    return _defaultObjectAdapter;
}

void
IceInternal::OutgoingConnectionFactory::setDefaultObjectAdapter(
    ObjectAdapterPtr adapter) noexcept // NOLINT(performance-unnecessary-value-param)
{
    lock_guard lock(_mutex);
    _defaultObjectAdapter = dynamic_pointer_cast<ObjectAdapterI>(adapter);
}

ConnectionIPtr
IceInternal::OutgoingConnectionFactory::findConnection(const vector<EndpointIPtr>& endpoints, bool& compress)
{
    lock_guard lock(_mutex);
    if (_destroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
    assert(!endpoints.empty());

    for (const auto& p : endpoints)
    {
        auto connection = find(
            _connectionsByEndpoint,
            p->timeout(-1), // clear the timeout
            [](const ConnectionIPtr& conn) { return conn->isActiveOrHolding(); });

        if (connection)
        {
            if (defaultsAndOverrides->overrideCompress.has_value())
            {
                compress = *defaultsAndOverrides->overrideCompress;
            }
            else
            {
                compress = p->compress();
            }
            return connection;
        }
    }
    return nullptr;
}

ConnectionIPtr
IceInternal::OutgoingConnectionFactory::findConnection(const vector<ConnectorInfo>& connectors, bool& compress)
{
    // This must be called with the mutex locked.

    DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
    for (const auto& p : connectors)
    {
        if (_pending.find(p.connector) != _pending.end())
        {
            continue;
        }

        auto connection =
            find(_connections, p.connector, [](const ConnectionIPtr& conn) { return conn->isActiveOrHolding(); });
        if (connection)
        {
            if (defaultsAndOverrides->overrideCompress.has_value())
            {
                compress = *defaultsAndOverrides->overrideCompress;
            }
            else
            {
                compress = p.endpoint->compress();
            }
            return connection;
        }
    }

    return nullptr;
}

void
IceInternal::OutgoingConnectionFactory::incPendingConnectCount()
{
    //
    // Keep track of the number of pending connects. The outgoing connection factory
    // waitUntilFinished() method waits for all the pending connects to terminate before
    // to return. This ensures that the communicator client thread pool isn't destroyed
    // too soon and will still be available to execute the ice_exception() callbacks for
    // the asynchronous requests waiting on a connection to be established.
    //

    lock_guard lock(_mutex);
    if (_destroyed)
    {
        throw Ice::CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    ++_pendingConnectCount;
}

void
IceInternal::OutgoingConnectionFactory::decPendingConnectCount()
{
    lock_guard lock(_mutex);
    --_pendingConnectCount;
    assert(_pendingConnectCount >= 0);
    if (_destroyed && _pendingConnectCount == 0)
    {
        _conditionVariable.notify_all();
    }
}

ConnectionIPtr
IceInternal::OutgoingConnectionFactory::getConnection(
    const vector<ConnectorInfo>& connectors,
    const ConnectCallbackPtr& cb,
    bool& compress)
{
    {
        assert(cb);
        unique_lock lock(_mutex);
        if (_destroyed)
        {
            throw Ice::CommunicatorDestroyedException(__FILE__, __LINE__);
        }

        // Search for an existing connections matching one of the given endpoints.
        Ice::ConnectionIPtr connection = findConnection(connectors, compress);
        if (connection)
        {
            return connection;
        }

        //
        // Determine whether another thread/request is currently attempting to connect to
        // one of our endpoints; if so we wait until it's done.
        //
        if (addToPending(cb, connectors))
        {
            // A connection to one of our endpoints is pending. The callback will be notified once the connection
            // is established. Returning null indicates that the connection is still pending.
            return nullptr;
        }
    }

    // No connection is pending. Call nextConnector to initiate connection establishment. Return null to indicate
    // that the connection is still pending.
    cb->nextConnector();
    return nullptr;
}

ConnectionIPtr
IceInternal::OutgoingConnectionFactory::createConnection(const TransceiverPtr& transceiver, const ConnectorInfo& ci)
{
    lock_guard lock(_mutex);
    assert(_pending.find(ci.connector) != _pending.end() && transceiver);

    //
    // Create and add the connection to the connection map. Adding the connection to the map
    // is necessary to support the interruption of the connection initialization and validation
    // in case the communicator is destroyed.
    //
    Ice::ConnectionIPtr connection;
    try
    {
        if (_destroyed)
        {
            throw Ice::CommunicatorDestroyedException(__FILE__, __LINE__);
        }

        // The connect, close and idle timeouts are ignored for UDP connections.

        connection = ConnectionI::create(
            _communicator,
            _instance,
            transceiver,
            ci.connector,
            ci.endpoint->compress(false)->timeout(-1),
            _defaultObjectAdapter,
            [weakSelf = weak_from_this()](const ConnectionIPtr& closedConnection)
            {
                if (auto self = weakSelf.lock())
                {
                    self->removeConnection(closedConnection);
                }
            },
            _connectionOptions);
    }
    catch (const Ice::LocalException&)
    {
        try
        {
            transceiver->close();
        }
        catch (const Ice::LocalException&)
        {
            // Ignore
        }
        throw;
    }

    _connections.insert(pair<const ConnectorPtr, ConnectionIPtr>(ci.connector, connection));
    _connectionsByEndpoint.insert(pair<const EndpointIPtr, ConnectionIPtr>(connection->endpoint(), connection));
    _connectionsByEndpoint.insert(
        pair<const EndpointIPtr, ConnectionIPtr>(connection->endpoint()->compress(true), connection));
    return connection;
}

void
IceInternal::OutgoingConnectionFactory::finishGetConnection(
    const vector<ConnectorInfo>& connectors,
    const ConnectorInfo& ci,
    const ConnectionIPtr& connection,
    const ConnectCallbackPtr& cb)
{
    ConnectCallbackSet connectionCallbacks;
    if (cb)
    {
        connectionCallbacks.insert(cb);
    }

    ConnectCallbackSet callbacks;
    {
        lock_guard lock(_mutex);
        for (const auto& connector : connectors)
        {
            auto q = _pending.find(connector.connector);
            if (q != _pending.end())
            {
                for (const auto& r : q->second)
                {
                    if (r->hasConnector(ci))
                    {
                        connectionCallbacks.insert(r);
                    }
                    else
                    {
                        callbacks.insert(r);
                    }
                }
                _pending.erase(q);
            }
        }

        for (const auto& connectionCallback : connectionCallbacks)
        {
            connectionCallback->removeFromPending();
            callbacks.erase(connectionCallback);
        }
        for (const auto& callback : callbacks)
        {
            callback->removeFromPending();
        }
        _conditionVariable.notify_all();
    }

    bool compress;
    DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
    if (defaultsAndOverrides->overrideCompress.has_value())
    {
        compress = *defaultsAndOverrides->overrideCompress;
    }
    else
    {
        compress = ci.endpoint->compress();
    }

    for (const auto& callback : callbacks)
    {
        callback->getConnection();
    }
    for (const auto& connectionCallback : connectionCallbacks)
    {
        connectionCallback->setConnection(connection, compress);
    }
}

void
IceInternal::OutgoingConnectionFactory::finishGetConnection(
    const vector<ConnectorInfo>& connectors,
    std::exception_ptr ex,
    const ConnectCallbackPtr& cb)
{
    ConnectCallbackSet failedCallbacks;
    if (cb)
    {
        failedCallbacks.insert(cb);
    }

    ConnectCallbackSet callbacks;
    {
        lock_guard lock(_mutex);
        for (auto p = connectors.begin(); p != connectors.end(); ++p)
        {
            auto q = _pending.find(p->connector);
            if (q != _pending.end())
            {
                for (const auto& r : q->second)
                {
                    if (r->removeConnectors(connectors))
                    {
                        failedCallbacks.insert(r);
                    }
                    else
                    {
                        callbacks.insert(r);
                    }
                }
                _pending.erase(q);
            }
        }

        for (const auto& callback : callbacks)
        {
            assert(failedCallbacks.find(callback) == failedCallbacks.end());
            callback->removeFromPending();
        }
        _conditionVariable.notify_all();
    }

    for (const auto& callback : callbacks)
    {
        callback->getConnection();
    }
    for (const auto& failedCallback : failedCallbacks)
    {
        failedCallback->setException(ex);
    }
}

bool
IceInternal::OutgoingConnectionFactory::addToPending(
    const ConnectCallbackPtr& cb,
    const vector<ConnectorInfo>& connectors)
{
    //
    // Add the callback to each connector pending list.
    //
    bool found = false;
    for (const auto& connector : connectors)
    {
        auto q = _pending.find(connector.connector);
        if (q != _pending.end())
        {
            found = true;
            if (cb)
            {
                q->second.insert(cb);
            }
        }
    }

    if (found)
    {
        return true;
    }

    //
    // If there's no pending connection for the given connectors, we're
    // responsible for its establishment. We add empty pending lists,
    // other callbacks to the same connectors will be queued.
    //
    for (const auto& connector : connectors)
    {
        if (_pending.find(connector.connector) == _pending.end())
        {
            _pending.insert(make_pair(connector.connector, ConnectCallbackSet()));
        }
    }
    return false;
}

void
IceInternal::OutgoingConnectionFactory::removeFromPending(
    const ConnectCallbackPtr& cb,
    const vector<ConnectorInfo>& connectors)
{
    for (const auto& connector : connectors)
    {
        auto q = _pending.find(connector.connector);
        if (q != _pending.end())
        {
            q->second.erase(cb);
        }
    }
}

void
IceInternal::OutgoingConnectionFactory::handleException(exception_ptr ex, bool hasMore)
{
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if (traceLevels->network >= 2)
    {
        Trace out(_instance->initializationData().logger, traceLevels->networkCat);

        out << "couldn't resolve endpoint host";

        try
        {
            rethrow_exception(ex);
        }
        catch (const CommunicatorDestroyedException& e)
        {
            out << "\n" << e;
        }
        catch (const std::exception& e)
        {
            if (hasMore)
            {
                out << ", trying next endpoint\n";
            }
            else
            {
                out << " and no more endpoints to try\n";
            }
            out << e;
        }
    }
}

void
IceInternal::OutgoingConnectionFactory::handleConnectionException(exception_ptr ex, bool hasMore)
{
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if (traceLevels->network >= 2)
    {
        Trace out(_instance->initializationData().logger, traceLevels->networkCat);

        out << "connection to endpoint failed";

        try
        {
            rethrow_exception(ex);
        }
        catch (const CommunicatorDestroyedException& e)
        {
            out << "\n" << e;
        }
        catch (const std::exception& e)
        {
            if (hasMore)
            {
                out << ", trying next endpoint\n";
            }
            else
            {
                out << " and no more endpoints to try\n";
            }
            out << e;
        }
    }
}

IceInternal::OutgoingConnectionFactory::ConnectCallback::ConnectCallback(
    InstancePtr instance,
    OutgoingConnectionFactoryPtr factory,
    vector<EndpointIPtr> endpoints,
    bool hasMore,
    std::function<void(Ice::ConnectionIPtr, bool)> createConnectionResponse,
    std::function<void(std::exception_ptr)> createConnectionException)
    : _instance(std::move(instance)),
      _factory(std::move(factory)),
      _endpoints(std::move(endpoints)),
      _hasMore(hasMore),
      _createConnectionResponse(std::move(createConnectionResponse)),
      _createConnectionException(std::move(createConnectionException))
{
    _endpointsIter = _endpoints.begin();
}

//
// Methods from ConnectionI.StartCallback
//
void
IceInternal::OutgoingConnectionFactory::ConnectCallback::connectionStartCompleted(const ConnectionIPtr& connection)
{
    if (_observer)
    {
        _observer->detach();
    }

    connection->activate();
    _factory->finishGetConnection(_connectors, *_iter, connection, shared_from_this());
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::connectionStartFailed(
    const ConnectionIPtr& /*connection*/,
    exception_ptr ex)
{
    assert(_iter != _connectors.end());
    if (connectionStartFailedImpl(ex))
    {
        nextConnector();
    }
}

//
// Methods from EndpointI_connectors
//
void
IceInternal::OutgoingConnectionFactory::ConnectCallback::connectors(const vector<ConnectorPtr>& connectors)
{
    for (const auto& connector : connectors)
    {
        _connectors.emplace_back(connector, *_endpointsIter);
    }

    if (++_endpointsIter != _endpoints.end())
    {
        nextEndpoint();
    }
    else
    {
        assert(!_connectors.empty());

        //
        // We now have all the connectors for the given endpoints. We can try to obtain the
        // connection.
        //
        _iter = _connectors.begin();
        getConnection();
    }
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::exception(exception_ptr ex)
{
    _factory->handleException(ex, _hasMore || _endpointsIter != _endpoints.end() - 1);
    if (++_endpointsIter != _endpoints.end())
    {
        nextEndpoint();
    }
    else if (!_connectors.empty())
    {
        //
        // We now have all the connectors for the given endpoints. We can try to obtain the
        // connection.
        //
        _iter = _connectors.begin();
        getConnection();
    }
    else
    {
        _createConnectionException(ex);
        _factory->decPendingConnectCount(); // Must be called last.
    }
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::getConnectors()
{
    try
    {
        //
        // Notify the factory that there's an async connect pending. This is necessary
        // to prevent the outgoing connection factory to be destroyed before all the
        // pending asynchronous connects are finished.
        //
        _factory->incPendingConnectCount();
    }
    catch (const std::exception&)
    {
        _createConnectionException(current_exception());
        return;
    }

    nextEndpoint();
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::nextEndpoint()
{
    try
    {
        auto self = shared_from_this();
        assert(_endpointsIter != _endpoints.end());
        (*_endpointsIter)
            ->connectorsAsync(
                [self](const vector<ConnectorPtr>& connectors) { self->connectors(connectors); },
                [self](exception_ptr ex) { self->exception(ex); });
    }
    catch (const std::exception&)
    {
        exception(current_exception());
    }
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::getConnection()
{
    try
    {
        //
        // If all the connectors have been created, we ask the factory to get a
        // connection.
        //
        bool compress;
        Ice::ConnectionIPtr connection = _factory->getConnection(_connectors, shared_from_this(), compress);
        if (!connection)
        {
            //
            // A null return value from getConnection indicates that the connection
            // is being established and that everthing has been done to ensure that
            // the callback will be notified when the connection establishment is
            // done or that the callback already obtain the connection.
            //
            return;
        }

        _createConnectionResponse(connection, compress);
        _factory->decPendingConnectCount(); // Must be called last.
    }
    catch (const std::exception&)
    {
        _createConnectionException(current_exception());
        _factory->decPendingConnectCount(); // Must be called last.
    }
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::nextConnector()
{
    while (true)
    {
        try
        {
            const CommunicatorObserverPtr& obsv = _factory->_instance->initializationData().observer;
            if (obsv)
            {
                _observer = obsv->getConnectionEstablishmentObserver(_iter->endpoint, _iter->connector->toString());
                if (_observer)
                {
                    _observer->attach();
                }
            }

            assert(_iter != _connectors.end());

            if (_instance->traceLevels()->network >= 2)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                out << "trying to establish " << _iter->endpoint->protocol() << " connection to "
                    << _iter->connector->toString();
            }
            Ice::ConnectionIPtr connection = _factory->createConnection(_iter->connector->connect(), *_iter);
            auto self = shared_from_this();
            connection->startAsync(
                [self](const ConnectionIPtr& conn) { self->connectionStartCompleted(conn); },
                [self](const ConnectionIPtr& conn, exception_ptr ex) { self->connectionStartFailed(conn, ex); });
        }
        catch (const Ice::LocalException& ex)
        {
            if (_instance->traceLevels()->network >= 2)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                out << "failed to establish " << _iter->endpoint->protocol() << " connection to "
                    << _iter->connector->toString() << "\n"
                    << ex;
            }

            if (connectionStartFailedImpl(current_exception()))
            {
                continue; // More connectors to try, continue.
            }
        }
        break;
    }
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::setConnection(
    const Ice::ConnectionIPtr& connection,
    bool compress)
{
    //
    // Callback from the factory: the connection to one of the callback
    // connectors has been established.
    //
    _createConnectionResponse(connection, compress);
    _factory->decPendingConnectCount(); // Must be called last.
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::setException(exception_ptr ex)
{
    //
    // Callback from the factory: connection establishment failed.
    //
    _createConnectionException(ex);
    _factory->decPendingConnectCount(); // Must be called last.
}

bool
IceInternal::OutgoingConnectionFactory::ConnectCallback::hasConnector(const ConnectorInfo& ci)
{
    return find(_connectors.begin(), _connectors.end(), ci) != _connectors.end();
}

bool
IceInternal::OutgoingConnectionFactory::ConnectCallback::removeConnectors(const vector<ConnectorInfo>& connectors)
{
    //
    // Callback from the factory: connecting to the given connectors
    // failed, we remove the connectors and return true if there's
    // no more connectors left to try.
    //
    for (const auto& p : connectors)
    {
        _connectors.erase(remove(_connectors.begin(), _connectors.end(), p), _connectors.end());
    }
    return _connectors.empty();
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::removeFromPending()
{
    _factory->removeFromPending(shared_from_this(), _connectors);
}

bool
IceInternal::OutgoingConnectionFactory::ConnectCallback::connectionStartFailedImpl(std::exception_ptr ex)
{
    if (_observer)
    {
        _observer->failed(getExceptionId(ex));
        _observer->detach();
    }

    _factory->handleConnectionException(ex, _hasMore || _iter != _connectors.end() - 1);

    bool keepGoing = ++_iter != _connectors.end();

    if (keepGoing)
    {
        try
        {
            rethrow_exception(ex);
        }
        catch (const CommunicatorDestroyedException&)
        {
            keepGoing = false;
        }
        catch (const ConnectTimeoutException&)
        {
            // We stop on ConnectTimeoutException to fail reasonably fast when the endpoint has many connectors
            // (IP addresses).
            keepGoing = false;
        }
        catch (...)
        {
        }
    }

    if (!keepGoing)
    {
        _factory->finishGetConnection(_connectors, ex, shared_from_this());
    }
    return keepGoing;
}

void
IceInternal::IncomingConnectionFactory::activate()
{
    lock_guard lock(_mutex);
    setState(StateActive);
}

void
IceInternal::IncomingConnectionFactory::hold()
{
    lock_guard lock(_mutex);
    setState(StateHolding);
}

void
IceInternal::IncomingConnectionFactory::destroy()
{
    lock_guard lock(_mutex);
    setState(StateClosed);
}

void
IceInternal::IncomingConnectionFactory::updateConnectionObservers()
{
    lock_guard lock(_mutex);
    for (const auto& connection : _connections)
    {
        connection->updateObserver();
    }
}

void
IceInternal::IncomingConnectionFactory::waitUntilHolding() const
{
    set<ConnectionIPtr> connections;

    {
        unique_lock lock(_mutex);

        //
        // First we wait until the connection factory itself is in holding
        // state.
        //
        _conditionVariable.wait(lock, [this] { return _state >= StateHolding; });

        //
        // We want to wait until all connections are in holding state
        // outside the thread synchronization.
        //
        connections = _connections;
    }

    //
    // Now we wait until each connection is in holding state.
    //
    for (const auto& conn : connections)
    {
        conn->waitUntilHolding();
    }
}

void
IceInternal::IncomingConnectionFactory::waitUntilFinished()
{
    set<ConnectionIPtr> connections;
    {
        unique_lock lock(_mutex);

        //
        // First we wait until the factory is destroyed. If we are using
        // an acceptor, we also wait for it to be closed.
        //
        _conditionVariable.wait(lock, [this] { return _state == StateFinished; });

        //
        // Clear the OA. See bug 1673 for the details of why this is necessary.
        //
        _adapter = nullptr;

        // We want to wait until all connections are finished outside the
        // thread synchronization.
        //
        connections = _connections;
    }

    for (const auto& conn : connections)
    {
        conn->waitUntilFinished();
    }

    {
        lock_guard lock(_mutex);
        _connections.clear();
    }
}

EndpointIPtr
IceInternal::IncomingConnectionFactory::endpoint() const
{
    lock_guard lock(_mutex);
    return _endpoint;
}

list<ConnectionIPtr>
IceInternal::IncomingConnectionFactory::connections() const
{
    lock_guard lock(_mutex);

    list<ConnectionIPtr> result;

    //
    // Only copy connections which have not been destroyed.
    //
    remove_copy_if(
        _connections.begin(),
        _connections.end(),
        back_inserter(result),
        [](const ConnectionIPtr& conn) { return !conn->isActiveOrHolding(); });
    return result;
}

void
IceInternal::IncomingConnectionFactory::removeConnection(const ConnectionIPtr& connection) noexcept
{
    lock_guard lock(_mutex);
    if (_state == StateActive || _state == StateHolding)
    {
        _connections.erase(connection);
    }
    // else it's already being cleaned up.
}

void
IceInternal::IncomingConnectionFactory::flushAsyncBatchRequests(
    const CommunicatorFlushBatchAsyncPtr& outAsync,
    Ice::CompressBatch compress)
{
    list<ConnectionIPtr> c = connections(); // connections() is synchronized, so no need to synchronize here.

    for (const auto& p : c)
    {
        try
        {
            outAsync->flushConnection(p, compress);
        }
        catch (const LocalException&)
        {
            // Ignore.
        }
    }
}

#if defined(ICE_USE_IOCP)
bool
IceInternal::IncomingConnectionFactory::startAsync(SocketOperation)
{
    assert(_acceptor);
    if (_state >= StateClosed)
    {
        return false;
    }

    try
    {
        _acceptor->startAccept();
    }
    catch (const Ice::LocalException&)
    {
        _acceptorException = current_exception();
        _acceptor->getNativeInfo()->completed(SocketOperationRead);
    }
    return true;
}

bool
IceInternal::IncomingConnectionFactory::finishAsync(SocketOperation)
{
    assert(_acceptor);
    try
    {
        if (_acceptorException)
        {
            rethrow_exception(_acceptorException);
        }
        _acceptor->finishAccept();
    }
    catch (const LocalException& ex)
    {
        _acceptorException = nullptr;

        if (_warn)
        {
            Warning out(_instance->initializationData().logger);
            out << "error accepting connection:\n" << ex << '\n' << _acceptor->toString();
        }
    }
    return _state < StateClosed;
}
#endif

void
IceInternal::IncomingConnectionFactory::message(ThreadPoolCurrent& current)
{
    ConnectionIPtr connection;

    ThreadPoolMessage<IncomingConnectionFactory> msg(current, *this);

    {
        lock_guard lock(_mutex);

        ThreadPoolMessage<IncomingConnectionFactory>::IOScope io(msg);
        if (!io)
        {
            return;
        }

        if (_state >= StateClosed)
        {
            return;
        }
        else if (_state == StateHolding)
        {
            this_thread::yield();
            return;
        }

        if (!_acceptorStarted)
        {
            return;
        }

        //
        // Now accept a new connection.
        //
        TransceiverPtr transceiver;
        try
        {
            transceiver = _acceptor->accept();

            if (_maxConnections > 0 && _connections.size() == static_cast<size_t>(_maxConnections))
            {
                // Can't accept more connections, so we abort this transport connection.

                if (_instance->traceLevels()->network >= 2)
                {
                    Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                    out << "rejecting new " << _endpoint->protocol() << " connection\n"
                        << transceiver->toString() << "\nbecause the maximum number of connections has been reached";
                }

                try
                {
                    transceiver->close();
                }
                catch (...)
                {
                    // Ignore.
                }
                return;
            }

            if (_instance->traceLevels()->network >= 2)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                out << "trying to accept " << _endpoint->protocol() << " connection\n" << transceiver->toString();
            }
        }
        catch (const LocalException& ex)
        {
            if (_warn)
            {
                Warning out(_instance->initializationData().logger);
                out << "error accepting connection:\n" << ex << '\n' << _acceptor->toString();
            }
            return;
        }

        assert(transceiver);

        try
        {
            connection = ConnectionI::create(
                _adapter->getCommunicator(),
                _instance,
                transceiver,
                nullptr, // connector
                _endpoint,
                _adapter,
                [weakSelf = weak_from_this()](const ConnectionIPtr& closedConnection)
                {
                    if (auto self = weakSelf.lock())
                    {
                        static_pointer_cast<IncomingConnectionFactory>(self)->removeConnection(closedConnection);
                    }
                },
                _connectionOptions);
        }
        catch (const LocalException& ex)
        {
            try
            {
                transceiver->close();
            }
            catch (const Ice::LocalException&)
            {
                // Ignore.
            }

            if (_warn)
            {
                Warning out(_instance->initializationData().logger);
                out << "error accepting connection:\n" << ex << '\n' << _acceptor->toString();
            }
            return;
        }

        _connections.insert(connection);
    }

    assert(connection);

    auto self = shared_from_this();
    connection->startAsync(
        [self](const ConnectionIPtr& conn) { self->connectionStartCompleted(conn); },
        [self](const ConnectionIPtr& conn, exception_ptr ex) { self->connectionStartFailed(conn, ex); });
}

void
IceInternal::IncomingConnectionFactory::finished(ThreadPoolCurrent&, bool close)
{
    unique_lock lock(_mutex);
    if (_state < StateClosed)
    {
        if (close)
        {
            closeAcceptor();
        }
        return;
    }

    assert(_state >= StateClosed);
    setState(StateFinished);

    if (close)
    {
        closeAcceptor();
    }

#if TARGET_OS_IPHONE != 0
    lock.unlock();
    finish();
#endif
}

#if TARGET_OS_IPHONE != 0
void
IceInternal::IncomingConnectionFactory::finish()
{
    unregisterForBackgroundNotification(shared_from_this());
}
#endif

string
IceInternal::IncomingConnectionFactory::toString() const
{
    lock_guard lock(_mutex);
    if (_transceiver)
    {
        return _transceiver->toString();
    }
    else if (_acceptor)
    {
        return _acceptor->toString();
    }
    else
    {
        return {};
    }
}

NativeInfoPtr
IceInternal::IncomingConnectionFactory::getNativeInfo()
{
    if (_transceiver)
    {
        return _transceiver->getNativeInfo();
    }
    else if (_acceptor)
    {
        return _acceptor->getNativeInfo();
    }
    else
    {
        return nullptr;
    }
}

void
IceInternal::IncomingConnectionFactory::connectionStartCompleted(const Ice::ConnectionIPtr& connection)
{
    lock_guard lock(_mutex);

    //
    // Initially, connections are in the holding state. If the factory is active
    // we activate the connection.
    //
    if (_state == StateActive)
    {
        connection->activate();
    }
}

void
IceInternal::IncomingConnectionFactory::connectionStartFailed(const Ice::ConnectionIPtr& /*connection*/, exception_ptr)
{
    // Do not warn about connection exceptions here. The connection is not yet validated.
}

//
// COMPILERFIX: The ConnectionFactory setup is broken out into a separate initialize
// function because when it was part of the constructor C++Builder 2007 apps would
// crash if an exception was thrown from any calls within the constructor.
//
IceInternal::IncomingConnectionFactory::IncomingConnectionFactory(
    const InstancePtr& instance,
    const EndpointIPtr& endpoint,
    const shared_ptr<ObjectAdapterI>& adapter)
    : _instance(instance),
      _connectionOptions(instance->serverConnectionOptions(adapter->getName())),
      _maxConnections(
          endpoint->datagram()
              ? 0
              : instance->initializationData().properties->getPropertyAsInt(adapter->getName() + ".MaxConnections")),
      _endpoint(endpoint),
      _adapter(adapter),
      _warn(_instance->initializationData().properties->getIcePropertyAsInt("Ice.Warn.Connections") > 0)
{
}

#if defined(__APPLE__) && TARGET_OS_IPHONE != 0
void
IceInternal::IncomingConnectionFactory::startAcceptor()
{
    lock_guard lock(_mutex);
    if (_state >= StateClosed || _acceptorStarted)
    {
        return;
    }

    _acceptorStopped = false;
    createAcceptor();
}

void
IceInternal::IncomingConnectionFactory::stopAcceptor()
{
    lock_guard lock(_mutex);
    if (_state >= StateClosed || !_acceptorStarted)
    {
        return;
    }

    _acceptorStopped = true;
    _acceptorStarted = false;
    if (_adapter->getThreadPool()->finish(shared_from_this(), true))
    {
        closeAcceptor();
    }
}
#endif

void
IceInternal::IncomingConnectionFactory::initialize()
{
    if (_instance->defaultsAndOverrides()->overrideCompress.has_value())
    {
        _endpoint = _endpoint->compress(*_instance->defaultsAndOverrides()->overrideCompress);
    }

    try
    {
        const_cast<TransceiverPtr&>(_transceiver) = _endpoint->transceiver();
        if (_transceiver)
        {
            // All this is for UDP "connections".

            if (_instance->traceLevels()->network >= 2)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                out << "attempting to bind to " << _endpoint->protocol() << " socket\n" << _transceiver->toString();
            }
            const_cast<EndpointIPtr&>(_endpoint) = _transceiver->bind();
            ConnectionIPtr connection(ConnectionI::create(
                _adapter->getCommunicator(),
                _instance,
                _transceiver,
                nullptr,
                _endpoint,
                _adapter,
                nullptr,
                _connectionOptions));
            connection->startAsync(nullptr, nullptr);
            _connections.insert(connection);
            assert(_maxConnections == 0); // UDP so no max connections
        }
        else
        {
#if TARGET_OS_IPHONE != 0
            //
            // The notification center will call back on the factory to
            // start the acceptor if necessary.
            //
            registerForBackgroundNotification(shared_from_this());
#else
            createAcceptor();
#endif
        }
    }
    catch (const Ice::Exception&)
    {
        if (_transceiver)
        {
            try
            {
                _transceiver->close();
            }
            catch (const Ice::LocalException&)
            {
                // Ignore
            }
        }

        _state = StateFinished;
        _connections.clear();
        throw;
    }
}

IceInternal::IncomingConnectionFactory::~IncomingConnectionFactory()
{
    assert(_state == StateFinished);
    assert(_connections.empty());
}

void
IceInternal::IncomingConnectionFactory::setState(State state)
{
    if (_state == state) // Don't switch twice.
    {
        return;
    }

    switch (state)
    {
        case StateActive:
        {
            if (_state != StateHolding) // Can only switch from holding to active.
            {
                return;
            }
            if (_acceptor)
            {
                if (_instance->traceLevels()->network >= 1)
                {
                    Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                    out << "accepting " << _endpoint->protocol() << " connections at " << _acceptor->toString();
                }
                _adapter->getThreadPool()->_register(shared_from_this(), SocketOperationRead);
            }

            for (const auto& connection : _connections)
            {
                connection->activate();
            }
            break;
        }

        case StateHolding:
        {
            if (_state != StateActive) // Can only switch from active to holding.
            {
                return;
            }
            if (_acceptor)
            {
                if (_instance->traceLevels()->network >= 1)
                {
                    Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                    out << "holding " << _endpoint->protocol() << " connections at " << _acceptor->toString();
                }
                _adapter->getThreadPool()->unregister(shared_from_this(), SocketOperationRead);
            }
            for (const auto& connection : _connections)
            {
                connection->hold();
            }
            break;
        }

        case StateClosed:
        {
            if (_acceptorStarted)
            {
                //
                // If possible, close the acceptor now to prevent new connections from
                // being accepted while we are deactivating. This is especially useful
                // if there are no more threads in the thread pool available to dispatch
                // the finish() call. Not all selector implementations do support this
                // however.
                //
                _acceptorStarted = false;
                if (_adapter->getThreadPool()->finish(shared_from_this(), true))
                {
                    closeAcceptor();
                }
            }
            else
            {
#if TARGET_OS_IPHONE != 0
                _adapter->getThreadPool()->execute([self = shared_from_this()]() { self->finish(); }, nullptr);
#endif
                state = StateFinished;
            }

            for (const auto& connection : _connections)
            {
                connection->destroy(ConnectionI::ObjectAdapterDeactivated);
            }
            break;
        }

        case StateFinished:
        {
            assert(_state == StateClosed);
            break;
        }
    }

    _state = state;
    _conditionVariable.notify_all();
}

void
IceInternal::IncomingConnectionFactory::createAcceptor()
{
    try
    {
        assert(!_acceptorStarted);
        _acceptor = _endpoint->acceptor(_adapter->getName(), _adapter->serverAuthenticationOptions());
        assert(_acceptor);
        if (_instance->traceLevels()->network >= 2)
        {
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
            out << "attempting to bind to " << _endpoint->protocol() << " socket " << _acceptor->toString();
        }

        _endpoint = _acceptor->listen();
        if (_instance->traceLevels()->network >= 1)
        {
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
            out << "listening for " << _endpoint->protocol() << " connections\n" << _acceptor->toDetailedString();
        }

        _adapter->getThreadPool()->initialize(shared_from_this());
        if (_state == StateActive)
        {
            _adapter->getThreadPool()->_register(shared_from_this(), SocketOperationRead);
        }

        _acceptorStarted = true;
    }
    catch (const Ice::Exception&)
    {
        if (_acceptor)
        {
            _acceptor->close();
        }
        throw;
    }
}

void
IceInternal::IncomingConnectionFactory::closeAcceptor()
{
    assert(_acceptor);

    if (_instance->traceLevels()->network >= 1)
    {
        Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
        out << "stopping to accept " << _endpoint->protocol() << " connections at " << _acceptor->toString();
    }

    assert(!_acceptorStarted);
    _acceptor->close();
}
