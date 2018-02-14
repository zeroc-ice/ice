// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceDiscovery
{
    using System;
    using System.Collections.Generic;
    using System.Threading.Tasks;
    using System.Text;
    using System.Diagnostics;
    using System.Linq;

    abstract class Request<T>
    {
        protected Request(LookupI lookup, T id, int retryCount)
        {
            lookup_ = lookup;
            retryCount_ = retryCount;
            _id = id;
            _requestId = Guid.NewGuid().ToString();
        }

        public T getId()
        {
            return _id;
        }

        public bool addCallback(TaskCompletionSource<Ice.ObjectPrx> cb)
        {
            callbacks_.Add(cb);
            return callbacks_.Count == 1;
        }

        public virtual bool retry()
        {
            return --retryCount_ >= 0;
        }

        public void invoke(String domainId, Dictionary<LookupPrx, LookupReplyPrx> lookups)
        {
            _lookupCount = lookups.Count;
            _failureCount = 0;
            Ice.Identity id = new Ice.Identity(_requestId, "");
            foreach(var entry in lookups)
            {
                invokeWithLookup(domainId,
                                 entry.Key,
                                 LookupReplyPrxHelper.uncheckedCast(entry.Value.ice_identity(id)));
            }
        }

        public bool exception()
        {
            if(++_failureCount == _lookupCount)
            {
                finished(null);
                return true;
            }
            return false;
        }

        public string getRequestId()
        {
            return _requestId;
        }

        abstract public void finished(Ice.ObjectPrx proxy);

        abstract protected void invokeWithLookup(string domainId, LookupPrx lookup, LookupReplyPrx lookupReply);

        private string _requestId;

        protected LookupI lookup_;
        protected int retryCount_;
        protected int _lookupCount;
        protected int _failureCount;
        protected List<TaskCompletionSource<Ice.ObjectPrx>> callbacks_ = new List<TaskCompletionSource<Ice.ObjectPrx>>();

        protected T _id;
    };

    class AdapterRequest : Request<string>, IceInternal.TimerTask
    {
        public AdapterRequest(LookupI lookup, string id, int retryCount) : base(lookup, id, retryCount)
        {
            _start = DateTime.Now.Ticks;
        }

        public override bool retry()
        {
            return _proxies.Count == 0 && --retryCount_ >= 0;
        }

        public bool response(Ice.ObjectPrx proxy, bool isReplicaGroup)
        {
            if(isReplicaGroup)
            {
                _proxies.Add(proxy);
                if(_latency == 0)
                {
                    _latency = (long)((DateTime.Now.Ticks - _start) * lookup_.latencyMultiplier() / 10000.0);
                    if(_latency == 0)
                    {
                        _latency = 1; // 1ms
                    }
                    lookup_.timer().cancel(this);
                    lookup_.timer().schedule(this, _latency);
                }
                return false;
            }
            finished(proxy);
            return true;
        }

        public override void finished(Ice.ObjectPrx proxy)
        {
            if(proxy != null || _proxies.Count == 0)
            {
                sendResponse(proxy);
            }
            else if(_proxies.Count == 1)
            {
                sendResponse(_proxies.First());
            }
            else
            {
                List<Ice.Endpoint> endpoints = new List<Ice.Endpoint>();
                Ice.ObjectPrx result = null;
                foreach(Ice.ObjectPrx prx in _proxies)
                {
                    if(result == null)
                    {
                        result = prx;
                    }
                    endpoints.AddRange(prx.ice_getEndpoints());
                }
                sendResponse(result.ice_endpoints(endpoints.ToArray()));
            }
        }

        public void runTimerTask()
        {
            lookup_.adapterRequestTimedOut(this);
        }

        protected override void invokeWithLookup(string domainId, LookupPrx lookup, LookupReplyPrx lookupReply)
        {
            lookup.findAdapterByIdAsync(domainId, _id, lookupReply).ContinueWith(task => {
                try
                {
                    task.Wait();
                }
                catch(AggregateException ex)
                {
                    lookup_.adapterRequestException(this, ex.InnerException);
                }
            });
        }

        private void sendResponse(Ice.ObjectPrx proxy)
        {
            foreach(var cb in callbacks_)
            {
                cb.SetResult(proxy);
            }
            callbacks_.Clear();
        }

        //
        // We use a HashSet because the same IceDiscovery plugin might return multiple times
        // the same proxy if it's accessible through multiple network interfaces and if we
        // also sent the request to multiple interfaces.
        //
        private HashSet<Ice.ObjectPrx> _proxies = new HashSet<Ice.ObjectPrx>();
        private long _start;
        private long _latency;
    };

    class ObjectRequest : Request<Ice.Identity>, IceInternal.TimerTask
    {
        public ObjectRequest(LookupI lookup, Ice.Identity id, int retryCount) : base(lookup, id, retryCount)
        {
        }

        public void response(Ice.ObjectPrx proxy)
        {
            finished(proxy);
        }

        public override void finished(Ice.ObjectPrx proxy)
        {
            foreach(var cb in callbacks_)
            {
                cb.SetResult(proxy);
            }
            callbacks_.Clear();
        }

        public void runTimerTask()
        {
            lookup_.objectRequestTimedOut(this);
        }

        protected override void invokeWithLookup(string domainId, LookupPrx lookup, LookupReplyPrx lookupReply)
        {
            lookup.findObjectByIdAsync(domainId, _id, lookupReply).ContinueWith(task => {
                try
                {
                    task.Wait();
                }
                catch(AggregateException ex)
                {
                    lookup_.objectRequestException(this, ex.InnerException);
                }
            });
        }
    };

    class LookupI : LookupDisp_
    {
        public LookupI(LocatorRegistryI registry, LookupPrx lookup, Ice.Properties properties)
        {
            _registry = registry;
            _lookup = lookup;
            _timeout = properties.getPropertyAsIntWithDefault("IceDiscovery.Timeout", 300);
            _retryCount = properties.getPropertyAsIntWithDefault("IceDiscovery.RetryCount", 3);
            _latencyMultiplier = properties.getPropertyAsIntWithDefault("IceDiscovery.LatencyMultiplier", 1);
            _domainId = properties.getProperty("IceDiscovery.DomainId");
            _timer = IceInternal.Util.getInstance(lookup.ice_getCommunicator()).timer();

            //
            // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast
            // datagram on each endpoint.
            //
            var single = new Ice.Endpoint[1];
            foreach(var endpt in lookup.ice_getEndpoints())
            {
                single[0] = endpt;
                _lookups[(LookupPrx)lookup.ice_endpoints(single)] = null;
            }
            Debug.Assert(_lookups.Count > 0);
        }

        public void setLookupReply(LookupReplyPrx lookupReply)
        {
            //
            // Use a lookup reply proxy whose adress matches the interface used to send multicast datagrams.
            //
            var single = new Ice.Endpoint[1];
            foreach(var key in new List<LookupPrx>(_lookups.Keys))
            {
                var info = (Ice.UDPEndpointInfo)key.ice_getEndpoints()[0].getInfo();
                if(info.mcastInterface.Length > 0)
                {
                    foreach(var q in lookupReply.ice_getEndpoints())
                    {
                        var r = q.getInfo();
                        if(r is Ice.IPEndpointInfo && ((Ice.IPEndpointInfo)r).host.Equals(info.mcastInterface))
                        {
                            single[0] = q;
                            _lookups[key] = (LookupReplyPrx)lookupReply.ice_endpoints(single);
                        }
                    }
                }

                if(_lookups[key] == null)
                {
                    // Fallback: just use the given lookup reply proxy if no matching endpoint found.
                    _lookups[key] = lookupReply;
                }
            }
        }

        public override void findObjectById(string domainId, Ice.Identity id, LookupReplyPrx reply,
                                            Ice.Current current)
        {
            if(!domainId.Equals(_domainId))
            {
                return; // Ignore
            }

            Ice.ObjectPrx proxy = _registry.findObject(id);
            if(proxy != null)
            {
                //
                // Reply to the mulicast request using the given proxy.
                //
                try
                {
                    reply.foundObjectByIdAsync(id, proxy);
                }
                catch(Ice.LocalException)
                {
                    // Ignore.
                }
            }
        }

        public override void findAdapterById(string domainId, string adapterId, LookupReplyPrx reply,
                                             Ice.Current current)
        {
            if(!domainId.Equals(_domainId))
            {
                return; // Ignore
            }

            bool isReplicaGroup;
            Ice.ObjectPrx proxy = _registry.findAdapter(adapterId, out isReplicaGroup);
            if(proxy != null)
            {
                //
                // Reply to the multicast request using the given proxy.
                //
                try
                {
                    reply.foundAdapterByIdAsync(adapterId, proxy, isReplicaGroup);
                }
                catch(Ice.LocalException)
                {
                    // Ignore.
                }
            }
        }

        internal Task<Ice.ObjectPrx> findObject(Ice.Identity id)
        {
            lock(this)
            {
                ObjectRequest request;
                if(!_objectRequests.TryGetValue(id, out request))
                {
                    request = new ObjectRequest(this, id, _retryCount);
                    _objectRequests.Add(id, request);
                }

                var task = new TaskCompletionSource<Ice.ObjectPrx>();
                if(request.addCallback(task))
                {
                    try
                    {
                        request.invoke(_domainId, _lookups);
                        _timer.schedule(request, _timeout);
                    }
                    catch(Ice.LocalException)
                    {
                        request.finished(null);
                        _objectRequests.Remove(id);
                    }
                }
                return task.Task;
            }
        }

        internal Task<Ice.ObjectPrx> findAdapter(string adapterId)
        {
            lock(this)
            {
                AdapterRequest request;
                if(!_adapterRequests.TryGetValue(adapterId, out request))
                {
                    request = new AdapterRequest(this, adapterId, _retryCount);
                    _adapterRequests.Add(adapterId, request);
                }

                var task = new TaskCompletionSource<Ice.ObjectPrx>();
                if(request.addCallback(task))
                {
                    try
                    {
                        request.invoke(_domainId, _lookups);
                        _timer.schedule(request, _timeout);
                    }
                    catch(Ice.LocalException)
                    {
                        request.finished(null);
                        _adapterRequests.Remove(adapterId);
                    }
                }
                return task.Task;
            }
        }

        internal void foundObject(Ice.Identity id, string requestId, Ice.ObjectPrx proxy)
        {
            lock(this)
            {
                ObjectRequest request;
                if(_objectRequests.TryGetValue(id, out request) && request.getRequestId() == requestId)
                {
                    request.response(proxy);
                    _timer.cancel(request);
                    _objectRequests.Remove(id);
                }
                // else ignore responses from old requests
            }
        }

        internal void foundAdapter(string adapterId, string requestId, Ice.ObjectPrx proxy, bool isReplicaGroup)
        {
            lock(this)
            {
                AdapterRequest request;
                if(_adapterRequests.TryGetValue(adapterId, out request) && request.getRequestId() == requestId)
                {
                    if(request.response(proxy, isReplicaGroup))
                    {
                        _timer.cancel(request);
                        _adapterRequests.Remove(request.getId());
                    }
                }
                // else ignore responses from old requests
            }
        }

        internal void objectRequestTimedOut(ObjectRequest request)
        {
            lock(this)
            {
                ObjectRequest r;
                if(!_objectRequests.TryGetValue(request.getId(), out r) || r != request)
                {
                    return;
                }

                if(request.retry())
                {
                    try
                    {
                        request.invoke(_domainId, _lookups);
                        _timer.schedule(request, _timeout);
                        return;
                    }
                    catch(Ice.LocalException)
                    {
                    }
                }

                request.finished(null);
                _objectRequests.Remove(request.getId());
                _timer.cancel(request);
            }
        }

        internal void objectRequestException(ObjectRequest request, Exception ex)
        {
            lock(this)
            {
                ObjectRequest r;
                if(!_objectRequests.TryGetValue(request.getId(), out r) || r != request)
                {
                    return;
                }

                if(request.exception())
                {
                    if(_warnOnce)
                    {
                        StringBuilder s = new StringBuilder();
                        s.Append("failed to lookup object `");
                        s.Append(_lookup.ice_getCommunicator().identityToString(request.getId()));
                        s.Append("' with lookup proxy `");
                        s.Append(_lookup);
                        s.Append("':\n");
                        s.Append(ex.ToString());
                        _lookup.ice_getCommunicator().getLogger().warning(s.ToString());
                        _warnOnce = false;
                    }
                    _timer.cancel(request);
                    _objectRequests.Remove(request.getId());
                }
            }
        }

        internal void adapterRequestTimedOut(AdapterRequest request)
        {
            lock(this)
            {
                AdapterRequest r;
                if(!_adapterRequests.TryGetValue(request.getId(), out r) || r != request)
                {
                    return;
                }

                if(request.retry())
                {
                    try
                    {
                        request.invoke(_domainId, _lookups);
                        _timer.schedule(request, _timeout);
                        return;
                    }
                    catch(Ice.LocalException)
                    {
                    }
                }

                request.finished(null);
                _adapterRequests.Remove(request.getId());
                _timer.cancel(request);
            }
        }

        internal void adapterRequestException(AdapterRequest request, Exception ex)
        {
            lock(this)
            {
                AdapterRequest r;
                if(!_adapterRequests.TryGetValue(request.getId(), out r) || r != request)
                {
                    return;
                }

                if(request.exception())
                {
                    if(_warnOnce)
                    {
                        StringBuilder s = new StringBuilder();
                        s.Append("failed to lookup adapter `");
                        s.Append(request.getId());
                        s.Append("' with lookup proxy `");
                        s.Append(_lookup);
                        s.Append("':\n");
                        s.Append(ex.ToString());
                        _lookup.ice_getCommunicator().getLogger().warning(s.ToString());
                        _warnOnce = false;
                    }
                    _timer.cancel(request);
                    _adapterRequests.Remove(request.getId());
                }
            }
        }

        internal IceInternal.Timer timer()
        {
            return _timer;
        }

        internal int latencyMultiplier()
        {
            return _latencyMultiplier;
        }

        private LocatorRegistryI _registry;
        private LookupPrx _lookup;
        private Dictionary<LookupPrx, LookupReplyPrx> _lookups = new Dictionary<LookupPrx, LookupReplyPrx>();
        private readonly int _timeout;
        private readonly int _retryCount;
        private readonly int _latencyMultiplier;
        private readonly string _domainId;

        private IceInternal.Timer _timer;
        private bool _warnOnce = true;
        private Dictionary<Ice.Identity, ObjectRequest> _objectRequests = new Dictionary<Ice.Identity, ObjectRequest>();
        private Dictionary<string, AdapterRequest> _adapterRequests = new Dictionary<string, AdapterRequest>();
    };

    class LookupReplyI : LookupReplyDisp_
    {
        public LookupReplyI(LookupI lookup)
        {
            _lookup = lookup;
        }

        public override void foundObjectById(Ice.Identity id, Ice.ObjectPrx proxy, Ice.Current c)
        {
            _lookup.foundObject(id, c.id.name, proxy);
        }

        public override void foundAdapterById(string adapterId, Ice.ObjectPrx proxy, bool isReplicaGroup, Ice.Current c)
        {
            _lookup.foundAdapter(adapterId, c.id.name, proxy, isReplicaGroup);
        }

        private LookupI _lookup;
    };

}
