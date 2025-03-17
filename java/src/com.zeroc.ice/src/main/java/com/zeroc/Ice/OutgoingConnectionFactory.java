// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.Arrays;

final class OutgoingConnectionFactory {
    //
    // Helper class to multi hash map.
    //
    private static class MultiHashMap<K, V> extends java.util.HashMap<K, java.util.List<V>> {
        public void putOne(K key, V value) {
            java.util.List<V> list = this.get(key);
            if (list == null) {
                list = new java.util.LinkedList<>();
                this.put(key, list);
            }
            list.add(value);
        }

        private static final long serialVersionUID = -8109942200313578944L;
    }

    interface CreateConnectionCallback {
        void setConnection(ConnectionI connection, boolean compress);

        void setException(LocalException ex);
    }

    public synchronized void destroy() {
        if (_destroyed) {
            return;
        }

        for (java.util.List<ConnectionI> connectionList : _connections.values()) {
            for (ConnectionI connection : connectionList) {
                connection.destroy(ConnectionI.CommunicatorDestroyed);
            }
        }

        _destroyed = true;
        _communicator = null;
        _defaultObjectAdapter = null;
        notifyAll();
    }

    public synchronized void updateConnectionObservers() {
        for (java.util.List<ConnectionI> connectionList : _connections.values()) {
            for (ConnectionI connection : connectionList) {
                connection.updateObserver();
            }
        }
    }

    // Called from Instance.destroy().
    public void waitUntilFinished() {
        java.util.Map<Connector, java.util.List<ConnectionI>> connections = null;
        synchronized (this) {
            //
            // First we wait until the factory is destroyed. We also wait until there are no pending
            // connections anymore. Only then we can be sure the _connections contains all
            // connections.
            //
            while (!_destroyed || !_pending.isEmpty() || _pendingConnectCount > 0) {
                try {
                    wait();
                } catch (InterruptedException ex) {
                    throw new OperationInterruptedException(ex);
                }
            }

            //
            // We want to wait until all connections are finished outside the thread
            // synchronization.
            //
            connections = new java.util.HashMap<>(_connections);
        }

        //
        // Now we wait until the destruction of each connection is finished.
        //
        for (java.util.List<ConnectionI> connectionList : connections.values()) {
            for (ConnectionI connection : connectionList) {
                try {
                    connection.waitUntilFinished();
                } catch (InterruptedException e) {
                    //
                    // Force close all of the connections.
                    //
                    for (java.util.List<ConnectionI> l : connections.values()) {
                        for (ConnectionI c : l) {
                            c.abort();
                        }
                    }
                    throw new OperationInterruptedException(e);
                }
            }
        }

        synchronized (this) {
            _connections.clear();
            _connectionsByEndpoint.clear();
        }
    }

    public void create(EndpointI[] endpts, boolean hasMore, CreateConnectionCallback callback) {
        assert (endpts.length > 0);

        // TODO: fix API to use List directly.
        var endpoints = Arrays.asList(endpts);

        //
        // Try to find a connection to one of the given endpoints.
        //
        try {
            Holder<Boolean> compress = new Holder<>();
            ConnectionI connection = findConnectionByEndpoint(endpoints, compress);
            if (connection != null) {
                callback.setConnection(connection, compress.value);
                return;
            }
        } catch (LocalException ex) {
            callback.setException(ex);
            return;
        }

        final ConnectCallback cb = new ConnectCallback(this, endpoints, hasMore, callback);
        cb.getConnectors();
    }

    public void setRouterInfo(RouterInfo routerInfo) {
        assert (routerInfo != null);
        ObjectAdapter adapter = routerInfo.getAdapter();
        EndpointI[] endpoints =
                routerInfo.getClientEndpoints(); // Must be called outside the synchronization

        synchronized (this) {
            if (_destroyed) {
                throw new CommunicatorDestroyedException();
            }

            //
            // Search for connections to the router's client proxy
            // endpoints, and update the object adapter for such
            // connections, so that callbacks from the router can be received over such connections.
            //
            for (EndpointI endpoint : endpoints) {
                //
                // The Connection object does not take the compression flag of
                // endpoints into account, but instead gets the information
                // about whether messages should be compressed or not from other sources. In order
                // to allow connection sharing for
                // endpoints that differ in the value of the compression flag
                // only, we always set the compression flag to false here in this connection
                // factory. We also clear the timeout as it is
                // no longer used for Ice 3.8.
                //
                endpoint = endpoint.compress(false).timeout(-1);

                for (java.util.List<ConnectionI> connectionList : _connections.values()) {
                    for (ConnectionI connection : connectionList) {
                        if (connection.endpoint().equals(endpoint)) {
                            connection.setAdapter(adapter);
                        }
                    }
                }
            }
        }
    }

    public synchronized void removeAdapter(ObjectAdapter adapter) {
        if (_destroyed) {
            return;
        }

        for (java.util.List<ConnectionI> connectionList : _connections.values()) {
            for (ConnectionI connection : connectionList) {
                if (connection.getAdapter() == adapter) {
                    connection.setAdapter(null);
                }
            }
        }
    }

    public void flushAsyncBatchRequests(
            CompressBatch compressBatch, CommunicatorFlushBatch outAsync) {
        java.util.List<ConnectionI> c = new java.util.LinkedList<>();

        synchronized (this) {
            if (!_destroyed) {
                for (java.util.List<ConnectionI> connectionList : _connections.values()) {
                    for (ConnectionI connection : connectionList) {
                        if (connection.isActiveOrHolding()) {
                            c.add(connection);
                        }
                    }
                }
            }
        }

        for (ConnectionI conn : c) {
            try {
                outAsync.flushConnection(conn, compressBatch);
            } catch (LocalException ex) {
                // Ignore.
            }
        }
    }

    //
    // Only for use by Instance.
    //
    OutgoingConnectionFactory(Communicator communicator, Instance instance) {
        _communicator = communicator;
        _instance = instance;
        _connectionOptions = instance.clientConnectionOptions();
        _destroyed = false;
    }

    @SuppressWarnings("deprecation")
    @Override
    protected synchronized void finalize() throws Throwable {
        try {
            Assert.FinalizerAssert(_destroyed);
            Assert.FinalizerAssert(_connections.isEmpty());
            Assert.FinalizerAssert(_connectionsByEndpoint.isEmpty());
            Assert.FinalizerAssert(_pendingConnectCount == 0);
            Assert.FinalizerAssert(_pending.isEmpty());
        } catch (Exception ex) {
        } finally {
            super.finalize();
        }
    }

    private synchronized ConnectionI findConnectionByEndpoint(
            java.util.List<EndpointI> endpoints, Holder<Boolean> compress) {
        if (_destroyed) {
            throw new CommunicatorDestroyedException();
        }

        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        assert (!endpoints.isEmpty());

        for (EndpointI proxyEndpoint : endpoints) {
            // Clear the timeout
            EndpointI endpoint = proxyEndpoint.timeout(-1);
            java.util.List<ConnectionI> connectionList = _connectionsByEndpoint.get(endpoint);
            if (connectionList == null) {
                continue;
            }

            for (ConnectionI connection : connectionList) {
                if (connection
                        .isActiveOrHolding()) // Don't return destroyed or un-validated connections
                {
                    if (defaultsAndOverrides.overrideCompress.isPresent()) {
                        compress.value = defaultsAndOverrides.overrideCompress.get();
                    } else {
                        compress.value = endpoint.compress();
                    }
                    return connection;
                }
            }
        }

        return null;
    }

    synchronized void setDefaultObjectAdapter(ObjectAdapter adapter) {
        _defaultObjectAdapter = adapter;
    }

    synchronized ObjectAdapter getDefaultObjectAdapter() {
        return _defaultObjectAdapter;
    }

    //
    // Must be called while synchronized.
    //
    private ConnectionI findConnection(
            java.util.List<ConnectorInfo> connectors, Holder<Boolean> compress) {
        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        for (ConnectorInfo ci : connectors) {
            if (_pending.containsKey(ci.connector)) {
                continue;
            }

            java.util.List<ConnectionI> connectionList = _connections.get(ci.connector);
            if (connectionList == null) {
                continue;
            }

            for (ConnectionI connection : connectionList) {
                if (connection
                        .isActiveOrHolding()) // Don't return destroyed or un-validated connections
                {
                    if (defaultsAndOverrides.overrideCompress.isPresent()) {
                        compress.value = defaultsAndOverrides.overrideCompress.get();
                    } else {
                        compress.value = ci.endpoint.compress();
                    }
                    return connection;
                }
            }
        }

        return null;
    }

    private synchronized void incPendingConnectCount() {
        //
        // Keep track of the number of pending connects. The outgoing connection factory
        // waitUntilFinished() method waits for all the pending connects to terminate before
        // to return. This ensures that the communicator client thread pool isn't destroyed
        // too soon and will still be available to execute the ice_exception() callbacks for
        // the asynchronous requests waiting on a connection to be established.
        //

        if (_destroyed) {
            throw new CommunicatorDestroyedException();
        }
        ++_pendingConnectCount;
    }

    private synchronized void decPendingConnectCount() {
        --_pendingConnectCount;
        assert (_pendingConnectCount >= 0);
        if (_destroyed && _pendingConnectCount == 0) {
            notifyAll();
        }
    }

    private ConnectionI getConnection(
            java.util.List<ConnectorInfo> connectors,
            ConnectCallback cb,
            Holder<Boolean> compress) {
        assert (cb != null);
        synchronized (this) {
            if (_destroyed) {
                throw new CommunicatorDestroyedException();
            }

            // Search for an existing connections matching one of the given endpoints.
            ConnectionI connection = findConnection(connectors, compress);
            if (connection != null) {
                return connection;
            }

            if (addToPending(cb, connectors)) {
                // A connection to one of our endpoints is pending. The callback will be notified
                // once the connection is established. Returning null indicates that the connection
                // is still pending.
                return null;
            }
        }

        // No connection is pending. Call nextConnector to initiate connection establishment. Return
        // null to indicate that the connection is still pending.
        cb.nextConnector();
        return null;
    }

    private synchronized ConnectionI createConnection(Transceiver transceiver, ConnectorInfo ci) {
        assert (_pending.containsKey(ci.connector) && transceiver != null);

        //
        // Create and add the connection to the connection map. Adding the connection to the map
        // is necessary to support the interruption of the connection initialization and validation
        // in case the communicator is destroyed.
        //
        ConnectionI connection = null;
        try {
            if (_destroyed) {
                throw new CommunicatorDestroyedException();
            }

            connection =
                    new ConnectionI(
                            _communicator,
                            _instance,
                            transceiver,
                            ci.connector,
                            ci.endpoint.compress(false).timeout(-1),
                            _defaultObjectAdapter,
                            this::removeConnection,
                            _connectionOptions);
        } catch (LocalException ex) {
            try {
                transceiver.close();
            } catch (LocalException exc) {
                // Ignore
            }
            throw ex;
        }

        _connections.putOne(ci.connector, connection);
        _connectionsByEndpoint.putOne(connection.endpoint(), connection);
        _connectionsByEndpoint.putOne(connection.endpoint().compress(true), connection);
        return connection;
    }

    private void finishGetConnection(
            java.util.List<ConnectorInfo> connectors,
            ConnectorInfo ci,
            ConnectionI connection,
            ConnectCallback cb) {
        java.util.Set<ConnectCallback> connectionCallbacks = new java.util.HashSet<>();
        if (cb != null) {
            connectionCallbacks.add(cb);
        }

        java.util.Set<ConnectCallback> callbacks = new java.util.HashSet<>();
        synchronized (this) {
            for (ConnectorInfo c : connectors) {
                java.util.Set<ConnectCallback> cbs = _pending.remove(c.connector);
                if (cbs != null) {
                    for (ConnectCallback cc : cbs) {
                        if (cc.hasConnector(ci)) {
                            connectionCallbacks.add(cc);
                        } else {
                            callbacks.add(cc);
                        }
                    }
                }
            }

            for (ConnectCallback cc : connectionCallbacks) {
                cc.removeFromPending();
                callbacks.remove(cc);
            }
            for (ConnectCallback cc : callbacks) {
                cc.removeFromPending();
            }
            notifyAll();
        }

        boolean compress;
        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        if (defaultsAndOverrides.overrideCompress.isPresent()) {
            compress = defaultsAndOverrides.overrideCompress.get();
        } else {
            compress = ci.endpoint.compress();
        }

        for (ConnectCallback cc : callbacks) {
            cc.getConnection();
        }
        for (ConnectCallback cc : connectionCallbacks) {
            cc.setConnection(connection, compress);
        }
    }

    private void finishGetConnection(
            java.util.List<ConnectorInfo> connectors, LocalException ex, ConnectCallback cb) {
        java.util.Set<ConnectCallback> failedCallbacks = new java.util.HashSet<>();
        if (cb != null) {
            failedCallbacks.add(cb);
        }

        java.util.Set<ConnectCallback> callbacks = new java.util.HashSet<>();
        synchronized (this) {
            for (ConnectorInfo c : connectors) {
                java.util.Set<ConnectCallback> cbs = _pending.remove(c.connector);
                if (cbs != null) {
                    for (ConnectCallback cc : cbs) {
                        if (cc.removeConnectors(connectors)) {
                            failedCallbacks.add(cc);
                        } else {
                            callbacks.add(cc);
                        }
                    }
                }
            }

            for (ConnectCallback cc : callbacks) {
                assert (!failedCallbacks.contains(cc));
                cc.removeFromPending();
            }
            notifyAll();
        }

        for (ConnectCallback cc : callbacks) {
            cc.getConnection();
        }
        for (ConnectCallback cc : failedCallbacks) {
            cc.setException(ex);
        }
    }

    private boolean addToPending(ConnectCallback cb, java.util.List<ConnectorInfo> connectors) {
        //
        // Add the callback to each connector pending list.
        //
        boolean found = false;
        for (ConnectorInfo p : connectors) {
            java.util.Set<ConnectCallback> cbs = _pending.get(p.connector);
            if (cbs != null) {
                found = true;
                if (cb != null) {
                    cbs.add(cb); // Add the callback to each pending connector.
                }
            }
        }

        if (found) {
            return true;
        }

        //
        // If there's no pending connection for the given connectors, we're responsible for its
        // establishment. We add empty pending lists, other callbacks to the same connectors will be
        // queued.
        //
        for (ConnectorInfo p : connectors) {
            if (!_pending.containsKey(p.connector)) {
                _pending.put(p.connector, new java.util.HashSet<>());
            }
        }

        return false;
    }

    private void removeFromPending(ConnectCallback cb, java.util.List<ConnectorInfo> connectors) {
        for (ConnectorInfo p : connectors) {
            java.util.Set<ConnectCallback> cbs = _pending.get(p.connector);
            if (cbs != null) {
                cbs.remove(cb);
            }
        }
    }

    private void handleConnectionException(LocalException ex, boolean hasMore) {
        TraceLevels traceLevels = _instance.traceLevels();
        if (traceLevels.network >= 2) {
            StringBuilder s = new StringBuilder(128);
            s.append("connection to endpoint failed");
            if (ex instanceof CommunicatorDestroyedException) {
                s.append("\n");
            } else {
                if (hasMore) {
                    s.append(", trying next endpoint\n");
                } else {
                    s.append(" and no more endpoints to try\n");
                }
            }
            s.append(ex.toString());
            _instance.initializationData().logger.trace(traceLevels.networkCat, s.toString());
        }
    }

    private void handleException(LocalException ex, boolean hasMore) {
        TraceLevels traceLevels = _instance.traceLevels();
        if (traceLevels.network >= 2) {
            StringBuilder s = new StringBuilder(128);
            s.append("couldn't resolve endpoint host");
            if (ex instanceof CommunicatorDestroyedException) {
                s.append("\n");
            } else {
                if (hasMore) {
                    s.append(", trying next endpoint\n");
                } else {
                    s.append(" and no more endpoints to try\n");
                }
            }
            s.append(ex.toString());
            _instance.initializationData().logger.trace(traceLevels.networkCat, s.toString());
        }
    }

    private synchronized void removeConnection(ConnectionI connection) {
        if (_destroyed) {
            return;
        }

        _connections.remove(connection.connector(), connection);
        _connectionsByEndpoint.remove(connection.endpoint(), connection);
        _connectionsByEndpoint.remove(connection.endpoint().compress(true), connection);
    }

    private static class ConnectorInfo {
        public ConnectorInfo(Connector c, EndpointI e) {
            connector = c;
            endpoint = e;
        }

        @Override
        public boolean equals(java.lang.Object obj) {
            ConnectorInfo r = (ConnectorInfo) obj;
            return connector.equals(r.connector);
        }

        @Override
        public int hashCode() {
            return connector.hashCode();
        }

        public Connector connector;
        public EndpointI endpoint;
    }

    private static class ConnectCallback
            implements ConnectionI.StartCallback, EndpointI_connectors {
        ConnectCallback(
                OutgoingConnectionFactory f,
                java.util.List<EndpointI> endpoints,
                boolean more,
                CreateConnectionCallback cb) {
            _factory = f;
            _endpoints = endpoints;
            _hasMore = more;
            _callback = cb;
            _endpointsIter = _endpoints.iterator();
        }

        //
        // Methods from ConnectionI.StartCallback
        //
        @Override
        public void connectionStartCompleted(ConnectionI connection) {
            if (_observer != null) {
                _observer.detach();
            }
            connection.activate();
            _factory.finishGetConnection(_connectors, _current, connection, this);
        }

        @Override
        public void connectionStartFailed(ConnectionI connection, LocalException ex) {
            assert (_current != null);
            if (connectionStartFailedImpl(ex)) {
                nextConnector();
            }
        }

        //
        // Methods from EndpointI_connectors
        //
        @Override
        public void connectors(java.util.List<Connector> cons) {
            for (Connector p : cons) {
                _connectors.add(new ConnectorInfo(p, _currentEndpoint));
            }

            if (_endpointsIter.hasNext()) {
                nextEndpoint();
            } else {
                assert (!_connectors.isEmpty());

                //
                // We now have all the connectors for the given endpoints. We can try to obtain the
                // connection.
                //
                _iter = _connectors.iterator();
                getConnection();
            }
        }

        @Override
        public void exception(LocalException ex) {
            _factory.handleException(ex, _hasMore || _endpointsIter.hasNext());
            if (_endpointsIter.hasNext()) {
                nextEndpoint();
            } else if (!_connectors.isEmpty()) {
                //
                // We now have all the connectors for the given endpoints. We can try to obtain the
                // connection.
                //
                _iter = _connectors.iterator();
                getConnection();
            } else {
                _callback.setException(ex);
                _factory.decPendingConnectCount(); // Must be called last.
            }
        }

        void setConnection(ConnectionI connection, boolean compress) {
            //
            // Callback from the factory: the connection to one of the callback
            // connectors has been established.
            //
            _callback.setConnection(connection, compress);
            _factory.decPendingConnectCount(); // Must be called last.
        }

        void setException(LocalException ex) {
            //
            // Callback from the factory: connection establishment failed.
            //
            _callback.setException(ex);
            _factory.decPendingConnectCount(); // Must be called last.
        }

        boolean hasConnector(ConnectorInfo ci) {
            return _connectors.contains(ci);
        }

        boolean removeConnectors(java.util.List<ConnectorInfo> connectors) {
            _connectors.removeAll(connectors);
            _iter = _connectors.iterator();
            return _connectors.isEmpty();
        }

        void removeFromPending() {
            _factory.removeFromPending(this, _connectors);
        }

        private void getConnectors() {
            try {
                //
                // Notify the factory that there's an async connect pending. This is necessary to
                // prevent the outgoing connection factory to be destroyed before all the pending
                // asynchronous connects are finished.
                //
                _factory.incPendingConnectCount();
            } catch (LocalException ex) {
                _callback.setException(ex);
                return;
            }

            nextEndpoint();
        }

        private void nextEndpoint() {
            try {
                assert (_endpointsIter.hasNext());
                _currentEndpoint = _endpointsIter.next();
                _currentEndpoint.connectors_async(this);
            } catch (LocalException ex) {
                exception(ex);
            }
        }

        private void getConnection() {
            try {
                //
                // If all the connectors have been created, we ask the factory to get a connection.
                //
                Holder<Boolean> compress = new Holder<>();
                ConnectionI connection = _factory.getConnection(_connectors, this, compress);
                if (connection == null) {
                    //
                    // A null return value from getConnection indicates that the connection
                    // is being established and that everything has been done to ensure that the
                    // callback will be notified when the connection establishment is done.
                    //
                    return;
                }

                _callback.setConnection(connection, compress.value);
                _factory.decPendingConnectCount(); // Must be called last.
            } catch (LocalException ex) {
                _callback.setException(ex);
                _factory.decPendingConnectCount(); // Must be called last.
            }
        }

        private void nextConnector() {
            while (true) {
                try {
                    assert (_iter.hasNext());
                    _current = _iter.next();

                    com.zeroc.Ice.Instrumentation.CommunicatorObserver observer =
                            _factory._instance.initializationData().observer;
                    if (observer != null) {
                        _observer =
                                observer.getConnectionEstablishmentObserver(
                                        _current.endpoint, _current.connector.toString());
                        if (_observer != null) {
                            _observer.attach();
                        }
                    }

                    if (_factory._instance.traceLevels().network >= 2) {
                        StringBuffer s = new StringBuffer("trying to establish ");
                        s.append(_current.endpoint.protocol());
                        s.append(" connection to ");
                        s.append(_current.connector.toString());
                        _factory._instance
                                .initializationData()
                                .logger
                                .trace(_factory._instance.traceLevels().networkCat, s.toString());
                    }

                    ConnectionI connection =
                            _factory.createConnection(_current.connector.connect(), _current);
                    connection.start(this);
                } catch (LocalException ex) {
                    if (_factory._instance.traceLevels().network >= 2) {
                        StringBuffer s = new StringBuffer("failed to establish ");
                        s.append(_current.endpoint.protocol());
                        s.append(" connection to ");
                        s.append(_current.connector.toString());
                        s.append("\n");
                        s.append(ex);
                        _factory._instance
                                .initializationData()
                                .logger
                                .trace(_factory._instance.traceLevels().networkCat, s.toString());
                    }

                    if (connectionStartFailedImpl(ex)) {
                        continue;
                    }
                }
                break;
            }
        }

        private boolean connectionStartFailedImpl(LocalException ex) {
            if (_observer != null) {
                _observer.failed(ex.ice_id());
                _observer.detach();
            }

            _factory.handleConnectionException(ex, _hasMore || _iter.hasNext());

            // We stop on ConnectTimeoutException to fail reasonably fast when the endpoint has many
            // connectors (IP addresses).
            if (_iter.hasNext()
                    && !(ex instanceof CommunicatorDestroyedException
                            || ex instanceof ConnectTimeoutException)) {
                return true; // keep going
            }

            _factory.finishGetConnection(_connectors, ex, this);
            return false;
        }

        private final OutgoingConnectionFactory _factory;
        private final boolean _hasMore;
        private final CreateConnectionCallback _callback;
        private final java.util.List<EndpointI> _endpoints;
        private java.util.Iterator<EndpointI> _endpointsIter;
        private EndpointI _currentEndpoint;
        private java.util.List<ConnectorInfo> _connectors = new java.util.ArrayList<>();
        private java.util.Iterator<ConnectorInfo> _iter;
        private ConnectorInfo _current;
        private com.zeroc.Ice.Instrumentation.Observer _observer;
    }

    private Communicator _communicator;
    private final Instance _instance;
    private final ConnectionOptions _connectionOptions;
    private ObjectAdapter _defaultObjectAdapter;
    private boolean _destroyed;

    private MultiHashMap<Connector, ConnectionI> _connections = new MultiHashMap<>();
    private MultiHashMap<EndpointI, ConnectionI> _connectionsByEndpoint = new MultiHashMap<>();
    private java.util.Map<Connector, java.util.HashSet<ConnectCallback>> _pending =
            new java.util.HashMap<>();
    private int _pendingConnectCount = 0;
}
