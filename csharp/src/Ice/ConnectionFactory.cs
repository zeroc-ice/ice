//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;

namespace IceInternal
{
    public class MultiDictionary<TKey, TValue> : Dictionary<TKey, ICollection<TValue>> where TKey : notnull
    {
        public void
        Add(TKey key, TValue value)
        {
            if (!TryGetValue(key, out ICollection<TValue>? list))
            {
                list = new List<TValue>();
                Add(key, list);
            }
            list.Add(value);
        }

        public void
        Remove(TKey key, TValue value)
        {
            ICollection<TValue> list = this[key];
            list.Remove(value);
            if (list.Count == 0)
            {
                Remove(key);
            }
        }
    }

    public sealed class OutgoingConnectionFactory
    {
        public interface ICreateConnectionCallback
        {
            void SetConnection(Connection connection, bool compress);
            void SetException(System.Exception ex);
        }

        public void Destroy()
        {
            lock (this)
            {
                if (_destroyed)
                {
                    return;
                }

                foreach (ICollection<Connection> connections in _connections.Values)
                {
                    foreach (Connection c in connections)
                    {
                        c.Destroy(new CommunicatorDestroyedException());
                    }
                }

                _destroyed = true;
                System.Threading.Monitor.PulseAll(this);
            }
        }

        public void UpdateConnectionObservers()
        {
            lock (this)
            {
                foreach (ICollection<Connection> connections in _connections.Values)
                {
                    foreach (Connection c in connections)
                    {
                        c.UpdateObserver();
                    }
                }
            }
        }

        public void WaitUntilFinished()
        {
            Dictionary<IConnector, ICollection<Connection>> connections;
            lock (this)
            {
                //
                // First we wait until the factory is destroyed. We also
                // wait until there are no pending connections
                // anymore. Only then we can be sure the _connections
                // contains all connections.
                //
                while (!_destroyed || _pending.Count > 0 || _pendingConnectCount > 0)
                {
                    System.Threading.Monitor.Wait(this);
                }

                //
                // We want to wait until all connections are finished outside the
                // thread synchronization.
                //
                connections = new Dictionary<IConnector, ICollection<Connection>>(_connections);
            }

            //
            // Now we wait until the destruction of each connection is finished.
            //
            foreach (ICollection<Connection> cl in connections.Values)
            {
                foreach (Connection c in cl)
                {
                    c.WaitUntilFinished();
                }
            }

            lock (this)
            {
#if DEBUG
                // Ensure all the connections are finished and reapable at this point.
                foreach (Connection c in _monitor.SwapReapedConnections())
                {
                    _connections.Remove(c.Connector, c);
                    _connectionsByEndpoint.Remove(c.Endpoint, c);
                    _connectionsByEndpoint.Remove(c.Endpoint.NewCompressionFlag(true), c);
                }
                Debug.Assert(_connections.Count == 0);
                Debug.Assert(_connectionsByEndpoint.Count == 0);
#else
                _monitor.SwapReapedConnections();
                _connections.Clear();
                _connectionsByEndpoint.Clear();
#endif
            }

            //
            // Must be destroyed outside the synchronization since this might block waiting for
            // a timer task to execute.
            //
            _monitor.Destroy();
        }

        public void Create(IReadOnlyList<Endpoint> endpts, bool hasMore, Ice.EndpointSelectionType selType,
                           ICreateConnectionCallback callback)
        {
            Debug.Assert(endpts.Count > 0);

            //
            // Apply the overrides.
            //
            IReadOnlyList<Endpoint> endpoints = ApplyOverrides(endpts).ToArray();

            //
            // Try to find a connection to one of the given endpoints.
            //
            try
            {
                Connection? connection = FindConnection(endpoints, out bool compress);
                if (connection != null)
                {
                    callback.SetConnection(connection, compress);
                    return;
                }
            }
            catch (System.Exception ex)
            {
                callback.SetException(ex);
                return;
            }
            var cb = new ConnectCallback(this, endpoints, hasMore, callback, selType);
            cb.GetConnectors();
        }

        public void SetRouterInfo(RouterInfo routerInfo)
        {
            Debug.Assert(routerInfo != null);
            Ice.ObjectAdapter? adapter = routerInfo.Adapter;
            IReadOnlyList<Endpoint> endpoints = routerInfo.GetClientEndpoints(); // Must be called outside the synchronization

            lock (this)
            {
                if (_destroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                //
                // Search for connections to the router's client proxy
                // endpoints, and update the object adapter for such
                // connections, so that callbacks from the router can be
                // received over such connections.
                //
                for (int i = 0; i < endpoints.Count; ++i)
                {
                    Endpoint endpoint = endpoints[i];

                    //
                    // Modify endpoints with overrides.
                    //
                    if (_communicator.OverrideTimeout != null)
                    {
                        endpoint = endpoint.NewTimeout(_communicator.OverrideTimeout.Value);
                    }

                    //
                    // The ConnectionI object does not take the compression flag of
                    // endpoints into account, but instead gets the information
                    // about whether messages should be compressed or not from
                    // other sources. In order to allow connection sharing for
                    // endpoints that differ in the value of the compression flag
                    // only, we always set the compression flag to false here in
                    // this connection factory.
                    //
                    endpoint = endpoint.NewCompressionFlag(false);

                    foreach (ICollection<Connection> connections in _connections.Values)
                    {
                        foreach (Connection connection in connections)
                        {
                            if (connection.Endpoint == endpoint)
                            {
                                connection.SetAdapter(adapter);
                            }
                        }
                    }
                }
            }
        }

        public void RemoveAdapter(Ice.ObjectAdapter adapter)
        {
            lock (this)
            {
                if (_destroyed)
                {
                    return;
                }

                foreach (ICollection<Connection> connectionList in _connections.Values)
                {
                    foreach (Connection connection in connectionList)
                    {
                        if (connection.GetAdapter() == adapter)
                        {
                            connection.SetAdapter(null);
                        }
                    }
                }
            }
        }

        //
        // Only for use by Instance.
        //
        internal OutgoingConnectionFactory(Ice.Communicator communicator)
        {
            _communicator = communicator;
            _destroyed = false;
            _monitor = new FactoryACMMonitor(communicator, communicator.ClientACM);
            _pendingConnectCount = 0;
        }

        private IEnumerable<Endpoint> ApplyOverrides(IReadOnlyList<Endpoint> endpoints)
        {
            // TODO: why do we apply only the timeout override?
            return endpoints.Select(endpoint =>
                (_communicator.OverrideTimeout != null) ? endpoint.NewTimeout(_communicator.OverrideTimeout.Value) :
                    endpoint);
        }

        private Connection? FindConnection(IReadOnlyList<Endpoint> endpoints, out bool compress)
        {
            lock (this)
            {
                if (_destroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                Debug.Assert(endpoints.Count > 0);

                foreach (Endpoint endpoint in endpoints)
                {
                    if (!_connectionsByEndpoint.TryGetValue(endpoint, out ICollection<Connection>? connectionList))
                    {
                        continue;
                    }

                    foreach (Connection connection in connectionList)
                    {
                        if (connection.Active) // Don't return destroyed or unvalidated connections
                        {
                            compress = _communicator.OverrideCompress ?? endpoint.HasCompressionFlag;
                            return connection;
                        }
                    }
                }

                compress = false; // Satisfy the compiler
                return null;
            }
        }

        //
        // Must be called while synchronized.
        //
        private Connection? FindConnection(List<ConnectorInfo> connectors, out bool compress)
        {
            foreach (ConnectorInfo ci in connectors)
            {
                if (_pending.ContainsKey(ci.Connector))
                {
                    continue;
                }

                if (!_connections.TryGetValue(ci.Connector, out ICollection<Connection>? connectionList))
                {
                    continue;
                }

                foreach (Connection connection in connectionList)
                {
                    if (connection.Active) // Don't return destroyed or un-validated connections
                    {
                        compress = _communicator.OverrideCompress ?? ci.Endpoint.HasCompressionFlag;
                        return connection;
                    }
                }
            }

            compress = false; // Satisfy the compiler
            return null;
        }

        internal void IncPendingConnectCount()
        {
            //
            // Keep track of the number of pending connects. The outgoing connection factory
            // waitUntilFinished() method waits for all the pending connects to terminate before
            // to return. This ensures that the communicator client thread pool isn't destroyed
            // too soon and will still be available to execute the ice_exception() callbacks for
            // the asynchronous requests waiting on a connection to be established.
            //

            lock (this)
            {
                if (_destroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                ++_pendingConnectCount;
            }
        }

        internal void DecPendingConnectCount()
        {
            lock (this)
            {
                --_pendingConnectCount;
                Debug.Assert(_pendingConnectCount >= 0);
                if (_destroyed && _pendingConnectCount == 0)
                {
                    System.Threading.Monitor.PulseAll(this);
                }
            }
        }

        private Connection? GetConnection(List<ConnectorInfo> connectors, ConnectCallback cb, out bool compress)
        {
            lock (this)
            {
                if (_destroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                //
                // Reap closed connections
                //
                foreach (Connection c in _monitor.SwapReapedConnections())
                {
                    _connections.Remove(c.Connector, c);
                    _connectionsByEndpoint.Remove(c.Endpoint, c);
                    _connectionsByEndpoint.Remove(c.Endpoint.NewCompressionFlag(true), c);
                }

                //
                // Try to get the connection. We may need to wait for other threads to
                // finish if one of them is currently establishing a connection to one
                // of our connectors.
                //
                while (true)
                {
                    if (_destroyed)
                    {
                        throw new Ice.CommunicatorDestroyedException();
                    }

                    //
                    // Search for a matching connection. If we find one, we're done.
                    //
                    Connection? connection = FindConnection(connectors, out compress);
                    if (connection != null)
                    {
                        return connection;
                    }

                    if (AddToPending(cb, connectors))
                    {
                        //
                        // If a callback is not specified we wait until another thread notifies us about a
                        // change to the pending list. Otherwise, if a callback is provided we're done:
                        // when the pending list changes the callback will be notified and will try to
                        // get the connection again.
                        //
                        if (cb == null)
                        {
                            System.Threading.Monitor.Wait(this);
                        }
                        else
                        {
                            return null;
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
            if (cb != null)
            {
                cb.NextConnector();
            }

            compress = false; // Satisfy the compiler
            return null;
        }

        private Connection CreateConnection(ITransceiver transceiver, ConnectorInfo ci)
        {
            lock (this)
            {
                Debug.Assert(_pending.ContainsKey(ci.Connector) && transceiver != null);

                //
                // Create and add the connection to the connection map. Adding the connection to the map
                // is necessary to support the interruption of the connection initialization and validation
                // in case the communicator is destroyed.
                //
                Connection connection;
                try
                {
                    if (_destroyed)
                    {
                        throw new Ice.CommunicatorDestroyedException();
                    }

                    connection = new Connection(_communicator, _monitor, transceiver, ci.Connector,
                                                    ci.Endpoint.NewCompressionFlag(false), null);
                }
                catch (System.Exception)
                {
                    try
                    {
                        transceiver.Close();
                    }
                    catch (System.Exception)
                    {
                        // Ignore
                    }
                    throw;
                }
                _connections.Add(ci.Connector, connection);
                _connectionsByEndpoint.Add(connection.Endpoint, connection);
                _connectionsByEndpoint.Add(connection.Endpoint.NewCompressionFlag(true), connection);
                return connection;
            }
        }

        private void FinishGetConnection(List<ConnectorInfo> connectors,
                                         ConnectorInfo ci,
                                         Connection connection,
                                         ConnectCallback cb)
        {
            var connectionCallbacks = new HashSet<ConnectCallback>();
            if (cb != null)
            {
                connectionCallbacks.Add(cb);
            }

            var callbacks = new HashSet<ConnectCallback>();
            lock (this)
            {
                foreach (ConnectorInfo c in connectors)
                {
                    if (_pending.TryGetValue(c.Connector, out HashSet<ConnectCallback>? s))
                    {
                        foreach (ConnectCallback cc in s)
                        {
                            if (cc.HasConnector(ci))
                            {
                                connectionCallbacks.Add(cc);
                            }
                            else
                            {
                                callbacks.Add(cc);
                            }
                        }
                        _pending.Remove(c.Connector);
                    }
                }

                foreach (ConnectCallback cc in connectionCallbacks)
                {
                    cc.RemoveFromPending();
                    callbacks.Remove(cc);
                }
                foreach (ConnectCallback cc in callbacks)
                {
                    cc.RemoveFromPending();
                }
                System.Threading.Monitor.PulseAll(this);
            }

            bool compress = _communicator.OverrideCompress ?? ci.Endpoint.HasCompressionFlag;

            foreach (ConnectCallback cc in callbacks)
            {
                cc.GetConnection();
            }
            foreach (ConnectCallback cc in connectionCallbacks)
            {
                cc.SetConnection(connection, compress);
            }
        }

        private void FinishGetConnection(List<ConnectorInfo> connectors, System.Exception ex, ConnectCallback cb)
        {
            var failedCallbacks = new HashSet<ConnectCallback>();
            if (cb != null)
            {
                failedCallbacks.Add(cb);
            }

            var callbacks = new HashSet<ConnectCallback>();
            lock (this)
            {
                foreach (ConnectorInfo c in connectors)
                {
                    if (_pending.TryGetValue(c.Connector, out HashSet<ConnectCallback>? s))
                    {
                        foreach (ConnectCallback cc in s)
                        {
                            if (cc.RemoveConnectors(connectors))
                            {
                                failedCallbacks.Add(cc);
                            }
                            else
                            {
                                callbacks.Add(cc);
                            }
                        }
                        _pending.Remove(c.Connector);
                    }
                }

                foreach (ConnectCallback cc in callbacks)
                {
                    Debug.Assert(!failedCallbacks.Contains(cc));
                    cc.RemoveFromPending();
                }
                System.Threading.Monitor.PulseAll(this);
            }

            foreach (ConnectCallback cc in callbacks)
            {
                cc.GetConnection();
            }

            foreach (ConnectCallback cc in failedCallbacks)
            {
                cc.SetException(ex);
            }
        }

        private void HandleConnectionException(System.Exception ex, bool hasMore)
        {
            TraceLevels traceLevels = _communicator.TraceLevels;
            if (traceLevels.Network >= 2)
            {
                if (ex is Ice.CommunicatorDestroyedException)
                {
                    _communicator.Logger.Trace(traceLevels.NetworkCat, $"connection to endpoint failed\n{ex}");
                }
                else if (hasMore)
                {
                    _communicator.Logger.Trace(traceLevels.NetworkCat,
                        $"connection to endpoint failed, trying next endpoint\n{ex}");
                }
                else
                {
                    _communicator.Logger.Trace(traceLevels.NetworkCat,
                        $"connection to endpoint failed and no more endpoints to try\n{ex}");
                }
            }
        }

        private bool
        AddToPending(ConnectCallback? cb, List<ConnectorInfo> connectors)
        {
            //
            // Add the callback to each connector pending list.
            //
            bool found = false;
            foreach (ConnectorInfo ci in connectors)
            {
                if (_pending.TryGetValue(ci.Connector, out HashSet<ConnectCallback>? cbs))
                {
                    found = true;
                    if (cb != null)
                    {
                        cbs.Add(cb); // Add the callback to each pending connector.
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
            foreach (ConnectorInfo ci in connectors)
            {
                if (!_pending.ContainsKey(ci.Connector))
                {
                    _pending.Add(ci.Connector, new HashSet<ConnectCallback>());
                }
            }
            return false;
        }

        private void
        RemoveFromPending(ConnectCallback cb, List<ConnectorInfo> connectors)
        {
            foreach (ConnectorInfo ci in connectors)
            {
                if (_pending.TryGetValue(ci.Connector, out HashSet<ConnectCallback>? cbs))
                {
                    cbs.Remove(cb);
                }
            }
        }

        internal void HandleException(System.Exception ex, bool hasMore)
        {
            TraceLevels traceLevels = _communicator.TraceLevels;
            if (traceLevels.Network >= 2)
            {
                if (ex is Ice.CommunicatorDestroyedException)
                {
                    _communicator.Logger.Trace(traceLevels.NetworkCat, $"couldn't resolve endpoint host\n{ex}");
                }
                else if (hasMore)
                {
                    _communicator.Logger.Trace(traceLevels.NetworkCat,
                        $"couldn't resolve endpoint host, trying next endpoint\n{ex}");
                }
                else
                {
                    _communicator.Logger.Trace(traceLevels.NetworkCat,
                        $"couldn't resolve endpoint host and no more endpoints to try\n{ex}");
                }
            }
        }

        private class ConnectorInfo
        {
            internal ConnectorInfo(IConnector c, Endpoint e)
            {
                Connector = c;
                Endpoint = e;
            }

            public override bool Equals(object? obj)
            {
                if (obj is ConnectorInfo r)
                {
                    return Connector.Equals(r.Connector);
                }
                return false;
            }

            public override int GetHashCode() => Connector.GetHashCode();

            public IConnector Connector;
            public Endpoint Endpoint;
        }

        private class ConnectCallback : Connection.IStartCallback, IEndpointConnectors
        {
            internal ConnectCallback(OutgoingConnectionFactory f, IReadOnlyList<Endpoint> endpoints, bool more,
                                     ICreateConnectionCallback cb, Ice.EndpointSelectionType selType)
            {
                Debug.Assert(endpoints.Count > 0);

                _factory = f;
                _endpoints = endpoints;
                _endpointEnumerator = _endpoints.GetEnumerator();
                _hasMoreEndpoints = _endpointEnumerator.MoveNext();
                Debug.Assert(_hasMoreEndpoints); // even if there is only one endpoint, we have not read it yet with
                                                // _endpointsEnumerator.Current
                _hasMore = more;
                _callback = cb;
                _selType = selType;
            }

            //
            // Methods from ConnectionI.StartCallback
            //
            public void ConnectionStartCompleted(Connection connection)
            {
                if (_observer != null)
                {
                    _observer.Detach();
                }
                Debug.Assert(_current != null);
                _factory.FinishGetConnection(_connectors, _current, connection, this);
            }

            public void ConnectionStartFailed(Connection connection, System.Exception ex)
            {
                if (ConnectionStartFailedImpl(ex))
                {
                    NextConnector();
                }
            }

            //
            // Methods from EndpointI_connectors
            //
            public void Connectors(List<IConnector> cons)
            {
                Debug.Assert(_currentEndpoint != null);
                foreach (IConnector connector in cons)
                {
                    _connectors.Add(new ConnectorInfo(connector, _currentEndpoint));
                }

                if (_hasMoreEndpoints)
                {
                    NextEndpoint();
                }
                else
                {
                    Debug.Assert(_connectors.Count > 0);

                    //
                    // We now have all the connectors for the given endpoints. We can try to obtain the
                    // connection.
                    //
                    _iter = 0;
                    GetConnection();
                }
            }

            public void Exception(System.Exception ex)
            {
                _factory.HandleException(ex, _hasMore || _hasMoreEndpoints);
                if (_hasMoreEndpoints)
                {
                    NextEndpoint();
                }
                else if (_connectors.Count > 0)
                {
                    //
                    // We now have all the connectors for the given endpoints. We can try to obtain the
                    // connection.
                    //
                    _iter = 0;
                    GetConnection();
                }
                else
                {
                    _callback.SetException(ex);
                    _factory.DecPendingConnectCount(); // Must be called last.
                }
            }

            public void SetConnection(Connection connection, bool compress)
            {
                //
                // Callback from the factory: the connection to one of the callback
                // connectors has been established.
                //
                _callback.SetConnection(connection, compress);
                _factory.DecPendingConnectCount(); // Must be called last.
            }

            public void SetException(System.Exception ex)
            {
                //
                // Callback from the factory: connection establishment failed.
                //
                _callback.SetException(ex);
                _factory.DecPendingConnectCount(); // Must be called last.
            }

            public bool HasConnector(ConnectorInfo ci) => _connectors.Contains(ci);

            public bool RemoveConnectors(List<ConnectorInfo> connectors)
            {
                foreach (ConnectorInfo ci in connectors)
                {
                    while (_connectors.Remove(ci))
                    {
                        ; // Remove all of them.
                    }
                }
                return _connectors.Count == 0;
            }

            public void RemoveFromPending() => _factory.RemoveFromPending(this, _connectors);

            public void GetConnectors()
            {
                try
                {
                    //
                    // Notify the factory that there's an async connect pending. This is necessary
                    // to prevent the outgoing connection factory to be destroyed before all the
                    // pending asynchronous connects are finished.
                    //
                    _factory.IncPendingConnectCount();
                }
                catch (System.Exception ex)
                {
                    _callback.SetException(ex);
                    return;
                }

                NextEndpoint();
            }

            private void NextEndpoint()
            {
                try
                {
                    Debug.Assert(_hasMoreEndpoints);
                    _currentEndpoint = _endpointEnumerator.Current;
                    _hasMoreEndpoints = _endpointEnumerator.MoveNext();
                    _currentEndpoint.ConnectorsAsync(_selType, this);
                }
                catch (System.Exception ex)
                {
                    Exception(ex);
                }
            }

            internal void GetConnection()
            {
                try
                {
                    //
                    // If all the connectors have been created, we ask the factory to get a
                    // connection.
                    //
                    Connection? connection = _factory.GetConnection(_connectors, this, out bool compress);
                    if (connection == null)
                    {
                        //
                        // A null return value from getConnection indicates that the connection
                        // is being established and that everthing has been done to ensure that
                        // the callback will be notified when the connection establishment is
                        // done.
                        //
                        return;
                    }

                    _callback.SetConnection(connection, compress);
                    _factory.DecPendingConnectCount(); // Must be called last.
                }
                catch (System.Exception ex)
                {
                    _callback.SetException(ex);
                    _factory.DecPendingConnectCount(); // Must be called last.
                }
            }

            internal void NextConnector()
            {
                while (true)
                {
                    try
                    {
                        Debug.Assert(_iter < _connectors.Count);
                        _current = _connectors[_iter++];

                        Ice.Instrumentation.ICommunicatorObserver? obsv = _factory._communicator.Observer;
                        if (obsv != null)
                        {
                            _observer = obsv.GetConnectionEstablishmentObserver(_current.Endpoint,
                                                                                _current.Connector.ToString()!);
                            if (_observer != null)
                            {
                                _observer.Attach();
                            }
                        }

                        if (_factory._communicator.TraceLevels.Network >= 2)
                        {
                            _factory._communicator.Logger.Trace(_factory._communicator.TraceLevels.NetworkCat,
                                $"trying to establish {_current.Endpoint.Name} connection to " +
                                $"{_current.Connector}");
                        }

                        Connection connection = _factory.CreateConnection(_current.Connector.Connect(), _current);
                        connection.Start(this);
                    }
                    catch (System.Exception ex)
                    {
                        if (_factory._communicator.TraceLevels.Network >= 2)
                        {
                            Debug.Assert(_current != null);
                            _factory._communicator.Logger.Trace(_factory._communicator.TraceLevels.NetworkCat,
                                $"failed to establish {_current.Endpoint.Name} connection to " +
                                $"{_current.Connector}\n{ex}");
                        }

                        if (ConnectionStartFailedImpl(ex))
                        {
                            continue;
                        }
                    }
                    break;
                }
            }

            private bool ConnectionStartFailedImpl(System.Exception ex)
            {
                if (_observer != null)
                {
                    _observer.Failed(ex.GetType().FullName ?? "System.Exception");
                    _observer.Detach();
                }
                _factory.HandleConnectionException(ex, _hasMore || _iter < _connectors.Count);
                if (ex is Ice.CommunicatorDestroyedException) // No need to continue.
                {
                    _factory.FinishGetConnection(_connectors, ex, this);
                }
                else if (_iter < _connectors.Count) // Try the next connector.
                {
                    return true;
                }
                else
                {
                    _factory.FinishGetConnection(_connectors, ex, this);
                }
                return false;
            }

            private readonly OutgoingConnectionFactory _factory;
            private readonly bool _hasMore;
            private readonly ICreateConnectionCallback _callback;
            private readonly IReadOnlyList<Endpoint> _endpoints;
            private readonly EndpointSelectionType _selType;
            private readonly IEnumerator<Endpoint> _endpointEnumerator;
            private bool _hasMoreEndpoints;
            private Endpoint? _currentEndpoint;
            private readonly List<ConnectorInfo> _connectors = new List<ConnectorInfo>();
            private int _iter;
            private ConnectorInfo? _current;
            private Ice.Instrumentation.IObserver? _observer;
        }

        private readonly Ice.Communicator _communicator;
        private readonly FactoryACMMonitor _monitor;
        private bool _destroyed;

        private readonly MultiDictionary<IConnector, Connection> _connections =
            new MultiDictionary<IConnector, Connection>();
        private readonly MultiDictionary<Endpoint, Connection> _connectionsByEndpoint =
            new MultiDictionary<Endpoint, Connection>();
        private readonly Dictionary<IConnector, HashSet<ConnectCallback>> _pending =
            new Dictionary<IConnector, HashSet<ConnectCallback>>();
        private int _pendingConnectCount;
    }

    public sealed class IncomingConnectionFactory : EventHandler
    {
        public void Activate()
        {
            lock (this)
            {
                SetState(State.Active);
            }
        }

        public void Destroy()
        {
            lock (this)
            {
                SetState(State.Closed);
            }
        }

        public void UpdateConnectionObservers()
        {
            lock (this)
            {
                foreach (Connection connection in _connections)
                {
                    connection.UpdateObserver();
                }
            }
        }

        public void WaitUntilFinished()
        {
            lock (this)
            {
                // First we wait until the factory is destroyed. If we are using
                // an acceptor, we also wait for it to be closed.
                while (_state != State.Finished)
                {
                    System.Threading.Monitor.Wait(this);
                }
            }

            // _connections is immutable in this state
            foreach (Connection connection in _connections)
            {
                connection.WaitUntilFinished();
            }

            // Ensure all the connections are finished and reaped.
            if (_transceiver == null)
            {
#if DEBUG
                IEnumerable<Connection> cons = _monitor.SwapReapedConnections();
                Debug.Assert(cons.Count() == _connections.Count);
#else
                _monitor.SwapReapedConnections();
#endif
            }
            _connections.Clear();

            // Must be destroyed outside the synchronization since this might block waiting for
            // a timer task to execute.
            _monitor.Destroy();
        }

        public bool IsLocal(Endpoint endpoint)
        {
            if (_publishedEndpoint != null && endpoint.Equivalent(_publishedEndpoint))
            {
                return true;
            }
            lock (this)
            {
                return endpoint.Equivalent(_endpoint);
            }
        }

        public Endpoint Endpoint()
        {
            if (_publishedEndpoint != null)
            {
                return _publishedEndpoint;
            }
            lock (this)
            {
                return _endpoint;
            }
        }

        public ICollection<Connection> Connections()
        {
            lock (this)
            {
                var connections = new List<Connection>();

                // Only copy connections which have not been destroyed.
                foreach (Connection connection in _connections)
                {
                    if (connection.Active)
                    {
                        connections.Add(connection);
                    }
                }

                return connections;
            }
        }

        // Operations from EventHandler.

        public override bool StartAsync(int operation, AsyncCallback callback, ref bool completedSynchronously)
        {
            if (_state >= State.Closed)
            {
                return false;
            }

            try
            {
                completedSynchronously = _acceptor!.StartAccept(callback, this);
            }
            catch (System.Exception ex)
            {
                _acceptorException = ex;
                completedSynchronously = true;
            }
            return true;
        }

        public override bool FinishAsync(int unused)
        {
            Debug.Assert(_adapter != null);
            if (_acceptorException == null)
            {
                _acceptor!.FinishAccept();
            }
            return _state < State.Closed;
        }

        public override void Message(ref ThreadPoolCurrent current)
        {
            Connection? connection = null;

            var msg = new ThreadPoolMessage(this);

            lock (this)
            {
                if (!msg.StartIOScope(ref current))
                {
                    return;
                }

                try
                {
                    if (_state >= State.Closed)
                    {
                        return;
                    }

                    // Reap closed connections
                    foreach (Connection c in _monitor.SwapReapedConnections())
                    {
                        _connections.Remove(c);
                    }

                    // Now accept a new connection.
                    ITransceiver transceiver;
                    try
                    {
                        if (_acceptorException != null)
                        {
                            throw _acceptorException;
                        }

                        transceiver = _acceptor!.Accept();

                        if (_communicator.TraceLevels.Network >= 2)
                        {
                            _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                                $"trying to accept {_endpoint.Name} connection\n{transceiver}");
                        }
                    }
                    catch (System.Exception ex)
                    {
                        _communicator.Logger.Error($"can't accept more connections:\n{ex}\n{_acceptor}");
                        _acceptorException = null;

                        // Ignore socket exceptions.
                        return;
                    }

                    Debug.Assert(transceiver != null);

                    try
                    {
                        connection = new Connection(_communicator, _monitor, transceiver, null, _endpoint, _adapter);
                    }
                    catch (System.Exception ex)
                    {
                        try
                        {
                            transceiver.Close();
                        }
                        catch (System.Exception)
                        {
                            // Ignore
                        }

                        if (_warn)
                        {
                            _communicator.Logger.Warning($"connection exception:\n{ex}\n{_acceptor}");
                        }
                        return;
                    }

                    _connections.Add(connection);
                }
                finally
                {
                    msg.FinishIOScope(ref current);
                }
            }

            Debug.Assert(connection != null);
            connection.Start(null);
        }

        public override void Finished(ref ThreadPoolCurrent current)
        {
            lock (this)
            {
                Debug.Assert(_state == State.Closed);
                SetState(State.Finished);
            }
        }

        public override string ToString()
        {
            if (_transceiver != null)
            {
                return _transceiver.ToString()!;
            }
            else
            {
                return _acceptor!.ToString();
            }
        }

        public IncomingConnectionFactory(Ice.ObjectAdapter adapter, Endpoint endpoint, Endpoint? publish,
                                         ACMConfig acmConfig)
        {
            _communicator = adapter.Communicator;
            _endpoint = endpoint;
            _publishedEndpoint = publish;
            _adapter = adapter;
            _warn = _communicator.GetPropertyAsBool("Ice.Warn.Connections") ?? false;
            _state = State.Uninitialized;
            _monitor = new FactoryACMMonitor(_communicator, acmConfig);

            if (_communicator.OverrideTimeout != null)
            {
                _endpoint = _endpoint.NewTimeout(_communicator.OverrideTimeout.Value);
            }

            if (_communicator.OverrideCompress != null)
            {
                _endpoint = _endpoint.NewCompressionFlag(_communicator.OverrideCompress.Value);
            }

            try
            {
                _transceiver = _endpoint.GetTransceiver();
                if (_transceiver != null)
                {
                    if (_communicator.TraceLevels.Network >= 2)
                    {
                        _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                            $"attempting to bind to {_endpoint.Name} socket\n{_transceiver}");
                    }
                    _endpoint = _transceiver.Bind();

                    var connection = new Connection(_communicator, null, _transceiver, null, _endpoint, _adapter);
                    connection.StartAndWait();
                    _connections.Add(connection);
                }
                else
                {
                    _acceptor = _endpoint.GetAcceptor(_adapter!.Name);

                    if (_communicator.TraceLevels.Network >= 2)
                    {
                        _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                            $"attempting to bind to {_endpoint.Name} socket {_acceptor}");
                    }
                    _endpoint = _acceptor!.Listen();

                    if (_communicator.TraceLevels.Network >= 1)
                    {
                        _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                            $"listening for {_endpoint.Name} connections\n{_acceptor!.ToDetailedString()}");
                    }

                    _adapter.ThreadPool.Initialize(this);
                }
            }
            catch (System.Exception)
            {
                //
                // Clean up.
                //
                try
                {
                    _transceiver?.Close();
                    _acceptor?.Close();
                }
                catch (System.Exception)
                {
                    // Ignore
                }

                _monitor.Destroy();
                _connections.Clear();

                throw;
            }
        }

        private void SetState(State state)
        {
            // Don't switch twice.
            if (_state == state)
            {
                return;
            }

            switch (state)
            {
                case State.Active:
                    {
                        Debug.Assert(_state == State.Uninitialized);

                        if (_acceptor != null)
                        {
                            if (_communicator.TraceLevels.Network >= 1)
                            {
                                _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                                    $"accepting {_endpoint.Name} connections at {_acceptor}");
                            }
                            _adapter!.ThreadPool.Register(this, SocketOperation.Read);
                        }
                        break;
                    }

                case State.Closed:
                    {
                        if (_acceptor != null)
                        {
                            if (_state == State.Active && _communicator.TraceLevels.Network >= 1)
                            {
                                _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                                    $"stopping to accept {_endpoint.Name} connections at {_acceptor}");
                            }

                            _acceptor!.Close();

                            // Unregister the factory from the thread pool and wait for Finished to be
                            // called. Wait for Finished to be called is necessary for orderely shutdown
                            // of the communicator.
                            _adapter!.ThreadPool.Finish(this);
                        }
                        else
                        {
                            // The factory is not registered with the thread pool, no need to wait for
                            // Finished to be called.
                            state = State.Finished;
                        }

                        foreach (Connection connection in _connections)
                        {
                            connection.Destroy(new ObjectAdapterDeactivatedException(_adapter!.Name));
                        }
                        break;
                    }

                case State.Finished:
                    {
                        Debug.Assert(_state == State.Closed);
                        break;
                    }
            }

            _state = state;
            System.Threading.Monitor.PulseAll(this);
        }

        private enum State
        {
            // The factory is not activated yet.
            Uninitialized,

            // The factory is active, it can accept new connections.
            Active,

            // The factory is closed and waiting to be unregistered from the thread pool.
            Closed,

            // The factory is unregistered from the thread pool.
            Finished
        }

        private readonly IAcceptor? _acceptor;
        private System.Exception? _acceptorException;
        private readonly ObjectAdapter? _adapter;
        private readonly Communicator _communicator;
        private readonly HashSet<Connection> _connections = new HashSet<Connection>();
        private readonly Endpoint _endpoint;
        private readonly FactoryACMMonitor _monitor;
        private readonly Endpoint? _publishedEndpoint;
        private State _state;
        private readonly ITransceiver? _transceiver;
        private readonly bool _warn;
    }
}
