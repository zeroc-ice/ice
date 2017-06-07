// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

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
                list = new java.util.LinkedList<V>();
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
    }

    interface CreateConnectionCallback
    {
        void setConnection(Ice.ConnectionI connection, boolean compress);
        void setException(Ice.LocalException ex);
    }

    public synchronized void
    destroy()
    {
        if(_destroyed)
        {
            return;
        }

        for(java.util.List<Ice.ConnectionI> connectionList : _connections.values())
        {
            for(Ice.ConnectionI connection : connectionList)
            {
                connection.destroy(Ice.ConnectionI.CommunicatorDestroyed);
            }
        }

        _destroyed = true;
        _communicator = null;
        notifyAll();
    }

    public synchronized void
    updateConnectionObservers()
    {
        for(java.util.List<Ice.ConnectionI> connectionList : _connections.values())
        {
            for(Ice.ConnectionI connection : connectionList)
            {
                connection.updateObserver();
            }
        }
    }

    // Called from Instance.destroy().
    public void
    waitUntilFinished()
    {
        java.util.Map<Connector, java.util.List<Ice.ConnectionI> > connections = null;
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
                    throw new Ice.OperationInterruptedException();
                }
            }

            //
            // We want to wait until all connections are finished outside the
            // thread synchronization.
            //
            connections = new java.util.HashMap<Connector, java.util.List<Ice.ConnectionI> >(_connections);
        }

        //
        // Now we wait until the destruction of each connection is finished.
        //
        for(java.util.List<Ice.ConnectionI> connectionList : connections.values())
        {
            for(Ice.ConnectionI connection : connectionList)
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
                    for(java.util.List<Ice.ConnectionI> l : connections.values())
                    {
                        for(Ice.ConnectionI c : l)
                        {
                            c.close(Ice.ConnectionClose.Forcefully);
                        }
                    }
                    throw new Ice.OperationInterruptedException();
                }
            }
        }

        synchronized(this)
        {
            // Ensure all the connections are finished and reapable at this point.
            java.util.List<Ice.ConnectionI> cons = _monitor.swapReapedConnections();
            if(cons != null)
            {
                int size = 0;
                for(java.util.List<Ice.ConnectionI> connectionList : _connections.values())
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
            _monitor.destroy();
        }
    }

    public void
    create(EndpointI[] endpts, boolean hasMore, Ice.EndpointSelectionType selType, CreateConnectionCallback callback)
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
            Ice.Holder<Boolean> compress = new Ice.Holder<Boolean>();
            Ice.ConnectionI connection = findConnectionByEndpoint(endpoints, compress);
            if(connection != null)
            {
                callback.setConnection(connection, compress.value);
                return;
            }
        }
        catch(Ice.LocalException ex)
        {
            callback.setException(ex);
            return;
        }

        ConnectCallback cb = new ConnectCallback(this, endpoints, hasMore, callback, selType);
        cb.getConnectors();
    }

    public synchronized void
    setRouterInfo(IceInternal.RouterInfo routerInfo)
    {
        if(_destroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        assert(routerInfo != null);

        //
        // Search for connections to the router's client proxy
        // endpoints, and update the object adapter for such
        // connections, so that callbacks from the router can be
        // received over such connections.
        //
        Ice.ObjectAdapter adapter = routerInfo.getAdapter();
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

            for(java.util.List<Ice.ConnectionI> connectionList : _connections.values())
            {
                for(Ice.ConnectionI connection : connectionList)
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
    removeAdapter(Ice.ObjectAdapter adapter)
    {
        if(_destroyed)
        {
            return;
        }

        for(java.util.List<Ice.ConnectionI> connectionList : _connections.values())
        {
            for(Ice.ConnectionI connection : connectionList)
            {
                if(connection.getAdapter() == adapter)
                {
                    connection.setAdapter(null);
                }
            }
        }
    }

    public void
    flushAsyncBatchRequests(Ice.CompressBatch compressBatch, CommunicatorFlushBatch outAsync)
    {
        java.util.List<Ice.ConnectionI> c = new java.util.LinkedList<Ice.ConnectionI>();

        synchronized(this)
        {
            if(!_destroyed)
            {
                for(java.util.List<Ice.ConnectionI> connectionList : _connections.values())
                {
                    for(Ice.ConnectionI connection : connectionList)
                    {
                        if(connection.isActiveOrHolding())
                        {
                            c.add(connection);
                        }
                    }
                }
            }
        }

        for(Ice.ConnectionI conn : c)
        {
            try
            {
                outAsync.flushConnection(conn, compressBatch);
            }
            catch(Ice.LocalException ex)
            {
                // Ignore.
            }
        }
    }

    //
    // Only for use by Instance.
    //
    OutgoingConnectionFactory(Ice.Communicator communicator, Instance instance)
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
            IceUtilInternal.Assert.FinalizerAssert(_destroyed);
            IceUtilInternal.Assert.FinalizerAssert(_connections.isEmpty());
            IceUtilInternal.Assert.FinalizerAssert(_connectionsByEndpoint.isEmpty());
            IceUtilInternal.Assert.FinalizerAssert(_pendingConnectCount == 0);
            IceUtilInternal.Assert.FinalizerAssert(_pending.isEmpty());
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
        java.util.List<EndpointI> endpoints = new java.util.ArrayList<EndpointI>();
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

    synchronized private Ice.ConnectionI
    findConnectionByEndpoint(java.util.List<EndpointI> endpoints, Ice.Holder<Boolean> compress)
    {
        if(_destroyed)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        assert(!endpoints.isEmpty());

        for(EndpointI endpoint : endpoints)
        {
            java.util.List<Ice.ConnectionI> connectionList = _connectionsByEndpoint.get(endpoint);
            if(connectionList == null)
            {
                continue;
            }

            for(Ice.ConnectionI connection : connectionList)
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
    private Ice.ConnectionI
    findConnection(java.util.List<ConnectorInfo> connectors, Ice.Holder<Boolean> compress)
    {
        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        for(ConnectorInfo ci : connectors)
        {
            if(_pending.containsKey(ci.connector))
            {
                continue;
            }

            java.util.List<Ice.ConnectionI> connectionList = _connections.get(ci.connector);
            if(connectionList == null)
            {
                continue;
            }

            for(Ice.ConnectionI connection : connectionList)
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
            throw new Ice.CommunicatorDestroyedException();
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

    private Ice.ConnectionI
    getConnection(java.util.List<ConnectorInfo> connectors, ConnectCallback cb, Ice.Holder<Boolean> compress)
    {
        assert(cb != null);
        synchronized(this)
        {
            if(_destroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            //
            // Reap closed connections
            //
            java.util.List<Ice.ConnectionI> cons = _monitor.swapReapedConnections();
            if(cons != null)
            {
                for(Ice.ConnectionI c : cons)
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
                    throw new Ice.CommunicatorDestroyedException();
                }

                //
                // Search for a matching connection. If we find one, we're done.
                //
                Ice.ConnectionI connection = findConnection(connectors, compress);
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

    private synchronized Ice.ConnectionI
    createConnection(Transceiver transceiver, ConnectorInfo ci)
    {
        assert(_pending.containsKey(ci.connector) && transceiver != null);

        //
        // Create and add the connection to the connection map. Adding the connection to the map
        // is necessary to support the interruption of the connection initialization and validation
        // in case the communicator is destroyed.
        //
        Ice.ConnectionI connection = null;
        try
        {
            if(_destroyed)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            connection = new Ice.ConnectionI(_communicator, _instance, _monitor, transceiver, ci.connector,
                                             ci.endpoint.compress(false), null);
        }
        catch(Ice.LocalException ex)
        {
            try
            {
                transceiver.close();
            }
            catch(Ice.LocalException exc)
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
                        Ice.ConnectionI connection,
                        ConnectCallback cb)
    {
        java.util.Set<ConnectCallback> connectionCallbacks = new java.util.HashSet<ConnectCallback>();
        if(cb != null)
        {
            connectionCallbacks.add(cb);
        }

        java.util.Set<ConnectCallback> callbacks = new java.util.HashSet<ConnectCallback>();
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
    finishGetConnection(java.util.List<ConnectorInfo> connectors, Ice.LocalException ex, ConnectCallback cb)
    {
        java.util.Set<ConnectCallback> failedCallbacks = new java.util.HashSet<ConnectCallback>();
        if(cb != null)
        {
            failedCallbacks.add(cb);
        }

        java.util.Set<ConnectCallback> callbacks = new java.util.HashSet<ConnectCallback>();
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
                _pending.put(p.connector, new java.util.HashSet<ConnectCallback>());
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
    handleConnectionException(Ice.LocalException ex, boolean hasMore)
    {
        TraceLevels traceLevels = _instance.traceLevels();
        if(traceLevels.network >= 2)
        {
            StringBuilder s = new StringBuilder(128);
            s.append("connection to endpoint failed");
            if(ex instanceof Ice.CommunicatorDestroyedException)
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
    handleException(Ice.LocalException ex, boolean hasMore)
    {
        TraceLevels traceLevels = _instance.traceLevels();
        if(traceLevels.network >= 2)
        {
            StringBuilder s = new StringBuilder(128);
            s.append("couldn't resolve endpoint host");
            if(ex instanceof Ice.CommunicatorDestroyedException)
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

    private static class ConnectCallback implements Ice.ConnectionI.StartCallback, EndpointI_connectors
    {
        ConnectCallback(OutgoingConnectionFactory f, java.util.List<EndpointI> endpoints, boolean more,
                        CreateConnectionCallback cb, Ice.EndpointSelectionType selType)
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
        connectionStartCompleted(Ice.ConnectionI connection)
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
        connectionStartFailed(Ice.ConnectionI connection, Ice.LocalException ex)
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
        exception(Ice.LocalException ex)
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
        setConnection(Ice.ConnectionI connection, boolean compress)
        {
            //
            // Callback from the factory: the connection to one of the callback
            // connectors has been established.
            //
            _callback.setConnection(connection, compress);
            _factory.decPendingConnectCount(); // Must be called last.
        }

        void
        setException(Ice.LocalException ex)
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
            catch(Ice.LocalException ex)
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
            catch(Ice.LocalException ex)
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
                Ice.Holder<Boolean> compress = new Ice.Holder<Boolean>();
                Ice.ConnectionI connection = _factory.getConnection(_connectors, this, compress);
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
            catch(Ice.LocalException ex)
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

                    Ice.Instrumentation.CommunicatorObserver obsv = _factory._instance.initializationData().observer;
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
                        _factory._instance.initializationData().logger.trace(_factory._instance.traceLevels().networkCat,
                                                                             s.toString());
                    }

                    Ice.ConnectionI connection = _factory.createConnection(_current.connector.connect(), _current);
                    connection.start(this);
                }
                catch(Ice.LocalException ex)
                {
                    if(_factory._instance.traceLevels().network >= 2)
                    {
                        StringBuffer s = new StringBuffer("failed to establish ");
                        s.append(_current.endpoint.protocol());
                        s.append(" connection to ");
                        s.append(_current.connector.toString());
                        s.append("\n");
                        s.append(ex);
                        _factory._instance.initializationData().logger.trace(_factory._instance.traceLevels().networkCat,
                                                                             s.toString());
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
        connectionStartFailedImpl(Ice.LocalException ex)
        {
            if(_observer != null)
            {
                _observer.failed(ex.ice_id());
                _observer.detach();
            }

            _factory.handleConnectionException(ex, _hasMore || _iter.hasNext());
            if(ex instanceof Ice.CommunicatorDestroyedException) // No need to continue.
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
        private final Ice.EndpointSelectionType _selType;
        private java.util.Iterator<EndpointI> _endpointsIter;
        private EndpointI _currentEndpoint;
        private java.util.List<ConnectorInfo> _connectors = new java.util.ArrayList<ConnectorInfo>();
        private java.util.Iterator<ConnectorInfo> _iter;
        private ConnectorInfo _current;
        private Ice.Instrumentation.Observer _observer;
    }

    private Ice.Communicator _communicator;
    private final Instance _instance;
    private final FactoryACMMonitor _monitor;
    private boolean _destroyed;

    private MultiHashMap<Connector, Ice.ConnectionI> _connections = new MultiHashMap<Connector, Ice.ConnectionI>();
    private MultiHashMap<EndpointI, Ice.ConnectionI> _connectionsByEndpoint =
        new MultiHashMap<EndpointI, Ice.ConnectionI>();
    private java.util.Map<Connector, java.util.HashSet<ConnectCallback> > _pending =
        new java.util.HashMap<Connector, java.util.HashSet<ConnectCallback> >();
    private int _pendingConnectCount = 0;
}
