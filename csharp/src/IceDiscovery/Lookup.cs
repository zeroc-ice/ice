//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        private readonly Dictionary<string, LookupReply> _adapterReplies =
            new Dictionary<string, LookupReply>();
        private readonly string _domainId;
        private readonly int _latencyMultiplier;
        private readonly ILookupPrx _lookup;
        private readonly Dictionary<ILookupPrx, ILookupReplyPrx> _lookups =
            new Dictionary<ILookupPrx, ILookupReplyPrx>();
        private readonly object _mutex = new object();
        private readonly Dictionary<Identity, LookupReply> _objectReplies =
            new Dictionary<Identity, LookupReply>();
        private readonly LocatorRegistry _registry;
        private readonly ObjectAdapter _replyAdapter;
        private readonly int _retryCount;
        private readonly int _timeout;

        public void FindAdapterById(string domainId, string adapterId, ILookupReplyPrx? reply, Current current)
        {
            if (!domainId.Equals(_domainId))
            {
                return; // Ignore
            }

            (IObjectPrx? proxy, bool isReplicaGroup) = _registry.FindAdapter(adapterId);
            if (proxy != null)
            {
                // Reply to the multicast request using the given proxy.
                try
                {
                    Debug.Assert(reply != null);
                    reply.FoundAdapterByIdAsync(adapterId, proxy, isReplicaGroup);
                }
                catch
                {
                    // Ignore.
                }
            }
        }

        public void FindObjectById(string domainId, Identity id, ILookupReplyPrx? reply, Current current)
        {
            if (!domainId.Equals(_domainId))
            {
                return; // Ignore
            }

            IObjectPrx? proxy = _registry.FindObject(id);
            if (proxy != null)
            {
                // Reply to the mulicast request using the given proxy.
                try
                {
                    Debug.Assert(reply != null);
                    reply.FoundObjectByIdAsync(id, proxy);
                }
                catch
                {
                    // Ignore.
                }
            }
        }

        internal Lookup(LocatorRegistry registry, ILookupPrx lookup, Communicator communicator,
            ObjectAdapter replyAdapter)
        {
            _replyAdapter = replyAdapter;
            _registry = registry;
            _lookup = lookup;
            _timeout = communicator.GetPropertyAsInt("IceDiscovery.Timeout") ?? 300;
            _retryCount = communicator.GetPropertyAsInt("IceDiscovery.RetryCount") ?? 3;
            _latencyMultiplier = communicator.GetPropertyAsInt("IceDiscovery.LatencyMultiplier") ?? 1;
            _domainId = communicator.GetProperty("IceDiscovery.DomainId") ?? "";

            // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast
            // datagram on each endpoint.
            ILookupReplyPrx lookupReply = _replyAdapter.CreateProxy(
                "dummy", ILookupReplyPrx.Factory).Clone(invocationMode: InvocationMode.Datagram);
            var single = new Endpoint[1];
            foreach (UdpEndpoint endpoint in lookup.Endpoints.Cast<UdpEndpoint>())
            {
                single[0] = endpoint;

                ILookupPrx? key = lookup.Clone(endpoints: single);
                if (endpoint.McastInterface.Length > 0)
                {
                    Endpoint? q = lookupReply.Endpoints.FirstOrDefault(
                        e => e is IPEndpoint ipEndpoint && ipEndpoint.Host.Equals(endpoint.McastInterface));

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

        internal async ValueTask<IObjectPrx?> FindAdapterAsync(string id)
        {
            Task<IObjectPrx?>? task = null;
            LookupReply? replyServant;
            lock (_mutex)
            {
                if (!_adapterReplies.TryGetValue(id, out replyServant))
                {
                    replyServant = new LookupReply();
                    _adapterReplies.Add(id, replyServant);
                }
                else
                {
                    task = replyServant.CompletionSource.Task;
                }
            }

            if (task == null)
            {
                task = InvokeAsync(
                    async (lookup, lookupReply) =>
                    {
                        try
                        {
                            await lookup.FindAdapterByIdAsync(_domainId, id, lookupReply).ConfigureAwait(false);
                        }
                        catch (Exception ex)
                        {
                            throw new InvalidOperationException(
                                $"failed to lookup adapter `{id}' with lookup proxy `{_lookup}'", ex);
                        }
                    },
                    replyServant);

                await task.ConfigureAwait(false);

                lock (_mutex)
                {
                    _adapterReplies.Remove(id);
                }
            }
            return await task.ConfigureAwait(false);
        }

        internal async ValueTask<IObjectPrx?> FindObjectAsync(Identity id)
        {
            Task<IObjectPrx?>? task = null;
            LookupReply? replyServant;
            lock (_mutex)
            {
                if (!_objectReplies.TryGetValue(id, out replyServant))
                {
                    replyServant = new LookupReply();
                    _objectReplies.Add(id, replyServant);
                }
                else
                {
                    task = replyServant.CompletionSource.Task;
                }
            }

            if (task == null)
            {
                task = InvokeAsync(
                    async (lookup, lookupReply) =>
                    {
                        try
                        {
                            await lookup.FindObjectByIdAsync(_domainId, id, lookupReply).ConfigureAwait(false);
                        }
                        catch (Exception ex)
                        {
                            throw new InvalidOperationException(
                                $"failed to lookup object `{id}' with lookup proxy `{_lookup}'", ex);
                        }
                    },
                    replyServant);

                await task.ConfigureAwait(false);

                lock (_mutex)
                {
                    _objectReplies.Remove(id);
                }
            }
            return await task.ConfigureAwait(false);
        }

        internal async Task<IObjectPrx?> InvokeAsync(Func<ILookupPrx, ILookupReplyPrx, Task> find, LookupReply replyServant)
        {
            Identity requestId = _replyAdapter.AddWithUUID(replyServant, ILocatorRegistryPrx.Factory).Identity;

            Task<IObjectPrx?> replyTask = replyServant.CompletionSource.Task;
            try
            {
                for (int i = 0; i < _retryCount; ++i)
                {
                    long start = DateTime.Now.Ticks;
                    int failureCount = 0;
                    foreach ((ILookupPrx lookup, ILookupReplyPrx? reply) in _lookups)
                    {
                        ILookupReplyPrx? lookupReply = reply.Clone(requestId, ILookupReplyPrx.Factory);
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
                        return await replyServant.WaitForReplicaGroupRepliesAsync(start, _latencyMultiplier);
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

    internal class LookupReply : ILookupReply
    {
        internal CancellationTokenSource CancellationSource { get; }
        internal TaskCompletionSource<IObjectPrx?> CompletionSource { get; }

        private readonly object _mutex = new object();
        private readonly HashSet<IObjectPrx> _proxies = new HashSet<IObjectPrx>();

        public void FoundObjectById(Identity id, IObjectPrx? proxy, Current current) =>
            CompletionSource.SetResult(proxy);

        public void FoundAdapterById(string adapterId, IObjectPrx? proxy, bool isReplicaGroup, Current current)
        {
            if (isReplicaGroup)
            {
                lock (_mutex)
                {
                    _proxies.Add(proxy!);
                    if (_proxies.Count == 1)
                    {
                        // Cancel the request timeout and let InvokeAsync wait for additional replies from the replica
                        // group
                        CancellationSource.Cancel();
                    }
                }
            }
            else
            {
                CompletionSource.SetResult(proxy);
            }
        }

        internal async Task<IObjectPrx?> WaitForReplicaGroupRepliesAsync(long start, int latencyMultiplier)
        {
            Debug.Assert(_proxies.Count > 0);
            // This method is called by InvokeAsync after the first reply from a replica group to wait for additional
            // replies from the replica group.
            int latency = (int)((DateTime.Now.Ticks - start) * latencyMultiplier / 10000.0);
            if (latency == 0)
            {
                latency = 1;
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

        internal LookupReply()
        {
            CancellationSource = new CancellationTokenSource();
            CompletionSource = new TaskCompletionSource<IObjectPrx?>();
        }
    }
}
