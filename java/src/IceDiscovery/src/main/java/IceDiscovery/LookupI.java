// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceDiscovery;

import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;

class LookupI extends _LookupDisp
{
    abstract private class Request<T, AmdCB> implements Runnable
    {
        Request(T id, int retryCount)
        {
            _id = id;
            _nRetry = retryCount;
            _requestId = java.util.UUID.randomUUID().toString();
        }

        T
        getId()
        {
            return _id;
        }

        String
        getRequestId()
        {
            return _requestId;
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
            return --_nRetry >= 0;
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

        protected int _nRetry;
        protected List<AmdCB> _callbacks = new ArrayList<AmdCB>();
        private T _id;
        protected java.util.concurrent.Future<?> _future;
        final private String _requestId;
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
            return _proxies.size() == 0 && --_nRetry >= 0;
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

        void
        finished(Ice.ObjectPrx proxy)
        {
            if(proxy != null || _proxies.isEmpty())
            {
                sendResponse(proxy);
                return;
            }
            else if(_proxies.size() == 1)
            {
                sendResponse(_proxies.get(0));
                return;
            }

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

        @Override
        public void
        run()
        {
            adapterRequestTimedOut(this);
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

        private List<Ice.ObjectPrx> _proxies = new ArrayList<Ice.ObjectPrx>();
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
    }

    void
    setLookupReply(LookupReplyPrx lookupReply)
    {
        _lookupReply = lookupReply;
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
                Ice.Identity ident = new Ice.Identity();
                ident.name = request.getRequestId();
                _lookup.begin_findObjectById(_domainId,
                                             id,
                                             LookupReplyPrxHelper.uncheckedCast(_lookupReply.ice_identity(ident)));
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
                Ice.Identity ident = new Ice.Identity();
                ident.name = request.getRequestId();
                _lookup.begin_findAdapterById(_domainId,
                                              adapterId,
                                              LookupReplyPrxHelper.uncheckedCast(_lookupReply.ice_identity(ident)));
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
        if(request != null && requestId.equals(request.getRequestId()))
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
        if(request != null && requestId.equals(request.getRequestId()))
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
                _lookup.begin_findObjectById(_domainId, request.getId(), _lookupReply);
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
                _lookup.begin_findAdapterById(_domainId, request.getId(), _lookupReply);
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

    private LocatorRegistryI _registry;
    private final LookupPrx _lookup;
    private LookupReplyPrx _lookupReply;
    private final int _timeout;
    private final int _retryCount;
    private final int _latencyMultiplier;
    private final String _domainId;

    private final java.util.concurrent.ScheduledExecutorService _timer;

    private Map<Ice.Identity, ObjectRequest> _objectRequests = new HashMap<Ice.Identity, ObjectRequest>();
    private Map<String, AdapterRequest> _adapterRequests = new HashMap<String, AdapterRequest>();
}

