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
        private bool _warnOnce = true;

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

        internal async ValueTask<IObjectPrx?> FindAdapterAsync(string adapterId)
        {
            LookupReply? replyServant;
            Identity? requestId = null;
            lock (_mutex)
            {
                if (!_adapterReplies.TryGetValue(adapterId, out replyServant))
                {
                    replyServant = new LookupReply(this);
                    _adapterReplies.Add(adapterId, replyServant);
                    requestId = _replyAdapter.AddWithUUID(replyServant, ILookupReplyPrx.Factory).Identity;
                }
            }

            int retryCount = _retryCount;
            int failureCount = 0;

            long start = DateTime.Now.Ticks;
            while (requestId != null)
            {
                foreach ((ILookupPrx lookup, ILookupReplyPrx? reply) in _lookups)
                {
                    Debug.Assert(reply != null);
                    ILookupReplyPrx? lookupReply = reply.Clone(requestId.Value, ILookupReplyPrx.Factory);
                    try
                    {
                        await lookup.FindAdapterByIdAsync(_domainId, adapterId, lookupReply).ConfigureAwait(false);
                    }
                    catch (Exception ex)
                    {
                        lock (_mutex)
                        {
                            if (_warnOnce)
                            {
                                _lookup.Communicator.Logger.Warning(
                                    $"failed to lookup adapter `{adapterId}' with lookup proxy `{_lookup}':\n{ex}");
                                _warnOnce = false;
                            }

                            if (++failureCount == _lookups.Count)
                            {
                                replyServant.CompletionSource.SetResult(null);
                                _adapterReplies.Remove(adapterId);
                                _replyAdapter.Remove(requestId.Value);
                            }
                        }
                    }
                }
                Task? t = await Task.WhenAny(
                    replyServant.CompletionSource.Task,
                    Task.Delay(_timeout, replyServant.CancellationSource.Token)).ConfigureAwait(false);

                if (t == replyServant.CompletionSource.Task)
                {
                    break;
                }
                else if (t.IsCanceled && replyServant.Proxies.Count > 0)
                {
                    // If the timeout was canceled we delay the completion of the request to give a chance to other
                    // members of this replica group to reply
                    int latency = (int)((DateTime.Now.Ticks - start) * _latencyMultiplier / 10000.0);
                    if (latency == 0)
                    {
                        latency = 1;
                    }
                    await Task.Delay(latency);
                    var endpoints = new List<Endpoint>();
                    IObjectPrx result = replyServant.Proxies.First();
                    foreach (IObjectPrx prx in replyServant.Proxies)
                    {
                        endpoints.AddRange(prx.Endpoints);
                    }
                    replyServant.CompletionSource.SetResult(result.Clone(endpoints: endpoints));
                    lock (_mutex)
                    {
                        _adapterReplies.Remove(adapterId);
                        _replyAdapter.Remove(requestId.Value);
                    }
                    break;
                }
                else if (--retryCount < 0)
                {
                    // The request timeout and no more retries
                    replyServant.CompletionSource.SetResult(null);
                    lock (_mutex)
                    {
                        _adapterReplies.Remove(adapterId);
                        _replyAdapter.Remove(requestId.Value);
                    }
                    break;
                }
            }
            return await replyServant.CompletionSource.Task.ConfigureAwait(false);
        }

        internal async ValueTask<IObjectPrx?> FindObjectAsync(Identity id)
        {
            LookupReply? replyServant;
            Identity? requestId = null;
            lock (_mutex)
            {
                if (!_objectReplies.TryGetValue(id, out replyServant))
                {
                    replyServant = new LookupReply(this);
                    _objectReplies.Add(id, replyServant);
                    requestId = _replyAdapter.AddWithUUID(replyServant, ILocatorRegistryPrx.Factory).Identity;
                }
            }

            int retryCount = _retryCount;
            int failureCount = 0;

            while (requestId != null)
            {
                foreach ((ILookupPrx lookup, ILookupReplyPrx? reply) in _lookups)
                {
                    Debug.Assert(reply != null);
                    ILookupReplyPrx? lookupReply = reply.Clone(requestId.Value, ILookupReplyPrx.Factory);

                    try
                    {
                        await lookup.FindObjectByIdAsync(_domainId, id, lookupReply).ConfigureAwait(false);
                    }
                    catch (Exception ex)
                    {
                        lock (_mutex)
                        {
                            if (_warnOnce)
                            {
                                _lookup.Communicator.Logger.Warning(
                                    $"failed to lookup object `{id}' with lookup proxy `{_lookup}':\n{ex}");
                                _warnOnce = false;
                            }

                            if (++failureCount == _lookups.Count)
                            {
                                replyServant.CompletionSource.SetResult(null);
                                _objectReplies.Remove(id);
                                _replyAdapter.Remove(requestId.Value);
                            }
                        }
                    }
                }
                Task? t = await Task.WhenAny(
                    replyServant.CompletionSource.Task,
                    Task.Delay(_timeout)).ConfigureAwait(false);
                lock (_mutex)
                {
                    if (t == replyServant.CompletionSource.Task)
                    {
                        break;
                    }
                    else if (--retryCount < 0)
                    {
                        // Request timeout and no more retries
                        replyServant.CompletionSource.SetResult(null);
                        _objectReplies.Remove(id);
                        _replyAdapter.Remove(requestId.Value);
                        break;
                    }
                }
            }
            return await replyServant.CompletionSource.Task.ConfigureAwait(false);
        }

        internal void FoundAdapter(string adapterId, Identity requestId, IObjectPrx proxy, bool isReplicaGroup)
        {
            lock (_mutex)
            {
                if (_adapterReplies.TryGetValue(adapterId, out LookupReply? replyServant))
                {
                    if (isReplicaGroup)
                    {
                        replyServant.Proxies.Add(proxy);
                        if (replyServant.Proxies.Count == 1)
                        {
                            replyServant.CancellationSource.Cancel();
                        }
                    }
                    else
                    {
                        replyServant.CompletionSource.SetResult(proxy);
                        _adapterReplies.Remove(adapterId);
                        _replyAdapter.Remove(requestId);
                    }
                }
                // else ignore responses from old requests
            }
        }

        internal void FoundObject(Identity id, Identity requestId, IObjectPrx proxy)
        {
            lock (_mutex)
            {
                if (_objectReplies.TryGetValue(id, out LookupReply? replyServant))
                {
                    replyServant.CompletionSource.SetResult(proxy);
                    _objectReplies.Remove(id);
                    _replyAdapter.Remove(requestId);
                }
                // else ignore responses from old requests
            }
        }
    }

    internal class LookupReply : ILookupReply
    {
        internal CancellationTokenSource CancellationSource { get; }
        internal TaskCompletionSource<IObjectPrx?> CompletionSource { get; }

        internal readonly HashSet<IObjectPrx> Proxies = new HashSet<IObjectPrx>();

        private readonly Lookup _lookup;

        internal LookupReply(Lookup lookup)
        {
            CancellationSource = new CancellationTokenSource();
            CompletionSource = new TaskCompletionSource<IObjectPrx?>();
            _lookup = lookup;
        }

        public void FoundObjectById(Identity id, IObjectPrx? proxy, Current current) =>
            _lookup.FoundObject(id, current.Identity, proxy!);

        public void FoundAdapterById(string adapterId, IObjectPrx? proxy, bool isReplicaGroup, Current current) =>
            _lookup.FoundAdapter(adapterId, current.Identity, proxy!, isReplicaGroup);
    }
}
