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
        private readonly MultiDictionary<(Endpoint, string), Connection> _connectionsByEndpoint = new ();
        private Task? _disposeTask;
        private readonly object _mutex = new ();
        private readonly Dictionary<(Endpoint, string), Task<Connection>> _pending = new ();
        // We keep a map of the connectors that recently resulted in a transport failure. This is used to influence the
        // selection of connectors when creating new connections. Connectors with recent failures are tried last.
        private readonly ConcurrentDictionary<Endpoint, DateTime> _transportFailures = new ();

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
                    _connectionsByEndpoint.Values.SelectMany(connections => connections).Select(connection =>
                        connection.GoAwayAsync(new CommunicatorDisposedException()));
                await Task.WhenAll(tasks).ConfigureAwait(false);

#if DEBUG
                // Ensure all the connections are removed
                Debug.Assert(_connectionsByEndpoint.Count == 0);
#endif
            }
        }

        public void Remove(Connection connection)
        {
            lock (_mutex)
            {
                foreach (Endpoint endpoint in connection.Endpoints)
                {
                    _connectionsByEndpoint.Remove((endpoint, connection.ConnectionId), connection);
                }
            }
        }

        internal OutgoingConnectionFactory(Communicator communicator)
        {
            _communicator = communicator;
        }

        internal void AddTransportFailure(Endpoint endpoint) => _transportFailures[endpoint] = DateTime.Now;

        internal async ValueTask<Connection> CreateAsync(
            IReadOnlyList<Endpoint> endpoints,
            bool hasMore,
            string connectionId,
            CancellationToken cancel)
        {
            Debug.Assert(endpoints.Count > 0);
            lock (_mutex)
            {
                if (_disposeTask != null)
                {
                    throw new CommunicatorDisposedException();
                }

                // Try to find a connection to one of the given endpoints. Ignore the endpoint compression flag to
                // lookup for the connection.
                foreach (Endpoint endpoint in endpoints)
                {
                    if (_connectionsByEndpoint.TryGetValue((endpoint, connectionId),
                                                            out ICollection<Connection>? connectionList))
                    {
                        if (connectionList.FirstOrDefault(connection => connection.IsActive) is Connection connection)
                        {
                            return connection;
                        }
                    }
                }
            }

            lock (_mutex)
            {
                if (_disposeTask != null)
                {
                    throw new CommunicatorDisposedException();
                }

                // Purge expired hint failures
                DateTime expirationDate = DateTime.Now - TimeSpan.FromSeconds(5);
                foreach ((Endpoint endpoint, DateTime date) in _transportFailures)
                {
                    if (date <= expirationDate)
                    {
                        _ = _transportFailures.TryRemove(endpoint, out DateTime _);
                    }
                }

                // Order the remaining endpoints moving endpoints associated with recent failures to the end of the
                // list.
                endpoints = endpoints.OrderBy(
                    endpoint => _transportFailures.TryGetValue(endpoint, out DateTime value) ?
                        value : default).ToList();
            }

            // Wait for connection establishment to one or some of the endpoints to complete.
            while (true)
            {
                var connectTasks = new List<Task<Connection>>();
                var tried = new HashSet<Endpoint>();
                lock (_mutex)
                {
                    if (_disposeTask != null)
                    {
                        throw new CommunicatorDisposedException();
                    }

                    // Search for pending connects for the set of endpoints which weren't already tried.
                    foreach (Endpoint endpoint in endpoints)
                    {
                        if (_connectionsByEndpoint.TryGetValue((endpoint, connectionId),
                                                                out ICollection<Connection>? connectionList))
                        {
                            if (connectionList.FirstOrDefault(connection => connection.IsActive) is Connection connection)
                            {
                                return connection;
                            }
                        }

                        if (_pending.TryGetValue((endpoint, connectionId), out Task<Connection>? task))
                        {
                            connectTasks.Add(task);
                            tried.Add(endpoint);
                        }
                    }

                    // We didn't find pending connects for the remaining endpoints so we can try to establish
                    // a connection to them.
                    if (tried.Count == 0)
                    {
                        Task<Connection> connectTask = ConnectAsync(endpoints, connectionId, hasMore);
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

                        foreach (Endpoint endpoint in endpoints)
                        {
                            // Use TryAdd in case there are duplicates.
                            _pending.TryAdd((endpoint, connectionId), connectTask);
                            tried.Add(endpoint);
                        }
                        connectTasks.Add(connectTask);
                    }
                }

                // Wait for the first successful connection establishment
                Task<Connection> completedTask;
                do
                {
                    completedTask =
                        await Task.WhenAny(connectTasks).WaitAsync(cancel).ConfigureAwait(false);
                    if (completedTask.IsCompletedSuccessfully)
                    {
                        var connection = completedTask.Result;
                        if (endpoints.Contains(connection.Endpoint))
                        {
                            return connection;
                        }
                    }
                    connectTasks.Remove(completedTask);
                }
                while (connectTasks.Count > 0);

                // Remove the endpoints we tried from the set of remaining endpoints
                endpoints = endpoints.Except(tried).ToImmutableList();

                // If there are no more endpoints to try, we failed to establish a connection and we raise the
                // failure.
                if (endpoints.Count == 0)
                {
                    Debug.Assert(completedTask.IsFaulted);
                    return await completedTask.ConfigureAwait(false);
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

                foreach (ICollection<Connection> connectionList in _connectionsByEndpoint.Values)
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
                    lock (_mutex)
                    {
                        if (_disposeTask != null)
                        {
                            throw new CommunicatorDisposedException();
                        }

                        if (_connectionsByEndpoint.TryGetValue((endpoint, routerInfo.Router.ConnectionId),
                            out ICollection<Connection>? connections))
                        {
                            foreach (Connection connection in connections)
                            {
                                connection.Adapter = adapter;
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
                foreach (ICollection<Connection> connections in _connectionsByEndpoint.Values)
                {
                    foreach (Connection c in connections)
                    {
                        c.UpdateObserver();
                    }
                }
            }
        }

        private async Task<Connection> ConnectAsync(
            IReadOnlyList<Endpoint> endpoints,
            string connectionId,
            bool hasMore)
        {
            Debug.Assert(endpoints.Count > 0);

            try
            {
                for (int i = 0; i < endpoints.Count; ++i)
                {
                    Endpoint endpoint = endpoints[i];

                    IObserver? observer =
                        _communicator.Observer?.GetConnectionEstablishmentObserver(endpoint, endpoint.ToString()!);
                    try
                    {
                        observer?.Attach();

                        Connection connection = await endpoint.ConnectAsync(connectionId, default).ConfigureAwait(false);
                        lock (_mutex)
                        {
                            if (_disposeTask != null)
                            {
                                throw new CommunicatorDisposedException();
                            }
                            _connectionsByEndpoint.Add((endpoint, connectionId), connection);
                        }
                        await connection.InitializeAsync().ConfigureAwait(false);
                        return connection;
                    }
                    catch (CommunicatorDisposedException ex)
                    {
                        observer?.Failed(ex.GetType().FullName ?? "System.Exception");
                        throw; // No need to continue
                    }
                    catch (Exception ex)
                    {
                        observer?.Failed(ex.GetType().FullName ?? "System.Exception");

                        bool last = i == endpoints.Count - 1;

                        TraceLevels traceLevels = _communicator.TraceLevels;
                        _transportFailures[endpoint] = DateTime.Now;

                        if (traceLevels.Transport >= 2)
                        {
                            _communicator.Logger.Trace(TraceLevels.TransportCategory,
                                $"failed to establish {endpoint.TransportName} connection to {endpoint} " +
                                (last && !hasMore ?
                                    $"and no more endpoints to try\n{ex}" :
                                    $"trying next endpoint\n{ex}"));
                        }

                        if (last)
                        {
                            // If it's the last connector to try and we couldn't establish the connection, we're done.
                            throw;
                        }
                    }
                    finally
                    {
                        observer?.Detach();
                    }
                }
            }
            finally
            {
                lock (_mutex)
                {
                    foreach (Endpoint endpoint in endpoints)
                    {
                        _pending.Remove((endpoint, connectionId));
                    }
                }
            }

            // The loop either raised an exception on the last connector or returned if the connection establishment
            // succeeded.
            Debug.Assert(false);
            return null!;
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
