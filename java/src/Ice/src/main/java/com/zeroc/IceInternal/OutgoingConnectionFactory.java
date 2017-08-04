// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

import java.util.concurrent.Callable;

import com.zeroc.Ice.ConnectionI;
import com.zeroc.Ice.LocalException;

public final class OutgoingConnectionFactory
{
    //
    // Helper class to multi hash map.
    //
    private static class MultiHashMap<K, V> extends java.util.HashMap<K, java.util.List<V>>
    {
        public void
        putOne(K key, V value)
        {
            java.util.List<V> list = this.get(key);
            if(list == null)
            {
                list = new java.util.LinkedList<>();
                this.put(key, list);
            }
            list.add(value);
        }

        public boolean
        removeElementWithValue(K key, V value)
        {
            java.util.List<V> list = this.get(key);
            assert(list != null);
            boolean v = list.remove(value);
            if(list.isEmpty())
            {
                this.remove(key);
            }
            return v;
        }

        public static final long serialVersionUID = 0L;
    }

    interface CreateConnectionCallback
    {
        void setConnection(ConnectionI connection, boolean compress);
        void setException(LocalException ex);
    }

    public synchronized void
    destroy()
    {
        if(_destroyed)
        {
            return;
        }

        for(java.util.List<ConnectionI> connectionList : _connections.values())
        {
            for(ConnectionI connection : connectionList)
            {
                connection.destroy(ConnectionI.CommunicatorDestroyed);
            }
        }

        _destroyed = true;
        _communicator = null;
        notifyAll();
    }

    public synchronized void
    updateConnectionObservers()
    {
        for(java.util.List<ConnectionI> connectionList : _connections.values())
        {
            for(ConnectionI connection : connectionList)
            {
                connection.updateObserver();
            }
        }
    }

    // Called from Instance.destroy().
    public void
    waitUntilFinished()
    {
        java.util.Map<Connector, java.util.List<ConnectionI> > connections = null;
        synchronized(this)
        {
            //
            // First we wait until the factory is destroyed. We also
            // wait until there are no pending connections
            // anymore. Only then we can be sure the _connections
            // contains all connections.
            //
            while(!_destroyed || !_pending.isEmpty() || _pendingConnectCount > 0)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                    throw new com.zeroc.Ice.OperationInterruptedException();
                }
            }

            //
            // We want to wait until all connections are finished outside the
            // thread synchronization.
            //
            connections = new java.util.HashMap<>(_connections);
        }

        //
        // Now we wait until the destruction of each connection is finished.
        //
        for(java.util.List<ConnectionI> connectionList : connections.values())
        {
            for(ConnectionI connection : connectionList)
            {
                try
                {
                    connection.waitUntilFinished();
                }
                catch(InterruptedException e)
                {
                    //
                    // Force close all of the connections.
                    //
                    for(java.util.List<ConnectionI> l : connections.values())
                    {
                        for(ConnectionI c : l)
                        {
                            c.close(com.zeroc.Ice.ConnectionClose.Forcefully);
                        }
                    }
                    throw new com.zeroc.Ice.OperationInterruptedException();
                }
            }
        }

        synchronized(this)
        {
            // Ensure all the connections are finished and reapable at this point.
            java.util.List<ConnectionI> cons = _monitor.swapReapedConnections();
            if(cons != null)
            {
                int size = 0;
                for(java.util.List<ConnectionI> connectionList : _connections.values())
                {
                    size += connectionList.size();
                }
                assert(cons.size() == size);
                _connections.clear();
                _connectionsByEndpoint.clear();
            }
            else
            {
                assert(_connections.isEmpty());
                assert(_connectionsByEndpoint.isEmpty());
            }
        }

        //
        // Must be destroyed outside the synchronization since this might block waiting for
        // a timer task to complete.
        //
        _monitor.destroy();
    }

    public void
    create(EndpointI[] endpts, boolean hasMore, com.zeroc.Ice.EndpointSelectionType selType,
           CreateConnectionCallback callback)
    {
        assert(endpts.length > 0);

        //
        // Apply the overrides.
        //
        java.util.List<EndpointI> endpoints = applyOverrides(endpts);

        //
        // Try to find a connection to one of the given endpoints.
        //
        try
        {
            Holder<Boolean> compress = new Holder<>();
            ConnectionI connection = findConnectionByEndpoint(endpoints, compress);
            if(connection != null)
            {
                callback.setConnection(connection, compress.value);
                return;
            }
        }
        catch(LocalException ex)
        {
            callback.setException(ex);
            return;
        }

        final ConnectCallback cb = new ConnectCallback(this, endpoints, hasMore, callback, selType);
        //
        // Calling cb.getConnectors() can eventually result in a call to connect() on a socket, which is not
        // allowed while in Android's main thread (with a dispatcher installed).
        //
        if(_instance.queueRequests())
        {
            _instance.getQueueExecutor().executeNoThrow(new Callable<Void>()
            {
                @Override
                public Void call()
                    throws Exception
                {
                    cb.getConnectors();
                    return null;
                }
            });
        }
        else
        {
            cb.getConnectors();
        }
    }

    public synchronized void
    setRouterInfo(RouterInfo routerInfo)
    {
        if(_destroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        assert(routerInfo != null);

        //
        // Search for connections to the router's client proxy
        // endpoints, and update the object adapter for such
        // connections, so that callbacks from the router can be
        // received over such connections.
        //
        com.zeroc.Ice.ObjectAdapter adapter = routerInfo.getAdapter();
        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        for(EndpointI endpoint : routerInfo.getClientEndpoints())
        {
            //
            // Modify endpoints with overrides.
            //
            if(defaultsAndOverrides.overrideTimeout)
            {
                endpoint = endpoint.timeout(defaultsAndOverrides.overrideTimeoutValue);
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
            endpoint = endpoint.compress(false);

            for(java.util.List<ConnectionI> connectionList : _connections.values())
            {
                for(ConnectionI connection : connectionList)
                {
                    if(connection.endpoint() == endpoint)
                    {
                        connection.setAdapter(adapter);
                    }
                }
            }
        }
    }

    public synchronized void
    removeAdapter(com.zeroc.Ice.ObjectAdapter adapter)
    {
        if(_destroyed)
        {
            return;
        }

        for(java.util.List<ConnectionI> connectionList : _connections.values())
        {
            for(ConnectionI connection : connectionList)
            {
                if(connection.getAdapter() == adapter)
                {
                    connection.setAdapter(null);
                }
            }
        }
    }

    public void
    flushAsyncBatchRequests(com.zeroc.Ice.CompressBatch compressBatch, CommunicatorFlushBatch outAsync)
    {
        java.util.List<ConnectionI> c = new java.util.LinkedList<>();

        synchronized(this)
        {
            if(!_destroyed)
            {
                for(java.util.List<ConnectionI> connectionList : _connections.values())
                {
                    for(ConnectionI connection : connectionList)
                    {
                        if(connection.isActiveOrHolding())
                        {
                            c.add(connection);
                        }
                    }
                }
            }
        }

        for(ConnectionI conn : c)
        {
            try
            {
                outAsync.flushConnection(conn, compressBatch);
            }
            catch(LocalException ex)
            {
                // Ignore.
            }
        }
    }

    //
    // Only for use by Instance.
    //
    OutgoingConnectionFactory(com.zeroc.Ice.Communicator communicator, Instance instance)
    {
        _communicator = communicator;
        _instance = instance;
        _monitor = new FactoryACMMonitor(instance, instance.clientACM());
        _destroyed = false;
    }

    @Override
    protected synchronized void
    finalize()
        throws Throwable
    {
        try
        {
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_destroyed);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_connections.isEmpty());
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_connectionsByEndpoint.isEmpty());
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_pendingConnectCount == 0);
            com.zeroc.IceUtilInternal.Assert.FinalizerAssert(_pending.isEmpty());
        }
        catch(java.lang.Exception ex)
        {
        }
        finally
        {
            super.finalize();
        }
    }

    private java.util.List<EndpointI>
    applyOverrides(EndpointI[] endpts)
    {
        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        java.util.List<EndpointI> endpoints = new java.util.ArrayList<>();
        for(EndpointI endpoint : endpts)
        {
            //
            // Modify endpoints with overrides.
            //
            if(defaultsAndOverrides.overrideTimeout)
            {
                endpoints.add(endpoint.timeout(defaultsAndOverrides.overrideTimeoutValue));
            }
            else
            {
                endpoints.add(endpoint);
            }
        }

        return endpoints;
    }

    synchronized private ConnectionI
    findConnectionByEndpoint(java.util.List<EndpointI> endpoints, Holder<Boolean> compress)
    {
        if(_destroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }

        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        assert(!endpoints.isEmpty());

        for(EndpointI endpoint : endpoints)
        {
            java.util.List<ConnectionI> connectionList = _connectionsByEndpoint.get(endpoint);
            if(connectionList == null)
            {
                continue;
            }

            for(ConnectionI connection : connectionList)
            {
                if(connection.isActiveOrHolding()) // Don't return destroyed or un-validated connections
                {
                    if(defaultsAndOverrides.overrideCompress)
                    {
                        compress.value = defaultsAndOverrides.overrideCompressValue;
                    }
                    else
                    {
                        compress.value = endpoint.compress();
                    }
                    return connection;
                }
            }
        }

        return null;
    }

    //
    // Must be called while synchronized.
    //
    private ConnectionI
    findConnection(java.util.List<ConnectorInfo> connectors, Holder<Boolean> compress)
    {
        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        for(ConnectorInfo ci : connectors)
        {
            if(_pending.containsKey(ci.connector))
            {
                continue;
            }

            java.util.List<ConnectionI> connectionList = _connections.get(ci.connector);
            if(connectionList == null)
            {
                continue;
            }

            for(ConnectionI connection : connectionList)
            {
                if(connection.isActiveOrHolding()) // Don't return destroyed or un-validated connections
                {
                    if(defaultsAndOverrides.overrideCompress)
                    {
                        compress.value = defaultsAndOverrides.overrideCompressValue;
                    }
                    else
                    {
                        compress.value = ci.endpoint.compress();
                    }
                    return connection;
                }
            }
        }

        return null;
    }

    synchronized private void
    incPendingConnectCount()
    {
        //
        // Keep track of the number of pending connects. The outgoing connection factory
        // waitUntilFinished() method waits for all the pending connects to terminate before
        // to return. This ensures that the communicator client thread pool isn't destroyed
        // too soon and will still be available to execute the ice_exception() callbacks for
        // the asynchronous requests waiting on a connection to be established.
        //

        if(_destroyed)
        {
            throw new com.zeroc.Ice.CommunicatorDestroyedException();
        }
        ++_pendingConnectCount;
    }

    synchronized private void
    decPendingConnectCount()
    {
        --_pendingConnectCount;
        assert(_pendingConnectCount >= 0);
        if(_destroyed && _pendingConnectCount == 0)
        {
            notifyAll();
        }
    }

    private ConnectionI
    getConnection(java.util.List<ConnectorInfo> connectors, ConnectCallback cb, Holder<Boolean> compress)
    {
        assert(cb != null);
        synchronized(this)
        {
            if(_destroyed)
            {
                throw new com.zeroc.Ice.CommunicatorDestroyedException();
            }

            //
            // Reap closed connections
            //
            java.util.List<ConnectionI> cons = _monitor.swapReapedConnections();
            if(cons != null)
            {
                for(ConnectionI c : cons)
                {
                    _connections.removeElementWithValue(c.connector(), c);
                    _connectionsByEndpoint.removeElementWithValue(c.endpoint(), c);
                    _connectionsByEndpoint.removeElementWithValue(c.endpoint().compress(true), c);
                }
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
                    throw new com.zeroc.Ice.CommunicatorDestroyedException();
                }

                //
                // Search for a matching connection. If we find one, we're done.
                //
                ConnectionI connection = findConnection(connectors, compress);
                if(connection != null)
                {
                    return connection;
                }

                if(addToPending(cb, connectors))
                {
                    return null;
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
        if(cb != null)
        {
            cb.nextConnector();
        }

        return null;
    }

    private synchronized ConnectionI
    createConnection(Transceiver transceiver, ConnectorInfo ci)
    {
        assert(_pending.containsKey(ci.connector) && transceiver != null);

        //
        // Create and add the connection to the connection map. Adding the connection to the map
        // is necessary to support the interruption of the connection initialization and validation
        // in case the communicator is destroyed.
        //
        ConnectionI connection = null;
        try
        {
            if(_destroyed)
            {
                throw new com.zeroc.Ice.CommunicatorDestroyedException();
            }

            connection = new ConnectionI(_communicator, _instance, _monitor, transceiver, ci.connector,
                                             ci.endpoint.compress(false), null);
        }
        catch(LocalException ex)
        {
            try
            {
                transceiver.close();
            }
            catch(LocalException exc)
            {
                // Ignore
            }
            throw ex;
        }

        _connections.putOne(ci.connector, connection);
        _connectionsByEndpoint.putOne(connection.endpoint(), connection);
        _connectionsByEndpoint.putOne(connection.endpoint().compress(true), connection);
        return connection;
    }

    private void
    finishGetConnection(java.util.List<ConnectorInfo> connectors,
                        ConnectorInfo ci,
                        ConnectionI connection,
                        ConnectCallback cb)
    {
        java.util.Set<ConnectCallback> connectionCallbacks = new java.util.HashSet<>();
        if(cb != null)
        {
            connectionCallbacks.add(cb);
        }

        java.util.Set<ConnectCallback> callbacks = new java.util.HashSet<>();
        synchronized(this)
        {
            for(ConnectorInfo c : connectors)
            {
                java.util.Set<ConnectCallback> cbs = _pending.remove(c.connector);
                if(cbs != null)
                {
                    for(ConnectCallback cc : cbs)
                    {
                        if(cc.hasConnector(ci))
                        {
                            connectionCallbacks.add(cc);
                        }
                        else
                        {
                            callbacks.add(cc);
                        }
                    }
                }
            }

            for(ConnectCallback cc : connectionCallbacks)
            {
                cc.removeFromPending();
                callbacks.remove(cc);
            }
            for(ConnectCallback cc : callbacks)
            {
                cc.removeFromPending();
            }
            notifyAll();
        }

        boolean compress;
        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        if(defaultsAndOverrides.overrideCompress)
        {
            compress = defaultsAndOverrides.overrideCompressValue;
        }
        else
        {
            compress = ci.endpoint.compress();
        }

        for(ConnectCallback cc : callbacks)
        {
            cc.getConnection();
        }
        for(ConnectCallback cc : connectionCallbacks)
        {
            cc.setConnection(connection, compress);
        }
    }

    private void
    finishGetConnection(java.util.List<ConnectorInfo> connectors, LocalException ex, ConnectCallback cb)
    {
        java.util.Set<ConnectCallback> failedCallbacks = new java.util.HashSet<>();
        if(cb != null)
        {
            failedCallbacks.add(cb);
        }

        java.util.Set<ConnectCallback> callbacks = new java.util.HashSet<>();
        synchronized(this)
        {
            for(ConnectorInfo c : connectors)
            {
                java.util.Set<ConnectCallback> cbs = _pending.remove(c.connector);
                if(cbs != null)
                {
                    for(ConnectCallback cc : cbs)
                    {
                        if(cc.removeConnectors(connectors))
                        {
                            failedCallbacks.add(cc);
                        }
                        else
                        {
                            callbacks.add(cc);
                        }
                    }
                }
            }

            for(ConnectCallback cc : callbacks)
            {
                assert(!failedCallbacks.contains(cc));
                cc.removeFromPending();
            }
            notifyAll();
        }

        for(ConnectCallback cc : callbacks)
        {
            cc.getConnection();
        }
        for(ConnectCallback cc : failedCallbacks)
        {
            cc.setException(ex);
        }
    }

    private boolean
    addToPending(ConnectCallback cb, java.util.List<ConnectorInfo> connectors)
    {
        //
        // Add the callback to each connector pending list.
        //
        boolean found = false;
        for(ConnectorInfo p : connectors)
        {
            java.util.Set<ConnectCallback> cbs = _pending.get(p.connector);
            if(cbs != null)
            {
                found = true;
                if(cb != null)
                {
                    cbs.add(cb); // Add the callback to each pending connector.
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
        for(ConnectorInfo p : connectors)
        {
            if(!_pending.containsKey(p.connector))
            {
                _pending.put(p.connector, new java.util.HashSet<>());
            }
        }

        return false;
    }

    private void
    removeFromPending(ConnectCallback cb, java.util.List<ConnectorInfo> connectors)
    {
        for(ConnectorInfo p : connectors)
        {
            java.util.Set<ConnectCallback> cbs = _pending.get(p.connector);
            if(cbs != null)
            {
                cbs.remove(cb);
            }
        }
    }

    private void
    handleConnectionException(LocalException ex, boolean hasMore)
    {
        TraceLevels traceLevels = _instance.traceLevels();
        if(traceLevels.network >= 2)
        {
            StringBuilder s = new StringBuilder(128);
            s.append("connection to endpoint failed");
            if(ex instanceof com.zeroc.Ice.CommunicatorDestroyedException)
            {
                s.append("\n");
            }
            else
            {
                if(hasMore)
                {
                    s.append(", trying next endpoint\n");
                }
                else
                {
                    s.append(" and no more endpoints to try\n");
                }
            }
            s.append(ex.toString());
            _instance.initializationData().logger.trace(traceLevels.networkCat, s.toString());
        }
    }

    private void
    handleException(LocalException ex, boolean hasMore)
    {
        TraceLevels traceLevels = _instance.traceLevels();
        if(traceLevels.network >= 2)
        {
            StringBuilder s = new StringBuilder(128);
            s.append("couldn't resolve endpoint host");
            if(ex instanceof com.zeroc.Ice.CommunicatorDestroyedException)
            {
                s.append("\n");
            }
            else
            {
                if(hasMore)
                {
                    s.append(", trying next endpoint\n");
                }
                else
                {
                    s.append(" and no more endpoints to try\n");
                }
            }
            s.append(ex.toString());
            _instance.initializationData().logger.trace(traceLevels.networkCat, s.toString());
        }
    }

    private static class ConnectorInfo
    {
        public ConnectorInfo(Connector c, EndpointI e)
        {
            connector = c;
            endpoint = e;
        }

        @Override
        public boolean
        equals(Object obj)
        {
            ConnectorInfo r = (ConnectorInfo)obj;
            return connector.equals(r.connector);
        }

        @Override
        public int
        hashCode()
        {
            return connector.hashCode();
        }

        public Connector connector;
        public EndpointI endpoint;
    }

    private static class ConnectCallback implements ConnectionI.StartCallback, EndpointI_connectors
    {
        ConnectCallback(OutgoingConnectionFactory f, java.util.List<EndpointI> endpoints, boolean more,
                        CreateConnectionCallback cb, com.zeroc.Ice.EndpointSelectionType selType)
        {
            _factory = f;
            _endpoints = endpoints;
            _hasMore = more;
            _callback = cb;
            _selType = selType;
            _endpointsIter = _endpoints.iterator();
        }

        //
        // Methods from ConnectionI.StartCallback
        //
        @Override
        public void
        connectionStartCompleted(ConnectionI connection)
        {
            if(_observer != null)
            {
                _observer.detach();
            }
            connection.activate();
            _factory.finishGetConnection(_connectors, _current, connection, this);
        }

        @Override
        public void
        connectionStartFailed(ConnectionI connection, LocalException ex)
        {
            assert(_current != null);
            if(connectionStartFailedImpl(ex))
            {
                nextConnector();
            }
        }

        //
        // Methods from EndpointI_connectors
        //
        @Override
        public void
        connectors(java.util.List<Connector> cons)
        {
            for(Connector p : cons)
            {
                _connectors.add(new ConnectorInfo(p, _currentEndpoint));
            }

            if(_endpointsIter.hasNext())
            {
                nextEndpoint();
            }
            else
            {
                assert(!_connectors.isEmpty());

                //
                // We now have all the connectors for the given endpoints. We can try to obtain the
                // connection.
                //
                _iter = _connectors.iterator();
                getConnection();
            }
        }

        @Override
        public void
        exception(LocalException ex)
        {
            _factory.handleException(ex, _hasMore || _endpointsIter.hasNext());
            if(_endpointsIter.hasNext())
            {
                nextEndpoint();
            }
            else if(!_connectors.isEmpty())
            {
                //
                // We now have all the connectors for the given endpoints. We can try to obtain the
                // connection.
                //
                _iter = _connectors.iterator();
                getConnection();
            }
            else
            {
                _callback.setException(ex);
                _factory.decPendingConnectCount(); // Must be called last.
            }
        }

        void
        setConnection(ConnectionI connection, boolean compress)
        {
            //
            // Callback from the factory: the connection to one of the callback
            // connectors has been established.
            //
            _callback.setConnection(connection, compress);
            _factory.decPendingConnectCount(); // Must be called last.
        }

        void
        setException(LocalException ex)
        {
            //
            // Callback from the factory: connection establishment failed.
            //
            _callback.setException(ex);
            _factory.decPendingConnectCount(); // Must be called last.
        }

        boolean
        hasConnector(ConnectorInfo ci)
        {
            return _connectors.contains(ci);
        }

        boolean
        removeConnectors(java.util.List<ConnectorInfo> connectors)
        {
            _connectors.removeAll(connectors);
            _iter = _connectors.iterator();
            return _connectors.isEmpty();
        }

        void
        removeFromPending()
        {
            _factory.removeFromPending(this, _connectors);
        }

        private void
        getConnectors()
        {
            try
            {
                //
                // Notify the factory that there's an async connect pending. This is necessary
                // to prevent the outgoing connection factory to be destroyed before all the
                // pending asynchronous connects are finished.
                //
                _factory.incPendingConnectCount();
            }
            catch(LocalException ex)
            {
                _callback.setException(ex);
                return;
            }

            nextEndpoint();
        }

        private void
        nextEndpoint()
        {
            try
            {
                assert(_endpointsIter.hasNext());
                _currentEndpoint = _endpointsIter.next();
                _currentEndpoint.connectors_async(_selType, this);
            }
            catch(LocalException ex)
            {
                exception(ex);
            }
        }

        private void
        getConnection()
        {
            try
            {
                //
                // If all the connectors have been created, we ask the factory to get a
                // connection.
                //
                Holder<Boolean> compress = new Holder<>();
                ConnectionI connection = _factory.getConnection(_connectors, this, compress);
                if(connection == null)
                {
                    //
                    // A null return value from getConnection indicates that the connection
                    // is being established and that everything has been done to ensure that
                    // the callback will be notified when the connection establishment is
                    // done.
                    //
                    return;
                }

                _callback.setConnection(connection, compress.value);
                _factory.decPendingConnectCount(); // Must be called last.
            }
            catch(LocalException ex)
            {
                _callback.setException(ex);
                _factory.decPendingConnectCount(); // Must be called last.
            }
        }

        private void
        nextConnector()
        {
            while(true)
            {
                try
                {
                    assert(_iter.hasNext());
                    _current = _iter.next();

                    com.zeroc.Ice.Instrumentation.CommunicatorObserver obsv =
                        _factory._instance.initializationData().observer;
                    if(obsv != null)
                    {
                        _observer = obsv.getConnectionEstablishmentObserver(_current.endpoint,
                                                                            _current.connector.toString());
                        if(_observer != null)
                        {
                            _observer.attach();
                        }
                    }

                    if(_factory._instance.traceLevels().network >= 2)
                    {
                        StringBuffer s = new StringBuffer("trying to establish ");
                        s.append(_current.endpoint.protocol());
                        s.append(" connection to ");
                        s.append(_current.connector.toString());
                        _factory._instance.initializationData().logger.trace(
                            _factory._instance.traceLevels().networkCat, s.toString());
                    }

                    ConnectionI connection =
                        _factory.createConnection(_current.connector.connect(), _current);
                    connection.start(this);
                }
                catch(LocalException ex)
                {
                    if(_factory._instance.traceLevels().network >= 2)
                    {
                        StringBuffer s = new StringBuffer("failed to establish ");
                        s.append(_current.endpoint.protocol());
                        s.append(" connection to ");
                        s.append(_current.connector.toString());
                        s.append("\n");
                        s.append(ex);
                        _factory._instance.initializationData().logger.trace(
                            _factory._instance.traceLevels().networkCat, s.toString());
                    }

                    if(connectionStartFailedImpl(ex))
                    {
                        continue;
                    }
                }
                break;
            }
        }

        private boolean
        connectionStartFailedImpl(LocalException ex)
        {
            if(_observer != null)
            {
                _observer.failed(ex.ice_id());
                _observer.detach();
            }

            _factory.handleConnectionException(ex, _hasMore || _iter.hasNext());
            if(ex instanceof com.zeroc.Ice.CommunicatorDestroyedException) // No need to continue.
            {
                _factory.finishGetConnection(_connectors, ex, this);
            }
            else if(_iter.hasNext()) // Try the next connector.
            {
                return true;
            }
            else
            {
                _factory.finishGetConnection(_connectors, ex, this);
            }
            return false;
        }

        private final OutgoingConnectionFactory _factory;
        private final boolean _hasMore;
        private final CreateConnectionCallback _callback;
        private final java.util.List<EndpointI> _endpoints;
        private final com.zeroc.Ice.EndpointSelectionType _selType;
        private java.util.Iterator<EndpointI> _endpointsIter;
        private EndpointI _currentEndpoint;
        private java.util.List<ConnectorInfo> _connectors = new java.util.ArrayList<>();
        private java.util.Iterator<ConnectorInfo> _iter;
        private ConnectorInfo _current;
        private com.zeroc.Ice.Instrumentation.Observer _observer;
    }

    private com.zeroc.Ice.Communicator _communicator;
    private final Instance _instance;
    private final FactoryACMMonitor _monitor;
    private boolean _destroyed;

    private MultiHashMap<Connector, ConnectionI> _connections = new MultiHashMap<>();
    private MultiHashMap<EndpointI, ConnectionI> _connectionsByEndpoint = new MultiHashMap<>();
    private java.util.Map<Connector, java.util.HashSet<ConnectCallback> > _pending = new java.util.HashMap<>();
    private int _pendingConnectCount = 0;
}
