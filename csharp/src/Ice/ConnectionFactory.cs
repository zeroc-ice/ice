//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice
{
    internal sealed class OutgoingConnectionFactory
    {
        private readonly Communicator _communicator;
        private readonly MultiDictionary<IConnector, Connection> _connectionsByConnector =
            new MultiDictionary<IConnector, Connection>();
        private readonly MultiDictionary<Endpoint, Connection> _connectionsByEndpoint =
            new MultiDictionary<Endpoint, Connection>();
        private Task? _destroyTask = null;
        private SemaphoreSlim? _destroyTaskSemaphore = null;
        private readonly FactoryACMMonitor _monitor;
        private readonly object _mutex = new object();
        private readonly Dictionary<IConnector, Task<Connection>> _pending =
            new Dictionary<IConnector, Task<Connection>>();

        internal async ValueTask<Connection> CreateAsync(
            IReadOnlyList<Endpoint> endpoints,
            bool hasMore,
            EndpointSelectionType selType)
        {
            Debug.Assert(endpoints.Count > 0);

            lock (_mutex)
            {
                if (_destroyTask != null)
                {
                    throw new CommunicatorDestroyedException();
                }

                // Try to find a connection to one of the given endpoints. Ignore the endpoint compression flag to
                // lookup for the connection.
                foreach (Endpoint endpoint in endpoints)
                {
                    if (_connectionsByEndpoint.TryGetValue(endpoint, out ICollection<Connection>? connectionList))
                    {
                        foreach (Connection connection in connectionList)
                        {
                            if (connection.Active) // Don't return destroyed or un-validated connections
                            {
                                return connection;
                            }
                        }
                    }
                }
            }

            // For each endpoint, obtain the set of connectors. This might block if DNS lookups are required to
            // resolve an endpoint hostname into connector addresses.
            var connectors = new List<(IConnector, Endpoint)>();
            foreach (Endpoint endpoint in endpoints)
            {
                try
                {
                    foreach (IConnector connector in await endpoint.ConnectorsAsync(selType).ConfigureAwait(false))
                    {
                        connectors.Add((connector, endpoint));
                    }
                }
                catch (CommunicatorDestroyedException)
                {
                    throw; // No need to continue
                }
                catch (Exception ex)
                {
                    bool last = endpoint == endpoints[endpoints.Count - 1];

                    TraceLevels traceLevels = _communicator.TraceLevels;
                    if (traceLevels.Network >= 2)
                    {
                        _communicator.Logger.Trace(traceLevels.NetworkCat, last ?
                            $"couldn't resolve endpoint host and no more endpoints to try\n{ex}" :
                            $"couldn't resolve endpoint host, trying next endpoint\n{ex}");
                    }

                    if (connectors.Count == 0 && last)
                    {
                        // If this was the last endpoint and we didn't manage to get a single connector, we're done.
                        throw;
                    }
                }
            }

            lock (_mutex)
            {
                if (_destroyTask != null)
                {
                    throw new CommunicatorDestroyedException();
                }

                // Reap closed connections
                foreach (Connection connection in _monitor.SwapReapedConnections())
                {
                    _connectionsByConnector.Remove(connection.Connector, connection);
                    foreach (Endpoint endpoint in connection.Endpoints)
                    {
                        _connectionsByEndpoint.Remove(endpoint, connection);
                    }
                }

                // Try to find a connection matching one of the connector.
                foreach ((IConnector connector, Endpoint _) in connectors)
                {
                    if (!_pending.ContainsKey(connector) &&
                        _connectionsByConnector.TryGetValue(connector, out ICollection<Connection>? connectionList))
                    {
                        foreach (Connection connection in connectionList)
                        {
                            if (connection.Active) // Don't return destroyed or un-validated connections
                            {
                                return connection;
                            }
                        }
                    }
                }
            }

            // Wait for connection establishment to one or some of the connectors to complete.
            return await WaitForConnectOrConnectAsync(connectors, hasMore).ConfigureAwait(false);
        }

        public Task DestroyAsync()
        {
            lock (_mutex)
            {
                // If there are pending connects, we setup the destroy task semaphore to wait for the pending counts
                // to complete.
                if (_pending.Count > 0)
                {
                    _destroyTaskSemaphore ??= new SemaphoreSlim(0);
                }
                _destroyTask ??= PerformDestroyAsync();
            }
            return _destroyTask;
        }

        public void RemoveAdapter(ObjectAdapter adapter)
        {
            lock (_mutex)
            {
                if (_destroyTask != null)
                {
                    return;
                }

                foreach (ICollection<Connection> connectionList in _connectionsByConnector.Values)
                {
                    foreach (Connection connection in connectionList)
                    {
                        connection.ClearAdapter(adapter);
                    }
                }
            }
        }

        public void SetRouterInfo(RouterInfo routerInfo)
        {
            ObjectAdapter? adapter = routerInfo.Adapter;
            IReadOnlyList<Endpoint> endpoints;
            try
            {
                ValueTask<IReadOnlyList<Endpoint>> task = routerInfo.GetClientEndpointsAsync();
                endpoints = task.IsCompleted ? task.Result : task.AsTask().Result;
            }
            catch (AggregateException ex)
            {
                Debug.Assert(ex.InnerException != null);
                throw ExceptionUtil.Throw(ex.InnerException);
            }

            lock (_mutex)
            {
                if (_destroyTask != null)
                {
                    throw new CommunicatorDestroyedException();
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

                    foreach (ICollection<Connection> connections in _connectionsByConnector.Values)
                    {
                        foreach (Connection connection in connections)
                        {
                            if (connection.Endpoint == endpoint)
                            {
                                connection.Adapter = adapter;
                            }
                        }
                    }
                }
            }
        }
        public void UpdateConnectionObservers()
        {
            lock (_mutex)
            {
                foreach (ICollection<Connection> connections in _connectionsByConnector.Values)
                {
                    foreach (Connection c in connections)
                    {
                        c.UpdateObserver();
                    }
                }
            }
        }

        internal OutgoingConnectionFactory(Communicator communicator)
        {
            _communicator = communicator;
            _monitor = new FactoryACMMonitor(communicator, communicator.ClientACM);
        }

        private async Task<Connection> ConnectAsync(
            IReadOnlyList<(IConnector Connector, Endpoint Endpoint)> connectors,
            bool hasMore)
        {
            Debug.Assert(connectors.Count > 0);

            foreach ((IConnector connector, Endpoint endpoint) in connectors)
            {
                IObserver? observer = _communicator.Observer?.GetConnectionEstablishmentObserver(endpoint,
                                                                                                 connector.ToString()!);
                try
                {
                    observer?.Attach();

                    if (_communicator.TraceLevels.Network >= 2)
                    {
                        _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                            $"trying to establish {endpoint.TransportName} connection to {connector}");
                    }

                    Connection connection;
                    lock (_mutex)
                    {
                        if (_destroyTask != null)
                        {
                            throw new CommunicatorDestroyedException();
                        }

                        connection = connector.Connect().CreateConnection(endpoint,
                                                                          _monitor,
                                                                          connector,
                                                                          null);

                        _connectionsByConnector.Add(connector, connection);
                        _connectionsByEndpoint.Add(endpoint, connection);
                    }

                    await connection.StartAsync().ConfigureAwait(false);
                    return connection;
                }
                catch (CommunicatorDestroyedException ex)
                {
                    observer?.Failed(ex.GetType().FullName ?? "System.Exception");
                    throw; // No need to continue
                }
                catch (Exception ex)
                {
                    observer?.Failed(ex.GetType().FullName ?? "System.Exception");

                    bool last = hasMore || connector == connectors[connectors.Count - 1].Connector;

                    TraceLevels traceLevels = _communicator.TraceLevels;
                    if (traceLevels.Network >= 2)
                    {
                        _communicator.Logger.Trace(traceLevels.NetworkCat,
                            $"failed to establish {endpoint.TransportName} connection to {connector} " +
                            (last ? $"and no more endpoints to try\n{ex}" : $"trying next endpoint\n{ex}"));
                    }

                    if (last)
                    {
                        // If it's the last connector to try and we couldn't establish the connection, we're done.
                        throw;
                    }
                }
                finally
                {
                    lock (_mutex)
                    {
                        foreach ((IConnector connectorToRemove, Endpoint _) in connectors)
                        {
                            _pending.Remove(connectorToRemove);
                        }

                        // If destroy is waiting for pending connection establishments and we're done with connection
                        // establishments, we release the semaphore to allow destroy to continue.
                        if (_pending.Count == 0 && _destroyTaskSemaphore != null)
                        {
                            _destroyTaskSemaphore.Release();
                        }
                    }
                    observer?.Detach();
                }
            }

            // The loop either raised an exception on the last connector or returned if the connection establishment
            // succeeded.
            Debug.Assert(false);
            return null!;
        }

        private async Task PerformDestroyAsync()
        {
            // Wait for pending connection establishments to complete.
            if (_destroyTaskSemaphore != null)
            {
                await _destroyTaskSemaphore.WaitAsync().ConfigureAwait(false);
            }

            // Wait for connections to be closed.
            IEnumerable<Task> tasks =
                _connectionsByConnector.Values.SelectMany(connections => connections).Select(connection =>
                    connection.GracefulCloseAsync(new CommunicatorDestroyedException()));
            await Task.WhenAll(tasks).ConfigureAwait(false);

#if DEBUG
            // Ensure all the connections are finished and reapable at this point.
            foreach (Connection connection in _monitor.SwapReapedConnections())
            {
                _connectionsByConnector.Remove(connection.Connector, connection);
                foreach (Endpoint endpoint in connection.Endpoints)
                {
                    _connectionsByEndpoint.Remove(endpoint, connection);
                }
            }
            Debug.Assert(_connectionsByConnector.Count == 0);
            Debug.Assert(_connectionsByEndpoint.Count == 0);
#endif

            _monitor.Destroy();
        }

        private async Task<Connection> WaitForConnectOrConnectAsync(
            IReadOnlyList<(IConnector, Endpoint)> connectors,
            bool hasMore)
        {
            var tried = new HashSet<(IConnector, Endpoint)>();
            while (true)
            {
                var connectTasks = new List<Task<Connection>>();
                lock (_mutex)
                {
                    // Search for pending connects for the set of connectors which weren't already tried.
                    foreach ((IConnector connector, Endpoint endpoint) in connectors.Where(t => !tried.Contains(t)))
                    {
                        if (_pending.TryGetValue(connector, out Task<Connection>? task))
                        {
                            connectTasks.Add(task);
                            tried.Add((connector, endpoint));
                        }
                    }

                    // We didn't find pending connects for the remaining connectors so we can try to establish
                    // a connection to them.
                    if (connectTasks.Count == 0)
                    {
                        Task<Connection> connectTask = ConnectAsync(connectors, hasMore);
                        if (connectTask.IsCompleted)
                        {
                            try
                            {
                                return connectTask.Result;
                            }
                            catch (AggregateException ex)
                            {
                                Debug.Assert(ex.InnerException != null);
                                throw ExceptionUtil.Throw(ex.InnerException);
                            }
                        }

                        foreach ((IConnector connector, Endpoint endpoint) in connectors)
                        {
                            // Use TryAdd in case there are duplicate (connector, endpoint) pairs.
                            _pending.TryAdd(connector, connectTask);
                            tried.Add((connector, endpoint));
                        }
                        connectTasks.Add(connectTask);
                    }
                }

                // Wait for the first successful connection establishment
                Task<Connection> completedTask;
                do
                {
                    completedTask = await Task.WhenAny(connectTasks).ConfigureAwait(false);
                    if (completedTask.IsCompletedSuccessfully)
                    {
                        Connection connection = completedTask.Result;
                        foreach ((IConnector connector, Endpoint endpoint) in connectors)
                        {
                            // If the connection was established for another endpoint but to the same connector,
                            // we ensure to also associate the connection with this endpoint.
                            if (connection.Connector == connector && !connection.Endpoints.Contains(endpoint))
                            {
                                connection.Endpoints.Add(endpoint);
                                _connectionsByEndpoint.Add(endpoint, connection);
                            }
                        }
                        return await completedTask.ConfigureAwait(false);
                    }
                    connectTasks.Remove(completedTask);
                }
                while (connectTasks.Count > 0);

                // If there are no more connectors to try, we failed to establish a connection and we raise the
                // failure.
                if (tried.Count == connectors.Count)
                {
                    return await completedTask.ConfigureAwait(false);
                }
            }
        }

        private class MultiDictionary<TKey, TValue> : Dictionary<TKey, ICollection<TValue>> where TKey : notnull
        {
            public void Add(TKey key, TValue value)
            {
                if (!TryGetValue(key, out ICollection<TValue>? list))
                {
                    list = new List<TValue>();
                    Add(key, list);
                }
                list.Add(value);
            }

            public void Remove(TKey key, TValue value)
            {
                ICollection<TValue> list = this[key];
                list.Remove(value);
                if (list.Count == 0)
                {
                    Remove(key);
                }
            }
        }
    }

    internal sealed class IncomingConnectionFactory
    {
        private readonly IAcceptor? _acceptor;
        private readonly ObjectAdapter _adapter;
        private readonly Communicator _communicator;
        private readonly HashSet<Connection> _connections = new HashSet<Connection>();
        private Task? _destroyTask = null;
        private readonly Endpoint _endpoint;
        private readonly FactoryACMMonitor _monitor;
        private readonly object _mutex = new object();
        private readonly Endpoint? _publishedEndpoint;
        private readonly ITransceiver? _transceiver;
        private readonly bool _warn;

        public IncomingConnectionFactory(
            ObjectAdapter adapter,
            Endpoint endpoint,
            Endpoint? publish,
            ACMConfig acmConfig)
        {
            _communicator = adapter.Communicator;
            _endpoint = endpoint;
            _publishedEndpoint = publish;
            _adapter = adapter;
            _warn = _communicator.GetPropertyAsBool("Ice.Warn.Connections") ?? false;
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
                            $"attempting to bind to {_endpoint.TransportName} socket\n{_transceiver}");
                    }
                    _endpoint = _transceiver.Bind();

                    Connection connection = _transceiver.CreateConnection(_endpoint, null, null, _adapter);
                    _ = connection.StartAsync();
                    _connections.Add(connection);
                }
                else
                {
                    _acceptor = _endpoint.GetAcceptor(_adapter!.Name);

                    if (_communicator.TraceLevels.Network >= 2)
                    {
                        _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                            $"attempting to bind to {_endpoint.TransportName} socket {_acceptor}");
                    }
                    _endpoint = _acceptor!.Listen();

                    if (_communicator.TraceLevels.Network >= 1)
                    {
                        _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                            $"listening for {_endpoint.TransportName} connections\n{_acceptor!.ToDetailedString()}");
                    }
                }
            }
            catch (Exception)
            {
                //
                // Clean up.
                //
                try
                {
                    _transceiver?.Close();
                    _acceptor?.Close();
                }
                catch
                {
                    // Ignore
                }

                _monitor.Destroy();
                _connections.Clear();

                throw;
            }
        }

        public void Activate()
        {
            lock (_mutex)
            {
                Debug.Assert(_destroyTask == null);
                if (_acceptor != null)
                {
                    if (_communicator.TraceLevels.Network >= 1)
                    {
                        _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                            $"accepting {_endpoint.TransportName} connections at {_acceptor}");
                    }

                    // Start the asynchronous operation from the thread pool to prevent eventually accepting
                    // synchronously new connections from this thread.
                    Task.Run(AcceptAsync);
                }
            }
        }

        public Task DestroyAsync()
        {
            lock (_mutex)
            {
                _destroyTask ??= PerformDestroyAsync();
            }
            return _destroyTask;
        }

        public Endpoint Endpoint()
        {
            if (_publishedEndpoint != null)
            {
                return _publishedEndpoint;
            }
            return _endpoint;
        }
        public bool IsLocal(Endpoint endpoint)
        {
            if (_publishedEndpoint != null && endpoint.IsLocal(_publishedEndpoint))
            {
                return true;
            }
            return endpoint.IsLocal(_endpoint);
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

        public void UpdateConnectionObservers()
        {
            lock (_mutex)
            {
                foreach (Connection connection in _connections)
                {
                    connection.UpdateObserver();
                }
            }
        }

        private async ValueTask AcceptAsync()
        {
            while (true)
            {
                ITransceiver transceiver;
                try
                {
                    transceiver = await _acceptor!.AcceptAsync().ConfigureAwait(false);
                }
                catch (Exception ex)
                {
                    // If Accept failed because the acceptor has been closed, just return, we're done. Otherwise
                    // we print an error and wait for one second to avoid running in a tight loop in case the
                    // failures occurs immediately again. Failures here are unexpected and could be considered
                    // fatal.
                    lock (_mutex)
                    {
                        if (_destroyTask != null)
                        {
                            return;
                        }
                    }
                    _communicator.Logger.Error($"failed to accept connection:\n{ex}\n{_acceptor}");
                    await Task.Delay(1000).ConfigureAwait(false); // Retry in 1 second
                    continue;
                }

                Connection connection;
                lock (_mutex)
                {
                    Debug.Assert(transceiver != null);
                    if (_destroyTask != null)
                    {
                        try
                        {
                            transceiver.Close();
                        }
                        catch
                        {
                        }
                        return;
                    }

                    // Reap closed connections
                    foreach (Connection c in _monitor.SwapReapedConnections())
                    {
                        _connections.Remove(c);
                    }

                    if (_communicator.TraceLevels.Network >= 2)
                    {
                        _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                            $"trying to accept {_endpoint.TransportName} connection\n{transceiver}");
                    }

                    try
                    {
                        connection = transceiver.CreateConnection(_endpoint, _monitor, null, _adapter);
                    }
                    catch (Exception ex)
                    {
                        try
                        {
                            transceiver.Close();
                        }
                        catch
                        {
                            // Ignore
                        }

                        if (_warn)
                        {
                            _communicator.Logger.Warning($"connection exception:\n{ex}\n{_acceptor}");
                        }
                        continue;
                    }

                    _connections.Add(connection);
                }

                Debug.Assert(connection != null);
                try
                {
                    // We don't wait for the connection to be activated. This could take a while for some transports
                    // such as TLS based transports where the handshake requires few round trips between the client
                    // and server.
                    _ = connection.StartAsync();
                }
                catch (ObjectAdapterDeactivatedException)
                {
                    // Ignore
                }
                catch (Exception ex)
                {
                    if (_communicator.TraceLevels.Network >= 2)
                    {
                        _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                            $"failed to accept {_endpoint.TransportName} connection\n{connection}\n{ex}");
                    }
                }
            }
        }

        private async Task PerformDestroyAsync()
        {
            // Close the acceptor
            if (_acceptor != null)
            {
                if (_communicator.TraceLevels.Network >= 1)
                {
                    _communicator.Logger.Trace(_communicator.TraceLevels.NetworkCat,
                        $"stopping to accept {_endpoint.TransportName} connections at {_acceptor}");
                }

                _acceptor.Close();
            }

            // Wait for all the connections to be closed
            IEnumerable<Task> tasks = _connections.Select(
                connection => connection.GracefulCloseAsync(new ObjectAdapterDeactivatedException(_adapter.Name)));
            await Task.WhenAll(tasks).ConfigureAwait(false);

            _monitor.Destroy();
        }
    }
}
