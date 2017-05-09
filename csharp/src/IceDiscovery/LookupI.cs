// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

    class Request<T>
    {
        protected Request(LookupI lookup, T id, int retryCount)
        {
            lookup_ = lookup;
            nRetry_ = retryCount;
            _id = id;
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
            return --nRetry_ >= 0;
        }

        protected LookupI lookup_;
        protected int nRetry_;
        protected List<TaskCompletionSource<Ice.ObjectPrx>> callbacks_ = new List<TaskCompletionSource<Ice.ObjectPrx>>();

        private T _id;
    };

    class AdapterRequest : Request<string>, IceInternal.TimerTask
    {
        public AdapterRequest(LookupI lookup, string id, int retryCount) : base(lookup, id, retryCount)
        {
            _start = DateTime.Now.Ticks;
        }

        public override bool retry()
        {
            return _proxies.Count == 0 && --nRetry_ >= 0;
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

        public void finished(Ice.ObjectPrx proxy)
        {
            if(proxy != null || _proxies.Count == 0)
            {
                sendResponse(proxy);
                return;
            }
            else if(_proxies.Count == 1)
            {
                sendResponse(_proxies[0]);
                return;
            }

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

        public void runTimerTask()
        {
            lookup_.adapterRequestTimedOut(this);
        }

        private void sendResponse(Ice.ObjectPrx proxy)
        {
            foreach(var cb in callbacks_)
            {
                cb.SetResult(proxy);
            }
            callbacks_.Clear();
        }

        private List<Ice.ObjectPrx> _proxies = new List<Ice.ObjectPrx>();
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

        public void finished(Ice.ObjectPrx proxy)
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
    };

    class LookupI : LookupDisp_
    {
        public LookupI(LocatorRegistryI registry, LookupPrx lookup, Ice.Properties properties)
        {
            _registry = registry;
            _timeout = properties.getPropertyAsIntWithDefault("IceDiscovery.Timeout", 300);
            _retryCount = properties.getPropertyAsIntWithDefault("IceDiscovery.RetryCount", 3);
            _latencyMultiplier = properties.getPropertyAsIntWithDefault("IceDiscovery.LatencyMultiplier", 1);
            _domainId = properties.getProperty("IceDiscovery.DomainId");
            _timer = IceInternal.Util.getInstance(lookup.ice_getCommunicator()).timer();

            try
            {
                lookup.ice_getConnection();
            }
            catch(Ice.LocalException ex)
            {
                StringBuilder b = new StringBuilder();
                b.Append("IceDiscovery is unable to establish a multicast connection:\n");
                b.Append("proxy = ");
                b.Append(lookup.ToString());
                b.Append('\n');
                b.Append(ex.ToString());
                throw new Ice.PluginInitializationException(b.ToString());
            }

            //
            // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast
            // datagram on each endpoint.
            //
            var single = new Ice.Endpoint[1];
            foreach(var endpt in lookup.ice_getEndpoints())
            {
                try
                {
                    single[0] = endpt;
                    LookupPrx l = (LookupPrx)lookup.ice_endpoints(single);
                    l.ice_getConnection();
                    _lookup[(LookupPrx)lookup.ice_endpoints(single)] = null;
                }
                catch(Ice.LocalException)
                {
                    // Ignore
                }
            }
            Debug.Assert(_lookup.Count > 0);
        }

        public void setLookupReply(LookupReplyPrx lookupReply)
        {
            //
            // Use a lookup reply proxy whose adress matches the interface used to send multicast datagrams.
            //
            var single = new Ice.Endpoint[1];
            foreach(var key in new List<LookupPrx>(_lookup.Keys))
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
                            _lookup[key] = (LookupReplyPrx)lookupReply.ice_endpoints(single);
                        }
                    }
                }

                if(_lookup[key] == null)
                {
                    // Fallback: just use the given lookup reply proxy if no matching endpoint found.
                    _lookup[key] = lookupReply;
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
                        foreach(var l in _lookup)
                        {
                            l.Key.findObjectByIdAsync(_domainId, id, l.Value);
                        }
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
                        foreach(var l in _lookup)
                        {
                            l.Key.findAdapterByIdAsync(_domainId, adapterId, l.Value);
                        }
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

        internal void foundObject(Ice.Identity id, Ice.ObjectPrx proxy)
        {
            lock(this)
            {
                ObjectRequest request;
                if(!_objectRequests.TryGetValue(id, out request))
                {
                    return;
                }
                request.response(proxy);
                _timer.cancel(request);
                _objectRequests.Remove(id);
            }
        }

        internal void foundAdapter(string adapterId, Ice.ObjectPrx proxy, bool isReplicaGroup)
        {
            lock(this)
            {
                AdapterRequest request;
                if(!_adapterRequests.TryGetValue(adapterId, out request))
                {
                    return;
                }

                if(request.response(proxy, isReplicaGroup))
                {
                    _timer.cancel(request);
                    _adapterRequests.Remove(request.getId());
                }
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
                        foreach(var l in _lookup)
                        {
                            l.Key.findObjectByIdAsync(_domainId, request.getId(), l.Value);
                        }
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
                        foreach(var l in _lookup)
                        {
                            l.Key.findAdapterByIdAsync(_domainId, request.getId(), l.Value);
                        }
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

        internal IceInternal.Timer timer()
        {
            return _timer;
        }

        internal int latencyMultiplier()
        {
            return _latencyMultiplier;
        }

        private LocatorRegistryI _registry;
        private Dictionary<LookupPrx, LookupReplyPrx> _lookup = new Dictionary<LookupPrx, LookupReplyPrx>();
        private readonly int _timeout;
        private readonly int _retryCount;
        private readonly int _latencyMultiplier;
        private readonly string _domainId;

        private IceInternal.Timer _timer;

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
            _lookup.foundObject(id, proxy);
        }

        public override void foundAdapterById(string adapterId, Ice.ObjectPrx proxy, bool isReplicaGroup, Ice.Current c)
        {
            _lookup.foundAdapter(adapterId, proxy, isReplicaGroup);
        }

        private LookupI _lookup;
    };

}
