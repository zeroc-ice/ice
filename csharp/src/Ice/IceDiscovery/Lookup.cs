// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Ice;

namespace ZeroC.IceDiscovery
{
    internal class Lookup : IAsyncLookup
    {
        private readonly string _domainId;
        private readonly int _latencyMultiplier;
        private readonly ILookupPrx _lookup;
        private readonly Dictionary<ILookupPrx, ILookupReplyPrx> _lookups = new ();
        private readonly LocatorRegistry _registry;
        private readonly ObjectAdapter _replyAdapter;
        private readonly int _retryCount;
        private readonly TimeSpan _timeout;

        public async ValueTask FindAdapterByIdAsync(
            string domainId,
            string adapterId,
            ILookupReplyPrx reply,
            Current current,
            CancellationToken cancel)
        {
            if (domainId != _domainId)
            {
                return; // Ignore
            }

            (IObjectPrx? proxy, bool isReplicaGroup) = _registry.FindAdapter(adapterId);
            if (proxy != null)
            {
                // Reply to the multicast request using the given proxy.
                try
                {
                    await reply.FoundAdapterByIdAsync(adapterId, proxy, isReplicaGroup, cancel: cancel);
                }
                catch
                {
                    // Ignore.
                }
            }
        }

        public async ValueTask FindObjectByIdAsync(
            string domainId,
            Identity id,
            ILookupReplyPrx reply,
            Current current,
            CancellationToken cancel)
        {
            if (domainId != _domainId)
            {
                return; // Ignore
            }

            if (_registry.FindObject(id) is IObjectPrx proxy)
            {
                // Reply to the multicast request using the given proxy.
                try
                {
                    await reply.FoundObjectByIdAsync(id, proxy, cancel: cancel);
                }
                catch
                {
                    // Ignore.
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

            (IReadOnlyList<EndpointData> endpoints, bool isReplicaGroup) = _registry.ResolveAdapterId(adapterId);
            if (endpoints.Count > 0)
            {
                try
                {
                    await reply.FoundAsync(endpoints, isReplicaGroup, cancel: cancel);
                }
                catch
                {
                    // Ignore.
                }
            }
        }

        public async ValueTask ResolveWellKnownProxyAsync(
            string domainId,
            Identity identity,
            IResolveWellKnownProxyReplyPrx reply,
            Current current,
            CancellationToken cancel)
        {
            if (domainId != _domainId)
            {
                return; // Ignore
            }

            (IReadOnlyList<EndpointData> endpoints, string adapterId) = _registry.ResolveWellKnownProxy(identity);
            try
            {
                if (endpoints.Count > 0)
                {
                    await reply.FoundEndpointsAsync(endpoints, cancel: cancel);
                }
                else if (adapterId.Length > 0)
                {
                    await reply.FoundAdapterIdAsync(adapterId, cancel: cancel);
                }
            }
            catch
            {
                // Ignore.
            }
        }

        internal Lookup(
            LocatorRegistry registry,
            ILookupPrx lookup,
            Communicator communicator,
            ObjectAdapter replyAdapter)
        {
            _replyAdapter = replyAdapter;
            _registry = registry;
            _lookup = lookup;
            _timeout = communicator.GetPropertyAsTimeSpan("IceDiscovery.Timeout") ?? TimeSpan.FromMilliseconds(300);
            if (_timeout == Timeout.InfiniteTimeSpan)
            {
                _timeout = TimeSpan.FromMilliseconds(300);
            }
            _retryCount = communicator.GetPropertyAsInt("IceDiscovery.RetryCount") ?? 3;

            _latencyMultiplier = communicator.GetPropertyAsInt("IceDiscovery.LatencyMultiplier") ?? 1;
            if (_latencyMultiplier < 1)
            {
                throw new InvalidConfigurationException(
                    "The value of `IceDiscovery.LatencyMultiplier' must be a positive integer greater than 0");
            }

            _domainId = communicator.GetProperty("IceDiscovery.DomainId") ?? "";

            // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast
            // datagram on each endpoint.
            ILookupReplyPrx lookupReply = _replyAdapter.CreateProxy(
                "dummy", ILookupReplyPrx.Factory).Clone(invocationMode: InvocationMode.Datagram);
            var single = new Endpoint[1];
            foreach (Endpoint endpoint in lookup.Endpoints)
            {
                // lookup's invocation mode is Datagram
                Debug.Assert(endpoint.Transport == Transport.UDP);

                single[0] = endpoint;

                ILookupPrx? key = lookup.Clone(endpoints: single);
                if (endpoint["interface"] is string mcastInterface && mcastInterface.Length > 0)
                {
                    Endpoint? q = lookupReply.Endpoints.FirstOrDefault(e => e.Host == mcastInterface);
                    if (q != null)
                    {
                        single[0] = q;
                        _lookups[key] = lookupReply.Clone(endpoints: single);
                    }
                }

                if (!_lookups.ContainsKey(key))
                {
                    // Fallback: just use the given lookup reply proxy if no matching endpoint found.
                    _lookups[key] = lookupReply;
                }
            }
            Debug.Assert(_lookups.Count > 0);
        }

        // This is a wrapper for the call to FindAdapterByIdAsync on the lookup proxy. The caller (LocatorInfo) ensures
        // there is no concurrent resolution for the same location.
        internal async ValueTask<IObjectPrx?> FindAdapterByIdAsync(string adapterId, CancellationToken cancel)
        {
            using var replyServant = new LookupReply();
            await InvokeAsync(
                async (lookup, dummyReply, replyIdentity) =>
                {
                    try
                    {
                        ILookupReplyPrx lookupReply =
                            dummyReply.Clone(ILookupReplyPrx.Factory, identity: replyIdentity);

                        await lookup.FindAdapterByIdAsync(_domainId,
                                                          adapterId,
                                                          lookupReply,
                                                          cancel: cancel).ConfigureAwait(false);
                    }
                    catch (Exception ex)
                    {
                        // TODO: is InvalidOperationException really appropriate here?
                        throw new InvalidOperationException(
                            $"failed to lookup adapter `{adapterId}' with lookup proxy `{_lookup}'", ex);
                    }
                },
                replyServant.ReplyHandler).ConfigureAwait(false);

            return replyServant.Result;
        }

        // This is a wrapper for FindObjectByIdAsync on the lookup proxy. The caller (LocatorInfo) ensures there is no
        // concurrent resolution for the same identity.
        internal async ValueTask<IObjectPrx?> FindObjectByIdAsync(Identity identity, CancellationToken cancel)
        {
            using var replyServant = new LookupReply();
            await InvokeAsync(
                async (lookup, dummyReply, replyIdentity) =>
                {
                    try
                    {
                        ILookupReplyPrx lookupReply =
                            dummyReply.Clone(ILookupReplyPrx.Factory, identity: replyIdentity);

                        await lookup.FindObjectByIdAsync(_domainId,
                                                         identity,
                                                         lookupReply,
                                                         cancel: cancel).ConfigureAwait(false);
                    }
                    catch (Exception ex)
                    {
                        throw new InvalidOperationException(
                            $"failed to lookup object `{identity}' with lookup proxy `{_lookup}'", ex);
                    }
                },
                replyServant.ReplyHandler).ConfigureAwait(false);

            return replyServant.Result;
        }

        // This is a wrapper for the call to ResolveAdapterIdAsync on the lookup proxy. The caller (LocatorInfo) ensures
        // there is no concurrent resolution for the same location.
        internal async ValueTask<IReadOnlyList<EndpointData>> ResolveAdapterIdAsync(
            string adapterId,
            CancellationToken cancel)
        {
            using var replyServant = new ResolveAdapterIdReply();
            await InvokeAsync(
                async (lookup, dummyReply, replyIdentity) =>
                {
                    try
                    {
                        IResolveAdapterIdReplyPrx reply =
                            dummyReply.Clone(IResolveAdapterIdReplyPrx.Factory, identity: replyIdentity);

                        await lookup.ResolveAdapterIdAsync(_domainId,
                                                            adapterId,
                                                            reply,
                                                            cancel: cancel).ConfigureAwait(false);
                    }
                    catch (Exception ex)
                    {
                        // TODO: is InvalidOperationException really appropriate here?
                        throw new InvalidOperationException(
                            $"failed to lookup adapter `{adapterId}' with lookup proxy `{_lookup}'", ex);
                    }
                },
                replyServant.ReplyHandler).ConfigureAwait(false);

            return replyServant.Result;
        }

        /*
        // This is a wrapper for ResolveWellKnownProxyAsync on the lookup proxy. The caller (LocatorInfo) ensures there
        // is no concurrent resolution for the same identity.
        internal async ValueTask<(IEnumerable<EndpointData> Endpoints, string AdapterId)> ResolveWellKnownProxyAsync(
            Identity identity,
            CancellationToken cancel)
        {
            using var replyServant = new ResolveWellKnownProxyReply();
            await InvokeAsync(
                async (lookup, dummyReply, replyIdentity) =>
                {
                    try
                    {
                        IResolveWellKnownProxyReplyPrx reply =
                            dummyReply.Clone(IResolveWellKnownProxyReplyPrx.Factory, identity: replyIdentity);

                        await lookup.ResolveWellKnownProxyAsync(_domainId,
                                                                identity,
                                                                reply,
                                                                cancel: cancel).ConfigureAwait(false);
                    }
                    catch (Exception ex)
                    {
                        throw new InvalidOperationException(
                            $"failed to lookup object `{identity}' with lookup proxy `{_lookup}'", ex);
                    }
                },
                replyServant.ReplyHandler).ConfigureAwait(false);

            return replyServant.Result;
        }
        */

        private async Task InvokeAsync(
            Func<ILookupPrx, ILookupReplyPrx, Identity, Task> find,
            ReplyHandler replyHandler)
        {
            Identity replyIdentity =
                _replyAdapter.AddWithUUID(replyHandler.Servant, ILocatorRegistryPrx.Factory).Identity;

            Task replyTask = replyHandler.CompletionSource.Task;
            try
            {
                for (int i = 0; i < _retryCount; ++i)
                {
                    TimeSpan start = Time.Elapsed;
                    int failureCount = 0;
                    foreach ((ILookupPrx lookup, ILookupReplyPrx dummyReply) in _lookups)
                    {
                        try
                        {
                            await find(lookup, dummyReply, replyIdentity);
                        }
                        catch (Exception ex)
                        {
                            if (++failureCount == _lookups.Count)
                            {
                                _lookup.Communicator.Logger.Warning(ex.ToString());
                                replyHandler.CompletionSource.SetResult();
                            }
                        }
                    }

                    Task? t = await Task.WhenAny(replyTask,
                        Task.Delay(_timeout, replyHandler.CancellationSource.Token)).ConfigureAwait(false);
                    if (t == replyTask)
                    {
                        await replyTask.ConfigureAwait(false); // We're done! TODO: is this await necessary?
                        return;
                    }
                    else if (t.IsCanceled)
                    {
                        // If the timeout was canceled we delay the completion of the request to give a chance to other
                        // members of this replica group to reply
                        await replyHandler.WaitForReplicaGroupRepliesAsync(start, _latencyMultiplier)
                            .ConfigureAwait(false);
                        return;
                    }
                    // else timeout
                }
                replyHandler.CompletionSource.SetResult(); // Timeout
            }
            finally
            {
                _replyAdapter.Remove(replyIdentity);
            }
        }
    }

    internal sealed class ReplyHandler : IDisposable
    {
        internal CancellationTokenSource CancellationSource { get; }
        internal TaskCompletionSource CompletionSource { get; }

        internal IObject Servant { get; }

        private readonly Action _replicaReplyCollector;

        public void Dispose() => CancellationSource.Dispose();

        internal ReplyHandler(IObject servant, Action replicaReplyCollector)
        {
            Servant = servant;
            CancellationSource = new CancellationTokenSource();
            CompletionSource = new TaskCompletionSource();
            _replicaReplyCollector = replicaReplyCollector;
        }

        internal async Task WaitForReplicaGroupRepliesAsync(TimeSpan start, int latencyMultiplier)
        {
            // This method is called by InvokeAsync after the first reply from a replica group to wait for additional
            // replies from the replica group.
            TimeSpan latency = (Time.Elapsed - start) * latencyMultiplier;
            if (latency == TimeSpan.Zero)
            {
                latency = TimeSpan.FromMilliseconds(1);
            }
            await Task.Delay(latency);

            _replicaReplyCollector();
        }
    }

    internal class LookupReply : ILookupReply, IDisposable
    {
        internal ReplyHandler ReplyHandler { get; }

        internal IObjectPrx? Result { get; private set; }

        private readonly object _mutex = new object();
        private readonly HashSet<IObjectPrx> _proxies = new HashSet<IObjectPrx>();

        public void Dispose() => ReplyHandler.Dispose();

        public void FoundObjectById(Identity id, IObjectPrx proxy, Current current, CancellationToken cancel)
        {
            _proxies.Add(proxy);
            ReplyHandler.CompletionSource.SetResult();
        }

        public void FoundAdapterById(
            string adapterId,
            IObjectPrx proxy,
            bool isReplicaGroup,
            Current current,
            CancellationToken cancel)
        {
            if (isReplicaGroup)
            {
                lock (_mutex)
                {
                    _proxies.Add(proxy);
                    if (_proxies.Count == 1)
                    {
                        // Cancel the WhenAny timeout and let InvokeAsync wait for additional replies from the
                        // replica group.
                        ReplyHandler.CancellationSource.Cancel();
                    }
                }
            }
            else
            {
                Result = proxy;
                ReplyHandler.CompletionSource.SetResult();
            }
        }

        internal LookupReply()
        {
            ReplyHandler = new ReplyHandler(this, CollectReplicaReplies);
            Result = null;
        }

        private void CollectReplicaReplies()
        {
            lock (_mutex)
            {
                Debug.Assert(_proxies.Count > 0);
                var endpoints = new List<Endpoint>();
                IObjectPrx result = _proxies.First();
                foreach (IObjectPrx prx in _proxies)
                {
                    endpoints.AddRange(prx.Endpoints);
                }
                Result = result.Clone(endpoints: endpoints);
            }
            ReplyHandler.CompletionSource.SetResult();
        }
    }

    internal class ResolveAdapterIdReply : IResolveAdapterIdReply, IDisposable
    {
        internal ReplyHandler ReplyHandler { get; }

        internal IReadOnlyList<EndpointData> Result { get; private set; }

        private readonly object _mutex = new object();

        private HashSet<EndpointData> _endpointDataSet = new (Endpoint.DataComparer);

        public void Dispose() => ReplyHandler.Dispose();

        public void Found(
            EndpointData[] endpoints,
            bool isReplicaGroup,
            Current current,
            CancellationToken cancel)
        {
            if (isReplicaGroup)
            {
                lock (_mutex)
                {
                    bool firstReply = _endpointDataSet.Count == 0;

                    _endpointDataSet.UnionWith(endpoints);
                    if (firstReply)
                    {
                        // Cancel the WhenAny timeout and let InvokeAsync wait for additional replies from the
                        // replica group.
                        ReplyHandler.CancellationSource.Cancel();
                    }
                }
            }
            else
            {
                Result = endpoints;
                ReplyHandler.CompletionSource.SetResult();
            }
        }

        internal ResolveAdapterIdReply()
        {
            ReplyHandler = new ReplyHandler(this, CollectReplicaReplies);
            Result = ImmutableArray<EndpointData>.Empty;
        }

        private void CollectReplicaReplies()
        {
            lock (_mutex)
            {
                Debug.Assert(_endpointDataSet.Count > 0);
                Result = _endpointDataSet.ToList();
            }
            ReplyHandler.CompletionSource.SetResult();
        }
    }
}
