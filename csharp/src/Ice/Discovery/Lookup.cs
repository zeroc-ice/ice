// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Discovery
{
    /// <summary>Servant class that implements the Slice interface Ice::Discovery::Lookup using the local
    /// LocatorRegistry servant.</summary>
    internal class Lookup : IAsyncLookup
    {
        private readonly string _domainId;

        private readonly LocatorRegistry _registryServant;

        public async ValueTask FindAdapterByIdAsync(
            string domainId,
            string adapterId,
            IFindAdapterByIdReplyPrx reply,
            Current current,
            CancellationToken cancel)
        {
            if (domainId != _domainId)
            {
                return; // Ignore
            }

            (IObjectPrx? proxy, bool isReplicaGroup) = _registryServant.FindAdapter(adapterId);
            if (proxy != null)
            {
                // Reply to the multicast request using the given proxy.
                try
                {
                    if (reply.InvocationMode == InvocationMode.Datagram)
                    {
                        reply = reply.Clone(preferNonSecure: NonSecure.Always);
                    }
                    await reply.FoundAdapterByIdAsync(adapterId, proxy, isReplicaGroup, cancel: cancel).
                        ConfigureAwait(false);
                }
                catch (Exception ex)
                {
                    current.Communicator.Logger.Warning(
                        $"Ice discovery failed to send foundAdapterById to `{reply}':\n{ex}");
                }
            }
        }

        public async ValueTask FindObjectByIdAsync(
            string domainId,
            Identity id,
            string? facet,
            IFindObjectByIdReplyPrx reply,
            Current current,
            CancellationToken cancel)
        {
            if (domainId != _domainId)
            {
                return; // Ignore
            }

            if (await _registryServant.FindObjectAsync(id, facet, cancel).ConfigureAwait(false) is IObjectPrx proxy)
            {
                // Reply to the multicast request using the given proxy.
                try
                {
                    if (reply.InvocationMode == InvocationMode.Datagram)
                    {
                        reply = reply.Clone(preferNonSecure: NonSecure.Always);
                    }
                    await reply.FoundObjectByIdAsync(id, proxy, cancel: cancel).ConfigureAwait(false);
                }
                catch (Exception ex)
                {
                    current.Communicator.Logger.Warning(
                        $"Ice discovery failed to send foundObjectById to `{reply}':\n{ex}");
                }
            }
        }

        public async ValueTask ResolveAdapterIdAsync(
            string domainId,
            string adapterId,
            IResolveAdapterIdReplyPrx reply,
            Current current,
            CancellationToken cancel)
        {
            if (domainId != _domainId)
            {
                return; // Ignore
            }

            (IReadOnlyList<EndpointData> endpoints, bool isReplicaGroup) = _registryServant.ResolveAdapterId(adapterId);
            if (endpoints.Count > 0)
            {
                try
                {
                    if (reply.InvocationMode == InvocationMode.Datagram)
                    {
                        reply = reply.Clone(preferNonSecure: NonSecure.Always);
                    }
                    await reply.FoundAdapterIdAsync(endpoints, isReplicaGroup, cancel: cancel).ConfigureAwait(false);
                }
                catch (Exception ex)
                {
                    current.Communicator.Logger.Warning(
                        $"Ice discovery failed to send foundAdapterId to `{reply}':\n{ex}");
                }
            }
        }

        public async ValueTask ResolveWellKnownProxyAsync(
            string domainId,
            Identity identity,
            string facet,
            IResolveWellKnownProxyReplyPrx reply,
            Current current,
            CancellationToken cancel)
        {
            if (domainId != _domainId)
            {
                return; // Ignore
            }

            string adapterId =
                await _registryServant.ResolveWellKnownProxyAsync(identity, facet, cancel).ConfigureAwait(false);

            if (adapterId.Length > 0)
            {
                try
                {
                    if (reply.InvocationMode == InvocationMode.Datagram)
                    {
                        reply = reply.Clone(preferNonSecure: NonSecure.Always);
                    }
                    await reply.FoundWellKnownProxyAsync(adapterId, cancel: cancel).ConfigureAwait(false);
                }
                catch (Exception ex)
                {
                    current.Communicator.Logger.Warning(
                        $"Ice discovery failed to send foundWellKnownProxy to `{reply}':\n{ex}");
                }
            }
        }

        internal Lookup(LocatorRegistry registryServant, Communicator communicator)
        {
            _registryServant = registryServant;
            _domainId = communicator.GetProperty("Ice.Discovery.DomainId") ?? "";
        }
    }
}
