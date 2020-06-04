//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ZeroC.Ice;

namespace ZeroC.IceDiscovery
{
    internal class Lookup : ILookup
    {
        internal int LatencyMultiplier { get; }
        internal Timer Timer { get; }

        private readonly Dictionary<string, TaskCompletionSource<IObjectPrx?>> _adapterRequests =
            new Dictionary<string, TaskCompletionSource<IObjectPrx?>>();
        private readonly Dictionary<string, HashSet<IObjectPrx>> _replicaGroupReplies =
            new Dictionary<string, HashSet<IObjectPrx>>();
        private readonly string _domainId;
        private readonly ILookupPrx _lookup;
        private readonly Dictionary<ILookupPrx, ILookupReplyPrx?> _lookups =
            new Dictionary<ILookupPrx, ILookupReplyPrx?>();
        private readonly object _mutex = new object();
        private readonly Dictionary<Identity, TaskCompletionSource<IObjectPrx?>> _objectRequests =
            new Dictionary<Identity, TaskCompletionSource<IObjectPrx?>>();
        private readonly LocatorRegistry _registry;
        private readonly int _retryCount;
        private readonly int _timeout;
        private bool _warnOnce = true;

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

        public void FindAdapterById(string domainId, string adapterId, ILookupReplyPrx? reply, Current current)
        {
            if (!domainId.Equals(_domainId))
            {
                return; // Ignore
            }

            IObjectPrx? proxy = _registry.FindAdapter(adapterId, out bool isReplicaGroup);
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

        internal Lookup(LocatorRegistry registry, ILookupPrx lookup, Communicator communicator)
        {
            _registry = registry;
            _lookup = lookup;
            _timeout = communicator.GetPropertyAsInt("IceDiscovery.Timeout") ?? 300;
            _retryCount = communicator.GetPropertyAsInt("IceDiscovery.RetryCount") ?? 3;
            LatencyMultiplier = communicator.GetPropertyAsInt("IceDiscovery.LatencyMultiplier") ?? 1;
            _domainId = communicator.GetProperty("IceDiscovery.DomainId") ?? "";
            Timer = lookup.Communicator.Timer();

            // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast
            // datagram on each endpoint.
            var single = new Endpoint[1];
            foreach (Endpoint endpt in lookup.Endpoints)
            {
                single[0] = endpt;
                _lookups[lookup.Clone(endpoints: single)] = null;
            }
            Debug.Assert(_lookups.Count > 0);
        }

        internal async ValueTask<IObjectPrx?> FindAdapterAsync(string adapterId)
        {
            TaskCompletionSource<IObjectPrx?>? requestSource;
            bool invoke = false;
            lock (_mutex)
            {
                if (!_adapterRequests.TryGetValue(adapterId, out requestSource))
                {
                    invoke = true;
                    requestSource = new TaskCompletionSource<IObjectPrx?>();
                    _adapterRequests.Add(adapterId, requestSource);
                }
            }

            while (invoke)
            {
                int retryCount = _retryCount;
                int lookupCount = _lookups.Count;
                int failureCount = 0;

                foreach ((ILookupPrx lookup, ILookupReplyPrx? reply) in _lookups)
                {
                    Debug.Assert(reply != null);
                    ILookupReplyPrx? lookupReply = reply.Clone(new Identity(adapterId, ""), ILookupReplyPrx.Factory);

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

                            if (++failureCount == lookupCount)
                            {
                                requestSource.SetResult(null);
                                _adapterRequests.Remove(adapterId);
                            }
                        }
                    }
                }
                Task? t = await Task.WhenAny(requestSource.Task, Task.Delay(_timeout)).ConfigureAwait(false);
                lock (_mutex)
                {
                    if (t == requestSource.Task)
                    {
                        break;
                    }
                    else if (_replicaGroupReplies.TryGetValue(adapterId, out var value))
                    {
                        // The request will be completed once the waiting for additional replies expires
                        break;
                    }
                    else if (retryCount-- == 0)
                    {
                        // Request timeout and no more retries
                        requestSource.SetResult(null);
                        _adapterRequests.Remove(adapterId);
                    }
                }
            }
            return await requestSource.Task.ConfigureAwait(false);
        }

        internal async ValueTask<IObjectPrx?> FindObjectAsync(Identity id)
        {
            TaskCompletionSource<IObjectPrx?>? requestSource;
            bool invoke = false;
            lock (_mutex)
            {
                if (!_objectRequests.TryGetValue(id, out requestSource))
                {
                    invoke = true;
                    requestSource = new TaskCompletionSource<IObjectPrx?>();
                    _objectRequests.Add(id, requestSource);
                }
            }

            while (invoke)
            {
                int retryCount = _retryCount;
                int lookupCount = _lookups.Count;
                int failureCount = 0;
                foreach ((ILookupPrx lookup, ILookupReplyPrx? reply) in _lookups)
                {
                    Debug.Assert(reply != null);
                    ILookupReplyPrx? lookupReply = reply.Clone(id, ILookupReplyPrx.Factory);

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

                            if (++failureCount == lookupCount)
                            {
                                requestSource.SetResult(null);
                                _objectRequests.Remove(id);
                            }
                        }
                    }
                }
                Task? t = await Task.WhenAny(requestSource.Task, Task.Delay(_timeout)).ConfigureAwait(false);
                if (t == requestSource.Task)
                {
                    break;
                }
                else if (retryCount-- == 0)
                {
                    // Request timeout and no more retries
                    requestSource.SetResult(null);
                    lock (_mutex)
                    {
                        _objectRequests.Remove(id);
                    }
                }
            }
            return await requestSource.Task.ConfigureAwait(false);
        }

        internal void FoundAdapter(string adapterId, IObjectPrx proxy, bool isReplicaGroup)
        {
            lock (_mutex)
            {
                if (_adapterRequests.TryGetValue(adapterId, out TaskCompletionSource<IObjectPrx?>? request))
                {
                    if (isReplicaGroup)
                    {
                        Console.WriteLine($"Replica group {adapterId} replay");
                        if (_replicaGroupReplies.TryGetValue(adapterId, out HashSet<IObjectPrx>? proxies))
                        {
                            proxies.Add(proxy);
                        }
                        else
                        {
                            proxies = new HashSet<IObjectPrx> { proxy };
                            _replicaGroupReplies.Add(adapterId, proxies);
                            int latency = 100 * LatencyMultiplier / 10000;
                            if (latency == 0)
                            {
                                latency = 1;
                            }
                            Task.Run(async () =>
                            {
                                await Task.Delay(latency);
                                var endpoints = new List<Endpoint>();
                                IObjectPrx result = proxies.First();
                                foreach (IObjectPrx prx in proxies)
                                {
                                    endpoints.AddRange(prx.Endpoints);
                                }
                                request.SetResult(result.Clone(endpoints: endpoints));
                                _replicaGroupReplies.Remove(adapterId);
                                _adapterRequests.Remove(adapterId);
                            });
                        }
                    }
                    else
                    {
                        Console.WriteLine($"found adapter {adapterId} replay");
                        request.SetResult(proxy);
                        _adapterRequests.Remove(adapterId);
                    }
                }
                // else ignore responses from old requests
            }
        }

        internal void FoundObject(Identity id, IObjectPrx proxy)
        {
            lock (_mutex)
            {
                if (_objectRequests.TryGetValue(id, out TaskCompletionSource<IObjectPrx?>? requestSource))
                {
                    requestSource.SetResult(proxy);
                    _objectRequests.Remove(id);
                }
                // else ignore responses from old requests
            }
        }

        internal void SetLookupReply(ILookupReplyPrx lookupReply)
        {
            // Use a lookup reply proxy whose address matches the interface used to send multicast datagrams.
            var single = new Endpoint[1];
            foreach (ILookupPrx key in _lookups.Keys.ToArray())
            {
                var endpoint = (UdpEndpoint)key.Endpoints[0];
                if (endpoint.McastInterface.Length > 0)
                {
                    Endpoint? q = lookupReply.Endpoints.FirstOrDefault(e =>
                        e is IPEndpoint ipEndpoint && ipEndpoint.Host.Equals(endpoint.McastInterface));

                    if (q != null)
                    {
                        single[0] = q;
                        _lookups[key] = lookupReply.Clone(endpoints: single);
                    }
                }

                if (_lookups[key] == null)
                {
                    // Fallback: just use the given lookup reply proxy if no matching endpoint found.
                    _lookups[key] = lookupReply;
                }
            }
        }
    }

    internal class LookupReply : ILookupReply
    {
        private readonly Lookup _lookup;

        public LookupReply(Lookup lookup) => _lookup = lookup;

        public void FoundObjectById(Identity id, IObjectPrx? proxy, Current c) =>
            _lookup.FoundObject(id, proxy!); // proxy cannot be null

        public void FoundAdapterById(string adapterId, IObjectPrx? proxy, bool isReplicaGroup, Current c) =>
            _lookup.FoundAdapter(adapterId, proxy!, isReplicaGroup); // proxy cannot be null
    }
}
