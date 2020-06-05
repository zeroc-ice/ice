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
    internal class AdapterRequest
    {
        internal CancellationTokenSource CancellationSource { get; }
        internal string RequestId { get; }
        internal TaskCompletionSource<IObjectPrx?> RequestSource { get; }

        internal readonly HashSet<IObjectPrx> Replies = new HashSet<IObjectPrx>();

        internal AdapterRequest()
        {
            CancellationSource = new CancellationTokenSource();
            RequestId = Guid.NewGuid().ToString();
            RequestSource = new TaskCompletionSource<IObjectPrx?>();
        }
    }

    internal class ObjectRequest
    {
        internal string RequestId { get; }
        internal TaskCompletionSource<IObjectPrx?> RequestSource { get; }
        internal ObjectRequest()
        {
            RequestId = Guid.NewGuid().ToString();
            RequestSource = new TaskCompletionSource<IObjectPrx?>();
        }
    }

    internal class Lookup : ILookup
    {
        private readonly Dictionary<string, AdapterRequest> _adapterRequests =
            new Dictionary<string, AdapterRequest>();
        private readonly string _domainId;
        private readonly int _latencyMultiplier;
        private readonly ILookupPrx _lookup;
        private readonly Dictionary<ILookupPrx, ILookupReplyPrx> _lookups =
            new Dictionary<ILookupPrx, ILookupReplyPrx>();
        private readonly object _mutex = new object();
        private readonly Dictionary<Identity, ObjectRequest> _objectRequests =
            new Dictionary<Identity, ObjectRequest>();
        private readonly LocatorRegistry _registry;
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
            ILookupReplyPrx lookupReply)
        {
            _registry = registry;
            _lookup = lookup;
            _timeout = communicator.GetPropertyAsInt("IceDiscovery.Timeout") ?? 300;
            _retryCount = communicator.GetPropertyAsInt("IceDiscovery.RetryCount") ?? 3;
            _latencyMultiplier = communicator.GetPropertyAsInt("IceDiscovery.LatencyMultiplier") ?? 1;
            _domainId = communicator.GetProperty("IceDiscovery.DomainId") ?? "";

            // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast
            // datagram on each endpoint.
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
            AdapterRequest? request;
            bool invoke = false;
            lock (_mutex)
            {
                if (!_adapterRequests.TryGetValue(adapterId, out request))
                {
                    invoke = true;
                    request = new AdapterRequest();
                    _adapterRequests.Add(adapterId, request);
                }
            }

            int retryCount = _retryCount;
            int failureCount = 0;
            var requestId = new Identity(request.RequestId, "");
            long start = DateTime.Now.Ticks;
            while (invoke)
            {
                foreach ((ILookupPrx lookup, ILookupReplyPrx? reply) in _lookups)
                {
                    Debug.Assert(reply != null);
                    ILookupReplyPrx? lookupReply = reply.Clone(requestId, ILookupReplyPrx.Factory);
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
                                request.RequestSource.SetResult(null);
                                _adapterRequests.Remove(adapterId);
                            }
                        }
                    }
                }
                Task? t = await Task.WhenAny(request.RequestSource.Task,
                    Task.Delay(_timeout, request.CancellationSource.Token)).ConfigureAwait(false);

                if (t == request.RequestSource.Task)
                {
                    break;
                }
                else if (t.IsCanceled && request.Replies.Count > 0)
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
                    IObjectPrx result = request.Replies.First();
                    foreach (IObjectPrx prx in request.Replies)
                    {
                        endpoints.AddRange(prx.Endpoints);
                    }
                    request.RequestSource.SetResult(result.Clone(endpoints: endpoints));
                    lock (_mutex)
                    {
                        _adapterRequests.Remove(adapterId);
                    }
                    break;
                }
                else if (--retryCount < 0)
                {
                    // The request timeout and no more retries
                    request.RequestSource.SetResult(null);
                    lock (_mutex)
                    {
                        _adapterRequests.Remove(adapterId);
                    }
                    break;
                }
            }
            return await request.RequestSource.Task.ConfigureAwait(false);
        }

        internal async ValueTask<IObjectPrx?> FindObjectAsync(Identity id)
        {
            ObjectRequest? request;
            bool invoke = false;
            lock (_mutex)
            {
                if (!_objectRequests.TryGetValue(id, out request))
                {
                    invoke = true;
                    request = new ObjectRequest();
                    _objectRequests.Add(id, request);
                }
            }

            int retryCount = _retryCount;
            int failureCount = 0;

            var requestId = new Identity(request.RequestId, "");
            while (invoke)
            {
                foreach ((ILookupPrx lookup, ILookupReplyPrx? reply) in _lookups)
                {
                    Debug.Assert(reply != null);
                    ILookupReplyPrx? lookupReply = reply.Clone(requestId, ILookupReplyPrx.Factory);

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
                                request.RequestSource.SetResult(null);
                                _objectRequests.Remove(id);
                            }
                        }
                    }
                }
                Task? t = await Task.WhenAny(request.RequestSource.Task, Task.Delay(_timeout)).ConfigureAwait(false);
                lock (_mutex)
                {
                    if (t == request.RequestSource.Task)
                    {
                        break;
                    }
                    else if (--retryCount < 0)
                    {
                        // Request timeout and no more retries
                        request.RequestSource.SetResult(null);
                        _objectRequests.Remove(id);
                        break;
                    }
                }
            }
            return await request.RequestSource.Task.ConfigureAwait(false);
        }

        internal void FoundAdapter(string adapterId, string requestId, IObjectPrx proxy, bool isReplicaGroup)
        {
            lock (_mutex)
            {
                if (_adapterRequests.TryGetValue(adapterId, out AdapterRequest? request) &&
                    request.RequestId == requestId)
                {
                    if (isReplicaGroup)
                    {
                        request.Replies.Add(proxy);
                        if (request.Replies.Count == 1)
                        {
                            request.CancellationSource.Cancel();
                        }
                    }
                    else
                    {
                        request.RequestSource.SetResult(proxy);
                        _adapterRequests.Remove(adapterId);
                    }
                }
                // else ignore responses from old requests
            }
        }

        internal void FoundObject(Identity id, string requestId, IObjectPrx proxy)
        {
            lock (_mutex)
            {
                if (_objectRequests.TryGetValue(id, out ObjectRequest? request) &&
                    request.RequestId == requestId)
                {
                    request.RequestSource.SetResult(proxy);
                    _objectRequests.Remove(id);
                }
                // else ignore responses from old requests
            }
        }
    }

    internal class LookupReply : ILookupReply
    {
        private readonly Lookup _lookup;

        public LookupReply(Lookup lookup) => _lookup = lookup;

        public void FoundObjectById(Identity id, IObjectPrx? proxy, Current current) =>
            _lookup.FoundObject(id, current.Identity.Name, proxy!);

        public void FoundAdapterById(string adapterId, IObjectPrx? proxy, bool isReplicaGroup, Current current) =>
            _lookup.FoundAdapter(adapterId, current.Identity.Name, proxy!, isReplicaGroup);
    }
}
