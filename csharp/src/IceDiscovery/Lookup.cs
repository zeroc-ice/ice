//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Ice;

namespace IceDiscovery
{
    internal abstract class Request<T>
    {
        protected Request(Lookup lookup, T id, int retryCount)
        {
            Lookup = lookup;
            RetryCount = retryCount;
            Id = id;
            _requestId = Guid.NewGuid().ToString();
        }

        public T GetId() => Id;

        public bool AddCallback(TaskCompletionSource<IObjectPrx?> cb)
        {
            Callbacks.Add(cb);
            return Callbacks.Count == 1;
        }

        public virtual bool Retry() => --RetryCount >= 0;

        public void Invoke(string domainId, Dictionary<ILookupPrx, ILookupReplyPrx?> lookups)
        {
            LookupCount = lookups.Count;
            FailureCount = 0;
            var identity = new Identity(_requestId, "");
            foreach (KeyValuePair<ILookupPrx, ILookupReplyPrx?> entry in lookups)
            {
                InvokeWithLookup(domainId, entry.Key, entry.Value!.Clone(identity, ILookupReplyPrx.Factory));
            }
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

        public string GetRequestId() => _requestId;

        public abstract void Finished(IObjectPrx? proxy);

        protected abstract void InvokeWithLookup(string domainId, ILookupPrx lookup, ILookupReplyPrx lookupReply);

        private readonly string _requestId;

        protected Lookup Lookup;
        protected int RetryCount;
        protected int LookupCount;
        protected int FailureCount;
        protected List<TaskCompletionSource<IObjectPrx?>> Callbacks = new List<TaskCompletionSource<IObjectPrx?>>();

        protected T Id;
    }

    internal class AdapterRequest : Request<string>, IceInternal.ITimerTask
    {
        public AdapterRequest(Lookup lookup, string id, int retryCount)
            : base(lookup, id, retryCount) => _start = DateTime.Now.Ticks;

        public override bool Retry() => _proxies.Count == 0 && --RetryCount >= 0;

        public bool Response(IObjectPrx proxy, bool isReplicaGroup)
        {
            if (isReplicaGroup)
            {
                _proxies.Add(proxy);
                if (_latency == 0)
                {
                    _latency = (long)((DateTime.Now.Ticks - _start) * Lookup.LatencyMultiplier() / 10000.0);
                    if (_latency == 0)
                    {
                        _latency = 1; // 1ms
                    }
                    Lookup.Timer().Cancel(this);
                    Lookup.Timer().Schedule(this, _latency);
                }
                return false;
            }
            Finished(proxy);
            return true;
        }

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

        public void RunTimerTask() => Lookup.AdapterRequestTimedOut(this);

        protected override void InvokeWithLookup(string domainId, ILookupPrx lookup, ILookupReplyPrx lookupReply)
        {
            lookup.FindAdapterByIdAsync(domainId, Id, lookupReply).ContinueWith(task =>
            {
                try
                {
                    task.Wait();
                }
                catch (AggregateException ex)
                {
                    Debug.Assert(ex.InnerException != null);
                    Lookup.AdapterRequestException(this, ex.InnerException);
                }
            }, lookup.Scheduler);
        }

        private void SendResponse(IObjectPrx? proxy)
        {
            foreach (TaskCompletionSource<IObjectPrx?> cb in Callbacks)
            {
                cb.SetResult(proxy);
            }
            Callbacks.Clear();
        }

        //
        // We use a HashSet because the same IceDiscovery plugin might return multiple times
        // the same proxy if it's accessible through multiple network interfaces and if we
        // also sent the request to multiple interfaces.
        //
        private readonly HashSet<IObjectPrx> _proxies = new HashSet<IObjectPrx>();
        private readonly long _start;
        private long _latency;
    }

    internal class ObjectRequest : Request<Identity>, IceInternal.ITimerTask
    {
        public ObjectRequest(Lookup lookup, Identity id, int retryCount)
            : base(lookup, id, retryCount)
        {
        }

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

        protected override void InvokeWithLookup(string domainId, ILookupPrx lookup, ILookupReplyPrx lookupReply)
        {
            lookup.FindObjectByIdAsync(domainId, Id, lookupReply).ContinueWith(task =>
            {
                try
                {
                    task.Wait();
                }
                catch (AggregateException ex)
                {
                    Debug.Assert(ex.InnerException != null);
                    Lookup.ObjectRequestException(this, ex.InnerException);
                }
            }, lookup.Scheduler);
        }
    }

    internal class Lookup : ILookup
    {
        public Lookup(LocatorRegistry registry, ILookupPrx lookup, Communicator communicator)
        {
            _registry = registry;
            _lookup = lookup;
            _timeout = communicator.GetPropertyAsInt("IceDiscovery.Timeout") ?? 300;
            _retryCount = communicator.GetPropertyAsInt("IceDiscovery.RetryCount") ?? 3;
            _latencyMultiplier = communicator.GetPropertyAsInt("IceDiscovery.LatencyMultiplier") ?? 1;
            _domainId = communicator.GetProperty("IceDiscovery.DomainId") ?? "";
            _timer = lookup.Communicator.Timer();

            //
            // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast
            // datagram on each endpoint.
            //
            var single = new Ice.Endpoint[1];
            foreach (Endpoint endpt in lookup.Endpoints)
            {
                single[0] = endpt;
                _lookups[lookup.Clone(endpoints: single)] = null;
            }
            Debug.Assert(_lookups.Count > 0);
        }

        public void SetLookupReply(ILookupReplyPrx lookupReply)
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

        public void FindObjectById(string domainId, Identity id, ILookupReplyPrx? reply, Current current)
        {
            if (!domainId.Equals(_domainId))
            {
                return; // Ignore
            }

            IObjectPrx? proxy = _registry.FindObject(id);
            if (proxy != null)
            {
                //
                // Reply to the mulicast request using the given proxy.
                //
                try
                {
                    Debug.Assert(reply != null);
                    reply.FoundObjectByIdAsync(id, proxy);
                }
                catch (System.Exception)
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
                //
                // Reply to the multicast request using the given proxy.
                //
                try
                {
                    Debug.Assert(reply != null);
                    reply.FoundAdapterByIdAsync(adapterId, proxy, isReplicaGroup);
                }
                catch (System.Exception)
                {
                    // Ignore.
                }
            }
        }

        internal ValueTask<IObjectPrx?> FindObject(Identity id)
        {
            lock (this)
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
                        _timer.Schedule(request, _timeout);
                    }
                    catch (System.Exception)
                    {
                        request.Finished(null);
                        _objectRequests.Remove(id);
                    }
                }
                return new ValueTask<IObjectPrx?>(task.Task);
            }
        }

        internal ValueTask<IObjectPrx?> FindAdapter(string adapterId)
        {
            lock (this)
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
                        _timer.Schedule(request, _timeout);
                    }
                    catch (System.Exception)
                    {
                        request.Finished(null);
                        _adapterRequests.Remove(adapterId);
                    }
                }
                return new ValueTask<IObjectPrx?>(task.Task);
            }
        }

        internal void FoundObject(Identity id, string requestId, IObjectPrx proxy)
        {
            lock (this)
            {
                if (_objectRequests.TryGetValue(id, out ObjectRequest? request) && request.GetRequestId() == requestId)
                {
                    request.Response(proxy);
                    _timer.Cancel(request);
                    _objectRequests.Remove(id);
                }
                // else ignore responses from old requests
            }
        }

        internal void FoundAdapter(string adapterId, string requestId, IObjectPrx proxy, bool isReplicaGroup)
        {
            lock (this)
            {
                if (_adapterRequests.TryGetValue(adapterId, out AdapterRequest? request) && request.GetRequestId() == requestId)
                {
                    if (request.Response(proxy, isReplicaGroup))
                    {
                        _timer.Cancel(request);
                        _adapterRequests.Remove(request.GetId());
                    }
                }
                // else ignore responses from old requests
            }
        }

        internal void ObjectRequestTimedOut(ObjectRequest request)
        {
            lock (this)
            {
                if (!_objectRequests.TryGetValue(request.GetId(), out ObjectRequest? r) || r != request)
                {
                    return;
                }

                if (request.Retry())
                {
                    try
                    {
                        request.Invoke(_domainId, _lookups);
                        _timer.Schedule(request, _timeout);
                        return;
                    }
                    catch (System.Exception)
                    {
                    }
                }

                request.Finished(null);
                _objectRequests.Remove(request.GetId());
                _timer.Cancel(request);
            }
        }

        internal void ObjectRequestException(ObjectRequest request, Exception ex)
        {
            lock (this)
            {
                if (!_objectRequests.TryGetValue(request.GetId(), out ObjectRequest? r) || r != request)
                {
                    return;
                }

                if (request.Exception())
                {
                    if (_warnOnce)
                    {
                        var s = new StringBuilder();
                        s.Append("failed to lookup object `");
                        s.Append(request.GetId().ToString(_lookup.Communicator.ToStringMode));
                        s.Append("' with lookup proxy `");
                        s.Append(_lookup);
                        s.Append("':\n");
                        s.Append(ex.ToString());
                        _lookup.Communicator.Logger.Warning(s.ToString());
                        _warnOnce = false;
                    }
                    _timer.Cancel(request);
                    _objectRequests.Remove(request.GetId());
                }
            }
        }

        internal void AdapterRequestTimedOut(AdapterRequest request)
        {
            lock (this)
            {
                if (!_adapterRequests.TryGetValue(request.GetId(), out AdapterRequest? r) || r != request)
                {
                    return;
                }

                if (request.Retry())
                {
                    try
                    {
                        request.Invoke(_domainId, _lookups);
                        _timer.Schedule(request, _timeout);
                        return;
                    }
                    catch (System.Exception)
                    {
                    }
                }

                request.Finished(null);
                _adapterRequests.Remove(request.GetId());
                _timer.Cancel(request);
            }
        }

        internal void AdapterRequestException(AdapterRequest request, Exception ex)
        {
            lock (this)
            {
                if (!_adapterRequests.TryGetValue(request.GetId(), out AdapterRequest? r) || r != request)
                {
                    return;
                }

                if (request.Exception())
                {
                    if (_warnOnce)
                    {
                        var s = new StringBuilder();
                        s.Append("failed to lookup adapter `");
                        s.Append(request.GetId());
                        s.Append("' with lookup proxy `");
                        s.Append(_lookup);
                        s.Append("':\n");
                        s.Append(ex.ToString());
                        _lookup.Communicator.Logger.Warning(s.ToString());
                        _warnOnce = false;
                    }
                    _timer.Cancel(request);
                    _adapterRequests.Remove(request.GetId());
                }
            }
        }

        internal IceInternal.Timer Timer() => _timer;

        internal int LatencyMultiplier() => _latencyMultiplier;

        private readonly LocatorRegistry _registry;
        private readonly ILookupPrx _lookup;
        private readonly Dictionary<ILookupPrx, ILookupReplyPrx?> _lookups = new Dictionary<ILookupPrx, ILookupReplyPrx?>();
        private readonly int _timeout;
        private readonly int _retryCount;
        private readonly int _latencyMultiplier;
        private readonly string _domainId;

        private readonly IceInternal.Timer _timer;
        private bool _warnOnce = true;
        private readonly Dictionary<Identity, ObjectRequest> _objectRequests = new Dictionary<Identity, ObjectRequest>();
        private readonly Dictionary<string, AdapterRequest> _adapterRequests = new Dictionary<string, AdapterRequest>();
    }

    internal class LookupReply : ILookupReply
    {
        public LookupReply(Lookup lookup) => _lookup = lookup;

        public void FoundObjectById(Identity id, IObjectPrx? proxy, Current c)
            => _lookup.FoundObject(id, c.Identity.Name, proxy!); // proxy cannot be null

        public void FoundAdapterById(string adapterId, IObjectPrx? proxy, bool isReplicaGroup, Current c) =>
            _lookup.FoundAdapter(adapterId, c.Identity.Name, proxy!, isReplicaGroup); // proxy cannot be null

        private readonly Lookup _lookup;
    }
}
