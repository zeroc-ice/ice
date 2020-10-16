// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Ice;

namespace ZeroC.IceDiscovery
{
    internal class Lookup : ILookup
    {
        private readonly string _domainId;
        private readonly int _latencyMultiplier;
        private readonly ILookupPrx _lookup;
        private readonly Dictionary<ILookupPrx, ILookupReplyPrx> _lookups = new ();
        private readonly LocatorRegistry _registry;
        private readonly ObjectAdapter _replyAdapter;
        private readonly int _retryCount;
        private readonly TimeSpan _timeout;

        public void FindAdapterById(
            string domainId,
            string adapterId,
            ILookupReplyPrx reply,
            Protocol? protocol,
            Current current,
            CancellationToken cancel)
        {
            if (domainId != _domainId)
            {
                return; // Ignore
            }

            (IObjectPrx? proxy, bool isReplicaGroup) = _registry.FindAdapter(adapterId, protocol ?? Protocol.Ice1);
            if (proxy != null)
            {
                // Reply to the multicast request using the given proxy.
                try
                {
                    reply.FoundAdapterByIdAsync(adapterId, proxy, isReplicaGroup, cancel: cancel);
                }
                catch
                {
                    // Ignore.
                }
            }
        }

        public void FindObjectById(
            string domainId,
            Identity id,
            ILookupReplyPrx reply,
            Protocol? protocol,
            Current current,
            CancellationToken cancel)
        {
            if (domainId != _domainId)
            {
                return; // Ignore
            }

            if (_registry.FindObject(id, protocol ?? Protocol.Ice1) is IObjectPrx proxy)
            {
                // Reply to the multicast request using the given proxy.
                try
                {
                    reply.FoundObjectByIdAsync(id, proxy, cancel: cancel);
                }
                catch
                {
                    // Ignore.
                }
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
        internal async ValueTask<IObjectPrx?> FindAdapterByIdAsync(
            string adapterId,
            Protocol protocol,
            CancellationToken cancel)
        {
            var replyServant = new LookupReply(protocol);
            IObjectPrx? proxy = await InvokeAsync(
                async (lookup, lookupReply) =>
                {
                    try
                    {
                        await lookup.FindAdapterByIdAsync(_domainId,
                                                          adapterId,
                                                          lookupReply,
                                                          protocol,
                                                          cancel: cancel).ConfigureAwait(false);
                    }
                    catch (Exception ex)
                    {
                        // TODO: is InvalidOperationException really appropriate here?
                        throw new InvalidOperationException(
                            $"failed to lookup adapter `{adapterId}' with lookup proxy `{_lookup}'", ex);
                    }
                },
                replyServant).ConfigureAwait(false);
            return proxy;
        }

        // This is a wrapper for FindObjectByIdAsync on the lookup proxy. The caller (LocatorInfo) ensures there is no
        // concurrent resolution for the same identity.
        internal async ValueTask<IObjectPrx?> FindObjectByIdAsync(
            Identity identity,
            Protocol protocol,
            CancellationToken cancel)
        {
            var replyServant = new LookupReply(protocol);
            return await InvokeAsync(
                async (lookup, lookupReply) =>
                {
                    try
                    {
                        await lookup.FindObjectByIdAsync(_domainId,
                                                         identity,
                                                         lookupReply,
                                                         protocol,
                                                         cancel: cancel).ConfigureAwait(false);
                    }
                    catch (Exception ex)
                    {
                        throw new InvalidOperationException(
                            $"failed to lookup object `{identity}' with lookup proxy `{_lookup}'", ex);
                    }
                },
                replyServant).ConfigureAwait(false);
        }

        private async Task<IObjectPrx?> InvokeAsync(
            Func<ILookupPrx, ILookupReplyPrx, Task> find,
            LookupReply replyServant)
        {
            Identity requestId = _replyAdapter.AddWithUUID(replyServant, ILocatorRegistryPrx.Factory).Identity;

            Task<IObjectPrx?> replyTask = replyServant.CompletionSource.Task;
            try
            {
                for (int i = 0; i < _retryCount; ++i)
                {
                    TimeSpan start = Time.Elapsed;
                    int failureCount = 0;
                    foreach ((ILookupPrx lookup, ILookupReplyPrx reply) in _lookups)
                    {
                        ILookupReplyPrx lookupReply = reply.Clone(ILookupReplyPrx.Factory, identity: requestId);
                        try
                        {
                            await find(lookup, lookupReply);
                        }
                        catch (Exception ex)
                        {
                            if (++failureCount == _lookups.Count)
                            {
                                _lookup.Communicator.Logger.Warning(ex.ToString());
                                replyServant.CompletionSource.SetResult(null);
                            }
                        }
                    }

                    Task? t = await Task.WhenAny(replyTask,
                        Task.Delay(_timeout, replyServant.CancellationSource.Token)).ConfigureAwait(false);
                    if (t == replyTask)
                    {
                        return await replyTask.ConfigureAwait(false); // We're done!
                    }
                    else if (t.IsCanceled)
                    {
                        // If the timeout was canceled we delay the completion of the request to give a chance to other
                        // members of this replica group to reply
                        return await replyServant.WaitForReplicaGroupRepliesAsync(start, _latencyMultiplier)
                            .ConfigureAwait(false);
                    }
                }
                replyServant.CompletionSource.SetResult(null); // Timeout
                return await replyTask.ConfigureAwait(false);
            }
            finally
            {
                _replyAdapter.Remove(requestId);
            }
        }
    }

    // TODO: missing IDisposable?
    internal class LookupReply : ILookupReply
    {
        internal CancellationTokenSource CancellationSource { get; }
        internal TaskCompletionSource<IObjectPrx?> CompletionSource { get; }

        private readonly object _mutex = new object();

        private readonly Protocol _protocol;
        private readonly HashSet<IObjectPrx> _proxies = new HashSet<IObjectPrx>();

        public void FoundObjectById(Identity id, IObjectPrx proxy, Current current, CancellationToken cancel) =>
            CompletionSource.SetResult(proxy);

        public void FoundAdapterById(
            string adapterId,
            IObjectPrx proxy,
            bool isReplicaGroup,
            Current current,
            CancellationToken cancel)
        {
            if (proxy.Protocol == _protocol)
            {
                if (isReplicaGroup)
                {
                    lock (_mutex)
                    {
                        _proxies.Add(proxy);
                        if (_proxies.Count == 1)
                        {
                            // Cancel the request timeout and let InvokeAsync wait for additional replies from the
                            // replica group
                            CancellationSource.Cancel();
                        }
                    }
                }
                else
                {
                    CompletionSource.SetResult(proxy);
                }
            }
            // else bogus proxy. TODO: add trace?
        }

        internal async Task<IObjectPrx?> WaitForReplicaGroupRepliesAsync(TimeSpan start, int latencyMultiplier)
        {
            Debug.Assert(_proxies.Count > 0);
            // This method is called by InvokeAsync after the first reply from a replica group to wait for additional
            // replies from the replica group.
            TimeSpan latency = (Time.Elapsed - start) * latencyMultiplier;
            if (latency == TimeSpan.Zero)
            {
                latency = TimeSpan.FromMilliseconds(1);
            }
            await Task.Delay(latency);
            lock (_mutex)
            {
                var endpoints = new List<Endpoint>();
                IObjectPrx result = _proxies.First();
                foreach (IObjectPrx prx in _proxies)
                {
                    endpoints.AddRange(prx.Endpoints);
                }
                CompletionSource.SetResult(result.Clone(endpoints: endpoints));
            }
            return CompletionSource.Task.Result;
        }

        internal LookupReply(Protocol protocol)
        {
            _protocol = protocol;
            CancellationSource = new CancellationTokenSource();
            CompletionSource = new TaskCompletionSource<IObjectPrx?>();
        }
    }
}
