// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ConnectionFactory.h>
#include <Ice/ConnectionI.h>
#include <Ice/Instance.h>
#include <Ice/LoggerUtil.h>
#include <Ice/TraceLevels.h>
#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Properties.h>
#include <Ice/Transceiver.h>
#include <Ice/Connector.h>
#include <Ice/Acceptor.h>
#include <Ice/ThreadPool.h>
#include <Ice/ObjectAdapterI.h> // For getThreadPool().
#include <Ice/Reference.h>
#include <Ice/EndpointI.h>
#include <Ice/RouterInfo.h>
#include <Ice/LocalException.h>
#include <Ice/Functional.h>
#include <Ice/OutgoingAsync.h>
#include <IceUtil/Random.h>
#include <iterator>

#if TARGET_OS_IPHONE != 0
namespace IceInternal
{

bool registerForBackgroundNotification(IceInternal::IncomingConnectionFactory*);
void unregisterForBackgroundNotification(IceInternal::IncomingConnectionFactory*);

}
#endif

using namespace std;
using namespace Ice;
using namespace Ice::Instrumentation;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(OutgoingConnectionFactory* p) { return p; }
IceUtil::Shared* IceInternal::upCast(IncomingConnectionFactory* p) { return p; }

namespace
{

struct RandomNumberGenerator : public std::unary_function<ptrdiff_t, ptrdiff_t>
{
    ptrdiff_t operator()(ptrdiff_t d)
    {
        return IceUtilInternal::random(static_cast<int>(d));
    }
};

template <typename K, typename V> void
remove(multimap<K, V>& m, K k, V v)
{
    pair<typename multimap<K, V>::iterator, typename multimap<K, V>::iterator> pr = m.equal_range(k);
    assert(pr.first != pr.second);
    for(typename multimap<K, V>::iterator q = pr.first; q != pr.second; ++q)
    {
        if(q->second.get() == v.get())
        {
            m.erase(q);
            return;
        }
    }
    assert(false); // Nothing was removed which is an error.
}

template <typename K, typename V> ::IceInternal::Handle<V>
find(const multimap<K,::IceInternal::Handle<V> >& m,
     K k,
     const ::IceUtilInternal::ConstMemFun<bool, V, ::IceInternal::Handle<V> >& predicate)
{
    pair<typename multimap<K, ::IceInternal::Handle<V> >::const_iterator,
         typename multimap<K, ::IceInternal::Handle<V> >::const_iterator> pr = m.equal_range(k);
    for(typename multimap<K, ::IceInternal::Handle<V> >::const_iterator q = pr.first; q != pr.second; ++q)
    {
        if(predicate(q->second))
        {
            return q->second;
        }
    }
    return IceInternal::Handle<V>();
}

}

bool
IceInternal::OutgoingConnectionFactory::ConnectorInfo::operator==(const ConnectorInfo& other) const
{
    return connector == other.connector;
}

void
IceInternal::OutgoingConnectionFactory::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_destroyed)
    {
        return;
    }

    for_each(_connections.begin(), _connections.end(),
             bind2nd(Ice::secondVoidMemFun1<const ConnectorPtr, ConnectionI, ConnectionI::DestructionReason>
                     (&ConnectionI::destroy), ConnectionI::CommunicatorDestroyed));

    _destroyed = true;
    _communicator = 0;

    notifyAll();
}

void
IceInternal::OutgoingConnectionFactory::updateConnectionObservers()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    for_each(_connections.begin(), _connections.end(),
             Ice::secondVoidMemFun<const ConnectorPtr, ConnectionI>(&ConnectionI::updateObserver));
}

void
IceInternal::OutgoingConnectionFactory::waitUntilFinished()
{
    multimap<ConnectorPtr, ConnectionIPtr> connections;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        //
        // First we wait until the factory is destroyed. We also wait
        // until there are no pending connections anymore. Only then
        // we can be sure the _connections contains all connections.
        //
        while(!_destroyed || !_pending.empty() || _pendingConnectCount > 0)
        {
            wait();
        }

        //
        // We want to wait until all connections are finished outside the
        // thread synchronization.
        //
        connections = _connections;
    }

    for_each(connections.begin(), connections.end(),
             Ice::secondVoidMemFun<const ConnectorPtr, ConnectionI>(&ConnectionI::waitUntilFinished));

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        // Ensure all the connections are finished and reapable at this point.
        vector<Ice::ConnectionIPtr> cons;
        _monitor->swapReapedConnections(cons);
        assert(cons.size() == _connections.size());
        cons.clear();
        _connections.clear();
        _connectionsByEndpoint.clear();
        _monitor->destroy();
    }
}

void
IceInternal::OutgoingConnectionFactory::create(const vector<EndpointIPtr>& endpts, bool hasMore,
                                               Ice::EndpointSelectionType selType,
                                               const CreateConnectionCallbackPtr& callback)
{
    assert(!endpts.empty());

    //
    // Apply the overrides.
    //
    vector<EndpointIPtr> endpoints = applyOverrides(endpts);

    //
    // Try to find a connection to one of the given endpoints.
    //
    try
    {
        bool compress;
        Ice::ConnectionIPtr connection = findConnection(endpoints, compress);
        if(connection)
        {
            callback->setConnection(connection, compress);
            return;
        }
    }
    catch(const Ice::LocalException& ex)
    {
        callback->setException(ex);
        return;
    }

    ConnectCallbackPtr cb = new ConnectCallback(_instance, this, endpoints, hasMore, callback, selType);
    cb->getConnectors();
}

void
IceInternal::OutgoingConnectionFactory::setRouterInfo(const RouterInfoPtr& routerInfo)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_destroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(routerInfo);

    //
    // Search for connections to the router's client proxy endpoints,
    // and update the object adapter for such connections, so that
    // callbacks from the router can be received over such
    // connections.
    //
    ObjectAdapterPtr adapter = routerInfo->getAdapter();
    vector<EndpointIPtr> endpoints = routerInfo->getClientEndpoints();
    for(vector<EndpointIPtr>::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
    {
        EndpointIPtr endpoint = *p;

        //
        // Modify endpoints with overrides.
        //
        if(_instance->defaultsAndOverrides()->overrideTimeout)
        {
            endpoint = endpoint->timeout(_instance->defaultsAndOverrides()->overrideTimeoutValue);
        }

        //
        // The Connection object does not take the compression flag of
        // endpoints into account, but instead gets the information
        // about whether messages should be compressed or not from
        // other sources. In order to allow connection sharing for
        // endpoints that differ in the value of the compression flag
        // only, we always set the compression flag to false here in
        // this connection factory.
        //
        endpoint = endpoint->compress(false);

        for(multimap<ConnectorPtr, ConnectionIPtr>::const_iterator q = _connections.begin();
            q != _connections.end(); ++q)
        {
            if(q->second->endpoint() == endpoint)
            {
                q->second->setAdapter(adapter);
            }
        }
    }
}

void
IceInternal::OutgoingConnectionFactory::removeAdapter(const ObjectAdapterPtr& adapter)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_destroyed)
    {
        return;
    }

    for(multimap<ConnectorPtr, ConnectionIPtr>::const_iterator p = _connections.begin(); p != _connections.end(); ++p)
    {
        if(p->second->getAdapter() == adapter)
        {
            p->second->setAdapter(0);
        }
    }
}

void
IceInternal::OutgoingConnectionFactory::flushAsyncBatchRequests(const CommunicatorFlushBatchAsyncPtr& outAsync)
{
    list<ConnectionIPtr> c;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        for(multimap<ConnectorPtr,  ConnectionIPtr>::const_iterator p = _connections.begin(); p != _connections.end();
            ++p)
        {
            if(p->second->isActiveOrHolding())
            {
                c.push_back(p->second);
            }
        }
    }

    for(list<ConnectionIPtr>::const_iterator p = c.begin(); p != c.end(); ++p)
    {
        try
        {
            outAsync->flushConnection(*p);
        }
        catch(const LocalException&)
        {
            // Ignore.
        }
    }
}

IceInternal::OutgoingConnectionFactory::OutgoingConnectionFactory(const CommunicatorPtr& communicator,
                                                                  const InstancePtr& instance) :
    _communicator(communicator),
    _instance(instance),
    _monitor(new FactoryACMMonitor(instance, instance->clientACM())),
    _destroyed(false),
    _pendingConnectCount(0)
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

vector<EndpointIPtr>
IceInternal::OutgoingConnectionFactory::applyOverrides(const vector<EndpointIPtr>& endpts)
{
    DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
    vector<EndpointIPtr> endpoints = endpts;
    for(vector<EndpointIPtr>::iterator p = endpoints.begin(); p != endpoints.end(); ++p)
    {
        //
        // Modify endpoints with overrides.
        //
        if(defaultsAndOverrides->overrideTimeout)
        {
            *p = (*p)->timeout(defaultsAndOverrides->overrideTimeoutValue);
        }
    }
    return endpoints;
}

ConnectionIPtr
IceInternal::OutgoingConnectionFactory::findConnection(const vector<EndpointIPtr>& endpoints, bool& compress)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_destroyed)
    {
        throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
    assert(!endpoints.empty());
    for(vector<EndpointIPtr>::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
    {
        ConnectionIPtr connection = find(_connectionsByEndpoint, *p, Ice::constMemFun(&ConnectionI::isActiveOrHolding));
        if(connection)
        {
            if(defaultsAndOverrides->overrideCompress)
            {
                compress = defaultsAndOverrides->overrideCompressValue;
            }
            else
            {
                compress = (*p)->compress();
            }
            return connection;
        }
    }
    return 0;
}

ConnectionIPtr
IceInternal::OutgoingConnectionFactory::findConnection(const vector<ConnectorInfo>& connectors, bool& compress)
{
    // This must be called with the mutex locked.

    DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
    for(vector<ConnectorInfo>::const_iterator p = connectors.begin(); p != connectors.end(); ++p)
    {
        if(_pending.find(p->connector) != _pending.end())
        {
            continue;
        }

        ConnectionIPtr connection = find(_connections, p->connector, Ice::constMemFun(&ConnectionI::isActiveOrHolding));
        if(connection)
        {
            if(defaultsAndOverrides->overrideCompress)
            {
                compress = defaultsAndOverrides->overrideCompressValue;
            }
            else
            {
                    compress = p->endpoint->compress();
            }
            return connection;
        }
    }

    return 0;
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

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_destroyed)
    {
        throw Ice::CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    ++_pendingConnectCount;
}

void
IceInternal::OutgoingConnectionFactory::decPendingConnectCount()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    --_pendingConnectCount;
    assert(_pendingConnectCount >= 0);
    if(_destroyed && _pendingConnectCount == 0)
    {
        notifyAll();
    }
}

ConnectionIPtr
IceInternal::OutgoingConnectionFactory::getConnection(const vector<ConnectorInfo>& connectors,
                                                      const ConnectCallbackPtr& cb,
                                                      bool& compress)
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        if(_destroyed)
        {
            throw Ice::CommunicatorDestroyedException(__FILE__, __LINE__);
        }

        //
        // Reap closed connections
        //
        vector<Ice::ConnectionIPtr> cons;
        _monitor->swapReapedConnections(cons);
        for(vector<Ice::ConnectionIPtr>::const_iterator p = cons.begin(); p != cons.end(); ++p)
        {
            remove(_connections, (*p)->connector(), *p);
            remove(_connectionsByEndpoint, (*p)->endpoint(), *p);
            remove(_connectionsByEndpoint, (*p)->endpoint()->compress(true), *p);
        }

        //
        // Try to get the connection. We may need to wait for other threads to
        // finish if one of them is currently establishing a connection to one
        // of our connectors.
        //
        while(true)
        {
            if(_destroyed)
            {
                throw Ice::CommunicatorDestroyedException(__FILE__, __LINE__);
            }

            //
            // Search for a matching connection. If we find one, we're done.
            //
            Ice::ConnectionIPtr connection = findConnection(connectors, compress);
            if(connection)
            {
                return connection;
            }

            //
            // Determine whether another thread/request is currently attempting to connect to
            // one of our endpoints; if so we wait until it's done.
            //
            if(addToPending(cb, connectors))
            {
                //
                // If a callback is not specified we wait until another thread notifies us about a
                // change to the pending list. Otherwise, if a callback is provided we're done:
                // when the pending list changes the callback will be notified and will try to
                // get the connection again.
                //
                if(!cb)
                {
                    wait();
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                //
                // If no thread is currently establishing a connection to one of our connectors,
                // we get out of this loop and start the connection establishment to one of the
                // given connectors.
                //
                break;
            }
        }
    }

    //
    // At this point, we're responsible for establishing the connection to one of
    // the given connectors. If it's a non-blocking connect, calling nextConnector
    // will start the connection establishment. Otherwise, we return null to get
    // the caller to establish the connection.
    //
    if(cb)
    {
        cb->nextConnector();
    }

    return 0;
}

ConnectionIPtr
IceInternal::OutgoingConnectionFactory::createConnection(const TransceiverPtr& transceiver, const ConnectorInfo& ci)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(_pending.find(ci.connector) != _pending.end() && transceiver);

    //
    // Create and add the connection to the connection map. Adding the connection to the map
    // is necessary to support the interruption of the connection initialization and validation
    // in case the communicator is destroyed.
    //
    Ice::ConnectionIPtr connection;
    try
    {
        if(_destroyed)
        {
            throw Ice::CommunicatorDestroyedException(__FILE__, __LINE__);
        }

        connection = new ConnectionI(_communicator, _instance, _monitor, transceiver, ci.connector,
                                     ci.endpoint->compress(false), 0);
    }
    catch(const Ice::LocalException&)
    {
        try
        {
            transceiver->close();
        }
        catch(const Ice::LocalException&)
        {
            // Ignore
        }
        throw;
    }

    _connections.insert(pair<const ConnectorPtr, ConnectionIPtr>(ci.connector, connection));
    _connectionsByEndpoint.insert(pair<const EndpointIPtr, ConnectionIPtr>(connection->endpoint(), connection));
    _connectionsByEndpoint.insert(pair<const EndpointIPtr, ConnectionIPtr>(connection->endpoint()->compress(true),
                                                                           connection));
    return connection;
}

void
IceInternal::OutgoingConnectionFactory::finishGetConnection(const vector<ConnectorInfo>& connectors,
                                                            const ConnectorInfo& ci,
                                                            const ConnectionIPtr& connection,
                                                            const ConnectCallbackPtr& cb)
{
    set<ConnectCallbackPtr> connectionCallbacks;
    if(cb)
    {
        connectionCallbacks.insert(cb);
    }

    set<ConnectCallbackPtr> callbacks;
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        for(vector<ConnectorInfo>::const_iterator p = connectors.begin(); p != connectors.end(); ++p)
        {
            map<ConnectorPtr, set<ConnectCallbackPtr> >::iterator q = _pending.find(p->connector);
            if(q != _pending.end())
            {
                for(set<ConnectCallbackPtr>::const_iterator r = q->second.begin(); r != q->second.end(); ++r)
                {
                    if((*r)->hasConnector(ci))
                    {
                        connectionCallbacks.insert(*r);
                    }
                    else
                    {
                        callbacks.insert(*r);
                    }
                }
                _pending.erase(q);
            }
        }

        for(set<ConnectCallbackPtr>::iterator r = connectionCallbacks.begin(); r != connectionCallbacks.end(); ++r)
        {
            (*r)->removeFromPending();
            callbacks.erase(*r);
        }
        for(set<ConnectCallbackPtr>::iterator r = callbacks.begin(); r != callbacks.end(); ++r)
        {
            (*r)->removeFromPending();
        }
        notifyAll();
    }

    bool compress;
    DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
    if(defaultsAndOverrides->overrideCompress)
    {
        compress = defaultsAndOverrides->overrideCompressValue;
    }
    else
    {
        compress = ci.endpoint->compress();
    }

    for(set<ConnectCallbackPtr>::const_iterator p = callbacks.begin(); p != callbacks.end(); ++p)
    {
        (*p)->getConnection();
    }
    for(set<ConnectCallbackPtr>::const_iterator p = connectionCallbacks.begin(); p != connectionCallbacks.end(); ++p)
    {
        (*p)->setConnection(connection, compress);
    }
}

void
IceInternal::OutgoingConnectionFactory::finishGetConnection(const vector<ConnectorInfo>& connectors,
                                                            const Ice::LocalException& ex,
                                                            const ConnectCallbackPtr& cb)
{
    set<ConnectCallbackPtr> failedCallbacks;
    if(cb)
    {
        failedCallbacks.insert(cb);
    }

    set<ConnectCallbackPtr> callbacks;
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        for(vector<ConnectorInfo>::const_iterator p = connectors.begin(); p != connectors.end(); ++p)
        {
            map<ConnectorPtr, set<ConnectCallbackPtr> >::iterator q = _pending.find(p->connector);
            if(q != _pending.end())
            {
                for(set<ConnectCallbackPtr>::const_iterator r = q->second.begin(); r != q->second.end(); ++r)
                {
                    if((*r)->removeConnectors(connectors))
                    {
                        failedCallbacks.insert(*r);
                    }
                    else
                    {
                        callbacks.insert(*r);
                    }
                }
                _pending.erase(q);
            }
        }

        for(set<ConnectCallbackPtr>::iterator r = callbacks.begin(); r != callbacks.end(); ++r)
        {
            assert(failedCallbacks.find(*r) == failedCallbacks.end());
            (*r)->removeFromPending();
        }
        notifyAll();
    }

    for(set<ConnectCallbackPtr>::const_iterator p = callbacks.begin(); p != callbacks.end(); ++p)
    {
        (*p)->getConnection();
    }
    for(set<ConnectCallbackPtr>::const_iterator p = failedCallbacks.begin(); p != failedCallbacks.end(); ++p)
    {
        (*p)->setException(ex);
    }
}

bool
IceInternal::OutgoingConnectionFactory::addToPending(const ConnectCallbackPtr& cb,
                                                     const vector<ConnectorInfo>& connectors)
{
    //
    // Add the callback to each connector pending list.
    //
    bool found = false;
    for(vector<ConnectorInfo>::const_iterator p = connectors.begin(); p != connectors.end(); ++p)
    {
        map<ConnectorPtr, set<ConnectCallbackPtr> >::iterator q = _pending.find(p->connector);
        if(q != _pending.end())
        {
            found = true;
            if(cb)
            {
                q->second.insert(cb);
            }
        }
    }

    if(found)
    {
        return true;
    }

    //
    // If there's no pending connection for the given connectors, we're
    // responsible for its establishment. We add empty pending lists,
    // other callbacks to the same connectors will be queued.
    //
    for(vector<ConnectorInfo>::const_iterator r = connectors.begin(); r != connectors.end(); ++r)
    {
        if(_pending.find(r->connector) == _pending.end())
        {
            _pending.insert(pair<ConnectorPtr, set<ConnectCallbackPtr> >(r->connector, set<ConnectCallbackPtr>()));
        }
    }
    return false;
}

void
IceInternal::OutgoingConnectionFactory::removeFromPending(const ConnectCallbackPtr& cb,
                                                          const vector<ConnectorInfo>& connectors)
{
    for(vector<ConnectorInfo>::const_iterator p = connectors.begin(); p != connectors.end(); ++p)
    {
        map<ConnectorPtr, set<ConnectCallbackPtr> >::iterator q = _pending.find(p->connector);
        if(q != _pending.end())
        {
            q->second.erase(cb);
        }
    }
}

void
IceInternal::OutgoingConnectionFactory::handleException(const LocalException& ex, bool hasMore)
{
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->retry >= 2)
    {
        Trace out(_instance->initializationData().logger, traceLevels->retryCat);

        out << "couldn't resolve endpoint host";
        if(dynamic_cast<const CommunicatorDestroyedException*>(&ex))
        {
            out << "\n";
        }
        else
        {
            if(hasMore)
            {
                out << ", trying next endpoint\n";
            }
            else
            {
                out << " and no more endpoints to try\n";
            }
        }
        out << ex;
    }
}

void
IceInternal::OutgoingConnectionFactory::handleConnectionException(const LocalException& ex, bool hasMore)
{
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    if(traceLevels->retry >= 2)
    {
        Trace out(_instance->initializationData().logger, traceLevels->retryCat);

        out << "connection to endpoint failed";
        if(dynamic_cast<const CommunicatorDestroyedException*>(&ex))
        {
            out << "\n";
        }
        else
        {
            if(hasMore)
            {
                out << ", trying next endpoint\n";
            }
            else
            {
                out << " and no more endpoints to try\n";
            }
        }
        out << ex;
    }
}

IceInternal::OutgoingConnectionFactory::ConnectCallback::ConnectCallback(const InstancePtr& instance,
                                                                         const OutgoingConnectionFactoryPtr& factory,
                                                                         const vector<EndpointIPtr>& endpoints,
                                                                         bool hasMore,
                                                                         const CreateConnectionCallbackPtr& cb,
                                                                         Ice::EndpointSelectionType selType) :
    _instance(instance),
    _factory(factory),
    _endpoints(endpoints),
    _hasMore(hasMore),
    _callback(cb),
    _selType(selType)
{
    _endpointsIter = _endpoints.begin();
}

//
// Methods from ConnectionI.StartCallback
//
void
IceInternal::OutgoingConnectionFactory::ConnectCallback::connectionStartCompleted(const ConnectionIPtr& connection)
{
    if(_observer)
    {
        _observer->detach();
    }

    connection->activate();
    _factory->finishGetConnection(_connectors, *_iter, connection, this);
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::connectionStartFailed(const ConnectionIPtr& /*connection*/,
                                                                               const LocalException& ex)
{
    assert(_iter != _connectors.end());
    if(connectionStartFailedImpl(ex))
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
    for(vector<ConnectorPtr>::const_iterator p = connectors.begin(); p != connectors.end(); ++p)
    {
        _connectors.push_back(ConnectorInfo(*p, *_endpointsIter));
    }

    if(++_endpointsIter != _endpoints.end())
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
IceInternal::OutgoingConnectionFactory::ConnectCallback::exception(const Ice::LocalException& ex)
{
    _factory->handleException(ex, _hasMore || _endpointsIter != _endpoints.end() - 1);
    if(++_endpointsIter != _endpoints.end())
    {
        nextEndpoint();
    }
    else if(!_connectors.empty())
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
        _callback->setException(ex);
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
    catch(const Ice::LocalException& ex)
    {
        _callback->setException(ex);
        return;
    }

    nextEndpoint();
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::nextEndpoint()
{
    try
    {
        assert(_endpointsIter != _endpoints.end());
        (*_endpointsIter)->connectors_async(_selType, this);
    }
    catch(const Ice::LocalException& ex)
    {
        exception(ex);
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
        Ice::ConnectionIPtr connection = _factory->getConnection(_connectors, this, compress);
        if(!connection)
        {
            //
            // A null return value from getConnection indicates that the connection
            // is being established and that everthing has been done to ensure that
            // the callback will be notified when the connection establishment is
            // done or that the callback already obtain the connection.
            //
            return;
        }

        _callback->setConnection(connection, compress);
        _factory->decPendingConnectCount(); // Must be called last.
    }
    catch(const Ice::LocalException& ex)
    {
        _callback->setException(ex);
        _factory->decPendingConnectCount(); // Must be called last.
    }
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::nextConnector()
{
    while(true)
    {
        try
        {
            const CommunicatorObserverPtr& obsv = _factory->_instance->initializationData().observer;
            if(obsv)
            {
                _observer = obsv->getConnectionEstablishmentObserver(_iter->endpoint, _iter->connector->toString());
                if(_observer)
                {
                    _observer->attach();
                }
            }

            assert(_iter != _connectors.end());

            if(_instance->traceLevels()->network >= 2)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                out << "trying to establish " << _iter->endpoint->protocol() << " connection to "
                    << _iter->connector->toString();
            }
            Ice::ConnectionIPtr connection = _factory->createConnection(_iter->connector->connect(), *_iter);
            connection->start(this);
        }
        catch(const Ice::LocalException& ex)
        {
            if(_instance->traceLevels()->network >= 2)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                out << "failed to establish " << _iter->endpoint->protocol() << " connection to "
                    << _iter->connector->toString() << "\n" << ex;
            }

            if(connectionStartFailedImpl(ex))
            {
                continue; // More connectors to try, continue.
            }
        }
        break;
    }
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::setConnection(const Ice::ConnectionIPtr& connection,
                                                                       bool compress)
{
    //
    // Callback from the factory: the connection to one of the callback
    // connectors has been established.
    //
    _callback->setConnection(connection, compress);
    _factory->decPendingConnectCount(); // Must be called last.
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::setException(const Ice::LocalException& ex)
{
    //
    // Callback from the factory: connection establishment failed.
    //
    _callback->setException(ex);
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
    for(vector<ConnectorInfo>::const_iterator p = connectors.begin(); p != connectors.end(); ++p)
    {
        _connectors.erase(remove(_connectors.begin(), _connectors.end(), *p), _connectors.end());
    }
    return _connectors.empty();
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::removeFromPending()
{
    _factory->removeFromPending(this, _connectors);
}

bool
IceInternal::OutgoingConnectionFactory::ConnectCallback::operator<(const ConnectCallback& rhs) const
{
    return this < &rhs;
}

bool
IceInternal::OutgoingConnectionFactory::ConnectCallback::connectionStartFailedImpl(const Ice::LocalException& ex)
{
    if(_observer)
    {
        _observer->failed(ex.ice_name());
        _observer->detach();
    }

    _factory->handleConnectionException(ex, _hasMore || _iter != _connectors.end() - 1);
    if(dynamic_cast<const Ice::CommunicatorDestroyedException*>(&ex)) // No need to continue.
    {
        _factory->finishGetConnection(_connectors, ex, this);
    }
    else if(++_iter != _connectors.end()) // Try the next connector.
    {
        return true;
    }
    else
    {
        _factory->finishGetConnection(_connectors, ex, this);
    }
    return false;
}

void
IceInternal::IncomingConnectionFactory::activate()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateActive);
}

void
IceInternal::IncomingConnectionFactory::hold()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateHolding);
}

void
IceInternal::IncomingConnectionFactory::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    setState(StateClosed);
}

void
IceInternal::IncomingConnectionFactory::updateConnectionObservers()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    for_each(_connections.begin(), _connections.end(), Ice::voidMemFun(&ConnectionI::updateObserver));
}

void
IceInternal::IncomingConnectionFactory::waitUntilHolding() const
{
    set<ConnectionIPtr> connections;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        //
        // First we wait until the connection factory itself is in holding
        // state.
        //
        while(_state < StateHolding)
        {
            wait();
        }

        //
        // We want to wait until all connections are in holding state
        // outside the thread synchronization.
        //
        connections = _connections;
    }

    //
    // Now we wait until each connection is in holding state.
    //
    for_each(connections.begin(), connections.end(), Ice::constVoidMemFun(&ConnectionI::waitUntilHolding));
}

void
IceInternal::IncomingConnectionFactory::waitUntilFinished()
{
    set<ConnectionIPtr> connections;
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        //
        // First we wait until the factory is destroyed. If we are using
        // an acceptor, we also wait for it to be closed.
        //
        while(_state != StateFinished)
        {
            wait();
        }

        //
        // Clear the OA. See bug 1673 for the details of why this is necessary.
        //
        _adapter = 0;

        // We want to wait until all connections are finished outside the
        // thread synchronization.
        //
        connections = _connections;
    }

    for_each(connections.begin(), connections.end(), Ice::voidMemFun(&ConnectionI::waitUntilFinished));

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        if(_transceiver)
        {
            assert(_connections.size() <= 1); // The connection isn't monitored or reaped.
        }
        else
        {
            // Ensure all the connections are finished and reapable at this point.
            vector<Ice::ConnectionIPtr> cons;
            _monitor->swapReapedConnections(cons);
            assert(cons.size() == _connections.size());
            cons.clear();
        }
        _connections.clear();
        _monitor->destroy();
    }
}

EndpointIPtr
IceInternal::IncomingConnectionFactory::endpoint() const
{
    // No mutex protection necessary, _endpoint is immutable.
    return _endpoint;
}

list<ConnectionIPtr>
IceInternal::IncomingConnectionFactory::connections() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    list<ConnectionIPtr> result;

    //
    // Only copy connections which have not been destroyed.
    //
    remove_copy_if(_connections.begin(), _connections.end(), back_inserter(result),
                   not1(Ice::constMemFun(&ConnectionI::isActiveOrHolding)));

    return result;
}

void
IceInternal::IncomingConnectionFactory::flushAsyncBatchRequests(const CommunicatorFlushBatchAsyncPtr& outAsync)
{
    list<ConnectionIPtr> c = connections(); // connections() is synchronized, so no need to synchronize here.

    for(list<ConnectionIPtr>::const_iterator p = c.begin(); p != c.end(); ++p)
    {
        try
        {
            outAsync->flushConnection(*p);
        }
        catch(const LocalException&)
        {
            // Ignore.
        }
    }
}

#if defined(ICE_USE_IOCP) || defined(ICE_OS_WINRT)
bool
IceInternal::IncomingConnectionFactory::startAsync(SocketOperation)
{
    assert(_acceptor);
    if(_state >= StateClosed)
    {
        return false;
    }

    try
    {
        _acceptor->startAccept();
    }
    catch(const Ice::LocalException& ex)
    {
        {
            Error out(_instance->initializationData().logger);
            out << "can't accept connections:\n" << ex << '\n' << _acceptor->toString();
        }
        abort();
    }
    return true;
}

bool
IceInternal::IncomingConnectionFactory::finishAsync(SocketOperation)
{
    assert(_acceptor);
    try
    {
        _acceptor->finishAccept();
    }
    catch(const LocalException& ex)
    {
        Error out(_instance->initializationData().logger);
        out << "couldn't accept connection:\n" << ex << '\n' << _acceptor->toString();
        return false;
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
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        ThreadPoolMessage<IncomingConnectionFactory>::IOScope io(msg);
        if(!io)
        {
            return;
        }

        if(_state >= StateClosed)
        {
            return;
        }
        else if(_state == StateHolding)
        {
            IceUtil::ThreadControl::yield();
            return;
        }

        //
        // Reap closed connections
        //
        vector<Ice::ConnectionIPtr> cons;
        _monitor->swapReapedConnections(cons);
        for(vector<Ice::ConnectionIPtr>::const_iterator p = cons.begin(); p != cons.end(); ++p)
        {
            _connections.erase(*p);
        }

        if(!_acceptor)
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

            if(_instance->traceLevels()->network >= 2)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                out << "trying to accept " << _endpoint->protocol() << " connection\n" << transceiver->toString();
            }
        }
        catch(const SocketException& ex)
        {
            if(noMoreFds(ex.error))
            {
                {
                    Error out(_instance->initializationData().logger);
                    out << "fatal error: can't accept more connections:\n" << ex << '\n' << _acceptor->toString();
                }
                abort();
            }

            // Ignore socket exceptions.
            return;
        }
        catch(const LocalException& ex)
        {
            // Warn about other Ice local exceptions.
            if(_warn)
            {
                Warning out(_instance->initializationData().logger);
                out << "connection exception:\n" << ex << '\n' << _acceptor->toString();
            }
            return;
        }

        assert(transceiver);

        try
        {
            connection = new ConnectionI(_adapter->getCommunicator(), _instance, _monitor, transceiver, 0, _endpoint,
                                         _adapter);
        }
        catch(const LocalException& ex)
        {
            try
            {
                transceiver->close();
            }
            catch(const Ice::LocalException&)
            {
                // Ignore.
            }

            if(_warn)
            {
                Warning out(_instance->initializationData().logger);
                out << "connection exception:\n" << ex << '\n' << _acceptor->toString();
            }
            return;
        }

        _connections.insert(connection);
    }

    assert(connection);
    connection->start(this);
}

void
IceInternal::IncomingConnectionFactory::finished(ThreadPoolCurrent&, bool close)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

#if TARGET_OS_IPHONE != 0
    if(_state < StateClosed)
    {
        //
        // Finished has been called by stopAcceptor if the state isn't
        // closed.
        //
        if(_acceptorStarted && close)
        {
            _acceptorStarted = false;
            closeAcceptor();
        }
        return;
    }
#endif

    assert(_state == StateClosed);
    setState(StateFinished);

    if(close)
    {
        closeAcceptor();
    }

#if TARGET_OS_IPHONE != 0
    sync.release();
    unregisterForBackgroundNotification(this);
#endif
}

string
IceInternal::IncomingConnectionFactory::toString() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_transceiver)
    {
        return _transceiver->toString();
    }
    else if(_acceptor)
    {
        return _acceptor->toString();
    }
    else
    {
        return string();
    }
}

NativeInfoPtr
IceInternal::IncomingConnectionFactory::getNativeInfo()
{
    if(_transceiver)
    {
        return _transceiver->getNativeInfo();
    }
    else if(_acceptor)
    {
        return _acceptor->getNativeInfo();
    }
    else
    {
        return 0;
    }
}

void
IceInternal::IncomingConnectionFactory::connectionStartCompleted(const Ice::ConnectionIPtr& connection)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    //
    // Initialy, connections are in the holding state. If the factory is active
    // we activate the connection.
    //
    if(_state == StateActive)
    {
        connection->activate();
    }
}

void
IceInternal::IncomingConnectionFactory::connectionStartFailed(const Ice::ConnectionIPtr& /*connection*/,
                                                              const Ice::LocalException& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state >= StateClosed)
    {
        return;
    }

    //
    // Do not warn about connection exceptions here. The connection is not yet validated.
    //
}

//
// COMPILERFIX: The ConnectionFactory setup is broken out into a separate initialize
// function because when it was part of the constructor C++Builder 2007 apps would
// crash if an execption was thrown from any calls within the constructor.
//
IceInternal::IncomingConnectionFactory::IncomingConnectionFactory(const InstancePtr& instance,
                                                                  const EndpointIPtr& endpoint,
                                                                  const ObjectAdapterIPtr& adapter) :
    _instance(instance),
    _monitor(new FactoryACMMonitor(instance, dynamic_cast<ObjectAdapterI*>(adapter.get())->getACM())),
    _endpoint(endpoint),
    _adapter(adapter),
    _warn(_instance->initializationData().properties->getPropertyAsInt("Ice.Warn.Connections") > 0),
    _state(StateHolding)
{
}

#if TARGET_OS_IPHONE != 0
void
IceInternal::IncomingConnectionFactory::startAcceptor()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state >= StateClosed || _acceptorStarted)
    {
        return;
    }

    try
    {
        createAcceptor();
        _acceptorStarted = true;
    }
    catch(const Ice::Exception& ex)
    {
        if(_warn)
        {
            Warning out(_instance->initializationData().logger);
            out << "unable to create acceptor:\n" << ex;
        }
    }
}

void
IceInternal::IncomingConnectionFactory::stopAcceptor()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state >= StateClosed || !_acceptorStarted)
    {
        return;
    }

    if(_adapter->getThreadPool()->finish(this, true))
    {
        _acceptorStarted = false;
        closeAcceptor();
    }
}
#endif

void
IceInternal::IncomingConnectionFactory::initialize()
{
    if(_instance->defaultsAndOverrides()->overrideTimeout)
    {
        _endpoint = _endpoint->timeout(_instance->defaultsAndOverrides()->overrideTimeoutValue);
    }

    if(_instance->defaultsAndOverrides()->overrideCompress)
    {
        _endpoint = _endpoint->compress(_instance->defaultsAndOverrides()->overrideCompressValue);
    }

    try
    {
        const_cast<TransceiverPtr&>(_transceiver) = _endpoint->transceiver();
        if(_transceiver)
        {
            if(_instance->traceLevels()->network >= 2)
            {
                Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                out << "attempting to bind to " << _endpoint->protocol() << " socket\n" << _transceiver->toString();
            }
            const_cast<EndpointIPtr&>(_endpoint) = _transceiver->bind();

            ConnectionIPtr connection = new ConnectionI(_adapter->getCommunicator(), _instance, 0, _transceiver, 0,
                                                        _endpoint, _adapter);
            connection->start(0);
            _connections.insert(connection);
        }
        else
        {
#if TARGET_OS_IPHONE != 0
            //
            // The notification center will call back on the factory to
            // start the acceptor if necessary.
            //
            _acceptorStarted = false;
            registerForBackgroundNotification(this);
#else
            createAcceptor();
#endif
        }
    }
    catch(const Ice::Exception&)
    {
        if(_transceiver)
        {
            try
            {
                _transceiver->close();
            }
            catch(const Ice::LocalException&)
            {
                // Ignore
            }
        }

        _state = StateFinished;
        _monitor->destroy();
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
    if(_state == state) // Don't switch twice.
    {
        return;
    }

    switch(state)
    {
        case StateActive:
        {
            if(_state != StateHolding) // Can only switch from holding to active.
            {
                return;
            }
            if(_acceptor)
            {
                if(_instance->traceLevels()->network >= 1)
                {
                    Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                    out << "accepting " << _endpoint->protocol() << " connections at " << _acceptor->toString();
                }
                _adapter->getThreadPool()->_register(this, SocketOperationRead);
            }
            for_each(_connections.begin(), _connections.end(), Ice::voidMemFun(&ConnectionI::activate));
            break;
        }

        case StateHolding:
        {
            if(_state != StateActive) // Can only switch from active to holding.
            {
                return;
            }
            if(_acceptor)
            {
                if(_instance->traceLevels()->network >= 1)
                {
                    Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
                    out << "holding " << _endpoint->protocol() << " connections at " << _acceptor->toString();
                }
                _adapter->getThreadPool()->unregister(this, SocketOperationRead);
            }
            for_each(_connections.begin(), _connections.end(), Ice::voidMemFun(&ConnectionI::hold));
            break;
        }

        case StateClosed:
        {
            if(_acceptor)
            {
                //
                // If possible, close the acceptor now to prevent new connections from
                // being accepted while we are deactivating. This is especially useful
                // if there are no more threads in the thread pool available to dispatch
                // the finish() call. Not all selector implementations do support this
                // however.
                //
                if(_adapter->getThreadPool()->finish(this, true))
                {
                    closeAcceptor();
                }
            }
            else
            {
                state = StateFinished;
            }

            for_each(_connections.begin(), _connections.end(),
                     bind2nd(Ice::voidMemFun1(&ConnectionI::destroy), ConnectionI::ObjectAdapterDeactivated));
            break;
        }

        case StateFinished:
        {
            assert(_state == StateClosed);
            break;
        }
    }

    _state = state;
    notifyAll();
}

void
IceInternal::IncomingConnectionFactory::createAcceptor()
{
    try
    {
        _acceptor = _endpoint->acceptor(_adapter->getName());
        assert(_acceptor);

        if(_instance->traceLevels()->network >= 2)
        {
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
            out << "attempting to bind to " << _endpoint->protocol() << " socket " << _acceptor->toString();
        }

        _endpoint = _acceptor->listen();

        if(_instance->traceLevels()->network >= 1)
        {
            Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
            out << "listening for " << _endpoint->protocol() << " connections\n" << _acceptor->toDetailedString();
        }

        _adapter->getThreadPool()->initialize(this);

        if(_state == StateActive)
        {
            _adapter->getThreadPool()->_register(this, SocketOperationRead);
        }
    }
    catch(const Ice::Exception&)
    {
        if(_acceptor)
        {
            _acceptor->close();
            _acceptor = 0;
        }
        throw;
    }
}

void
IceInternal::IncomingConnectionFactory::closeAcceptor()
{
    assert(_acceptor);

    if(_instance->traceLevels()->network >= 1)
    {
        Trace out(_instance->initializationData().logger, _instance->traceLevels()->networkCat);
        out << "stopping to accept " << _endpoint->protocol() << " connections at " << _acceptor->toString();
    }

    _acceptor->close();

#if TARGET_OS_IPHONE != 0
    //
    // Only clear the acceptor on iOS where it can be destroyed/re-created during the lifetime of the incoming
    // connection factory. On other platforms, we keep it set. This is in particular import for IOCP/WinRT where
    // finishAsync can be called after the acceptor is closed.
    //
    _acceptor = 0;
#endif
}
