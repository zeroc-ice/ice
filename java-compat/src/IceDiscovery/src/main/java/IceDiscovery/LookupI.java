// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceDiscovery;

import java.util.List;
import java.util.ArrayList;
import java.util.Set;
import java.util.HashSet;
import java.util.Map;
import java.util.HashMap;

class LookupI extends _LookupDisp
{
    abstract private class Request<T, AmdCB> implements Runnable
    {
        Request(T id, int retryCount)
        {
            _id = id;
            _requestId = java.util.UUID.randomUUID().toString();
            _retryCount = retryCount;
        }

        T
        getId()
        {
            return _id;
        }

        boolean
        addCallback(AmdCB cb)
        {
            _callbacks.add(cb);
            return _callbacks.size() == 1;
        }

        boolean
        retry()
        {
            return --_retryCount >= 0;
        }

        void
        invoke(String domainId, Map<LookupPrx, LookupReplyPrx> lookups)
        {
            _lookupCount = lookups.size();
            _failureCount = 0;
            final Ice.Identity id = new Ice.Identity(_requestId, "");
            for(Map.Entry<LookupPrx, LookupReplyPrx> entry : lookups.entrySet())
            {
                invokeWithLookup(domainId,
                                 entry.getKey(),
                                 LookupReplyPrxHelper.uncheckedCast(entry.getValue().ice_identity(id)));
            }
        }

        boolean
        exception()
        {
            if(++_failureCount == _lookupCount)
            {
                finished(null);
                return true;
            }
            return false;
        }

        String
        getRequestId()
        {
            return _requestId;
        }

        void
        scheduleTimer(long timeout)
        {
            _future = _timer.schedule(this, timeout, java.util.concurrent.TimeUnit.MILLISECONDS);
        }

        void
        cancelTimer()
        {
            assert _future != null;
            _future.cancel(false);
            _future = null;
        }

        abstract void finished(Ice.ObjectPrx proxy);

        abstract protected void invokeWithLookup(String domainId, LookupPrx lookup, LookupReplyPrx lookupReply);

        private final String _requestId;

        protected int _retryCount;
        protected int _lookupCount;
        protected int _failureCount;
        protected List<AmdCB> _callbacks = new ArrayList<AmdCB>();
        protected T _id;
        protected java.util.concurrent.Future<?> _future;
    };

    private class AdapterRequest extends Request<String, Ice.AMD_Locator_findAdapterById>
    {
        AdapterRequest(String id, int retryCount)
        {
            super(id, retryCount);
            _start = System.nanoTime();
            _latency = 0;
        }

        @Override
        boolean
        retry()
        {
            return _proxies.size() == 0 && --_retryCount >= 0;
        }

        boolean
        response(Ice.ObjectPrx proxy, boolean isReplicaGroup)
        {
            if(isReplicaGroup)
            {
                _proxies.add(proxy);
                if(_latency == 0)
                {
                    _latency = (long)((System.nanoTime() - _start) * _latencyMultiplier / 100000.0);
                    if(_latency == 0)
                    {
                        _latency = 1; // 1ms
                    }
                    cancelTimer();
                    scheduleTimer(_latency);
                }
                return false;
            }
            finished(proxy);
            return true;
        }

        @Override
        void
        finished(Ice.ObjectPrx proxy)
        {
            if(proxy != null || _proxies.isEmpty())
            {
                sendResponse(proxy);
            }
            else if(_proxies.size() == 1)
            {
                sendResponse(_proxies.toArray(new Ice.ObjectPrx[1])[0]);
            }
            else
            {
                List<Ice.Endpoint> endpoints = new ArrayList<Ice.Endpoint>();
                Ice.ObjectPrx result = null;
                for(Ice.ObjectPrx prx : _proxies)
                {
                    if(result == null)
                    {
                        result = prx;
                    }
                    endpoints.addAll(java.util.Arrays.asList(prx.ice_getEndpoints()));
                }
                sendResponse(result.ice_endpoints(endpoints.toArray(new Ice.Endpoint[endpoints.size()])));
            }
        }

        @Override
        public void
        run()
        {
            adapterRequestTimedOut(this);
        }

        @Override
        protected void
        invokeWithLookup(String domainId, LookupPrx lookup, LookupReplyPrx lookupReply)
        {
            lookup.begin_findAdapterById(domainId, _id, lookupReply, new Ice.Callback()
            {
                @Override
                public void
                completed(Ice.AsyncResult r)
                {
                    try
                    {
                        r.throwLocalException();
                    }
                    catch(Ice.LocalException ex)
                    {
                        adapterRequestException(AdapterRequest.this, ex);
                    }
                }
            });
        }

        private void
        sendResponse(Ice.ObjectPrx proxy)
        {
            for(Ice.AMD_Locator_findAdapterById cb : _callbacks)
            {
                cb.ice_response(proxy);
            }
            _callbacks.clear();
        }

        //
        // We use a set because the same IceDiscovery plugin might return multiple times
        // the same proxy if it's accessible through multiple network interfaces and if we
        // also sent the request to multiple interfaces.
        //
        private Set<Ice.ObjectPrx> _proxies = new HashSet<Ice.ObjectPrx>();
        private long _start;
        private long _latency;
    };

    private class ObjectRequest extends Request<Ice.Identity, Ice.AMD_Locator_findObjectById>
    {
        ObjectRequest(Ice.Identity id, int retryCount)
        {
            super(id, retryCount);
        }

        void
        response(Ice.ObjectPrx proxy)
        {
            finished(proxy);
        }

        @Override
        void
        finished(Ice.ObjectPrx proxy)
        {
            for(Ice.AMD_Locator_findObjectById cb : _callbacks)
            {
                cb.ice_response(proxy);
            }
            _callbacks.clear();
        }

        @Override
        public void
        run()
        {
            objectRequestTimedOut(this);
        }

        @Override
        protected void
        invokeWithLookup(String domainId, LookupPrx lookup, LookupReplyPrx lookupReply)
        {
            lookup.begin_findObjectById(domainId, _id, lookupReply, new Ice.Callback()
            {
                @Override
                public void
                completed(Ice.AsyncResult r)
                {
                    try
                    {
                        r.throwLocalException();
                    }
                    catch(Ice.LocalException ex)
                    {
                        objectRequestException(ObjectRequest.this, ex);
                    }
                }
            });
        }
    };

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
        Ice.Endpoint[] single = new Ice.Endpoint[1];
        for(Ice.Endpoint endpt : lookup.ice_getEndpoints())
        {
            single[0] = endpt;
            _lookups.put((LookupPrx)lookup.ice_endpoints(single), null);
        }
        assert(!_lookups.isEmpty());
    }

    void
    setLookupReply(LookupReplyPrx lookupReply)
    {
        //
        // Use a lookup reply proxy whose adress matches the interface used to send multicast datagrams.
        //
        Ice.Endpoint[] single = new Ice.Endpoint[1];
        for(Map.Entry<LookupPrx, LookupReplyPrx> entry : _lookups.entrySet())
        {
            Ice.UDPEndpointInfo info = (Ice.UDPEndpointInfo)entry.getKey().ice_getEndpoints()[0].getInfo();
            if(!info.mcastInterface.isEmpty())
            {
                for(Ice.Endpoint q : lookupReply.ice_getEndpoints())
                {
                    Ice.EndpointInfo r = q.getInfo();
                    if(r instanceof Ice.IPEndpointInfo && ((Ice.IPEndpointInfo)r).host.equals(info.mcastInterface))
                    {
                        single[0] = q;
                        entry.setValue((LookupReplyPrx)lookupReply.ice_endpoints(single));
                    }
                }
            }

            if(entry.getValue() == null)
            {
                // Fallback: just use the given lookup reply proxy if no matching endpoint found.
                entry.setValue(lookupReply);
            }
        }
    }

    @Override
    public void
    findObjectById(String domainId, Ice.Identity id, IceDiscovery.LookupReplyPrx reply, Ice.Current c)
    {
        if(!domainId.equals(_domainId))
        {
            return; // Ignore.
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
            catch(Ice.LocalException ex)
            {
                // Ignore
            }
        }
    }

    @Override
    public void
    findAdapterById(String domainId, String adapterId, IceDiscovery.LookupReplyPrx reply, Ice.Current c)
    {
        if(!domainId.equals(_domainId))
        {
            return; // Ignore.
        }

        Ice.Holder<Boolean> isReplicaGroup = new Ice.Holder<Boolean>();
        Ice.ObjectPrx proxy = _registry.findAdapter(adapterId, isReplicaGroup);
        if(proxy != null)
        {
            //
            // Reply to the multicast request using the given proxy.
            //
            try
            {
                reply.begin_foundAdapterById(adapterId, proxy, isReplicaGroup.value);
            }
            catch(Ice.LocalException ex)
            {
                // Ignore
            }
        }
    }

    synchronized void
    findObject(Ice.AMD_Locator_findObjectById cb, Ice.Identity id)
    {
        ObjectRequest request = _objectRequests.get(id);
        if(request == null)
        {
            request = new ObjectRequest(id, _retryCount);
            _objectRequests.put(id, request);
        }

        if(request.addCallback(cb))
        {
            try
            {
                request.invoke(_domainId, _lookups);
                request.scheduleTimer(_timeout);
            }
            catch(Ice.LocalException ex)
            {
                request.finished(null);
                _objectRequests.remove(id);
            }
        }
    }

    synchronized void
    findAdapter(Ice.AMD_Locator_findAdapterById cb, String adapterId)
    {
        AdapterRequest request = _adapterRequests.get(adapterId);
        if(request == null)
        {
            request = new AdapterRequest(adapterId, _retryCount);
            _adapterRequests.put(adapterId, request);
        }

        if(request.addCallback(cb))
        {
            try
            {
                request.invoke(_domainId, _lookups);
                request.scheduleTimer(_timeout);
            }
            catch(Ice.LocalException ex)
            {
                request.finished(null);
                _adapterRequests.remove(adapterId);
            }
        }
    }

    synchronized void
    foundObject(Ice.Identity id, String requestId, Ice.ObjectPrx proxy)
    {
        ObjectRequest request = _objectRequests.get(id);
        if(request != null && request.getRequestId().equals(requestId)) // Ignore responses from old requests
        {
            request.response(proxy);
            request.cancelTimer();
            _objectRequests.remove(id);
        }
    }

    synchronized void
    foundAdapter(String adapterId, String requestId, Ice.ObjectPrx proxy, boolean isReplicaGroup)
    {
        AdapterRequest request = _adapterRequests.get(adapterId);
        if(request != null && request.getRequestId().equals(requestId)) // Ignore responses from old requests
        {
            if(request.response(proxy, isReplicaGroup))
            {
                request.cancelTimer();
                _adapterRequests.remove(adapterId);
            }
        }
    }

    synchronized void
    objectRequestTimedOut(ObjectRequest request)
    {
        ObjectRequest r = _objectRequests.get(request.getId());
        if(r == null || request != r)
        {
            return;
        }

        if(request.retry())
        {
            try
            {
                request.invoke(_domainId, _lookups);
                request.scheduleTimer(_timeout);
                return;
            }
            catch(Ice.LocalException ex)
            {
            }
        }

        request.finished(null);
        _objectRequests.remove(request.getId());
    }

    synchronized void
    objectRequestException(ObjectRequest request, Ice.LocalException ex)
    {
        ObjectRequest r = _objectRequests.get(request.getId());
        if(r == null || r != request)
        {
            return;
        }

        if(request.exception())
        {
            if(_warnOnce)
            {
                StringBuilder s = new StringBuilder();
                s.append("failed to lookup object `");
                s.append(_lookup.ice_getCommunicator().identityToString(request.getId()));
                s.append("' with lookup proxy `");
                s.append(_lookup);
                s.append("':\n");
                s.append(ex.toString());
                _lookup.ice_getCommunicator().getLogger().warning(s.toString());
                _warnOnce = false;
            }
            request.cancelTimer();
            _objectRequests.remove(request.getId());
        }
    }

    synchronized void
    adapterRequestTimedOut(AdapterRequest request)
    {
        AdapterRequest r = _adapterRequests.get(request.getId());
        if(r == null || r != request)
        {
            return;
        }

        if(request.retry())
        {
            try
            {
                request.invoke(_domainId, _lookups);
                request.scheduleTimer(_timeout);
                return;
            }
            catch(Ice.LocalException ex)
            {
            }
        }

        request.finished(null);
        _adapterRequests.remove(request.getId());
    }

    synchronized void
    adapterRequestException(AdapterRequest request, Ice.LocalException ex)
    {
        AdapterRequest r = _adapterRequests.get(request.getId());
        if(r == null || r != request)
        {
            return;
        }

        if(request.exception())
        {
            if(_warnOnce)
            {
                StringBuilder s = new StringBuilder();
                s.append("failed to lookup adapter `");
                s.append(request.getId());
                s.append("' with lookup proxy `");
                s.append(_lookup);
                s.append("':\n");
                s.append(ex.toString());
                _lookup.ice_getCommunicator().getLogger().warning(s.toString());
                _warnOnce = false;
            }
            request.cancelTimer();
            _adapterRequests.remove(request.getId());
        }
    }

    private LocatorRegistryI _registry;
    private LookupPrx _lookup;
    private java.util.Map<LookupPrx, LookupReplyPrx> _lookups = new java.util.HashMap<>();
    private final int _timeout;
    private final int _retryCount;
    private final int _latencyMultiplier;
    private final String _domainId;
    private final java.util.concurrent.ScheduledExecutorService _timer;
    private boolean _warnOnce = true;

    private Map<Ice.Identity, ObjectRequest> _objectRequests = new HashMap<Ice.Identity, ObjectRequest>();
    private Map<String, AdapterRequest> _adapterRequests = new HashMap<String, AdapterRequest>();
}
