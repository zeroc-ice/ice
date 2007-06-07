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
             voidbind2nd(Ice::secondVoidMemFun1<ConnectorPtr, ConnectionI, ConnectionI::DestructionReason>
                         (&ConnectionI::destroy), ConnectionI::CommunicatorDestroyed));
#else
    for_each(_connections.begin(), _connections.end(),
             bind2nd(Ice::secondVoidMemFun1<const ConnectorPtr, ConnectionI, ConnectionI::DestructionReason>
                     (&ConnectionI::destroy), ConnectionI::CommunicatorDestroyed));
#endif

    _destroyed = true;
    notifyAll();
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
        while(!_destroyed || !_pending.empty())
        {
            wait();
        }

        //
        // We want to wait until all connections are finished outside the
        // thread synchronization.
        //
        connections.swap(_connections);
    }

    for_each(connections.begin(), connections.end(),
             Ice::secondVoidMemFun<const ConnectorPtr, ConnectionI>(&ConnectionI::waitUntilFinished));
}

ConnectionIPtr
IceInternal::OutgoingConnectionFactory::create(const vector<EndpointIPtr>& endpts, bool moreEndpts,
                                               bool threadPerConnection, Ice::EndpointSelectionType selType, 
                                               bool& compress)
{
    assert(!endpts.empty());
    vector<pair<ConnectorPtr, EndpointIPtr> > connectors;

    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        if(_destroyed)
        {
            throw CommunicatorDestroyedException(__FILE__, __LINE__);
        }

        //
        // Reap connections for which destruction has completed.
        //
        std::multimap<ConnectorPtr, ConnectionIPtr>::iterator p = _connections.begin();
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

        vector<EndpointIPtr> endpoints = endpts;
        vector<EndpointIPtr>::iterator q;
        for(q = endpoints.begin(); q != endpoints.end(); ++q)
        {
            //
            // Modify endpoints with overrides.
            //
            if(_instance->defaultsAndOverrides()->overrideTimeout)
            {
                *q = (*q)->timeout(_instance->defaultsAndOverrides()->overrideTimeoutValue);
            }

            //
            // Create connectors for the endpoints.
            //
            vector<ConnectorPtr> cons = (*q)->connectors();
            assert(cons.size() > 0);

            //
            // Shuffle connectors is endpoint selection type is Random.
            //
            if(selType == Random)
            {
                RandomNumberGenerator rng;
                random_shuffle(cons.begin(), cons.end(), rng);
            }

            vector<ConnectorPtr>::const_iterator r;
            for(r = cons.begin(); r != cons.end(); ++r)
            {
                connectors.push_back(make_pair(*r, *q));
            }
        }

        //
        // Search for existing connections.
        //
        vector<pair<ConnectorPtr, EndpointIPtr> >::const_iterator r;
        for(r = connectors.begin(); r != connectors.end(); ++r)
        {
            pair<multimap<ConnectorPtr, ConnectionIPtr>::iterator,
                 multimap<ConnectorPtr, ConnectionIPtr>::iterator> pr = _connections.equal_range((*r).first);
            
            while(pr.first != pr.second)
            {
                //
                // Don't return connections for which destruction has
                // been initiated. The connection must also match the
                // requested thread-per-connection setting.
                //
                if(!pr.first->second->isDestroyed() &&
                   pr.first->second->threadPerConnection() == threadPerConnection)
                {
                    if(_instance->defaultsAndOverrides()->overrideCompress)
                    {
                        compress = _instance->defaultsAndOverrides()->overrideCompressValue;
                    }
                    else
                    {
                        compress = (*r).second->compress();
                    }

                    return pr.first->second;
                }

                ++pr.first;
            }
        }

        //
        // If some other thread is currently trying to establish a
        // connection to any of our endpoints, we wait until this
        // thread is finished.
        //
        bool searchAgain = false;
        while(!_destroyed)
        {
            for(r = connectors.begin(); r != connectors.end(); ++r)
            {
                if(_pending.find((*r).first) != _pending.end())
                {
                    break;
                }
            }

            if(r == connectors.end())
            {
                break;
            }

            searchAgain = true;

            wait();
        }

        if(_destroyed)
        {
            throw CommunicatorDestroyedException(__FILE__, __LINE__);
        }

        //
        // Search for existing connections again if we waited above,
        // as new connections might have been added in the meantime.
        //
        if(searchAgain)
        {       
            for(r = connectors.begin(); r != connectors.end(); ++r)
            {
                pair<multimap<ConnectorPtr, ConnectionIPtr>::iterator,
                     multimap<ConnectorPtr, ConnectionIPtr>::iterator> pr = _connections.equal_range((*r).first);
                
                while(pr.first != pr.second)
                {
                    //
                    // Don't return connections for which destruction has
                    // been initiated. The connection must also match the
                    // requested thread-per-connection setting.
                    //
                    if(!pr.first->second->isDestroyed() &&
                       pr.first->second->threadPerConnection() == threadPerConnection)
                    {
                        if(_instance->defaultsAndOverrides()->overrideCompress)
                        {
                            compress = _instance->defaultsAndOverrides()->overrideCompressValue;
                        }
                        else
                        {
                            compress = (*r).second->compress();
                        }

                        return pr.first->second;
                    }

                    ++pr.first;
                }
            }
        }

        //
        // No connection to any of our endpoints exists yet, so we
        // will try to create one. To avoid that other threads try to
        // create connections to the same endpoints, we add our
        // endpoints to _pending.
        //
        for(r = connectors.begin(); r != connectors.end(); ++r)
        {
            _pending.insert((*r).first);
        }
    }

    ConnectorPtr connector;
    ConnectionIPtr connection;
    auto_ptr<LocalException> exception;
    
    vector<pair<ConnectorPtr, EndpointIPtr> >::const_iterator q;
    for(q = connectors.begin(); q != connectors.end(); ++q)
    {
        connector = (*q).first;
        EndpointIPtr endpoint = (*q).second;
        
        try
        {

            int timeout;
            if(_instance->defaultsAndOverrides()->overrideConnectTimeout)
            {
                timeout = _instance->defaultsAndOverrides()->overrideConnectTimeoutValue;
            }
            // It is not necessary to check for overrideTimeout,
            // the endpoint has already been modified with this
            // override, if set.
            else
            {
                timeout = endpoint->timeout();
            }

            TransceiverPtr transceiver = connector->connect(timeout);
            assert(transceiver);

            connection = new ConnectionI(_instance, transceiver, endpoint->compress(false), 0, threadPerConnection,
                                                 _instance->threadPerConnectionStackSize());
            connection->start();
            connection->validate();

            if(_instance->defaultsAndOverrides()->overrideCompress)
            {
                compress = _instance->defaultsAndOverrides()->overrideCompressValue;
            }
            else
            {
                compress = endpoint->compress();
            }
            break;
        }
        catch(const LocalException& ex)
        {
            exception.reset(dynamic_cast<LocalException*>(ex.ice_clone()));

            //
            // If a connection object was constructed, then validate()
            // must have raised the exception.
            //
            if(connection)
            {
                connection->waitUntilFinished(); // We must call waitUntilFinished() for cleanup.
                connection = 0;
            }
        }
        
        TraceLevelsPtr traceLevels = _instance->traceLevels();
        if(traceLevels->retry >= 2)
        {
            Trace out(_instance->initializationData().logger, traceLevels->retryCat);

            out << "connection to endpoint failed";
            if(moreEndpts || q + 1 != connectors.end())
            {
                out << ", trying next endpoint\n";
            }
            else
            {
                out << " and no more endpoints to try\n";
            }
            out << *exception.get();
        }
    }
    
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        
        //
        // Signal other threads that we are done with trying to
        // establish connections to our endpoints.
        //
        for(q = connectors.begin(); q != connectors.end(); ++q)
        {
            _pending.erase((*q).first);
        }
        notifyAll();

        if(!connection)
        {
            assert(exception.get());
            exception->ice_throw();
        }
        else
        {
            _connections.insert(_connections.end(), pair<const ConnectorPtr, ConnectionIPtr>(connector, connection));

            if(_destroyed)
            {
                connection->destroy(ConnectionI::CommunicatorDestroyed);
                throw CommunicatorDestroyedException(__FILE__, __LINE__);
            }
            else
            {
                connection->activate();
            }
        }
    }

    assert(connection);
    return connection;
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

        multimap<ConnectorPtr, ConnectionIPtr>::const_iterator q;
        for(q = _connections.begin(); q != _connections.end(); ++q)
        {
            if((*q).second->endpoint() == endpoint)
            {
                try
                {
                    (*q).second->setAdapter(adapter);
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
    
    for(multimap<ConnectorPtr, ConnectionIPtr>::const_iterator p = _connections.begin(); p != _connections.end(); ++p)
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

        for(std::multimap<ConnectorPtr, ConnectionIPtr>::const_iterator p = _connections.begin();
            p != _connections.end();
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
    _destroyed(false)
{
}

IceInternal::OutgoingConnectionFactory::~OutgoingConnectionFactory()
{
    assert(_destroyed);
    assert(_connections.empty());
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
        connections.swap(_connections);
    }

    if(threadPerIncomingConnectionFactory)
    {
        threadPerIncomingConnectionFactory->getThreadControl().join();
    }

    for_each(connections.begin(), connections.end(), Ice::voidMemFun(&ConnectionI::waitUntilFinished));
}

EndpointIPtr
IceInternal::IncomingConnectionFactory::endpoint() const
{
    // No mutex protection necessary, _endpoint is immutable.
    return _endpoint;
}

bool
IceInternal::IncomingConnectionFactory::equivalent(const EndpointIPtr& endp) const
{
    if(_transceiver)
    {
        return endp->equivalent(_transceiver);
    }
    
    assert(_acceptor);
    return endp->equivalent(_acceptor);
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
                   Ice::constMemFun(&ConnectionI::isDestroyed));

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

void
IceInternal::IncomingConnectionFactory::read(BasicStream&)
{
    assert(!_threadPerConnection); // Only for use with a thread pool.
    assert(false); // Must not be called.
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
            connection->start();
        }
        catch(const LocalException&)
        {
            return;
        }

        _connections.push_back(connection);
    }

    assert(connection);

    //
    // We validate outside the thread synchronization, to not block
    // the factory.
    //
    try
    {
        connection->validate();
    }
    catch(const LocalException&)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        connection->waitUntilFinished(); // We must call waitUntilFinished() for cleanup.
        _connections.remove(connection);
        return;
    }

    connection->activate();
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
            connection->start();
            connection->validate();
        }
        catch(const LocalException&)
        {
            //
            // If a connection object was constructed, then validate()
            // must have raised the exception.
            //
            if(connection)
            {
                connection->waitUntilFinished(); // We must call waitUntilFinished() for cleanup.
            }

            return;
        }

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
            if(transceiver)
            {
                try
                {
                    connection = new ConnectionI(_instance, transceiver, _endpoint, _adapter, _threadPerConnection,
                                                 _threadPerConnectionStackSize);
                    connection->start();
                }
                catch(const LocalException&)
                {
                    return;
                }

                _connections.push_back(connection);
            }
        }
        
        //
        // In thread per connection mode, the connection's thread will
        // take care of connection validation and activation (for
        // non-datagram connections). We don't want to block this
        // thread waiting until validation is complete, because in
        // contrast to thread pool mode, it is the only thread that
        // can accept connections with this factory's
        // acceptor. Therefore we don't call validate() and activate()
        // from the connection factory in thread per connection mode.
        //
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
    catch(const Exception& ex)
    {   
        Error out(_factory->_instance->initializationData().logger);
        out << "exception in thread per incoming connection factory:\n" << _factory->toString() << ex; 
    }
    catch(const std::exception& ex)
    {
        Error out(_factory->_instance->initializationData().logger);
        out << "std::exception in thread per incoming connection factory:\n" << _factory->toString() << ex.what();
    }
    catch(...)
    {
        Error out(_factory->_instance->initializationData().logger);
        out << "unknown exception in thread per incoming connection factory:\n" << _factory->toString();
    }

    _factory = 0; // Resolve cyclic dependency.
}
