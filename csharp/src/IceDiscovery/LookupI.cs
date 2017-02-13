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

    class Request<T, AmdCB>
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

        public bool addCallback(AmdCB cb)
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
        protected List<AmdCB> callbacks_ = new List<AmdCB>();

        private T _id;
    };

    class AdapterRequest : Request<string, Ice.AMD_Locator_findAdapterById>, IceInternal.TimerTask
    {
        public AdapterRequest(LookupI lookup, string id, int retryCount) : base(lookup, id, retryCount)
        {
            _start = System.DateTime.Now.Ticks;
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
                    _latency = (long)((System.DateTime.Now.Ticks - _start) * lookup_.latencyMultiplier() / 10000.0);
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
            foreach(Ice.AMD_Locator_findAdapterById cb in callbacks_)
            {
                cb.ice_response(proxy);
            }
            callbacks_.Clear();
        }

        private List<Ice.ObjectPrx> _proxies = new List<Ice.ObjectPrx>();
        private long _start;
        private long _latency;
    };

    class ObjectRequest : Request<Ice.Identity, Ice.AMD_Locator_findObjectById>, IceInternal.TimerTask
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
            foreach(Ice.AMD_Locator_findObjectById cb in callbacks_)
            {
                cb.ice_response(proxy);
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
            _lookup = lookup;
            _timeout = properties.getPropertyAsIntWithDefault("IceDiscovery.Timeout", 300);
            _retryCount = properties.getPropertyAsIntWithDefault("IceDiscovery.RetryCount", 3);
            _latencyMultiplier = properties.getPropertyAsIntWithDefault("IceDiscovery.LatencyMultiplier", 1);
            _domainId = properties.getProperty("IceDiscovery.DomainId");
            _timer = IceInternal.Util.getInstance(lookup.ice_getCommunicator()).timer();
        }

        public void setLookupReply(LookupReplyPrx lookupReply)
        {
            _lookupReply = lookupReply;
        }

        public override void findObjectById(string domainId, Ice.Identity id, IceDiscovery.LookupReplyPrx reply,
                                            Ice.Current c)
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
                    reply.begin_foundObjectById(id, proxy);
                }
                catch(Ice.LocalException)
                {
                    // Ignore.
                }
            }
        }

        public override void findAdapterById(string domainId, string adapterId, IceDiscovery.LookupReplyPrx reply,
                                             Ice.Current c)
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
                    reply.begin_foundAdapterById(adapterId, proxy, isReplicaGroup);
                }
                catch(Ice.LocalException)
                {
                    // Ignore.
                }
            }
        }

        internal void findObject(Ice.AMD_Locator_findObjectById cb, Ice.Identity id)
        {
            lock(this)
            {
                ObjectRequest request;
                if(!_objectRequests.TryGetValue(id, out request))
                {
                    request = new ObjectRequest(this, id, _retryCount);
                    _objectRequests.Add(id, request);
                }
                if(request.addCallback(cb))
                {
                    try
                    {
                        _lookup.begin_findObjectById(_domainId, id, _lookupReply);
                        _timer.schedule(request, _timeout);
                    }
                    catch(Ice.LocalException)
                    {
                        request.finished(null);
                        _objectRequests.Remove(id);
                    }
                }
            }
        }

        internal void findAdapter(Ice.AMD_Locator_findAdapterById cb, string adapterId)
        {
            lock(this)
            {
                AdapterRequest request;
                if(!_adapterRequests.TryGetValue(adapterId, out request))
                {
                    request = new AdapterRequest(this, adapterId, _retryCount);
                    _adapterRequests.Add(adapterId, request);
                }
                if(request.addCallback(cb))
                {
                    try
                    {
                        _lookup.begin_findAdapterById(_domainId, adapterId, _lookupReply);
                        _timer.schedule(request, _timeout);
                    }
                    catch(Ice.LocalException)
                    {
                        request.finished(null);
                        _adapterRequests.Remove(adapterId);
                    }
                }
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
                        _lookup.begin_findObjectById(_domainId, request.getId(), _lookupReply);
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
                        _lookup.begin_findAdapterById(_domainId, request.getId(), _lookupReply);
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
        private readonly LookupPrx _lookup;
        private LookupReplyPrx _lookupReply;
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

