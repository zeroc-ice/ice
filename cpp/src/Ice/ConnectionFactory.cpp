// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
#include <IceUtil/Random.h>
#ifdef __BCPLUSPLUS__
#  include <iterator>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(OutgoingConnectionFactory* p) { return p; }
IceUtil::Shared* IceInternal::upCast(IncomingConnectionFactory* p) { return p; }

namespace
{

struct RandomNumberGenerator : public std::unary_function<ptrdiff_t, ptrdiff_t>
{
    ptrdiff_t operator()(ptrdiff_t d)
    {
        return IceUtil::random(static_cast<int>(d));
    }
};
    
}

bool
IceInternal::OutgoingConnectionFactory::ConnectorInfo::operator<(const ConnectorInfo& other) const
{
    if(!threadPerConnection && other.threadPerConnection)
    {
        return true;
    }
    else if(other.threadPerConnection < threadPerConnection)
    {
        return false;
    }
    
    return connector < other.connector;
}

void
IceInternal::OutgoingConnectionFactory::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_destroyed)
    {
        return;
    }

#ifdef _STLP_BEGIN_NAMESPACE
    // voidbind2nd is an STLport extension for broken compilers in IceUtil/Functional.h
    for_each(_connections.begin(), _connections.end(),
             voidbind2nd(Ice::secondVoidMemFun1<ConnectorInfo, ConnectionI, ConnectionI::DestructionReason>
                         (&ConnectionI::destroy), ConnectionI::CommunicatorDestroyed));
#else
    for_each(_connections.begin(), _connections.end(),
             bind2nd(Ice::secondVoidMemFun1<const ConnectorInfo, ConnectionI, ConnectionI::DestructionReason>
                     (&ConnectionI::destroy), ConnectionI::CommunicatorDestroyed));
#endif

    _destroyed = true;
    notifyAll();
}

void
IceInternal::OutgoingConnectionFactory::waitUntilFinished()
{
    multimap<ConnectorInfo, ConnectionIPtr> connections;

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
             Ice::secondVoidMemFun<const ConnectorInfo, ConnectionI>(&ConnectionI::waitUntilFinished));

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        _connections.clear();
        _connectionsByEndpoint.clear();
    }
}

ConnectionIPtr
IceInternal::OutgoingConnectionFactory::create(const vector<EndpointIPtr>& endpts, bool hasMore,
                                               bool threadPerConnection, Ice::EndpointSelectionType selType, 
                                               bool& compress)
{
    assert(!endpts.empty());
    
    //
    // Apply the overrides.
    //
    vector<EndpointIPtr> endpoints = applyOverrides(endpts);

    //
    // Try to find a connection to one of the given endpoints.
    // 
    Ice::ConnectionIPtr connection = findConnection(endpoints, threadPerConnection, compress);
    if(connection)
    {
        return connection;
    }

    auto_ptr<Ice::LocalException> exception;

    //
    // If we didn't find a connection with the endpoints, we create the connectors
    // for the endpoints.
    //
    vector<ConnectorInfo> connectors;
    for(vector<EndpointIPtr>::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
    {
        //
        // Create connectors for the endpoint.
        //
        try
        {
            vector<ConnectorPtr> cons = (*p)->connectors();
            assert(!cons.empty());
            
            if(selType == Random)
            {
                RandomNumberGenerator rng;
                random_shuffle(cons.begin(), cons.end(), rng);
            }
            
            for(vector<ConnectorPtr>::const_iterator r = cons.begin(); r != cons.end(); ++r)
            {
                assert(*r);
                connectors.push_back(ConnectorInfo(*r, *p, threadPerConnection));
            }
        }
        catch(const Ice::LocalException& ex)
        {
            exception.reset(dynamic_cast<Ice::LocalException*>(ex.ice_clone()));
            handleException(ex, hasMore || p != endpoints.end() - 1);
        }
    }

    if(connectors.empty())
    {
        assert(exception.get());
        exception->ice_throw();
    }
    
    //
    // Try to get a connection to one of the connectors. A null result indicates that no
    // connection was found and that we should try to establish the connection (and that
    // the connectors were added to _pending to prevent other threads from establishing
    // the connection).
    //
    connection = getConnection(connectors, 0, compress);
    if(connection)
    {
        return connection;
    }

    //
    // Try to establish the connection to the connectors.
    //
    DefaultsAndOverridesPtr defaultsAndOverrides = _instance->defaultsAndOverrides();
    for(vector<ConnectorInfo>::const_iterator q = connectors.begin(); q != connectors.end(); ++q)
    {
        try
        {
            int timeout;
            if(defaultsAndOverrides->overrideConnectTimeout)
            {
                timeout = defaultsAndOverrides->overrideConnectTimeoutValue;
            }
            else
            {
                //
                // It is not necessary to check for overrideTimeout, the endpoint has already 
                // been modified with this override, if set.
                //
                timeout = q->endpoint->timeout();
            }

            connection = createConnection(q->connector->connect(timeout), *q);
            connection->start(0);

            if(defaultsAndOverrides->overrideCompress)
            {
                compress = defaultsAndOverrides->overrideCompressValue;
            }
            else
            {
                compress = q->endpoint->compress();
            }

            break;
        }
        catch(const Ice::CommunicatorDestroyedException& ex)
        {
            exception.reset(dynamic_cast<Ice::LocalException*>(ex.ice_clone()));
            handleException(*exception.get(), *q, connection, hasMore || q != connectors.end() - 1);
            connection = 0;
            break; // No need to continue
        }
        catch(const Ice::LocalException& ex)
        {
            exception.reset(dynamic_cast<Ice::LocalException*>(ex.ice_clone()));
            handleException(*exception.get(), *q, connection, hasMore || q != connectors.end() - 1);
            connection = 0;
        }
    }

    //
    // Finish creating the connection (this removes the connectors from the _pending
    // list and notifies any waiting threads).
    //
    finishGetConnection(connectors, 0, connection);

    if(!connection)
    {
        assert(exception.get());
        exception->ice_throw();
    }

    return connection;
}

void
IceInternal::OutgoingConnectionFactory::create(const vector<EndpointIPtr>& endpts, bool hasMore,
                                               bool tpc, Ice::EndpointSelectionType selType, 
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
        Ice::ConnectionIPtr connection = findConnection(endpoints, tpc, compress);
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
    
    ConnectCallbackPtr cb = new ConnectCallback(this, endpoints, hasMore, callback, selType, tpc);
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
    vector<EndpointIPtr>::const_iterator p;
    for(p = endpoints.begin(); p != endpoints.end(); ++p)
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

        multimap<ConnectorInfo, ConnectionIPtr>::const_iterator q;
        for(q = _connections.begin(); q != _connections.end(); ++q)
        {
            if(q->second->endpoint() == endpoint)
            {
                try
                {
                    q->second->setAdapter(adapter);
                }
                catch(const Ice::LocalException&)
                {
                    //
                    // Ignore, the connection is being closed or closed.
                    //
                }
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
    
    for(multimap<ConnectorInfo, ConnectionIPtr>::const_iterator p = _connections.begin(); p != _connections.end(); ++p)
    {
        if(p->second->getAdapter() == adapter)
        {
            try
            {
                p->second->setAdapter(0);
            }
            catch(const Ice::LocalException&)
            {
                //
                // Ignore, the connection is being closed or closed.
                //
            }
        }
    }
}

void
IceInternal::OutgoingConnectionFactory::flushBatchRequests()
{
    list<ConnectionIPtr> c;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        for(multimap<ConnectorInfo, ConnectionIPtr>::const_iterator p = _connections.begin(); p != _connections.end();
            ++p)
        {
            c.push_back(p->second);
        }
    }

    for(list<ConnectionIPtr>::const_iterator p = c.begin(); p != c.end(); ++p)
    {
        try
        {
            (*p)->flushBatchRequests();
        }
        catch(const LocalException&)
        {
            // Ignore.
        }
    }
}

IceInternal::OutgoingConnectionFactory::OutgoingConnectionFactory(const InstancePtr& instance) :
    _instance(instance),
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
IceInternal::OutgoingConnectionFactory::findConnection(const vector<EndpointIPtr>& endpoints, bool tpc, bool& compress)
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
        pair<multimap<EndpointIPtr, ConnectionIPtr>::iterator,
             multimap<EndpointIPtr, ConnectionIPtr>::iterator> pr = _connectionsByEndpoint.equal_range(*p);
        
        for(multimap<EndpointIPtr, ConnectionIPtr>::iterator q = pr.first; q != pr.second; ++q)
        {
            if(q->second->isActiveOrHolding() && 
               q->second->threadPerConnection() == tpc) // Don't return destroyed or un-validated connections
            {
                if(defaultsAndOverrides->overrideCompress)
                {
                    compress = defaultsAndOverrides->overrideCompressValue;
                }
                else
                {
                    compress = (*p)->compress();
                }
                return q->second;
            }
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
        pair<multimap<ConnectorInfo, ConnectionIPtr>::iterator,
             multimap<ConnectorInfo, ConnectionIPtr>::iterator> pr = _connections.equal_range(*p);
        
        if(pr.first == pr.second)
        {
            continue;
        }
        
        for(multimap<ConnectorInfo, ConnectionIPtr>::iterator q = pr.first; q != pr.second; ++q)
        {
            if(q->second->isActiveOrHolding()) // Don't return destroyed or un-validated connections
            {
                if(q->second->endpoint() != p->endpoint)
                {
                    _connectionsByEndpoint.insert(make_pair(p->endpoint, q->second));
                }

                if(defaultsAndOverrides->overrideCompress)
                {
                    compress = defaultsAndOverrides->overrideCompressValue;
                }
                else
                {
                    compress = p->endpoint->compress();
                }
                return q->second;
            }
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
                                                      const ConnectCallbackPtr& cb, bool& compress)
{
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        if(_destroyed)
        {
            throw Ice::CommunicatorDestroyedException(__FILE__, __LINE__);
        }
        
        //
        // Reap connections for which destruction has completed.
        //
        multimap<ConnectorInfo, ConnectionIPtr>::iterator p = _connections.begin();
        while(p != _connections.end())
        {
            if(p->second->isFinished())
            {
                _connections.erase(p++);
            }
            else
            {
                ++p;
            }
        }
        
        multimap<EndpointIPtr, ConnectionIPtr>::iterator q = _connectionsByEndpoint.begin();
        while(q != _connectionsByEndpoint.end())
        {
            if(q->second->isFinished())
            {
                _connectionsByEndpoint.erase(q++);
            }
            else
            {
                ++q;
            }
        }

        //
        // Try to get the connection. We may need to wait for other threads to
        // finish if one of them is currently establishing a connection to one
        // of our connectors.
        //
        while(!_destroyed)
        {
            //
            // Search for a matching connection. If we find one, we're done.
            //
            Ice::ConnectionIPtr connection = findConnection(connectors, compress);
            if(connection)
            {
                if(cb)
                {
                    //
                    // This might not be the first getConnection call for the callback. We need
                    // to ensure that the callback isn't registered with any other pending 
                    // connectors since we just found a connection and therefore don't need to
                    // wait anymore for other pending connectors.
                    // 
                    for(vector<ConnectorInfo>::const_iterator p = connectors.begin(); p != connectors.end(); ++p)
                    {
                        map<ConnectorInfo, set<ConnectCallbackPtr> >::iterator q = _pending.find(*p);
                        if(q != _pending.end())
                        {
                            q->second.erase(cb);
                        }
                    }
                }
                return connection;
            }

            //
            // Determine whether another thread is currently attempting to connect to one of our endpoints;
            // if so we wait until it's done.
            //
            bool found = false;
            for(vector<ConnectorInfo>::const_iterator p = connectors.begin(); p != connectors.end(); ++p)
            {
                map<ConnectorInfo, set<ConnectCallbackPtr> >::iterator q = _pending.find(*p);
                if(q != _pending.end())
                {
                    found = true;
                    if(cb)
                    {
                        q->second.insert(cb); // Add the callback to each pending connector.
                    }
                }
            }
                
            if(!found)
            {
                //
                // If no thread is currently establishing a connection to one of our connectors,
                // we get out of this loop and start the connection establishment to one of the
                // given connectors.
                //
                break;
            }
            else
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
        }
            
        if(_destroyed)
        {
            throw Ice::CommunicatorDestroyedException(__FILE__, __LINE__);
        }
        
        //
        // No connection to any of our endpoints exists yet; we add the given connectors to
        // the _pending set to indicate that we're attempting connection establishment to 
        // these connectors.
        //
        for(vector<ConnectorInfo>::const_iterator r = connectors.begin(); r != connectors.end(); ++r)
        {
            assert(_pending.find(*r) == _pending.end());
            _pending.insert(pair<ConnectorInfo, set<ConnectCallbackPtr> >(*r, set<ConnectCallbackPtr>()));
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
    assert(_pending.find(ci) != _pending.end() && transceiver);

    //
    // Create and add the connection to the connection map. Adding the connection to the map
    // is necessary to support the interruption of the connection initialization and validation
    // in case the communicator is destroyed.
    //
    try
    {
        if(_destroyed)
        {
            throw Ice::CommunicatorDestroyedException(__FILE__, __LINE__);
        }

        Ice::ConnectionIPtr connection = new ConnectionI(_instance, transceiver, ci.endpoint->compress(false),
                                                         0, ci.threadPerConnection, 
                                                         _instance->threadPerConnectionStackSize());
        _connections.insert(make_pair(ci, connection));
        _connectionsByEndpoint.insert(make_pair(ci.endpoint, connection));
        return connection;
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
}

void
IceInternal::OutgoingConnectionFactory::finishGetConnection(const vector<ConnectorInfo>& connectors,
                                                            const ConnectCallbackPtr& cb,
                                                            const ConnectionIPtr& connection)
{
    set<ConnectCallbackPtr> callbacks;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        
        //
        // We're done trying to connect to the given connectors so we remove the 
        // connectors from the pending list and notify waiting threads. We also 
        // notify the pending connect callbacks (outside the synchronization).
        //
        
        for(vector<ConnectorInfo>::const_iterator p = connectors.begin(); p != connectors.end(); ++p)
        {
            map<ConnectorInfo, set<ConnectCallbackPtr> >::iterator q = _pending.find(*p);
            assert(q != _pending.end());
            callbacks.insert(q->second.begin(), q->second.end());
            _pending.erase(q);
        }
        notifyAll();

        //
        // If the connect attempt succeeded and the communicator is not destroyed,
        // activate the connection!
        //
        if(connection && !_destroyed)
        {
            connection->activate();
        }
    }

    //
    // Notify any waiting callbacks.
    //
    for(set<ConnectCallbackPtr>::const_iterator p = callbacks.begin(); p != callbacks.end(); ++p)
    {
        (*p)->getConnection();
    }
}

void
IceInternal::OutgoingConnectionFactory::handleException(const LocalException& ex, const ConnectorInfo& ci,
                                                        const ConnectionIPtr& connection, bool hasMore)
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

    if(connection && connection->isFinished())
    {
        //
        // If the connection is finished, we remove it right away instead of
        // waiting for the reaping.
        //
        // NOTE: it's possible for the connection to not be finished yet. That's
        // for instance the case when using thread per connection and if it's the
        // thread which is calling back the outgoing connection factory to notify
        // it of the failure.
        //
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
            pair<multimap<ConnectorInfo, ConnectionIPtr>::iterator,
                 multimap<ConnectorInfo, ConnectionIPtr>::iterator> pr = _connections.equal_range(ci);
            
            for(multimap<ConnectorInfo, ConnectionIPtr>::iterator p = pr.first; p != pr.second; ++p)
            {
                if(p->second == connection)
                {
                    _connections.erase(p);
                    break;
                }
            }

            pair<multimap<EndpointIPtr, ConnectionIPtr>::iterator,
                 multimap<EndpointIPtr, ConnectionIPtr>::iterator> qr = _connectionsByEndpoint.equal_range(ci.endpoint);
            
            for(multimap<EndpointIPtr, ConnectionIPtr>::iterator q = qr.first; q != qr.second; ++q)
            {
                if(q->second == connection)
                {
                    _connectionsByEndpoint.erase(q);
                    break;
                }
            }
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

IceInternal::OutgoingConnectionFactory::ConnectCallback::ConnectCallback(const OutgoingConnectionFactoryPtr& factory,
                                                                         const vector<EndpointIPtr>& endpoints,
                                                                         bool hasMore,
                                                                         const CreateConnectionCallbackPtr& cb,
                                                                         Ice::EndpointSelectionType selType,
                                                                         bool threadPerConnection) :
    _factory(factory),
    _selectorThread(_factory->_instance->selectorThread()),
    _endpoints(endpoints),
    _hasMore(hasMore),
    _callback(cb),
    _selType(selType),
    _threadPerConnection(threadPerConnection)
{
    _endpointsIter = _endpoints.begin();
}

//
// Methods from ConnectionI.StartCallback
//
void
IceInternal::OutgoingConnectionFactory::ConnectCallback::connectionStartCompleted(const ConnectionIPtr& connection)
{
    bool compress;
    DefaultsAndOverridesPtr defaultsAndOverrides = _factory->_instance->defaultsAndOverrides();
    if(defaultsAndOverrides->overrideCompress)
    {
        compress = defaultsAndOverrides->overrideCompressValue;
    }
    else
    {
        compress = _iter->endpoint->compress();
    }
    
    _factory->finishGetConnection(_connectors, this, connection);
    _callback->setConnection(connection, compress);
    _factory->decPendingConnectCount(); // Must be called last.
}

void
IceInternal::OutgoingConnectionFactory::ConnectCallback::connectionStartFailed(const ConnectionIPtr& connection, 
                                                                               const LocalException& ex)
{
    assert(_iter != _connectors.end());
    
    _factory->handleException(ex, *_iter, connection, _hasMore || _iter != _connectors.end() - 1);
    if(dynamic_cast<const Ice::CommunicatorDestroyedException*>(&ex)) // No need to continue.
    {
        _factory->finishGetConnection(_connectors, this, 0);
        _callback->setException(ex);
        _factory->decPendingConnectCount(); // Must be called last.
    }
    else if(++_iter != _connectors.end()) // Try the next connector.
    {
        nextConnector();
    }
    else
    {
        _factory->finishGetConnection(_connectors, this, 0);
        _callback->setException(ex);
        _factory->decPendingConnectCount(); // Must be called last.
    }
}

//
// Methods from EndpointI_connectors
//
void
IceInternal::OutgoingConnectionFactory::ConnectCallback::connectors(const vector<ConnectorPtr>& connectors)
{
    vector<ConnectorPtr> cons = connectors;
    if(_selType == Random)
    {
        RandomNumberGenerator rng;
        random_shuffle(cons.begin(), cons.end(), rng);
    }

    for(vector<ConnectorPtr>::const_iterator p = cons.begin(); p != cons.end(); ++p)
    {
        _connectors.push_back(ConnectorInfo(*p, *_endpointsIter, _threadPerConnection));
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
        (*_endpointsIter)->connectors_async(this);
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
            // done.
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
    Ice::ConnectionIPtr connection;
    try
    {
        assert(_iter != _connectors.end());
        connection = _factory->createConnection(_iter->connector->connect(0), *_iter);
        connection->start(this);
    }
    catch(const Ice::LocalException& ex)
    {
        connectionStartFailed(connection, ex);
    }
}

bool
IceInternal::OutgoingConnectionFactory::ConnectCallback::operator<(const ConnectCallback& rhs) const
{
    return this < &rhs;
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
IceInternal::IncomingConnectionFactory::waitUntilHolding() const
{
    list<ConnectionIPtr> connections;

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
    IceUtil::ThreadPtr threadPerIncomingConnectionFactory;
    list<ConnectionIPtr> connections;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        
        //
        // First we wait until the factory is destroyed. If we are using
        // an acceptor, we also wait for it to be closed.
        //
        while(_state != StateClosed || _acceptor)
        {
            wait();
        }

        threadPerIncomingConnectionFactory = _threadPerIncomingConnectionFactory;
        _threadPerIncomingConnectionFactory = 0;

        //
        // Clear the OA. See bug 1673 for the details of why this is necessary.
        //
        _adapter = 0;

        //
        // We want to wait until all connections are finished outside the
        // thread synchronization.
        //
        connections = _connections;
    }

    if(threadPerIncomingConnectionFactory)
    {
        threadPerIncomingConnectionFactory->getThreadControl().join();
    }

    for_each(connections.begin(), connections.end(), Ice::voidMemFun(&ConnectionI::waitUntilFinished));

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        _connections.clear();
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
IceInternal::IncomingConnectionFactory::flushBatchRequests()
{
    list<ConnectionIPtr> c = connections(); // connections() is synchronized, so no need to synchronize here.

    for(list<ConnectionIPtr>::const_iterator p = c.begin(); p != c.end(); ++p)
    {
        try
        {
            (*p)->flushBatchRequests();
        }
        catch(const LocalException&)
        {
            // Ignore.
        }
    }
}

bool
IceInternal::IncomingConnectionFactory::datagram() const
{
    assert(!_threadPerConnection); // Only for use with a thread pool.
    return _endpoint->datagram();
}

bool
IceInternal::IncomingConnectionFactory::readable() const
{
    assert(!_threadPerConnection); // Only for use with a thread pool.
    return false;
}

bool
IceInternal::IncomingConnectionFactory::read(BasicStream&)
{
    assert(!_threadPerConnection); // Only for use with a thread pool.
    assert(false); // Must not be called, readable() returns false.
    return false;
}

class PromoteFollower
{
public:

    PromoteFollower(const ThreadPoolPtr& threadPool) :
        _threadPool(threadPool)
    {
    }

    ~PromoteFollower()
    {
        _threadPool->promoteFollower();
    }

private:

    const ThreadPoolPtr _threadPool;
};

void
IceInternal::IncomingConnectionFactory::message(BasicStream&, const ThreadPoolPtr& threadPool)
{
    assert(!_threadPerConnection); // Only for use with a thread pool.

    ConnectionIPtr connection;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        
        //
        // This makes sure that we promote a follower before we leave
        // the scope of the mutex above, but after we call accept()
        // (if we call it).
        //
        // If _threadPool is null, then this class doesn't do
        // anything.
        //
        PromoteFollower promote(threadPool);

        if(_state != StateActive)
        {
            IceUtil::ThreadControl::yield();
            return;
        }
        
        //
        // Reap connections for which destruction has completed.
        //
        _connections.erase(remove_if(_connections.begin(), _connections.end(),
                                     Ice::constMemFun(&ConnectionI::isFinished)),
                           _connections.end());
        
        //
        // Now accept a new connection.
        //
        TransceiverPtr transceiver;
        try
        {
            transceiver = _acceptor->accept(0);
        }
        catch(const SocketException&)
        {
            // Ignore socket exceptions.
            return;
        }
        catch(const TimeoutException&)
        {
            // Ignore timeouts.
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
            assert(!_threadPerConnection);
            connection = new ConnectionI(_instance, transceiver, _endpoint, _adapter, false, 0);
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

        _connections.push_back(connection);
    }

    assert(connection);

    connection->start(this);
}

void
IceInternal::IncomingConnectionFactory::finished(const ThreadPoolPtr& threadPool)
{
    assert(!_threadPerConnection); // Only for use with a thread pool.

    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    threadPool->promoteFollower();
    assert(threadPool.get() == dynamic_cast<ObjectAdapterI*>(_adapter.get())->getThreadPool().get());
    
    --_finishedCount;

    if(_finishedCount == 0 && _state == StateClosed)
    {
        dynamic_cast<ObjectAdapterI*>(_adapter.get())->getThreadPool()->decFdsInUse();
        _acceptor->close();
        _acceptor = 0;
        notifyAll();
    }
}

void
IceInternal::IncomingConnectionFactory::exception(const LocalException&)
{
    assert(false); // Must not be called.
}

string
IceInternal::IncomingConnectionFactory::toString() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    if(_transceiver)
    {
        return _transceiver->toString();
    }
    
    assert(_acceptor);
    return _acceptor->toString();
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
IceInternal::IncomingConnectionFactory::connectionStartFailed(const Ice::ConnectionIPtr& connection,
                                                              const Ice::LocalException& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state == StateClosed)
    {
        return;
    }
    
    if(_warn)
    {
        Warning out(_instance->initializationData().logger);
        out << "connection exception:\n" << ex << '\n' << _acceptor->toString();
    }
        
    //
    // If the connection is finished, remove it right away from
    // the connection map. Otherwise, we keep it in the map, it
    // will eventually be reaped.
    //
    if(connection->isFinished())
    {
        _connections.remove(connection);
    }
}

IceInternal::IncomingConnectionFactory::IncomingConnectionFactory(const InstancePtr& instance,
                                                                  const EndpointIPtr& endpoint,
                                                                  const ObjectAdapterPtr& adapter,
                                                                  const string& adapterName) :
    EventHandler(instance),
    _endpoint(endpoint),
    _adapter(adapter),
    _registeredWithPool(false),
    _finishedCount(0),
    _warn(_instance->initializationData().properties->getPropertyAsInt("Ice.Warn.Connections") > 0),
    _state(StateHolding)
{
    if(_instance->defaultsAndOverrides()->overrideTimeout)
    {
        const_cast<EndpointIPtr&>(_endpoint) =
            _endpoint->timeout(_instance->defaultsAndOverrides()->overrideTimeoutValue);
    }

    if(_instance->defaultsAndOverrides()->overrideCompress)
    {
        const_cast<EndpointIPtr&>(_endpoint) =
            _endpoint->compress(_instance->defaultsAndOverrides()->overrideCompressValue);
    }

    ObjectAdapterI* adapterImpl = dynamic_cast<ObjectAdapterI*>(_adapter.get());
    _threadPerConnection = adapterImpl->getThreadPerConnection();
    _threadPerConnectionStackSize = adapterImpl->getThreadPerConnectionStackSize();

    const_cast<TransceiverPtr&>(_transceiver) = _endpoint->transceiver(const_cast<EndpointIPtr&>(_endpoint));
    if(_transceiver)
    {
        ConnectionIPtr connection;

        try
        {
            connection = new ConnectionI(_instance, _transceiver, _endpoint, _adapter, _threadPerConnection,
                                         _threadPerConnectionStackSize);
        }
        catch(const LocalException&)
        {
            try
            {
                _transceiver->close();
            }
            catch(const Ice::LocalException&)
            {
                // Ignore
            }
            throw;
        }

        connection->start(0);

        _connections.push_back(connection);
    }
    else
    {
        _acceptor = _endpoint->acceptor(const_cast<EndpointIPtr&>(_endpoint), adapterName);
        assert(_acceptor);
        _acceptor->listen();

        __setNoDelete(true);
        try
        {
            if(_threadPerConnection)
            {
                //
                // If we are in thread per connection mode, we also use
                // one thread per incoming connection factory, that
                // accepts new connections on this endpoint.
                //
                _threadPerIncomingConnectionFactory = new ThreadPerIncomingConnectionFactory(this);
                _threadPerIncomingConnectionFactory->start(_threadPerConnectionStackSize);
            }
            else
            {
                adapterImpl->getThreadPool()->incFdsInUse();
            }
        }
        catch(const IceUtil::Exception& ex)
        {
            if(_threadPerConnection)
            {
                Error out(_instance->initializationData().logger);
                out << "cannot create thread for incoming connection factory:\n" << ex;
            }
            
            try
            {
                _acceptor->close();
            }
            catch(const LocalException&)
            {
                // Here we ignore any exceptions in close().
            }
            
            __setNoDelete(false);
            ex.ice_throw();
        }
        __setNoDelete(false);
    }
}

IceInternal::IncomingConnectionFactory::~IncomingConnectionFactory()
{
    assert(_state == StateClosed);
    assert(!_acceptor);
    assert(_connections.empty());
    assert(!_threadPerIncomingConnectionFactory);
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
            if(!_threadPerConnection && _acceptor)
            {
                registerWithPool();
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
            if(!_threadPerConnection && _acceptor)
            {
                unregisterWithPool();
            }
            for_each(_connections.begin(), _connections.end(), Ice::voidMemFun(&ConnectionI::hold));
            break;
        }
        
        case StateClosed:
        {
            if(_acceptor)
            {
                if(_threadPerConnection)
                {
                    //
                    // If we are in thread per connection mode, we connect
                    // to our own acceptor, which unblocks our thread per
                    // incoming connection factory stuck in accept().
                    //
                    _acceptor->connectToSelf();
                }
                else
                {
                    //
                    // Otherwise we first must make sure that we are
                    // registered, then we unregister, and let finished()
                    // do the close.
                    //
                    registerWithPool();
                    unregisterWithPool();
                }
            }

#ifdef _STLP_BEGIN_NAMESPACE
            // voidbind2nd is an STLport extension for broken compilers in IceUtil/Functional.h
            for_each(_connections.begin(), _connections.end(),
                     voidbind2nd(Ice::voidMemFun1(&ConnectionI::destroy), ConnectionI::ObjectAdapterDeactivated));
#else
            for_each(_connections.begin(), _connections.end(),
                     bind2nd(Ice::voidMemFun1(&ConnectionI::destroy), ConnectionI::ObjectAdapterDeactivated));
#endif
            break;
        }
    }

    _state = state;
    notifyAll();
}

void
IceInternal::IncomingConnectionFactory::registerWithPool()
{
    assert(!_threadPerConnection); // Only for use with a thread pool.
    assert(_acceptor); // Not for datagram connections.

    if(!_registeredWithPool)
    {
        dynamic_cast<ObjectAdapterI*>(_adapter.get())->getThreadPool()->_register(_acceptor->fd(), this);
        _registeredWithPool = true;
    }
}

void
IceInternal::IncomingConnectionFactory::unregisterWithPool()
{
    assert(!_threadPerConnection); // Only for use with a thread pool.
    assert(_acceptor); // Not for datagram connections.

    if(_registeredWithPool)
    {
        dynamic_cast<ObjectAdapterI*>(_adapter.get())->getThreadPool()->unregister(_acceptor->fd());
        _registeredWithPool = false;
        ++_finishedCount; // For each unregistration, finished() is called once.
    }
}

void
IceInternal::IncomingConnectionFactory::run()
{
    assert(_acceptor);

    while(true)
    {
        //
        // We must accept new connections outside the thread
        // synchronization, because we use blocking accept.
        //
        TransceiverPtr transceiver;
        try
        {
            transceiver = _acceptor->accept(-1);
        }
        catch(const SocketException&)
        {
            // Ignore socket exceptions.
        }
        catch(const TimeoutException&)
        {
            // Ignore timeouts.
        }
        catch(const LocalException& ex)
        {
            // Warn about other Ice local exceptions.
            if(_warn)
            {
                Warning out(_instance->initializationData().logger);
                out << "connection exception:\n" << ex << '\n' << _acceptor->toString();
            }
        }
        
        ConnectionIPtr connection;
        
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
            
            while(_state == StateHolding)
            {
                wait();
            }
            
            if(_state == StateClosed)
            {
                if(transceiver)
                {
                    try
                    {
                        transceiver->close();
                    }
                    catch(const LocalException&)
                    {
                        // Here we ignore any exceptions in close().
                    }
                }
                
                try
                {
                    _acceptor->close();
                }
                catch(const LocalException& ex)
                {
                    _acceptor = 0;
                    notifyAll();
                    ex.ice_throw();
                }
                
                _acceptor = 0;
                notifyAll();
                return;
            }
            
            assert(_state == StateActive);
            
            //
            // Reap connections for which destruction has completed.
            //
            _connections.erase(remove_if(_connections.begin(), _connections.end(),
                                         Ice::constMemFun(&ConnectionI::isFinished)),
                               _connections.end());
            
            //
            // Create a connection object for the connection.
            //
            if(!transceiver)
            {
                continue;
            }

            try
            {
                connection = new ConnectionI(_instance, transceiver, _endpoint, _adapter, _threadPerConnection,
                                                 _threadPerConnectionStackSize);
            }
            catch(const LocalException& ex)
            {
                try
                {
                    transceiver->close();
                }
                catch(const Ice::LocalException&)
                {
                }

                if(_warn)
                {
                    Warning out(_instance->initializationData().logger);
                    out << "connection exception:\n" << ex << '\n' << _acceptor->toString();
                }
                continue;
            }

            _connections.push_back(connection);
        }
        
        //
        // In thread-per-connection mode and regardless of the background mode, 
        // start() doesn't block. The connection thread is started and takes 
        // care of the connection validation and notifies the factory through
        // the callback when it's done.
        //
        connection->start(this);
    }
}

IceInternal::IncomingConnectionFactory::ThreadPerIncomingConnectionFactory::ThreadPerIncomingConnectionFactory(
    const IncomingConnectionFactoryPtr& factory) :
    _factory(factory)
{
}

void
IceInternal::IncomingConnectionFactory::ThreadPerIncomingConnectionFactory::run()
{
    try
    {
        _factory->run();
    }
    catch(const std::exception& ex)
    {
        Error out(_factory->_instance->initializationData().logger);
        out << "exception in thread per incoming connection factory:\n" << _factory->toString() << ex.what();
    }
    catch(...)
    {
        Error out(_factory->_instance->initializationData().logger);
        out << "unknown exception in thread per incoming connection factory:\n" << _factory->toString();
    }

    _factory = 0; // Resolve cyclic dependency.
}
