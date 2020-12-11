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
        private readonly Dictionary<(Endpoint, object?), LinkedList<Connection>> _outgoingConnections =
            new(EndpointComparer.Equivalent);
        private readonly Dictionary<(Endpoint, object?), Task<Connection>> _pendingOutgoingConnections =
            new(EndpointComparer.Equivalent);
        // We keep a map of the endpoints that recently resulted in a failure while establishing a connection. This is
        // used to influence the selection of endpoints when creating new connections. Endpoints with recent failures
        // are tried last.
        // TODO consider including endpoints with transport failures during invocation?
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
                    if (_destroyTask != null)
                    {
                        throw new CommunicatorDisposedException();
                    }

                    // Check if there is an active connection that we can use according to the endpoint settings.
                    if (_outgoingConnections.TryGetValue((endpoint, label), out LinkedList<Connection>? connections))
                    {
                        // The list of connections is already sorted with non-secure connections first, this will
                        // return the first active and trusted connection according to the non-secure preference.
                        connection = connections.FirstOrDefault(
                            connection => connection.IsActive && connection.CanTrust(preferNonSecure));

                        if (connection != null)
                        {
                            // TODO should CollocatedConnection.IsSecure return always true?, currently IsSecure
                            // is only true for SSL connections.
                            Debug.Assert(preferNonSecure != NonSecure.Never ||
                                         connection is ColocatedConnection ||
                                         connection.IsSecure);
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
                // After the connect task completed check if the connection can be trusted.
                if (!connection.CanTrust(preferNonSecure))
                {
                    // The connection cannot be trusted clear the connection and try again.
                    connection = null;
                }
            }
            while (connection == null);
            Debug.Assert(preferNonSecure != NonSecure.Never ||
                         connection is ColocatedConnection ||
                         connection.IsSecure);
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
                    using var linkedSource = CancellationTokenSource.CreateLinkedTokenSource(
                        source.Token,
                        endpoint.Communicator.CancellationToken);
                    CancellationToken cancel = linkedSource.Token;
                    Connection connection = await endpoint.ConnectAsync(preferNonSecure,
                                                                        label,
                                                                        cancel).ConfigureAwait(false);
                    lock (_mutex)
                    {
                        if (_destroyTask != null)
                        {
                            // If the communicator has been disposed return the connection here and avoid adding the
                            // connection to the outgoing connections map, the connection will be disposed from the
                            // pending connections map.
                            return connection;
                        }

                        if (!_outgoingConnections.TryGetValue((endpoint, label), out LinkedList<Connection>? list))
                        {
                            list = new LinkedList<Connection>();
                            _outgoingConnections[(endpoint, label)] = list;
                        }

                        // Keep the list of connections sorted with non-secure connections first so that when we check
                        // for non-secure connections they are tried first.
                        if (list.Count == 0 || connection.IsSecure)
                        {
                            list.AddLast(connection);
                        }
                        else
                        {
                            LinkedListNode<Connection>? next = list.First;
                            while (next != null)
                            {
                                if (next.Value.IsSecure)
                                {
                                    break;
                                }
                                next = next.Next;
                            }

                            if (next == null)
                            {
                                list.AddLast(connection);
                            }
                            else
                            {
                                list.AddBefore(next, connection);
                            }
                        }
                    }
                    // Set the callback used to remove the connection from the factory.
                    connection.Remove = connection => Remove(connection);
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
                        // Don't modify the pending connections map after the communicator was disposed.
                        if (_destroyTask == null)
                        {
                            _pendingOutgoingConnections.Remove((endpoint, label));
                        }
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
                                                         out LinkedList<Connection>? connections) &&
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

        internal List<Endpoint> OrderEndpointsByTransportFailures(List<Endpoint> endpoints)
        {
            if (_transportFailures.IsEmpty)
            {
                return endpoints;
            }
            else
            {
                // Purge expired transport failures

                // TODO avoid purge failures with each call
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
                    endpoint => _transportFailures.TryGetValue(endpoint, out DateTime value) ? value : default).ToList();
            }
        }

        internal void Remove(Connection connection)
        {
            lock (_mutex)
            {
                LinkedList<Connection> list = _outgoingConnections[(connection.Endpoint, connection.Label)];
                list.Remove(connection);
                if (list.Count == 0)
                {
                    _outgoingConnections.Remove((connection.Endpoint, connection.Label));
                }
            }
        }

        internal async ValueTask SetRouterInfoAsync(RouterInfo routerInfo, CancellationToken cancel)
        {
            ObjectAdapter? adapter = routerInfo.Adapter;
            IReadOnlyList<Endpoint> endpoints = await routerInfo.GetClientEndpointsAsync(cancel).ConfigureAwait(false);

            // Search for connections to the router's client proxy endpoints, and update the object adapter for
            // such connections, so that callbacks from the router can be received over such connections.
            lock (_mutex)
            {
                if (_destroyTask != null)
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

        private abstract class EndpointComparer : EqualityComparer<(Endpoint Endpoint, object? Label)>
        {
            internal static EndpointComparer Equivalent { get; } = new EquivalentEndpointComparer();
        }

        private class EquivalentEndpointComparer : EndpointComparer
        {
            public override bool Equals((Endpoint Endpoint, object? Label) lhs,
                                       (Endpoint Endpoint, object? Label) rhs) =>
               lhs.Endpoint.IsEquivalent(rhs.Endpoint) && Equals(lhs.Label, rhs.Label);

            public override int GetHashCode((Endpoint Endpoint, object? Label) obj) =>
                HashCode.Combine(obj.Endpoint.GetEquivalentHashCode(), obj.Label);
        }
    }
}
