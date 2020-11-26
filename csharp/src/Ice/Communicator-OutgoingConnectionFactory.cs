// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    public sealed partial class Communicator
    {
        private readonly Dictionary<(Endpoint, object?), List<Connection>> _outgoingConnections =
            new(ConnectionMapKeyComparer.Instance);
        private readonly Dictionary<(Endpoint, object?), Task<Connection>> _pendingOutgoingConnections =
            new(ConnectionMapKeyComparer.Instance);
        // We keep a map of the endpoints that recently resulted in a failure while establishing a connection. This is
        // used to influence the selection of endpoints when creating new connections. Endpoints with recent failures
        // are tried last.
        private readonly ConcurrentDictionary<Endpoint, DateTime> _transportFailures = new();

        internal async ValueTask<Connection> ConnectAsync(
            Endpoint endpoint,
            NonSecure preferNonSecure,
            object? label,
            CancellationToken cancel)
        {
            Task<Connection>? connectTask;
            Connection? connection;
            do
            {
                lock (_mutex)
                {
                    // Check if there is an active connection that we can use according to the endpoint settings.
                    if (_outgoingConnections.TryGetValue((endpoint, label), out List<Connection>? connections))
                    {
                        if (preferNonSecure != NonSecure.Never)
                        {
                            // If non-secure connections are allowed check them first
                            connection = connections.OrderBy(connection => connection.IsSecure).FirstOrDefault(
                                connection => connection.IsActive && connection.CanTrust(preferNonSecure));
                        }
                        else
                        {
                            connection = connections.FirstOrDefault(connection => connection.IsActive);
                        }

                        if (connection != null)
                        {
                            return connection;
                        }
                    }

                    // If we didn't find an active connection check if there is a pending connect task for the same
                    // endpoint and label.
                    if (!_pendingOutgoingConnections.TryGetValue((endpoint, label), out connectTask))
                    {
                        connectTask = PerformConnectAsync(endpoint, preferNonSecure, label);
                        if (!connectTask.IsCompleted)
                        {
                            // If the task didn't complete synchronously we add it to the pending map
                            // and it will be removed once PerformConnectAsync completes.
                            _pendingOutgoingConnections[(endpoint, label)] = connectTask;
                        }
                    }
                }

                connection = await connectTask.WaitAsync(cancel).ConfigureAwait(false);
                // After the task completed we check if the connection can be used according to the endpoint setting.
                if (!connection.CanTrust(preferNonSecure))
                {
                    // The connection cannot be trust clear the connection and try again.
                    connection = null;
                }
            }
            while (connection == null);
            return connection;

            async Task<Connection> PerformConnectAsync(
                Endpoint endpoint,
                NonSecure preferNonSecure,
                object? label)
            {
                try
                {
                    Debug.Assert(ConnectTimeout > TimeSpan.Zero);
                    using var source = new CancellationTokenSource(ConnectTimeout);
                    CancellationToken cancel = source.Token;
                    Connection connection = await endpoint.ConnectAsync(preferNonSecure,
                                                                        label,
                                                                        cancel).ConfigureAwait(false);
                    lock (_mutex)
                    {
                        if (!_outgoingConnections.TryGetValue((endpoint, label), out List<Connection>? list))
                        {
                            list = new List<Connection>();
                            _outgoingConnections[(endpoint, label)] = list;
                        }

                        list.Add(connection);
                    }
                    // Set the callback used to remove the connection from the factory.
                    connection.Remove = connection => Remove(connection, label);
                    return connection;
                }
                catch (TransportException)
                {
                    _transportFailures[endpoint] = DateTime.Now;
                    throw;
                }
                finally
                {
                    lock (_mutex)
                    {
                        _pendingOutgoingConnections.Remove((endpoint, label));
                    }
                }
            }
        }

        internal Connection? GetConnection(List<Endpoint> endpoints, NonSecure preferNonSecure, object? label)
        {
            lock (_mutex)
            {
                foreach (Endpoint endpoint in endpoints)
                {
                    if (_outgoingConnections.TryGetValue((endpoint, label),
                                                         out List<Connection>? connections) &&
                        connections.FirstOrDefault(
                            connection => connection.IsActive && connection.CanTrust(preferNonSecure))
                        is Connection connection)
                    {
                        return connection;
                    }
                }
                return null;
            }
        }

        internal IEnumerable<Endpoint> OrderEndpointsByTransportFailures(IEnumerable<Endpoint> endpoints)
        {
            // Purge expired transport failures
            DateTime expirationDate = DateTime.Now - TimeSpan.FromSeconds(5);
            foreach ((Endpoint endpoint, DateTime date) in _transportFailures)
            {
                if (date <= expirationDate)
                {
                    _ = ((ICollection<KeyValuePair<Endpoint, DateTime>>)_transportFailures).Remove(
                        new KeyValuePair<Endpoint, DateTime>(endpoint, date));
                }
            }
            return endpoints.OrderBy(
                endpoint => _transportFailures.TryGetValue(endpoint, out DateTime value) ? value : default);
        }

        internal void Remove(Connection connection, object? label)
        {
            lock (_mutex)
            {
                List<Connection> list = _outgoingConnections[(connection.Endpoint, label)];
                list.Remove(connection);
                if (list.Count == 0)
                {
                    _outgoingConnections.Remove((connection.Endpoint, label));
                }
            }
        }

        internal void SetRouterInfo(RouterInfo routerInfo)
        {
            ObjectAdapter? adapter = routerInfo.Adapter;
            IReadOnlyList<Endpoint> endpoints = routerInfo.GetClientEndpoints(); // can make a synchronous remote call

            // Search for connections to the router's client proxy endpoints, and update the object adapter for
            // such connections, so that callbacks from the router can be received over such connections.
            lock (_mutex)
            {
                if (_disposeTask != null)
                {
                    throw new CommunicatorDisposedException();
                }

                foreach (Endpoint endpoint in endpoints)
                {
                    foreach (ICollection<Connection> connections in _outgoingConnections.Values)
                    {
                        foreach (Connection connection in connections)
                        {
                            connection.Adapter = adapter;
                        }
                    }
                }
            }
        }

        private class ConnectionMapKeyComparer : EqualityComparer<(Endpoint Endpoint, object? Label)>
        {
            internal static ConnectionMapKeyComparer Instance = new();

            public override bool Equals((Endpoint Endpoint, object? Label) lhs,
                                        (Endpoint Endpoint, object? Label) rhs) =>
                lhs.Endpoint.IsEquivalent(rhs.Endpoint) && Equals(lhs.Label, rhs.Label);

            public override int GetHashCode((Endpoint Endpoint, object? Label) obj) =>
                HashCode.Combine(obj.Endpoint.GetEquivalentHashCode(), obj.Label);
        }
    }
}
