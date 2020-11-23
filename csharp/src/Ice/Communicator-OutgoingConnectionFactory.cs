// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    public sealed partial class Communicator : IConnectionManager
    {
        private readonly MultiDictionary<(Endpoint, NonSecure, object), Connection> _activeConnectionMap = new(ConnectionMapKeyComparer.Instance);
        private readonly Dictionary<(Endpoint, NonSecure, object), Task<Connection>> _pendingConnectionMap = new(ConnectionMapKeyComparer.Instance);
        // We keep a map of the endpoints that recently resulted in a failure while establishing a connection. This is
        // used to influence the selection of endpoints when creating new connections. Endpoints with recent failures
        // are tried last.
        private readonly ConcurrentDictionary<Endpoint, DateTime> _transportFailures = new();

        public void Remove(Connection connection)
        {
            lock (_mutex)
            {
                _activeConnectionMap.Remove((connection.Endpoint, connection.PreferNonSecure, connection.ConnectionId),
                                            connection);
            }
        }

        internal async ValueTask<Connection> ConnectAsync(
            Endpoint endpoint,
            NonSecure preferNonSecure,
            object cookie,
            CancellationToken cancel)
        {
            Task<Connection>? connectTask;
            lock (_mutex)
            {
                // Check if there is an active connection that we can use.
                if (_activeConnectionMap.TryGetValue((endpoint, preferNonSecure, cookie),
                                                      out ICollection<Connection>? connections) &&
                    connections.FirstOrDefault(connection => connection.IsActive) is Connection connection)
                {
                    return connection;
                }
                // If there is a pending connection that matches the connection requirements await it otherwise try to
                // create a new connection and add the task to the pending connection map.
                if (!_pendingConnectionMap.TryGetValue((endpoint, preferNonSecure, cookie), out connectTask))
                {
                    connectTask = PerformConnectAsync(endpoint, preferNonSecure, cookie, cancel);
                    _pendingConnectionMap[(endpoint, preferNonSecure, cookie)] = connectTask;
                }
            }

            return await connectTask.ConfigureAwait(false);

            async Task<Connection> PerformConnectAsync(
                Endpoint endpoint,
                NonSecure preferNonSecure,
                object cookie,
                CancellationToken cancel)
            {
                // Ensure we don't hold the mutex while waiting for the endpoint to establish a connection
                await Task.Yield();
                try
                {
                    Connection connection = await endpoint.ConnectAsync(preferNonSecure, cookie, cancel).ConfigureAwait(false);
                    lock (_mutex)
                    {
                        _activeConnectionMap.Add((endpoint, preferNonSecure, cookie), connection);
                    }
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
                        _pendingConnectionMap.Remove((endpoint, preferNonSecure, cookie));
                    }
                }
            }
        }

        internal Connection? GetConnection(List<Endpoint> endpoints, NonSecure preferNonSecure, object cookie)
        {
            lock (_mutex)
            {
                foreach (Endpoint endpoint in endpoints)
                {
                    if (_activeConnectionMap.TryGetValue((endpoint, preferNonSecure, cookie),
                                                         out ICollection<Connection>? connections))
                    {
                        if (connections.FirstOrDefault(connection => connection.IsActive) is Connection connection)
                        {
                            return connection;
                        }
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
                    foreach (ICollection<Connection> connections in _activeConnectionMap.Values)
                    {
                        foreach (Connection connection in connections)
                        {
                            connection.Adapter = adapter;
                        }
                    }
                }
            }
        }

        private class ConnectionMapKeyComparer : EqualityComparer<(Endpoint Endpoint, NonSecure PreferNonSecure, object Cookie)>
        {
            internal static ConnectionMapKeyComparer Instance = new();

            public override bool Equals((Endpoint Endpoint, NonSecure PreferNonSecure, object Cookie) lhs,
                                        (Endpoint Endpoint, NonSecure PreferNonSecure, object Cookie) rhs) =>
                lhs.Endpoint.IsEquivalent(rhs.Endpoint) &&
                lhs.PreferNonSecure == rhs.PreferNonSecure &&
                lhs.Cookie.Equals(rhs.Cookie);

            public override int GetHashCode((Endpoint Endpoint, NonSecure PreferNonSecure, object Cookie) obj) =>
                HashCode.Combine(obj.Endpoint.GetEquivalentHashCode(), obj.PreferNonSecure, obj.Cookie);
        }

        private class MultiDictionary<TKey, TValue> : Dictionary<TKey, ICollection<TValue>> where TKey : notnull
        {
            public MultiDictionary(IEqualityComparer<TKey> comparer)
                : base(comparer)
            {
            }

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
}
