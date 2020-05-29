//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ZeroC.Ice;

namespace ZeroC.IceDiscovery
{
    internal class AdapterRequest : Request<string>, ITimerTask
    {
        // We use a HashSet because the same IceDiscovery plugin might return multiple times the same proxy if it's
        // accessible through multiple network interfaces and if we also sent the request to multiple interfaces.
        private long _latency;
        private readonly HashSet<IObjectPrx> _proxies = new HashSet<IObjectPrx>();
        private readonly long _start;

        public AdapterRequest(Lookup lookup, string id, int retryCount)
            : base(lookup, id, retryCount) => _start = DateTime.Now.Ticks;

        public override void Finished(IObjectPrx? proxy)
        {
            if (proxy != null || _proxies.Count == 0)
            {
                SendResponse(proxy);
            }
            else if (_proxies.Count == 1)
            {
                SendResponse(_proxies.First());
            }
            else
            {
                var endpoints = new List<Endpoint>();
                IObjectPrx? result = null;
                foreach (IObjectPrx prx in _proxies)
                {
                    if (result == null)
                    {
                        result = prx;
                    }
                    endpoints.AddRange(prx.Endpoints);
                }
                Debug.Assert(result != null);
                SendResponse(result.Clone(endpoints: endpoints));
            }
        }

        public bool Response(IObjectPrx proxy, bool isReplicaGroup)
        {
            if (isReplicaGroup)
            {
                _proxies.Add(proxy);
                if (_latency == 0)
                {
                    _latency = (long)((DateTime.Now.Ticks - _start) * Lookup.LatencyMultiplier / 10000.0);
                    if (_latency == 0)
                    {
                        _latency = 1; // 1ms
                    }
                    Lookup.Timer.Cancel(this);
                    Lookup.Timer.Schedule(this, _latency);
                }
                return false;
            }
            Finished(proxy);
            return true;
        }

        public override bool Retry() => _proxies.Count == 0 && --RetryCount >= 0;

        public void RunTimerTask() => Lookup.AdapterRequestTimedOut(this);

        protected override async Task InvokeWithLookup(string domainId, ILookupPrx lookup, ILookupReplyPrx lookupReply)
        {
            try
            {
                await lookup.FindAdapterByIdAsync(domainId, Id, lookupReply).ConfigureAwait(false);
            }
            catch (Exception ex)
            {
                Lookup.AdapterRequestException(this, ex);
            }
        }

        private void SendResponse(IObjectPrx? proxy)
        {
            foreach (TaskCompletionSource<IObjectPrx?> cb in Callbacks)
            {
                cb.SetResult(proxy);
            }
            Callbacks.Clear();
        }
    }

    internal class Lookup : ILookup
    {
        internal int LatencyMultiplier { get; }
        internal Timer Timer { get; }

        private readonly Dictionary<string, AdapterRequest> _adapterRequests =
            new Dictionary<string, AdapterRequest>();
        private readonly string _domainId;
        private readonly ILookupPrx _lookup;
        private readonly Dictionary<ILookupPrx, ILookupReplyPrx?> _lookups =
            new Dictionary<ILookupPrx, ILookupReplyPrx?>();
        private readonly object _mutex = new object();
        private readonly Dictionary<Identity, ObjectRequest> _objectRequests =
            new Dictionary<Identity, ObjectRequest>();
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

        internal void AdapterRequestException(AdapterRequest request, Exception ex)
        {
            lock (_mutex)
            {
                if (!_adapterRequests.TryGetValue(request.Id, out AdapterRequest? r) || r != request)
                {
                    return;
                }

                if (request.Exception())
                {
                    if (_warnOnce) // TODO remove this _warnOnce setting?
                    {
                        var s = new StringBuilder();
                        s.Append("failed to lookup adapter `");
                        s.Append(request.Id);
                        s.Append("' with lookup proxy `");
                        s.Append(_lookup);
                        s.Append("':\n");
                        s.Append(ex.ToString());
                        _lookup.Communicator.Logger.Warning(s.ToString());
                        _warnOnce = false;
                    }
                    Timer.Cancel(request);
                    _adapterRequests.Remove(request.Id);
                }
            }
        }

        internal void AdapterRequestTimedOut(AdapterRequest request)
        {
            lock (_mutex)
            {
                if (!_adapterRequests.TryGetValue(request.Id, out AdapterRequest? r) || r != request)
                {
                    return;
                }

                if (request.Retry())
                {
                    try
                    {
                        request.Invoke(_domainId, _lookups);
                        Timer.Schedule(request, _timeout);
                        return;
                    }
                    catch
                    {
                        // Ignore.
                    }
                }

                request.Finished(null);
                _adapterRequests.Remove(request.Id);
                Timer.Cancel(request);
            }
        }

        internal ValueTask<IObjectPrx?> FindAdapter(string adapterId)
        {
            lock (_mutex)
            {
                if (!_adapterRequests.TryGetValue(adapterId, out AdapterRequest? request))
                {
                    request = new AdapterRequest(this, adapterId, _retryCount);
                    _adapterRequests.Add(adapterId, request);
                }

                var task = new TaskCompletionSource<IObjectPrx?>();
                if (request.AddCallback(task))
                {
                    try
                    {
                        request.Invoke(_domainId, _lookups);
                        Timer.Schedule(request, _timeout);
                    }
                    catch
                    {
                        request.Finished(null);
                        _adapterRequests.Remove(adapterId);
                    }
                }
                return new ValueTask<IObjectPrx?>(task.Task);
            }
        }

        internal ValueTask<IObjectPrx?> FindObject(Identity id)
        {
            lock (_mutex)
            {
                if (!_objectRequests.TryGetValue(id, out ObjectRequest? request))
                {
                    request = new ObjectRequest(this, id, _retryCount);
                    _objectRequests.Add(id, request);
                }

                var task = new TaskCompletionSource<IObjectPrx?>();
                if (request.AddCallback(task))
                {
                    try
                    {
                        request.Invoke(_domainId, _lookups);
                        Timer.Schedule(request, _timeout);
                    }
                    catch
                    {
                        request.Finished(null);
                        _objectRequests.Remove(id);
                    }
                }
                return new ValueTask<IObjectPrx?>(task.Task);
            }
        }

        internal void FoundAdapter(string adapterId, string requestId, IObjectPrx proxy, bool isReplicaGroup)
        {
            lock (_mutex)
            {
                if (_adapterRequests.TryGetValue(adapterId, out AdapterRequest? request) && request.RequestId == requestId)
                {
                    if (request.Response(proxy, isReplicaGroup))
                    {
                        Timer.Cancel(request);
                        _adapterRequests.Remove(request.Id);
                    }
                }
                // else ignore responses from old requests
            }
        }

        internal void FoundObject(Identity id, string requestId, IObjectPrx proxy)
        {
            lock (_mutex)
            {
                if (_objectRequests.TryGetValue(id, out ObjectRequest? request) && request.RequestId == requestId)
                {
                    request.Response(proxy);
                    Timer.Cancel(request);
                    _objectRequests.Remove(id);
                }
                // else ignore responses from old requests
            }
        }

        internal void ObjectRequestException(ObjectRequest request, Exception ex)
        {
            lock (_mutex)
            {
                if (!_objectRequests.TryGetValue(request.Id, out ObjectRequest? r) || r != request)
                {
                    return;
                }

                if (request.Exception())
                {
                    if (_warnOnce) // TODO remove this _warnOnce setting?
                    {
                        var s = new StringBuilder();
                        s.Append("failed to lookup object `");
                        s.Append(request.Id.ToString(_lookup.Communicator.ToStringMode));
                        s.Append("' with lookup proxy `");
                        s.Append(_lookup);
                        s.Append("':\n");
                        s.Append(ex.ToString());
                        _lookup.Communicator.Logger.Warning(s.ToString());
                        _warnOnce = false;
                    }
                    Timer.Cancel(request);
                    _objectRequests.Remove(request.Id);
                }
            }
        }

        internal void ObjectRequestTimedOut(ObjectRequest request)
        {
            lock (_mutex)
            {
                if (!_objectRequests.TryGetValue(request.Id, out ObjectRequest? r) || r != request)
                {
                    return;
                }

                if (request.Retry())
                {
                    try
                    {
                        request.Invoke(_domainId, _lookups);
                        Timer.Schedule(request, _timeout);
                        return;
                    }
                    catch
                    {
                        // Ignore.
                    }
                }

                request.Finished(null);
                _objectRequests.Remove(request.Id);
                Timer.Cancel(request);
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
            _lookup.FoundObject(id, c.Identity.Name, proxy!); // proxy cannot be null

        public void FoundAdapterById(string adapterId, IObjectPrx? proxy, bool isReplicaGroup, Current c) =>
            _lookup.FoundAdapter(adapterId, c.Identity.Name, proxy!, isReplicaGroup); // proxy cannot be null
    }

    internal class ObjectRequest : Request<Identity>, ITimerTask
    {
        public void Response(IObjectPrx proxy) => Finished(proxy);

        public override void Finished(IObjectPrx? proxy)
        {
            foreach (TaskCompletionSource<IObjectPrx?> cb in Callbacks)
            {
                cb.SetResult(proxy);
            }
            Callbacks.Clear();
        }

        public void RunTimerTask() => Lookup.ObjectRequestTimedOut(this);

        protected override async Task InvokeWithLookup(string domainId, ILookupPrx lookup, ILookupReplyPrx lookupReply)
        {
            try
            {
                await lookup.FindObjectByIdAsync(domainId, Id, lookupReply).ConfigureAwait(false);
            }
            catch (Exception ex)
            {
                Lookup.ObjectRequestException(this, ex);
            }
        }

        internal ObjectRequest(Lookup lookup, Identity id, int retryCount)
            : base(lookup, id, retryCount)
        {
        }

    }

    internal abstract class Request<T>
    {
        public T Id { get; }
        public string RequestId { get; }

        protected List<TaskCompletionSource<IObjectPrx?>> Callbacks = new List<TaskCompletionSource<IObjectPrx?>>();
        protected int FailureCount;
        protected Lookup Lookup;
        protected int LookupCount;
        protected int RetryCount;

        public bool AddCallback(TaskCompletionSource<IObjectPrx?> cb)
        {
            Callbacks.Add(cb);
            return Callbacks.Count == 1;
        }

        public bool Exception()
        {
            if (++FailureCount == LookupCount)
            {
                Finished(null);
                return true;
            }
            return false;
        }

        public abstract void Finished(IObjectPrx? proxy);

        public void Invoke(string domainId, Dictionary<ILookupPrx, ILookupReplyPrx?> lookups)
        {
            LookupCount = lookups.Count;
            FailureCount = 0;
            var identity = new Identity(RequestId, "");
            foreach (KeyValuePair<ILookupPrx, ILookupReplyPrx?> entry in lookups)
            {
                InvokeWithLookup(domainId, entry.Key, entry.Value!.Clone(identity, ILookupReplyPrx.Factory));
            }
        }

        public virtual bool Retry() => --RetryCount >= 0;

        protected Request(Lookup lookup, T id, int retryCount)
        {
            Lookup = lookup;
            RetryCount = retryCount;
            Id = id;
            RequestId = Guid.NewGuid().ToString();
        }

        protected abstract Task InvokeWithLookup(string domainId, ILookupPrx lookup, ILookupReplyPrx lookupReply);
    }
}
