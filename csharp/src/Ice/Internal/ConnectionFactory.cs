// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Text;

#nullable enable

namespace Ice.Internal;

internal class MultiDictionary<K, V> : Dictionary<K, ICollection<V>> where K : class
{
    internal void Add(K key, V value)
    {
        if (!TryGetValue(key, out ICollection<V>? list))
        {
            list = [];
            Add(key, list);
        }
        list.Add(value);
    }

    internal void Remove(K key, V value)
    {
        ICollection<V> list = this[key];
        list.Remove(value);
        if (list.Count == 0)
        {
            Remove(key);
        }
    }
}

internal sealed class OutgoingConnectionFactory
{
    internal interface CreateConnectionCallback
    {
        void setConnection(ConnectionI connection, bool compress);

        void setException(LocalException ex);
    }

    internal void destroy()
    {
        lock (_mutex)
        {
            if (_destroyed)
            {
                return;
            }

            foreach (ICollection<ConnectionI> connections in _connections.Values)
            {
                foreach (ConnectionI c in connections)
                {
                    c.destroy(ConnectionI.CommunicatorDestroyed);
                }
            }

            _destroyed = true;
            _defaultObjectAdapter = null;
            Monitor.PulseAll(_mutex);
        }
    }

    internal void updateConnectionObservers()
    {
        lock (_mutex)
        {
            foreach (ICollection<ConnectionI> connections in _connections.Values)
            {
                foreach (ConnectionI c in connections)
                {
                    c.updateObserver();
                }
            }
        }
    }

    internal void waitUntilFinished()
    {
        Dictionary<Connector, ICollection<ConnectionI>> connections;
        lock (_mutex)
        {
            // First we wait until the factory is destroyed. We also wait until there are no pending connections
            // anymore. Only then we can be sure the _connections contains all connections.
            while (!_destroyed || _pending.Count > 0 || _pendingConnectCount > 0)
            {
                Monitor.Wait(_mutex);
            }

            // We want to wait until all connections are finished outside the thread synchronization.
            connections = new Dictionary<Connector, ICollection<ConnectionI>>(_connections);
        }

        // Now we wait until the destruction of each connection is finished.
        foreach (ICollection<ConnectionI> cl in connections.Values)
        {
            foreach (ConnectionI c in cl)
            {
                c.waitUntilFinished();
            }
        }
    }

    internal void create(List<EndpointI> endpoints, bool hasMore, CreateConnectionCallback callback)
    {
        Debug.Assert(endpoints.Count > 0);

        // Try to find a connection to one of the given endpoints.
        try
        {
            if (findConnection(endpoints, out bool compress) is ConnectionI connection)
            {
                callback.setConnection(connection, compress);
                return;
            }
        }
        catch (LocalException ex)
        {
            callback.setException(ex);
            return;
        }

        var cb = new ConnectCallback(this, endpoints, hasMore, callback);
        cb.getConnectors();
    }

    internal void setRouterInfo(RouterInfo routerInfo)
    {
        Debug.Assert(routerInfo is not null);
        ObjectAdapter adapter = routerInfo.getAdapter();
        EndpointI[] endpoints = routerInfo.getClientEndpoints(); // Must be called outside the synchronization

        lock (_mutex)
        {
            if (_destroyed)
            {
                throw new CommunicatorDestroyedException();
            }

            // Search for connections to the router's client proxy endpoints, and update the object adapter for such
            // connections, so that callbacks from the router can be received over such connections.
            for (int i = 0; i < endpoints.Length; i++)
            {
                EndpointI endpoint = endpoints[i];

                // The Ice.ConnectionI object does not take the compression flag of endpoints into account, but instead
                // gets the information about whether messages should be compressed or not fro other sources. In order
                // to allow connection sharing for endpoints that differ in the value of the compression flag only, we
                // always set the compression flag to false here in this connection factory. We also clear the timeout
                // as it is no longer used for Ice 3.8.
                endpoint = endpoint.compress(false).timeout(-1);

                foreach (ICollection<ConnectionI> connections in _connections.Values)
                {
                    foreach (ConnectionI connection in connections)
                    {
                        if (connection.endpoint().Equals(endpoint))
                        {
                            connection.setAdapter(adapter);
                        }
                    }
                }
            }
        }
    }

    internal void removeAdapter(ObjectAdapter adapter)
    {
        lock (_mutex)
        {
            if (_destroyed)
            {
                return;
            }

            foreach (ICollection<ConnectionI> connectionList in _connections.Values)
            {
                foreach (ConnectionI connection in connectionList)
                {
                    if (connection.getAdapter() == adapter)
                    {
                        connection.setAdapter(null);
                    }
                }
            }
        }
    }

    internal void flushAsyncBatchRequests(CompressBatch compressBatch, CommunicatorFlushBatchAsync outAsync)
    {
        var c = new List<ConnectionI>();

        lock (_mutex)
        {
            if (!_destroyed)
            {
                foreach (ICollection<ConnectionI> connectionList in _connections.Values)
                {
                    foreach (ConnectionI conn in connectionList)
                    {
                        if (conn.isActiveOrHolding())
                        {
                            c.Add(conn);
                        }
                    }
                }
            }
        }

        foreach (ConnectionI conn in c)
        {
            try
            {
                outAsync.flushConnection(conn, compressBatch);
            }
            catch (LocalException)
            {
                // Ignore.
            }
        }
    }

    // Only for use by Instance.
    internal OutgoingConnectionFactory(Instance instance)
    {
        _instance = instance;
        _connectionOptions = instance.clientConnectionOptions;
        _destroyed = false;
        _pendingConnectCount = 0;
    }

    internal void setDefaultObjectAdapter(ObjectAdapter? adapter)
    {
        lock (_mutex)
        {
            _defaultObjectAdapter = adapter;
        }
    }

    internal ObjectAdapter? getDefaultObjectAdapter()
    {
        lock (_mutex)
        {
            return _defaultObjectAdapter;
        }
    }

    private ConnectionI? findConnection(List<EndpointI> endpoints, out bool compress)
    {
        lock (_mutex)
        {
            if (_destroyed)
            {
                throw new CommunicatorDestroyedException();
            }

            DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
            Debug.Assert(endpoints.Count > 0);

            foreach (EndpointI proxyEndpoint in endpoints)
            {
                EndpointI endpoint = proxyEndpoint.timeout(-1); // Clear the timeout

                if (!_connectionsByEndpoint.TryGetValue(endpoint, out ICollection<ConnectionI>? connectionList))
                {
                    continue;
                }

                foreach (ConnectionI connection in connectionList)
                {
                    if (connection.isActiveOrHolding()) // Don't return destroyed or un-validated connections
                    {
                        compress = defaultsAndOverrides.overrideCompress ?? endpoint.compress();
                        return connection;
                    }
                }
            }

            compress = false; // Satisfy the compiler
            return null;
        }
    }

    // Must be called while synchronized.
    private ConnectionI? findConnection(List<ConnectorInfo> connectors, out bool compress)
    {
        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        foreach (ConnectorInfo ci in connectors)
        {
            if (_pending.ContainsKey(ci.connector))
            {
                continue;
            }

            if (!_connections.TryGetValue(ci.connector, out ICollection<ConnectionI>? connectionList))
            {
                continue;
            }

            foreach (ConnectionI connection in connectionList)
            {
                if (connection.isActiveOrHolding()) // Don't return destroyed or un-validated connections
                {
                    compress = defaultsAndOverrides.overrideCompress ?? ci.endpoint.compress();
                    return connection;
                }
            }
        }

        compress = false; // Satisfy the compiler
        return null;
    }

    internal void incPendingConnectCount()
    {
        // Keep track of the number of pending connects. The outgoing connection factory waitUntilFinished() method
        // waits for all the pending connects to terminate before return. This ensures that the communicator client
        // thread pool isn't destroyed too soon and will still be available to execute the ice_exception() callbacks
        // for the asynchronous requests waiting on a connection to be established.

        lock (_mutex)
        {
            if (_destroyed)
            {
                throw new CommunicatorDestroyedException();
            }
            ++_pendingConnectCount;
        }
    }

    internal void decPendingConnectCount()
    {
        lock (_mutex)
        {
            --_pendingConnectCount;
            Debug.Assert(_pendingConnectCount >= 0);
            if (_destroyed && _pendingConnectCount == 0)
            {
                Monitor.PulseAll(_mutex);
            }
        }
    }

    private ConnectionI? getConnection(List<ConnectorInfo> connectors, ConnectCallback cb, out bool compress)
    {
        lock (_mutex)
        {
            if (_destroyed)
            {
                throw new CommunicatorDestroyedException();
            }

            // Search for an existing connections matching one of the given endpoints.
            if (findConnection(connectors, out compress) is ConnectionI connection)
            {
                return connection;
            }

            if (addToPending(cb, connectors))
            {
                // A connection to one of our endpoints is pending. The callback will be notified once the connection
                // is established. Returning null indicates that the connection is still pending.
                return null;
            }
        }

        // No connection is pending. Call nextConnector to initiate connection establishment. Return null to indicate
        // that the connection is still pending.
        cb.nextConnector();
        return null;
    }

    private ConnectionI createConnection(Transceiver transceiver, ConnectorInfo ci)
    {
        lock (_mutex)
        {
            Debug.Assert(_pending.ContainsKey(ci.connector) && transceiver != null);

            // Create and add the connection to the connection map. Adding the connection to the map is necessary to
            // support the interruption of the connection initialization and validation in case the communicator is
            // destroyed.
            ConnectionI connection;
            try
            {
                if (_destroyed)
                {
                    throw new CommunicatorDestroyedException();
                }

                connection = new ConnectionI(
                    _instance,
                    transceiver,
                    ci.connector,
                    ci.endpoint.compress(false).timeout(-1),
                    adapter: _defaultObjectAdapter,
                    removeConnection,
                    _connectionOptions);
            }
            catch (LocalException)
            {
                try
                {
                    transceiver.close();
                }
                catch (LocalException)
                {
                    // Ignore
                }
                throw;
            }

            _connections.Add(ci.connector, connection);
            _connectionsByEndpoint.Add(connection.endpoint(), connection);
            _connectionsByEndpoint.Add(connection.endpoint().compress(true), connection);
            return connection;
        }
    }

    private void finishGetConnection(
        List<ConnectorInfo> connectors,
        ConnectorInfo ci,
        ConnectionI connection,
        ConnectCallback cb)
    {
        var connectionCallbacks = new HashSet<ConnectCallback>
        {
            cb
        };

        var callbacks = new HashSet<ConnectCallback>();
        lock (_mutex)
        {
            foreach (ConnectorInfo c in connectors)
            {
                if (_pending.TryGetValue(c.connector, out HashSet<ConnectCallback>? s))
                {
                    foreach (ConnectCallback cc in s)
                    {
                        if (cc.hasConnector(ci))
                        {
                            connectionCallbacks.Add(cc);
                        }
                        else
                        {
                            callbacks.Add(cc);
                        }
                    }
                    _pending.Remove(c.connector);
                }
            }

            foreach (ConnectCallback cc in connectionCallbacks)
            {
                cc.removeFromPending();
                callbacks.Remove(cc);
            }

            foreach (ConnectCallback cc in callbacks)
            {
                cc.removeFromPending();
            }
            Monitor.PulseAll(_mutex);
        }

        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();
        bool compress = defaultsAndOverrides.overrideCompress ?? ci.endpoint.compress();

        foreach (ConnectCallback cc in callbacks)
        {
            cc.getConnection();
        }

        foreach (ConnectCallback cc in connectionCallbacks)
        {
            cc.setConnection(connection, compress);
        }
    }

    private void finishGetConnection(List<ConnectorInfo> connectors, LocalException ex, ConnectCallback cb)
    {
        var failedCallbacks = new HashSet<ConnectCallback>
        {
            cb
        };

        var callbacks = new HashSet<ConnectCallback>();
        lock (_mutex)
        {
            foreach (ConnectorInfo c in connectors)
            {
                if (_pending.TryGetValue(c.connector, out HashSet<ConnectCallback>? s))
                {
                    foreach (ConnectCallback cc in s)
                    {
                        if (cc.removeConnectors(connectors))
                        {
                            failedCallbacks.Add(cc);
                        }
                        else
                        {
                            callbacks.Add(cc);
                        }
                    }
                    _pending.Remove(c.connector);
                }
            }

            foreach (ConnectCallback cc in callbacks)
            {
                Debug.Assert(!failedCallbacks.Contains(cc));
                cc.removeFromPending();
            }
            Monitor.PulseAll(_mutex);
        }

        foreach (ConnectCallback cc in callbacks)
        {
            cc.getConnection();
        }

        foreach (ConnectCallback cc in failedCallbacks)
        {
            cc.setException(ex);
        }
    }

    private void handleConnectionException(LocalException ex, bool hasMore)
    {
        TraceLevels traceLevels = _instance.traceLevels();
        if (traceLevels.network >= 2)
        {
            var s = new StringBuilder();
            s.Append("connection to endpoint failed");
            if (ex is CommunicatorDestroyedException)
            {
                s.Append('\n');
            }
            else
            {
                if (hasMore)
                {
                    s.Append(", trying next endpoint\n");
                }
                else
                {
                    s.Append(" and no more endpoints to try\n");
                }
            }
            s.Append(ex);
            _instance.initializationData().logger!.trace(traceLevels.networkCat, s.ToString());
        }
    }

    private bool addToPending(ConnectCallback cb, List<ConnectorInfo> connectors)
    {
        // Add the callback to each connector pending list.
        bool found = false;
        foreach (ConnectorInfo ci in connectors)
        {
            if (_pending.TryGetValue(ci.connector, out HashSet<ConnectCallback>? cbs))
            {
                found = true;
                cbs.Add(cb); // Add the callback to each pending connector.
            }
        }

        if (found)
        {
            return true;
        }

        // If no pending connection exists for the specified connectors, the caller is responsible for initiating
        // connection establishment. An empty pending list is added, and any additional callbacks for the same
        // connectors will be queued.
        foreach (ConnectorInfo ci in connectors)
        {
            if (!_pending.ContainsKey(ci.connector))
            {
                _pending.Add(ci.connector, []);
            }
        }
        return false;
    }

    private void removeFromPending(ConnectCallback cb, List<ConnectorInfo> connectors)
    {
        foreach (ConnectorInfo ci in connectors)
        {
            if (_pending.TryGetValue(ci.connector, out HashSet<ConnectCallback>? cbs))
            {
                cbs.Remove(cb);
            }
        }
    }

    private void removeConnection(ConnectionI connection)
    {
        lock (_mutex)
        {
            if (_destroyed)
            {
                return;
            }

            _connections.Remove(connection.connector(), connection);
            _connectionsByEndpoint.Remove(connection.endpoint(), connection);
            _connectionsByEndpoint.Remove(connection.endpoint().compress(true), connection);
        }
    }

    internal void handleException(LocalException ex, bool hasMore)
    {
        TraceLevels traceLevels = _instance.traceLevels();
        if (traceLevels.network >= 2)
        {
            var s = new StringBuilder();
            s.Append("couldn't resolve endpoint host");
            if (ex is CommunicatorDestroyedException)
            {
                s.Append('\n');
            }
            else
            {
                if (hasMore)
                {
                    s.Append(", trying next endpoint\n");
                }
                else
                {
                    s.Append(" and no more endpoints to try\n");
                }
            }
            s.Append(ex);
            _instance.initializationData().logger!.trace(traceLevels.networkCat, s.ToString());
        }
    }

    private class ConnectorInfo
    {
        internal ConnectorInfo(Connector c, EndpointI e)
        {
            connector = c;
            endpoint = e;
        }

        public override bool Equals(object? obj) =>
            (obj is ConnectorInfo r) && connector.Equals(r.connector);

        public override int GetHashCode() => connector.GetHashCode();

        internal Connector connector;
        internal EndpointI endpoint;
    }

    private class ConnectCallback : ConnectionI.StartCallback, EndpointI_connectors
    {
        internal ConnectCallback(
            OutgoingConnectionFactory factory,
            List<EndpointI> endpoints,
            bool more,
            CreateConnectionCallback cb)
        {
            _factory = factory;
            _endpoints = endpoints;
            _hasMore = more;
            _callback = cb;
            _endpointsIter = 0;
        }

        //
        // Methods from ConnectionI.StartCallback
        //
        public void connectionStartCompleted(ConnectionI connection)
        {
            _observer?.detach();
            connection.activate();
            Debug.Assert(_current is not null);
            _factory.finishGetConnection(_connectors, _current, connection, this);
        }

        public void connectionStartFailed(ConnectionI connection, LocalException ex)
        {
            if (connectionStartFailedImpl(ex))
            {
                nextConnector();
            }
        }

        //
        // Methods from EndpointI_connectors
        //
        public void connectors(List<Connector> cons)
        {
            Debug.Assert(_currentEndpoint is not null);
            foreach (Connector connector in cons)
            {
                _connectors.Add(new ConnectorInfo(connector, _currentEndpoint));
            }

            if (_endpointsIter < _endpoints.Count)
            {
                nextEndpoint();
            }
            else
            {
                Debug.Assert(_connectors.Count > 0);

                //
                // We now have all the connectors for the given endpoints. We can try to obtain the
                // connection.
                //
                _iter = 0;
                getConnection();
            }
        }

        public void exception(LocalException ex)
        {
            _factory.handleException(ex, _hasMore || _endpointsIter < _endpoints.Count);
            if (_endpointsIter < _endpoints.Count)
            {
                nextEndpoint();
            }
            else if (_connectors.Count > 0)
            {
                //
                // We now have all the connectors for the given endpoints. We can try to obtain the
                // connection.
                //
                _iter = 0;
                getConnection();
            }
            else
            {
                _callback.setException(ex);
                _factory.decPendingConnectCount(); // Must be called last.
            }
        }

        internal void setConnection(ConnectionI connection, bool compress)
        {
            //
            // Callback from the factory: the connection to one of the callback
            // connectors has been established.
            //
            _callback.setConnection(connection, compress);
            _factory.decPendingConnectCount(); // Must be called last.
        }

        internal void setException(LocalException ex)
        {
            //
            // Callback from the factory: connection establishment failed.
            //
            _callback.setException(ex);
            _factory.decPendingConnectCount(); // Must be called last.
        }

        internal bool hasConnector(ConnectorInfo ci) => _connectors.Contains(ci);

        internal bool removeConnectors(List<ConnectorInfo> connectors)
        {
            _connectors.RemoveAll(ci => connectors.Contains(ci));
            return _connectors.Count == 0;
        }

        internal void removeFromPending() => _factory.removeFromPending(this, _connectors);

        internal void getConnectors()
        {
            try
            {
                // Notify the factory that there's an async connect pending. This is necessary
                // to prevent the outgoing connection factory to be destroyed before all the
                // pending asynchronous connects are finished.
                _factory.incPendingConnectCount();
            }
            catch (LocalException ex)
            {
                _callback.setException(ex);
                return;
            }

            nextEndpoint();
        }

        private void nextEndpoint()
        {
            try
            {
                Debug.Assert(_endpointsIter < _endpoints.Count);
                _currentEndpoint = _endpoints[_endpointsIter++];
                _currentEndpoint.connectors_async(this);
            }
            catch (LocalException ex)
            {
                exception(ex);
            }
        }

        internal void getConnection()
        {
            try
            {
                // If all the connectors have been created, we ask the factory to get a connection.
                ConnectionI? connection = _factory.getConnection(_connectors, this, out bool compress);
                if (connection is null)
                {
                    // A null return value from getConnection indicates that the connection is being established and
                    // the callback will be notified when the connection establishment is done.
                    return;
                }

                _callback.setConnection(connection, compress);
                _factory.decPendingConnectCount(); // Must be called last.
            }
            catch (LocalException ex)
            {
                _callback.setException(ex);
                _factory.decPendingConnectCount(); // Must be called last.
            }
        }

        internal void nextConnector()
        {
            while (true)
            {
                try
                {
                    Debug.Assert(_iter < _connectors.Count);
                    _current = _connectors[_iter++];
                    Debug.Assert(_current is not null);

                    if (
                        _factory._instance.initializationData().observer is 
                        Instrumentation.CommunicatorObserver observer)
                    {
                        _observer = observer.getConnectionEstablishmentObserver(
                            _current.endpoint,
                            _current.connector.ToString()!);

                        _observer?.attach();
                    }

                    if (_factory._instance.traceLevels().network >= 2)
                    {
                        var s = new StringBuilder("trying to establish ");
                        s.Append(_current.endpoint.protocol());
                        s.Append(" connection to ");
                        s.Append(_current.connector.ToString());
                        _factory._instance.initializationData().logger!.trace(
                            _factory._instance.traceLevels().networkCat, s.ToString());
                    }

                    ConnectionI connection = _factory.createConnection(_current.connector.connect(), _current);
                    connection.start(this);
                }
                catch (LocalException ex)
                {
                    if (_factory._instance.traceLevels().network >= 2)
                    {
                        Debug.Assert(_current is not null);
                        var s = new StringBuilder("failed to establish ");
                        s.Append(_current.endpoint.protocol());
                        s.Append(" connection to ");
                        s.Append(_current.connector.ToString());
                        s.Append('\n');
                        s.Append(ex);
                        _factory._instance.initializationData().logger!.trace(
                            _factory._instance.traceLevels().networkCat, s.ToString());
                    }

                    if (connectionStartFailedImpl(ex))
                    {
                        continue;
                    }
                }
                break;
            }
        }

        private bool connectionStartFailedImpl(LocalException ex)
        {
            if (_observer != null)
            {
                _observer.failed(ex.ice_id());
                _observer.detach();
            }
            _factory.handleConnectionException(ex, _hasMore || _iter < _connectors.Count); // just logging

            // We stop on ConnectTimeoutException to fail reasonably fast when the endpoint has many connectors
            // (IP addresses).
            if (_iter < _connectors.Count && !(ex is CommunicatorDestroyedException or ConnectTimeoutException))
            {
                return true; // keep going
            }

            _factory.finishGetConnection(_connectors, ex, this);
            return false;
        }

        private readonly OutgoingConnectionFactory _factory;
        private readonly bool _hasMore;
        private readonly CreateConnectionCallback _callback;
        private readonly List<EndpointI> _endpoints;
        private int _endpointsIter;
        private EndpointI? _currentEndpoint;
        private readonly List<ConnectorInfo> _connectors = [];
        private int _iter;
        private ConnectorInfo? _current;
        private Instrumentation.Observer? _observer;
    }

    private readonly Instance _instance;
    private readonly ConnectionOptions _connectionOptions;
    private bool _destroyed;

    private readonly MultiDictionary<Connector, ConnectionI> _connections = [];
    private readonly MultiDictionary<EndpointI, ConnectionI> _connectionsByEndpoint = [];
    private readonly Dictionary<Connector, HashSet<ConnectCallback>> _pending = [];
    private int _pendingConnectCount;

    private ObjectAdapter? _defaultObjectAdapter;
    private readonly object _mutex = new();
}

internal sealed class IncomingConnectionFactory : EventHandler, ConnectionI.StartCallback
{
    private class StartAcceptor(IncomingConnectionFactory factory) : TimerTask
    {
        public void runTimerTask() => _factory.startAcceptor();

        private readonly IncomingConnectionFactory _factory = factory;
    }

    internal void startAcceptor()
    {
        lock (_mutex)
        {
            if (_state >= StateClosed || _acceptorStarted)
            {
                return;
            }

            try
            {
                createAcceptor();
            }
            catch (System.Exception ex)
            {
                _instance.initializationData().logger!.error($"acceptor creation failed:\n{ex}\n{_acceptor}");
                _instance.timer().schedule(new StartAcceptor(this), 1000);
            }
        }
    }

    internal void activate()
    {
        lock (_mutex)
        {
            setState(StateActive);
        }
    }

    internal void hold()
    {
        lock (_mutex)
        {
            setState(StateHolding);
        }
    }

    internal void destroy()
    {
        lock (_mutex)
        {
            setState(StateClosed);
        }
    }

    internal void updateConnectionObservers()
    {
        lock (_mutex)
        {
            foreach (ConnectionI connection in _connections)
            {
                connection.updateObserver();
            }
        }
    }

    internal void waitUntilHolding()
    {
        ICollection<ConnectionI> connections;

        lock (_mutex)
        {
            // First we wait until the connection factory itself is in holding state.
            while (_state < StateHolding)
            {
                Monitor.Wait(_mutex);
            }

            // We want to wait until all connections are in holding state outside the thread synchronization.
            connections = new List<ConnectionI>(_connections);
        }

        // Now we wait until each connection is in holding state.
        foreach (ConnectionI connection in connections)
        {
            connection.waitUntilHolding();
        }
    }

    internal void waitUntilFinished()
    {
        ICollection<ConnectionI> connections;

        lock (_mutex)
        {
            // First we wait until the factory is destroyed. If we are using an acceptor, we also wait for it to be
            // closed.
            while (_state != StateFinished)
            {
                Monitor.Wait(_mutex);
            }

            // We want to wait until all connections are finished outside the thread synchronization.
            connections = new List<ConnectionI>(_connections);
        }

        foreach (ConnectionI connection in connections)
        {
            connection.waitUntilFinished();
        }

        lock (_mutex)
        {
            _connections.Clear();
        }
    }

    internal EndpointI endpoint()
    {
        lock (_mutex)
        {
            return _endpoint;
        }
    }

    internal ICollection<ConnectionI> connections()
    {
        lock (_mutex)
        {
            ICollection<ConnectionI> connections = [];

            // Only copy connections which have not been destroyed.
            foreach (ConnectionI connection in _connections)
            {
                if (connection.isActiveOrHolding())
                {
                    connections.Add(connection);
                }
            }

            return connections;
        }
    }

    internal void flushAsyncBatchRequests(CompressBatch compressBatch, CommunicatorFlushBatchAsync outAsync)
    {
        // connections() is synchronized, no need to synchronize here.
        foreach (ConnectionI connection in connections())
        {
            try
            {
                outAsync.flushConnection(connection, compressBatch);
            }
            catch (LocalException)
            {
                // Ignore.
            }
        }
    }

    //
    // Operations from EventHandler.
    //
    public override bool startAsync(int operation, AsyncCallback completedCallback)
    {
        if (_state >= StateClosed)
        {
            return false;
        }

        // Run the IO operation on a .NET thread pool thread to ensure the IO operation won't be interrupted if the
        // Ice thread pool thread is terminated.
        Task.Run(() =>
        {
            lock (_mutex)
            {
                if (_state >= StateClosed)
                {
                    completedCallback(this);
                    return;
                }

                try
                {
                    Debug.Assert(_acceptor is not null);
                    if (_acceptor.startAccept(completedCallback, this))
                    {
                        completedCallback(this);
                    }
                }
                catch (LocalException ex)
                {
                    _acceptorException = ex;
                    completedCallback(this);
                }
            }
        });

        return true;
    }

    public override bool finishAsync(int operation)
    {
        try
        {
            if (_acceptorException != null)
            {
                throw _acceptorException;
            }
            Debug.Assert(_acceptor is not null);
            _acceptor.finishAccept();
        }
        catch (LocalException ex)
        {
            _acceptorException = null;
            _instance.initializationData().logger!.error($"couldn't accept connection:\n{ex}\n{_acceptor}");
            if (_acceptorStarted)
            {
                _acceptorStarted = false;
                _adapter.getThreadPool().finish(this);
                closeAcceptor();
            }
        }
        return _state < StateClosed;
    }

    public override void message(ThreadPoolCurrent current)
    {
        ConnectionI? connection = null;

        using var msg = new ThreadPoolMessage(current, this);

        lock (_mutex)
        {
            if (!msg.startIOScope())
            {
                return;
            }

            try
            {
                if (_state >= StateClosed)
                {
                    return;
                }
                else if (_state == StateHolding)
                {
                    return;
                }

                if (!_acceptorStarted)
                {
                    return;
                }

                //
                // Now accept a new connection.
                //
                Transceiver? transceiver = null;
                try
                {
                    Debug.Assert(_acceptor is not null);
                    transceiver = _acceptor.accept();

                    if (_maxConnections > 0 && _connections.Count == _maxConnections)
                    {
                        // Can't accept more connections, so we abort this transport connection.

                        if (_instance.traceLevels().network >= 2)
                        {
                            _instance.initializationData().logger!.trace(
                                _instance.traceLevels().networkCat,
                                $"rejecting new {_endpoint.protocol()} connection\n{transceiver}\nbecause the maximum number of connections has been reached");
                        }

                        try
                        {
                            transceiver.close();
                        }
                        catch
                        {
                            // Ignore
                        }
                        transceiver.destroy();
                        return;
                    }

                    if (_instance.traceLevels().network >= 2)
                    {
                        var s = new StringBuilder("trying to accept ");
                        s.Append(_endpoint.protocol());
                        s.Append(" connection\n");
                        s.Append(transceiver.ToString());
                        _instance.initializationData().logger!.trace(_instance.traceLevels().networkCat, s.ToString());
                    }
                }
                catch (SocketException ex)
                {
                    if (Network.noMoreFds(ex.InnerException))
                    {
                        string s = "can't accept more connections:\n" + ex + '\n' + _acceptor?.ToString();
                        _instance.initializationData().logger!.error(s);
                        Debug.Assert(_acceptorStarted);
                        _acceptorStarted = false;
                        _adapter.getThreadPool().finish(this);
                        closeAcceptor();
                    }

                    // Ignore socket exceptions.
                    return;
                }
                catch (LocalException ex)
                {
                    // Warn about other Ice local exceptions.
                    if (_warn)
                    {
                        warning(ex);
                    }
                    return;
                }

                Debug.Assert(transceiver is not null);

                try
                {
                    connection = new ConnectionI(
                        _instance,
                        transceiver,
                        connector: null,
                        _endpoint,
                        _adapter,
                        removeConnection,
                        _connectionOptions);
                }
                catch (LocalException ex)
                {
                    try
                    {
                        transceiver.close();
                    }
                    catch (LocalException)
                    {
                        // Ignore
                    }

                    if (_warn)
                    {
                        warning(ex);
                    }
                    return;
                }

                _connections.Add(connection);
            }
            finally
            {
                msg.finishIOScope();
            }
        }

        Debug.Assert(connection != null);
        connection.start(this);
    }

    public override void finished(ThreadPoolCurrent current)
    {
        lock (_mutex)
        {
            if (_state < StateClosed)
            {
                // If the acceptor hasn't been explicitly stopped (which is the case if the acceptor got closed
                // because of an unexpected error), try to restart the acceptor in 1 second.
                _instance.timer().schedule(new StartAcceptor(this), 1000);
                return;
            }

            Debug.Assert(_state >= StateClosed);
            setState(StateFinished);
        }
    }

    public override string ToString() => _transceiver?.ToString() ?? _acceptor?.ToString() ?? "";

    //
    // Operations from ConnectionI.StartCallback
    //
    public void connectionStartCompleted(ConnectionI connection)
    {
        lock (_mutex)
        {
            // Initially, connections are in the holding state. If the factory is active we activate the connection.
            if (_state == StateActive)
            {
                connection.activate();
            }
        }
    }

    public void connectionStartFailed(ConnectionI connection, LocalException ex)
    {
        // Do not warn about connection exceptions here. The connection is not yet validated.
    }

    internal IncomingConnectionFactory(Instance instance, EndpointI endpoint, ObjectAdapter adapter)
    {
        _instance = instance;
        _connectionOptions = instance.serverConnectionOptions(adapter.getName());

        // Meaningful only for non-datagram (non-UDP) connections.
        _maxConnections = endpoint.datagram() ? 0 :
            instance.initializationData().properties!.getPropertyAsInt($"{adapter.getName()}.MaxConnections");

        _endpoint = endpoint;
        _adapter = adapter;
        _warn = _instance.initializationData().properties!.getIcePropertyAsInt("Ice.Warn.Connections") > 0;
        _connections = [];
        _state = StateHolding;
        _acceptorStarted = false;

        DefaultsAndOverrides defaultsAndOverrides = _instance.defaultsAndOverrides();

        if (defaultsAndOverrides.overrideCompress is not null)
        {
            _endpoint = _endpoint.compress(defaultsAndOverrides.overrideCompress.Value);
        }

        try
        {
            _transceiver = _endpoint.transceiver();
            if (_transceiver != null)
            {
                // All this is for UDP "connections".

                if (_instance.traceLevels().network >= 2)
                {
                    var s = new StringBuilder("attempting to bind to ");
                    s.Append(_endpoint.protocol());
                    s.Append(" socket\n");
                    s.Append(_transceiver.ToString());
                    _instance.initializationData().logger!.trace(_instance.traceLevels().networkCat, s.ToString());
                }
                _endpoint = _transceiver.bind();

                var connection = new ConnectionI(
                    _instance,
                    _transceiver,
                    connector: null,
                    _endpoint,
                    adapter,
                    removeFromFactory: null,
                    _connectionOptions);
                connection.startAndWait();
                _connections.Add(connection);
                Debug.Assert(_maxConnections == 0); // UDP so no max connections.
            }
            else
            {
                createAcceptor();
            }
        }
        catch (System.Exception ex)
        {
            //
            // Clean up.
            //
            if (_transceiver != null)
            {
                try
                {
                    _transceiver.close();
                }
                catch (LocalException)
                {
                    // Ignore
                }
            }

            _state = StateFinished;
            _connections.Clear();

            if (ex is LocalException)
            {
                throw;
            }
            else
            {
                throw new SyscallException(ex);
            }
        }
    }

    private const int StateActive = 0;
    private const int StateHolding = 1;
    private const int StateClosed = 2;
    private const int StateFinished = 3;

    private void setState(int state)
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

                if (_acceptor is not null)
                {
                    if (_instance.traceLevels().network >= 1)
                    {
                        _instance.initializationData().logger!.trace(
                            _instance.traceLevels().networkCat,
                            $"accepting {_endpoint.protocol()} connections at {_acceptor}");
                    }
                    _adapter.getThreadPool().register(this, SocketOperation.Read);
                }

                foreach (ConnectionI connection in _connections)
                {
                    connection.activate();
                }
                break;
            }

            case StateHolding:
            {
                if (_state != StateActive) // Can only switch from active to holding.
                {
                    return;
                }

                // Stop accepting new connections.
                if (_acceptor is not null)
                {
                    if (_instance.traceLevels().network >= 1)
                    {
                        _instance.initializationData().logger!.trace(
                            _instance.traceLevels().networkCat,
                            $"holding {_endpoint.protocol()} connections at {_acceptor}");
                    }
                    _adapter.getThreadPool().unregister(this, SocketOperation.Read);
                }

                foreach (ConnectionI connection in _connections)
                {
                    connection.hold();
                }
                break;
            }

            case StateClosed:
            {
                if (_acceptorStarted)
                {
                    _acceptorStarted = false;
                    _adapter.getThreadPool().finish(this);
                    closeAcceptor();
                }
                else
                {
                    state = StateFinished;
                }

                foreach (ConnectionI connection in _connections)
                {
                    connection.destroy(ConnectionI.ObjectAdapterDeactivated);
                }
                break;
            }

            case StateFinished:
            {
                Debug.Assert(_state == StateClosed);
                break;
            }
        }

        _state = state;
        System.Threading.Monitor.PulseAll(_mutex);
    }

    private void createAcceptor()
    {
        try
        {
            Debug.Assert(!_acceptorStarted);
            _acceptor = _endpoint.acceptor(_adapter.getName(), _adapter.getServerAuthenticationOptions());
            Debug.Assert(_acceptor != null);

            if (_instance.traceLevels().network >= 2)
            {
                var s = new StringBuilder("attempting to bind to ");
                s.Append(_endpoint.protocol());
                s.Append(" socket ");
                s.Append(_acceptor.ToString());
                _instance.initializationData().logger!.trace(_instance.traceLevels().networkCat, s.ToString());
            }
            _endpoint = _acceptor.listen();

            if (_instance.traceLevels().network >= 1)
            {
                var s = new StringBuilder("listening for ");
                s.Append(_endpoint.protocol());
                s.Append(" connections\n");
                s.Append(_acceptor.toDetailedString());
                _instance.initializationData().logger!.trace(_instance.traceLevels().networkCat, s.ToString());
            }

            _adapter.getThreadPool().initialize(this);

            if (_state == StateActive)
            {
                _adapter.getThreadPool().register(this, SocketOperation.Read);
            }

            _acceptorStarted = true;
        }
        catch (SystemException)
        {
            _acceptor?.close();
            throw;
        }
    }

    private void closeAcceptor()
    {
        Debug.Assert(_acceptor != null);

        if (_instance.traceLevels().network >= 1)
        {
            var s = new StringBuilder("stopping to accept ");
            s.Append(_endpoint.protocol());
            s.Append(" connections at ");
            s.Append(_acceptor.ToString());
            _instance.initializationData().logger!.trace(_instance.traceLevels().networkCat, s.ToString());
        }

        Debug.Assert(!_acceptorStarted);
        _acceptor.close();
    }

    private void removeConnection(ConnectionI connection)
    {
        lock (_mutex)
        {
            if (_state is StateActive or StateHolding)
            {
                _connections.Remove(connection);
            }
            // else it's already being cleaned up.
        }
    }

    private void warning(LocalException ex) =>
        _instance.initializationData().logger!.warning($"connection exception:\n{ex}\n{_acceptor}");

    private readonly Instance _instance;
    private readonly ConnectionOptions _connectionOptions;

    private readonly int _maxConnections;

    private Acceptor? _acceptor;
    private readonly Transceiver? _transceiver;
    private EndpointI _endpoint;
    private readonly ObjectAdapter _adapter;

    private readonly bool _warn;

    private readonly HashSet<ConnectionI> _connections;

    private int _state;
    private bool _acceptorStarted;
    private LocalException? _acceptorException;
    private readonly object _mutex = new();
}
