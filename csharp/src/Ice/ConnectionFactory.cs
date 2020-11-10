// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice
{
    public interface IConnectionManager
    {
        void Remove(Connection connection);
    }

    internal sealed class OutgoingConnectionFactory : IConnectionManager, IAsyncDisposable
    {
        private readonly Communicator _communicator;
        private readonly MultiDictionary<(Connector, string), Connection> _connectionsByConnector = new ();
        private readonly MultiDictionary<(Endpoint, string), Connection> _connectionsByEndpoint = new ();
        private Task? _disposeTask;
        private readonly object _mutex = new ();
        private readonly Dictionary<(Connector, string), Task<Connection>> _pending = new ();
        // We keep a map of the connectors that recently resulted in a transport failure. This is used to influence the
        // selection of connectors when creating new connections. Connectors with recent failures are tried last.
        private readonly ConcurrentDictionary<Connector, DateTime> _transportFailures = new ();

        public async ValueTask DisposeAsync()
        {
            lock (_mutex)
            {
                _disposeTask ??= PerformDisposeAsync();
            }
            await _disposeTask.ConfigureAwait(false);

            // TODO: We need to check for _pending and cancel pending connection establishment.

            async Task PerformDisposeAsync()
            {
                // Wait for connections to be closed.
                IEnumerable<Task> tasks =
                    _connectionsByConnector.Values.SelectMany(connections => connections).Select(connection =>
                        connection.GoAwayAsync(new CommunicatorDisposedException()));
                await Task.WhenAll(tasks).ConfigureAwait(false);

#if DEBUG
                // Ensure all the connections are removed
                Debug.Assert(_connectionsByConnector.Count == 0);
                Debug.Assert(_connectionsByEndpoint.Count == 0);
#endif
            }
        }

        public void Remove(Connection connection)
        {
            lock (_mutex)
            {
                Debug.Assert(connection.Connector != null);
                _connectionsByConnector.Remove((connection.Connector, connection.ConnectionId), connection);
                foreach (Endpoint endpoint in connection.Endpoints)
                {
                    _connectionsByEndpoint.Remove((endpoint, connection.ConnectionId), connection);
                }
            }
        }

        internal OutgoingConnectionFactory(Communicator communicator) => _communicator = communicator;

        internal void AddTransportFailure(Connector connector) => _transportFailures[connector] = DateTime.Now;

        internal Connection? GetConnection(IReadOnlyList<Endpoint> endpoints, string connectionId)
        {
            lock (_mutex)
            {
                // Try to find a connection to one of the given endpoints. Ignore the endpoint compression flag to
                // lookup for the connection.
                foreach (Endpoint endpoint in endpoints)
                {
                    if (_connectionsByEndpoint.TryGetValue((endpoint, connectionId),
                                                            out ICollection<Connection>? connectionList))
                    {
                        if (connectionList.FirstOrDefault(connection => connection.IsActive)
                            is Connection connection)
                        {
                            return connection;
                        }
                    }
                }
            }
            return null;
        }

        internal Connection? GetConnection(IReadOnlyList<Connector> connectors, string connectionId)
        {
            lock (_mutex)
            {
                // Try to find a connection to one of the given endpoints. Ignore the endpoint compression flag to
                // lookup for the connection.
                foreach (Connector connector in connectors)
                {
                    if (_connectionsByConnector.TryGetValue((connector, connectionId),
                                                            out ICollection<Connection>? connectionList))
                    {
                        if (connectionList.FirstOrDefault(connection => connection.IsActive)
                            is Connection connection)
                        {
                            lock (_mutex)
                            {
                                // If the connection was established for another endpoint but to the same connector,
                                // we ensure to also associate the connection with this endpoint.
                                if (!connection.Endpoints.Contains(connector.Endpoint))
                                {
                                    connection.Endpoints.Add(connector.Endpoint);
                                    _connectionsByEndpoint.Add((connector.Endpoint, connectionId), connection);
                                }
                            }
                            return connection;
                        }
                    }
                }
            }
            return null;
        }

        internal async ValueTask<List<Connector>> ComputeConnectorsAsync(
            Reference reference,
            IReadOnlyList<Endpoint> endpoints,
            IReadOnlyList<Connector> excludedConnectors,
            CancellationToken cancel)
        {
            // Purge expired hint failures
            DateTime expirationDate = DateTime.Now - TimeSpan.FromSeconds(5);
            foreach ((Connector connector, DateTime date) in _transportFailures)
            {
                if (date <= expirationDate)
                {
                    _ = _transportFailures.TryRemove(connector, out DateTime _);
                }
            }

            // For each endpoint, obtain the set of connectors. This might block if DNS lookups are required to
            // resolve an endpoint hostname into connector addresses.
            var connectors = new List<Connector>();
            foreach (Endpoint endpoint in endpoints)
            {
                try
                {
                    IEnumerable<Connector> endpointConnectors = await endpoint.ConnectorsAsync(cancel).ConfigureAwait(false);
                    foreach (Connector connector in endpointConnectors)
                    {
                        if (!excludedConnectors.Contains(connector))
                        {
                            connectors.Add(connector);
                        }
                    }
                }
                catch (CommunicatorDisposedException)
                {
                    throw; // No need to continue
                }
                catch (Exception ex)
                {
                    bool last = endpoint == endpoints[endpoints.Count - 1];

                    TraceLevels traceLevels = _communicator.TraceLevels;
                    if (traceLevels.Transport >= 2)
                    {
                        _communicator.Logger.Trace(TraceLevels.TransportCategory, last ?
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

            // If all the connectors are in the excludedConnectors throw NoEndpointException to indicate the caller
            // there is no more endpoints to try.
            if (connectors.Count == 0)
            {
                throw new NoEndpointException(reference.ToString());
            }

            // Order the connectors moving connectors with recent failures to the end of the list.
            return connectors.OrderBy(
                item => _transportFailures.TryGetValue(item, out DateTime value) ? value : default).ToList();
        }

        internal async ValueTask<Connection> CreateConnectionAsync(
            string connectionId,
            Connector connector,
            CancellationToken cancel)
        {
            Task<Connection>? connectTask;
            lock (_mutex)
            {
                if (_disposeTask != null)
                {
                    throw new CommunicatorDisposedException();
                }

                if (_connectionsByConnector.TryGetValue((connector, connectionId),
                                                        out ICollection<Connection>? connections))
                {
                    if (connections.FirstOrDefault(connection => connection.IsActive) is Connection connection)
                    {
                        return connection;
                    }
                }

                if (!_pending.TryGetValue((connector, connectionId), out connectTask))
                {
                    connectTask = PerformCreateConnectionAsync(connectionId, connector);
                    _pending.TryAdd((connector, connectionId), connectTask);
                }
            }

            return await connectTask.WaitAsync(cancel).ConfigureAwait(false);

            async Task<Connection> PerformCreateConnectionAsync(
                string connectionId,
                Connector connector)
            {
                // Yield to ensure the code below is executed without the mutex locked.
                await Task.Yield();

                IObserver? observer =
                    _communicator.Observer?.GetConnectionEstablishmentObserver(connector.Endpoint,
                                                                               connector.ToString()!);
                try
                {
                    observer?.Attach();

                    if (_communicator.TraceLevels.Transport >= 2)
                    {
                        _communicator.Logger.Trace(TraceLevels.TransportCategory,
                            $"trying to establish {connector.Endpoint.TransportName} connection to {connector}");
                    }

                    Connection connection;
                    lock (_mutex)
                    {
                        if (_disposeTask != null)
                        {
                            throw new CommunicatorDisposedException();
                        }
                        connection = connector.Connect(connectionId);
                        _connectionsByConnector.Add((connector, connectionId), connection);
                        _connectionsByEndpoint.Add((connector.Endpoint, connectionId), connection);
                    }
                    await connection.InitializeAsync().ConfigureAwait(false);
                    return connection;
                }
                catch (Exception ex)
                {
                    _transportFailures[connector] = DateTime.Now;
                    observer?.Failed(ex.GetType().FullName ?? "System.Exception");
                    throw;
                }
                finally
                {
                    _pending.Remove((connector, connectionId));
                    observer?.Detach();
                }
            }
        }

        internal void RemoveAdapter(ObjectAdapter adapter)
        {
            lock (_mutex)
            {
                if (_disposeTask != null)
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

        internal void SetRouterInfo(RouterInfo routerInfo)
        {
            ObjectAdapter? adapter = routerInfo.Adapter;
            IReadOnlyList<Endpoint> endpoints = routerInfo.GetClientEndpoints(); // can make a synchronous remote call

            // Search for connections to the router's client proxy endpoints, and update the object adapter for
            // such connections, so that callbacks from the router can be received over such connections.
            foreach (Endpoint endpoint in endpoints)
            {
                try
                {
                    foreach (Connector connector in endpoint.ConnectorsAsync(cancel: default).AsTask().Result)
                    {
                        lock (_mutex)
                        {
                            if (_disposeTask != null)
                            {
                                throw new CommunicatorDisposedException();
                            }

                            if (_connectionsByConnector.TryGetValue((connector, routerInfo.Router.ConnectionId),
                                out ICollection<Connection>? connections))
                            {
                                foreach (Connection connection in connections)
                                {
                                    connection.Adapter = adapter;
                                }
                            }
                        }
                    }
                }
                catch
                {
                    // Ignore
                }
            }
        }

        internal void UpdateConnectionObservers()
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

    internal abstract class IncomingConnectionFactory : IAsyncDisposable
    {
        internal Endpoint PublishedEndpoint => _publishedEndpoint ?? Endpoint;

        internal abstract void Activate();

        internal abstract void UpdateConnectionObservers();

        protected Endpoint Endpoint { get; set; }

        private readonly Endpoint? _publishedEndpoint;

        public abstract ValueTask DisposeAsync();

        internal bool IsLocal(Endpoint endpoint) =>
            _publishedEndpoint != null && (endpoint.IsLocal(_publishedEndpoint) || endpoint.IsLocal(Endpoint));

        protected IncomingConnectionFactory(Endpoint endpoint, Endpoint? publishedEndpoint)
        {
            Endpoint = endpoint;
            _publishedEndpoint = publishedEndpoint;
        }
    }

    // IncomingConnectionFactory for acceptor based transports.
    internal sealed class AcceptorIncomingConnectionFactory : IncomingConnectionFactory, IConnectionManager
    {
        private readonly IAcceptor _acceptor;
        private Task? _acceptTask;
        private readonly ObjectAdapter _adapter;
        private readonly Communicator _communicator;
        private readonly HashSet<Connection> _connections = new ();
        private bool _disposed;
        private readonly object _mutex = new ();

        public override async ValueTask DisposeAsync()
        {
            if (_communicator.TraceLevels.Transport >= 1)
            {
                _communicator.Logger.Trace(TraceLevels.TransportCategory,
                    $"stopping to accept {Endpoint.TransportName} connections at {_acceptor}");
            }

            // Dispose of the acceptor and close the connections. It's important to perform this synchronously without
            // any await in between to guarantee that once Communicator.ShutdownAsync returns the communicator no
            // longer accepts any requests.

            lock (_mutex)
            {
                _disposed = true;
                _acceptor.Dispose();
            }

            // The connection set is immutable once _disposed = true
            var exception = new ObjectDisposedException($"{typeof(ObjectAdapter).FullName}:{_adapter.Name}");
            IEnumerable<Task> tasks = _connections.Select(connection => connection.GoAwayAsync(exception));

            // Wait for AcceptAsync and the connection closure to return.
            if (_acceptTask != null)
            {
                await _acceptTask.ConfigureAwait(false);
            }
            await Task.WhenAll(tasks).ConfigureAwait(false);
        }

        public void Remove(Connection connection)
        {
            lock (_mutex)
            {
                if (!_disposed)
                {
                    _connections.Remove(connection);
                }
            }
        }

        public override string ToString() => _acceptor.ToString()!;

        internal AcceptorIncomingConnectionFactory(ObjectAdapter adapter, Endpoint endpoint, Endpoint? publish)
            : base(endpoint, publish)
        {
            _communicator = adapter.Communicator;
            _adapter = adapter;
            _acceptor = Endpoint.Acceptor(this, _adapter);

            Endpoint = _acceptor.Endpoint;

            if (_communicator.TraceLevels.Transport >= 1)
            {
                _communicator.Logger.Trace(TraceLevels.TransportCategory,
                    $"listening for {Endpoint.TransportName} connections\n{_acceptor.ToDetailedString()}");
            }
        }

        internal override void Activate()
        {
            if (_communicator.TraceLevels.Transport >= 1)
            {
                _communicator.Logger.Trace(TraceLevels.TransportCategory,
                    $"accepting {Endpoint.TransportName} connections at {_acceptor}");
            }

            // Start the asynchronous operation from the thread pool to prevent eventually accepting
            // synchronously new connections from this thread.
            lock (_mutex)
            {
                Debug.Assert(!_disposed);
                _acceptTask = Task.Factory.StartNew(AcceptAsync,
                                                    default,
                                                    TaskCreationOptions.None,
                                                    _adapter.TaskScheduler ?? TaskScheduler.Default);
            }
        }

        internal override void UpdateConnectionObservers()
        {
            lock (_mutex)
            {
                foreach (Connection connection in _connections)
                {
                    connection.UpdateObserver();
                }
            }
        }

        [System.Diagnostics.CodeAnalysis.SuppressMessage(
            "Reliability",
            "CA2007:Consider calling ConfigureAwait on the awaited task",
            Justification = "Ensure continuations execute on the object adapter scheduler if it is set")]
        private async ValueTask AcceptAsync()
        {
            while (true)
            {
                Connection? connection = null;
                try
                {
                    connection = await _acceptor.AcceptAsync();

                    if (_communicator.TraceLevels.Transport >= 2)
                    {
                        _communicator.Logger.Trace(TraceLevels.TransportCategory,
                            $"trying to accept {Endpoint.TransportName} connection\n{connection}");
                    }

                    lock (_mutex)
                    {
                        if (_disposed)
                        {
                            throw new ObjectDisposedException($"{typeof(ObjectAdapter).FullName}:{_adapter.Name}");
                        }

                        _connections.Add(connection);

                        // We don't wait for the connection to be activated. This could take a while for some transports
                        // such as TLS based transports where the handshake requires few round trips between the client
                        // and server.
                        _ = connection.InitializeAsync();
                    }
                }
                catch (Exception exception)
                {
                    if (connection != null)
                    {
                        await connection.GoAwayAsync(exception);
                    }
                    if (_disposed)
                    {
                        return;
                    }

                    // We print an error and wait for one second to avoid running in a tight loop in case the
                    // failures occurs immediately again. Failures here are unexpected and could be considered
                    // fatal.
                    _communicator.Logger.Error($"failed to accept connection:\n{exception}\n{_acceptor}");
                    await Task.Delay(TimeSpan.FromSeconds(1));
                    continue;
                }
            }
        }
    }

    // IncomingConnectionFactory for datagram based transports
    internal sealed class DatagramIncomingConnectionFactory : IncomingConnectionFactory
    {
        private readonly Connection _connection;

        public override async ValueTask DisposeAsync()
        {
            var exception = new ObjectDisposedException($"{typeof(ObjectAdapter).FullName}:{_connection.Adapter!.Name}");
            await _connection.GoAwayAsync(exception).ConfigureAwait(false);
        }

        public override string ToString() => _connection.ToString()!;

        internal DatagramIncomingConnectionFactory(ObjectAdapter adapter, Endpoint endpoint, Endpoint? publish)
            : base(endpoint, publish)
        {
            _connection = endpoint.CreateDatagramServerConnection(adapter);
            Endpoint = _connection.Endpoint;
            _ = _connection.InitializeAsync();
        }

        internal override void Activate()
        {
        }

        internal override void UpdateConnectionObservers() => _connection.UpdateObserver();
    }
}
