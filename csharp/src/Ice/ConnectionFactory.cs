//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace IceInternal
{
    public class MultiDictionary<K, V> : Dictionary<K, ICollection<V>>
    {
        public void
        Add(K key, V value)
        {
            if (!TryGetValue(key, out ICollection<V> list))
            {
                list = new List<V>();
                Add(key, list);
            }
            list.Add(value);
        }

        public void
        Remove(K key, V value)
        {
            ICollection<V> list = this[key];
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

                foreach (ICollection<Ice.Connection> connections in _connections.Values)
                {
                    foreach (Connection c in connections)
                    {
                        c.Destroy(Ice.Connection.CommunicatorDestroyed);
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
                foreach (ICollection<Ice.Connection> connections in _connections.Values)
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
            Dictionary<IConnector, ICollection<Ice.Connection>> connections;
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
                connections = new Dictionary<IConnector, ICollection<Ice.Connection>>(_connections);
            }

            //
            // Now we wait until the destruction of each connection is finished.
            //
            foreach (ICollection<Ice.Connection> cl in connections.Values)
            {
                foreach (Connection c in cl)
                {
                    c.WaitUntilFinished();
                }
            }

            lock (this)
            {
                // Ensure all the connections are finished and reapable at this point.
                ICollection<Ice.Connection>? cons = _monitor.SwapReapedConnections();
                if (cons != null)
                {
                    int size = 0;
                    foreach (ICollection<Ice.Connection> cl in _connections.Values)
                    {
                        size += cl.Count;
                    }
                    Debug.Assert(cons.Count == size);
                    _connections.Clear();
                    _connectionsByEndpoint.Clear();
                }
                else
                {
                    Debug.Assert(_connections.Count == 0);
                    Debug.Assert(_connectionsByEndpoint.Count == 0);
                }
            }

            //
            // Must be destroyed outside the synchronization since this might block waiting for
            // a timer task to execute.
            //
            _monitor.Destroy();
        }

        public void Create(Endpoint[] endpts, bool hasMore, Ice.EndpointSelectionType selType,
                           ICreateConnectionCallback callback)
        {
            Debug.Assert(endpts.Length > 0);

            //
            // Apply the overrides.
            //
            List<Endpoint> endpoints = ApplyOverrides(endpts);

            //
            // Try to find a connection to one of the given endpoints.
            //
            try
            {
                Ice.Connection? connection = FindConnection(endpoints, out bool compress);
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
            Endpoint[] endpoints = routerInfo.GetClientEndpoints(); // Must be called outside the synchronization

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
                DefaultsAndOverrides defaultsAndOverrides = _communicator.DefaultsAndOverrides;
                for (int i = 0; i < endpoints.Length; i++)
                {
                    Endpoint endpoint = endpoints[i];

                    //
                    // Modify endpoints with overrides.
                    //
                    if (defaultsAndOverrides.OverrideTimeout)
                    {
                        endpoint = endpoint.Timeout(defaultsAndOverrides.OverrideTimeoutValue);
                    }

                    //
                    // The Ice.ConnectionI object does not take the compression flag of
                    // endpoints into account, but instead gets the information
                    // about whether messages should be compressed or not from
                    // other sources. In order to allow connection sharing for
                    // endpoints that differ in the value of the compression flag
                    // only, we always set the compression flag to false here in
                    // this connection factory.
                    //
                    endpoint = endpoint.Compress(false);

                    foreach (ICollection<Ice.Connection> connections in _connections.Values)
                    {
                        foreach (Connection connection in connections)
                        {
                            if (connection.Endpoint.Equals(endpoint))
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

                foreach (ICollection<Ice.Connection> connectionList in _connections.Values)
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

        private List<Endpoint> ApplyOverrides(Endpoint[] endpts)
        {
            DefaultsAndOverrides defaultsAndOverrides = _communicator.DefaultsAndOverrides;
            var endpoints = new List<Endpoint>();
            for (int i = 0; i < endpts.Length; i++)
            {
                //
                // Modify endpoints with overrides.
                //
                if (defaultsAndOverrides.OverrideTimeout)
                {
                    endpoints.Add(endpts[i].Timeout(defaultsAndOverrides.OverrideTimeoutValue));
                }
                else
                {
                    endpoints.Add(endpts[i]);
                }
            }

            return endpoints;
        }

        private Ice.Connection? FindConnection(List<Endpoint> endpoints, out bool compress)
        {
            lock (this)
            {
                if (_destroyed)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                DefaultsAndOverrides defaultsAndOverrides = _communicator.DefaultsAndOverrides;
                Debug.Assert(endpoints.Count > 0);

                foreach (Endpoint endpoint in endpoints)
                {
                    if (!_connectionsByEndpoint.TryGetValue(endpoint, out ICollection<Connection> connectionList))
                    {
                        continue;
                    }

                    foreach (Connection connection in connectionList)
                    {
                        if (connection.ActiveOrHolding) // Don't return destroyed or unvalidated connections
                        {
                            if (defaultsAndOverrides.OverrideCompress)
                            {
                                compress = defaultsAndOverrides.OverrideCompressValue;
                            }
                            else
                            {
                                compress = endpoint.Compress();
                            }
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
        private Ice.Connection? FindConnection(List<ConnectorInfo> connectors, out bool compress)
        {
            DefaultsAndOverrides defaultsAndOverrides = _communicator.DefaultsAndOverrides;
            foreach (ConnectorInfo ci in connectors)
            {
                if (_pending.ContainsKey(ci.Connector))
                {
                    continue;
                }

                if (!_connections.TryGetValue(ci.Connector, out ICollection<Connection> connectionList))
                {
                    continue;
                }

                foreach (Ice.Connection connection in connectionList)
                {
                    if (connection.ActiveOrHolding) // Don't return destroyed or un-validated connections
                    {
                        if (defaultsAndOverrides.OverrideCompress)
                        {
                            compress = defaultsAndOverrides.OverrideCompressValue;
                        }
                        else
                        {
                            compress = ci.Endpoint.Compress();
                        }
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

        private Ice.Connection? GetConnection(List<ConnectorInfo> connectors, ConnectCallback cb, out bool compress)
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
                ICollection<Ice.Connection>? cons = _monitor.SwapReapedConnections();
                if (cons != null)
                {
                    foreach (Connection c in cons)
                    {
                        _connections.Remove(c.Connector, c);
                        _connectionsByEndpoint.Remove((Endpoint)c.Endpoint, c);
                        _connectionsByEndpoint.Remove(((Endpoint)c.Endpoint).Compress(true), c);
                    }
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
                    Ice.Connection? connection = FindConnection(connectors, out compress);
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

        private Ice.Connection CreateConnection(ITransceiver transceiver, ConnectorInfo ci)
        {
            lock (this)
            {
                Debug.Assert(_pending.ContainsKey(ci.Connector) && transceiver != null);

                //
                // Create and add the connection to the connection map. Adding the connection to the map
                // is necessary to support the interruption of the connection initialization and validation
                // in case the communicator is destroyed.
                //
                Ice.Connection connection;
                try
                {
                    if (_destroyed)
                    {
                        throw new Ice.CommunicatorDestroyedException();
                    }

                    connection = new Ice.Connection(_communicator, _monitor, transceiver, ci.Connector,
                                                    ci.Endpoint.Compress(false), null);
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
                _connectionsByEndpoint.Add((Endpoint)connection.Endpoint, connection);
                _connectionsByEndpoint.Add(((Endpoint)connection.Endpoint).Compress(true), connection);
                return connection;
            }
        }

        private void FinishGetConnection(List<ConnectorInfo> connectors,
                                         ConnectorInfo ci,
                                         Ice.Connection connection,
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
                    if (_pending.TryGetValue(c.Connector, out HashSet<ConnectCallback> s))
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

            bool compress;
            DefaultsAndOverrides defaultsAndOverrides = _communicator.DefaultsAndOverrides;
            if (defaultsAndOverrides.OverrideCompress)
            {
                compress = defaultsAndOverrides.OverrideCompressValue;
            }
            else
            {
                compress = ci.Endpoint.Compress();
            }

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
                    if (_pending.TryGetValue(c.Connector, out HashSet<ConnectCallback> s))
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
                var s = new StringBuilder();
                s.Append("connection to endpoint failed");
                if (ex is Ice.CommunicatorDestroyedException)
                {
                    s.Append("\n");
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
                _communicator.Logger.Trace(traceLevels.NetworkCat, s.ToString());
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
                if (_pending.TryGetValue(ci.Connector, out HashSet<ConnectCallback> cbs))
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
                if (_pending.TryGetValue(ci.Connector, out HashSet<ConnectCallback> cbs))
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
                var s = new StringBuilder();
                s.Append("couldn't resolve endpoint host");
                if (ex is Ice.CommunicatorDestroyedException)
                {
                    s.Append("\n");
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
                _communicator.Logger.Trace(traceLevels.NetworkCat, s.ToString());
            }
        }

        private class ConnectorInfo
        {
            internal ConnectorInfo(IConnector c, Endpoint e)
            {
                Connector = c;
                Endpoint = e;
            }

            public override bool Equals(object obj)
            {
                var r = (ConnectorInfo)obj;
                return Connector.Equals(r.Connector);
            }

            public override int GetHashCode() => Connector.GetHashCode();

            public IConnector Connector;
            public Endpoint Endpoint;
        }

        private class ConnectCallback : Ice.Connection.IStartCallback, IEndpointConnectors
        {
            internal ConnectCallback(OutgoingConnectionFactory f, List<Endpoint> endpoints, bool more,
                                     ICreateConnectionCallback cb, Ice.EndpointSelectionType selType)
            {
                _factory = f;
                _endpoints = endpoints;
                _hasMore = more;
                _callback = cb;
                _selType = selType;
                _endpointsIter = 0;
            }

            //
            // Methods from ConnectionI.StartCallback
            //
            public void ConnectionStartCompleted(Ice.Connection connection)
            {
                if (_observer != null)
                {
                    _observer.Detach();
                }
                connection.Activate();
                Debug.Assert(_current != null);
                _factory.FinishGetConnection(_connectors, _current, connection, this);
            }

            public void ConnectionStartFailed(Ice.Connection connection, System.Exception ex)
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

                if (_endpointsIter < _endpoints.Count)
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
                _factory.HandleException(ex, _hasMore || _endpointsIter < _endpoints.Count);
                if (_endpointsIter < _endpoints.Count)
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

            public void SetConnection(Ice.Connection connection, bool compress)
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
                    Debug.Assert(_endpointsIter < _endpoints.Count);
                    _currentEndpoint = _endpoints[_endpointsIter++];
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
                    Ice.Connection? connection = _factory.GetConnection(_connectors, this, out bool compress);
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
                                                                                _current.Connector.ToString());
                            if (_observer != null)
                            {
                                _observer.Attach();
                            }
                        }

                        if (_factory._communicator.TraceLevels.Network >= 2)
                        {
                            var s = new StringBuilder("trying to establish ");
                            s.Append(_current.Endpoint.Transport());
                            s.Append(" connection to ");
                            s.Append(_current.Connector.ToString());
                            _factory._communicator.Logger.Trace(
                                                _factory._communicator.TraceLevels.NetworkCat, s.ToString());
                        }

                        Ice.Connection connection = _factory.CreateConnection(_current.Connector.Connect(), _current);
                        connection.Start(this);
                    }
                    catch (System.Exception ex)
                    {
                        if (_factory._communicator.TraceLevels.Network >= 2)
                        {
                            Debug.Assert(_current != null);
                            var s = new StringBuilder("failed to establish ");
                            s.Append(_current.Endpoint.Transport());
                            s.Append(" connection to ");
                            s.Append(_current.Connector.ToString());
                            s.Append("\n");
                            s.Append(ex);
                            _factory._communicator.Logger.Trace(
                                                _factory._communicator.TraceLevels.NetworkCat, s.ToString());
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
                    _observer.Failed(ex.GetType().FullName);
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
            private readonly List<Endpoint> _endpoints;
            private readonly Ice.EndpointSelectionType _selType;
            private int _endpointsIter;
            private Endpoint? _currentEndpoint;
            private readonly List<ConnectorInfo> _connectors = new List<ConnectorInfo>();
            private int _iter;
            private ConnectorInfo? _current;
            private Ice.Instrumentation.IObserver? _observer;
        }

        private readonly Ice.Communicator _communicator;
        private readonly FactoryACMMonitor _monitor;
        private bool _destroyed;

        private readonly MultiDictionary<IConnector, Ice.Connection> _connections =
            new MultiDictionary<IConnector, Ice.Connection>();
        private readonly MultiDictionary<Endpoint, Ice.Connection> _connectionsByEndpoint =
            new MultiDictionary<Endpoint, Ice.Connection>();
        private readonly Dictionary<IConnector, HashSet<ConnectCallback>> _pending =
            new Dictionary<IConnector, HashSet<ConnectCallback>>();
        private int _pendingConnectCount;
    }

    public sealed class IncomingConnectionFactory : EventHandler, Ice.Connection.IStartCallback
    {
        private class StartAcceptorTask : ITimerTask
        {
            public StartAcceptorTask(IncomingConnectionFactory factory) => _factory = factory;

            public void RunTimerTask() => _factory.StartAcceptor();

            private readonly IncomingConnectionFactory _factory;
        }

        public void StartAcceptor()
        {
            lock (this)
            {
                if (_state >= StateClosed || _acceptorStarted)
                {
                    return;
                }

                try
                {
                    CreateAcceptor();
                }
                catch (System.Exception ex)
                {
                    Debug.Assert(_acceptor != null);
                    _communicator.Logger.Error($"acceptor creation failed:\n{ex}\n{_acceptor}");
                    _communicator.Timer().Schedule(new StartAcceptorTask(this), 1000);
                }
            }
        }

        public void Activate()
        {
            lock (this)
            {
                SetState(StateActive);
            }
        }

        public void Hold()
        {
            lock (this)
            {
                SetState(StateHolding);
            }
        }

        public void Destroy()
        {
            lock (this)
            {
                SetState(StateClosed);
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

        public void WaitUntilHolding()
        {
            ICollection<Ice.Connection> connections;

            lock (this)
            {
                //
                // First we wait until the connection factory itself is in
                // holding state.
                //
                while (_state < StateHolding)
                {
                    System.Threading.Monitor.Wait(this);
                }

                //
                // We want to wait until all connections are in holding state
                // outside the thread synchronization.
                //
                connections = new List<Ice.Connection>(_connections);
            }

            //
            // Now we wait until each connection is in holding state.
            //
            foreach (Connection connection in connections)
            {
                connection.WaitUntilHolding();
            }
        }

        public void WaitUntilFinished()
        {
            ICollection<Ice.Connection> connections;

            lock (this)
            {
                //
                // First we wait until the factory is destroyed. If we are using
                // an acceptor, we also wait for it to be closed.
                //
                while (_state != StateFinished)
                {
                    System.Threading.Monitor.Wait(this);
                }

                //
                // Clear the OA. See bug 1673 for the details of why this is necessary.
                //
                _adapter = null;

                //
                // We want to wait until all connections are finished outside the
                // thread synchronization.
                //
                connections = new List<Ice.Connection>(_connections);
            }

            foreach (Connection connection in connections)
            {
                connection.WaitUntilFinished();
            }

            lock (this)
            {
                if (_transceiver != null)
                {
                    Debug.Assert(_connections.Count <= 1); // The connection isn't monitored or reaped.
                }
                else
                {
                    // Ensure all the connections are finished and reapable at this point.
                    ICollection<Ice.Connection>? cons = _monitor.SwapReapedConnections();
                    Debug.Assert((cons == null ? 0 : cons.Count) == _connections.Count);
                    if (cons != null)
                    {
                        cons.Clear();
                    }
                }
                _connections.Clear();
            }

            //
            // Must be destroyed outside the synchronization since this might block waiting for
            // a timer task to execute.
            //
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

        public ICollection<Ice.Connection> Connections()
        {
            lock (this)
            {
                var connections = new List<Ice.Connection>();

                //
                // Only copy connections which have not been destroyed.
                //
                foreach (Connection connection in _connections)
                {
                    if (connection.ActiveOrHolding)
                    {
                        connections.Add(connection);
                    }
                }

                return connections;
            }
        }

        //
        // Operations from EventHandler.
        //
        public override bool StartAsync(int operation, AsyncCallback callback, ref bool completedSynchronously)
        {
            if (_state >= StateClosed)
            {
                return false;
            }

            Debug.Assert(_acceptor != null);
            try
            {
                completedSynchronously = _acceptor.StartAccept(callback, this);
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
            Debug.Assert(_acceptor != null);
            Debug.Assert(_adapter != null);
            try
            {
                if (_acceptorException != null)
                {
                    throw _acceptorException;
                }
                _acceptor.FinishAccept();
            }
            catch (System.Exception ex)
            {
                _acceptorException = null;
                _communicator.Logger.Error($"couldn't accept connection:\n{ex}\n{_acceptor}");
                if (_acceptorStarted)
                {
                    _acceptorStarted = false;
                    _adapter.ThreadPool.Finish(this);
                    CloseAcceptor();
                }
            }
            return _state < StateClosed;
        }

        public override void Message(ref ThreadPoolCurrent current)
        {
            Ice.Connection? connection = null;

            var msg = new ThreadPoolMessage(this);

            lock (this)
            {
                if (!msg.StartIOScope(ref current))
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

                    //
                    // Reap closed connections
                    //
                    ICollection<Ice.Connection>? cons = _monitor.SwapReapedConnections();
                    if (cons != null)
                    {
                        foreach (Connection c in cons)
                        {
                            _connections.Remove(c);
                        }
                    }

                    if (!_acceptorStarted)
                    {
                        return;
                    }

                    //
                    // Now accept a new connection.
                    //
                    ITransceiver transceiver;
                    try
                    {
                        transceiver = _acceptor!.Accept();

                        if (_communicator.TraceLevels.Network >= 2)
                        {
                            var s = new StringBuilder("trying to accept ");
                            s.Append(_endpoint.Transport());
                            s.Append(" connection\n");
                            s.Append(transceiver.ToString());
                            _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat, s.ToString());
                        }
                    }
                    catch (Ice.SocketException ex)
                    {
                        if (Network.NoMoreFds(ex.InnerException))
                        {
                            _communicator.Logger.Error(
                                $"can't accept more connections:\n{ex}\n{_acceptor}");
                            Debug.Assert(_acceptorStarted);
                            _acceptorStarted = false;
                            _adapter!.ThreadPool.Finish(this);
                            CloseAcceptor();
                        }

                        // Ignore socket exceptions.
                        return;
                    }
                    catch (System.Exception ex)
                    {
                        // Warn about other Ice local exceptions.
                        if (_warn)
                        {
                            Warning(ex);
                        }
                        return;
                    }

                    Debug.Assert(transceiver != null);

                    try
                    {
                        connection = new Ice.Connection(_communicator, _monitor, transceiver, null, _endpoint, _adapter);
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
                            Warning(ex);
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
            connection.Start(this);
        }

        public override void Finished(ref ThreadPoolCurrent current)
        {
            lock (this)
            {
                if (_state < StateClosed)
                {
                    //
                    // If the acceptor hasn't been explicitly stopped (which is the case if the acceptor got closed
                    // because of an unexpected error), try to restart the acceptor in 1 second.
                    //
                    _communicator.Timer().Schedule(new StartAcceptorTask(this), 1000);
                    return;
                }

                Debug.Assert(_state >= StateClosed);
                SetState(StateFinished);
            }
        }

        public override string ToString()
        {
            if (_transceiver != null)
            {
                return _transceiver.ToString();
            }
            else
            {
                Debug.Assert(_acceptor != null);
                return _acceptor.ToString();
            }
        }

        //
        // Operations from ConnectionI.StartCallback
        //
        public void ConnectionStartCompleted(Ice.Connection connection)
        {
            lock (this)
            {
                //
                // Initially, connections are in the holding state. If the factory is active
                // we activate the connection.
                //
                if (_state == StateActive)
                {
                    connection.Activate();
                }
            }
        }

        public void ConnectionStartFailed(Ice.Connection connection, System.Exception ex)
        {
            lock (this)
            {
                if (_state >= StateClosed)
                {
                    return;
                }

                //
                // Do not warn about connection exceptions here. The connection is not yet validated.
                //
            }
        }

        public IncomingConnectionFactory(Ice.ObjectAdapter adapter, Endpoint endpoint, Endpoint? publish,
                                         ACMConfig acmConfig)
        {
            _communicator = adapter.Communicator;
            _endpoint = endpoint;
            _publishedEndpoint = publish;
            _adapter = adapter;
            _warn = _communicator.GetPropertyAsInt("Ice.Warn.Connections") > 0;
            _connections = new HashSet<Ice.Connection>();
            _state = StateHolding;
            _acceptorStarted = false;
            _monitor = new FactoryACMMonitor(_communicator, acmConfig);

            DefaultsAndOverrides defaultsAndOverrides = _communicator.DefaultsAndOverrides;
            if (defaultsAndOverrides.OverrideTimeout)
            {
                _endpoint = _endpoint.Timeout(defaultsAndOverrides.OverrideTimeoutValue);
            }

            if (defaultsAndOverrides.OverrideCompress)
            {
                _endpoint = _endpoint.Compress(defaultsAndOverrides.OverrideCompressValue);
            }

            try
            {
                _transceiver = _endpoint.Transceiver();
                if (_transceiver != null)
                {
                    if (_communicator.TraceLevels.Network >= 2)
                    {
                        var s = new StringBuilder("attempting to bind to ");
                        s.Append(_endpoint.Transport());
                        s.Append(" socket\n");
                        s.Append(_transceiver.ToString());
                        _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat, s.ToString());
                    }
                    _endpoint = _transceiver.Bind();

                    var connection = new Ice.Connection(_communicator, null, _transceiver, null, _endpoint, _adapter);
                    connection.StartAndWait();
                    _connections.Add(connection);
                }
                else
                {
                    CreateAcceptor();
                }
            }
            catch (System.Exception)
            {
                //
                // Clean up.
                //
                if (_transceiver != null)
                {
                    try
                    {
                        _transceiver.Close();
                    }
                    catch (System.Exception)
                    {
                        // Ignore
                    }
                }

                _state = StateFinished;
                _monitor.Destroy();
                _connections.Clear();

                throw;
            }
        }

        private const int StateActive = 0;
        private const int StateHolding = 1;
        private const int StateClosed = 2;
        private const int StateFinished = 3;

        private void SetState(int state)
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
                        if (_acceptor != null)
                        {
                            if (_communicator.TraceLevels.Network >= 1)
                            {
                                var s = new StringBuilder("accepting ");
                                s.Append(_endpoint.Transport());
                                s.Append(" connections at ");
                                s.Append(_acceptor.ToString());
                                _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat, s.ToString());
                            }
                            _adapter!.ThreadPool.Register(this, SocketOperation.Read);
                        }

                        foreach (Connection connection in _connections)
                        {
                            connection.Activate();
                        }
                        break;
                    }

                case StateHolding:
                    {
                        if (_state != StateActive) // Can only switch from active to holding.
                        {
                            return;
                        }
                        if (_acceptor != null)
                        {
                            if (_communicator.TraceLevels.Network >= 1)
                            {
                                _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                                                           $"holding {_endpoint.Transport()} connections at {_acceptor}");
                            }
                            _adapter!.ThreadPool.Unregister(this, SocketOperation.Read);
                        }

                        foreach (Connection connection in _connections)
                        {
                            connection.Hold();
                        }
                        break;
                    }

                case StateClosed:
                    {
                        if (_acceptorStarted)
                        {
                            _acceptorStarted = false;
                            _adapter!.ThreadPool.Finish(this);
                            CloseAcceptor();
                        }
                        else
                        {
                            state = StateFinished;
                        }

                        foreach (Connection connection in _connections)
                        {
                            connection.Destroy(Ice.Connection.ObjectAdapterDeactivated);
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
            System.Threading.Monitor.PulseAll(this);
        }

        private void CreateAcceptor()
        {
            try
            {
                Debug.Assert(!_acceptorStarted);
                _acceptor = _endpoint.Acceptor(_adapter!.Name);
                Debug.Assert(_acceptor != null);

                if (_communicator.TraceLevels.Network >= 2)
                {
                    _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                        $"attempting to bind to {_endpoint.Transport()} socket {_acceptor}");
                }
                _endpoint = _acceptor.Listen();

                if (_communicator.TraceLevels.Network >= 1)
                {
                    _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                        $"listening for {_endpoint.Transport()} connections\n{_acceptor.ToDetailedString()}");
                }

                _adapter.ThreadPool.Initialize(this);

                if (_state == StateActive)
                {
                    _adapter.ThreadPool.Register(this, SocketOperation.Read);
                }

                _acceptorStarted = true;
            }
            catch (System.Exception)
            {
                if (_acceptor != null)
                {
                    _acceptor.Close();
                }
                throw;
            }
        }

        private void CloseAcceptor()
        {
            Debug.Assert(_acceptor != null);

            if (_communicator.TraceLevels.Network >= 1)
            {
                _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                    $"stopping to accept {_endpoint.Transport()} connections at {_acceptor}");
            }

            Debug.Assert(!_acceptorStarted);
            _acceptor.Close();
        }

        private void Warning(System.Exception ex) =>
            _communicator.Logger.Warning($"connection exception:\n{ex}\n{_acceptor}");

        private readonly Ice.Communicator _communicator;
        private readonly FactoryACMMonitor _monitor;

        private IAcceptor? _acceptor;
        private readonly ITransceiver? _transceiver;
        private Endpoint _endpoint;
        private readonly Endpoint? _publishedEndpoint;

        private Ice.ObjectAdapter? _adapter;

        private readonly bool _warn;

        private readonly HashSet<Ice.Connection> _connections;

        private int _state;
        private bool _acceptorStarted;
        private System.Exception? _acceptorException;
    }

}
