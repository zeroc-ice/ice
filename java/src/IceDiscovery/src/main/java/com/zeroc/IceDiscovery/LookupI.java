// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceDiscovery;

import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
import java.util.concurrent.CompletableFuture;

class LookupI implements Lookup
{
    abstract private class Request<T, Ret> implements Runnable
    {
        Request(T id, int retryCount)
        {
            _id = id;
            _nRetry = retryCount;
        }

        T getId()
        {
            return _id;
        }

        boolean addFuture(CompletableFuture<Ret> f)
        {
            _futures.add(f);
            return _futures.size() == 1;
        }

        boolean retry()
        {
            return --_nRetry >= 0;
        }

        void scheduleTimer(long timeout)
        {
            _future = _timer.schedule(this, timeout, java.util.concurrent.TimeUnit.MILLISECONDS);
        }

        void cancelTimer()
        {
            assert _future != null;
            _future.cancel(false);
            _future = null;
        }

        protected int _nRetry;
        protected List<CompletableFuture<Ret>> _futures = new ArrayList<>();
        private T _id;
        protected java.util.concurrent.Future<?> _future;
    }

    private class AdapterRequest extends Request<String, com.zeroc.Ice.ObjectPrx>
    {
        AdapterRequest(String id, int retryCount)
        {
            super(id, retryCount);
            _start = System.nanoTime();
            _latency = 0;
        }

        @Override
        boolean retry()
        {
            return _proxies.size() == 0 && --_nRetry >= 0;
        }

        boolean response(com.zeroc.Ice.ObjectPrx proxy, boolean isReplicaGroup)
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

        void finished(com.zeroc.Ice.ObjectPrx proxy)
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

            List<com.zeroc.Ice.Endpoint> endpoints = new ArrayList<>();
            com.zeroc.Ice.ObjectPrx result = null;
            for(com.zeroc.Ice.ObjectPrx prx : _proxies)
            {
                if(result == null)
                {
                    result = prx;
                }
                endpoints.addAll(java.util.Arrays.asList(prx.ice_getEndpoints()));
            }
            sendResponse(result.ice_endpoints(endpoints.toArray(new com.zeroc.Ice.Endpoint[endpoints.size()])));
        }

        @Override
        public void run()
        {
            adapterRequestTimedOut(this);
        }

        private void sendResponse(com.zeroc.Ice.ObjectPrx proxy)
        {
            for(CompletableFuture<com.zeroc.Ice.ObjectPrx> f : _futures)
            {
                f.complete(proxy);
            }
            _futures.clear();
        }

        private List<com.zeroc.Ice.ObjectPrx> _proxies = new ArrayList<>();
        private long _start;
        private long _latency;
    }

    private class ObjectRequest extends Request<com.zeroc.Ice.Identity, com.zeroc.Ice.ObjectPrx>
    {
        ObjectRequest(com.zeroc.Ice.Identity id, int retryCount)
        {
            super(id, retryCount);
        }

        void response(com.zeroc.Ice.ObjectPrx proxy)
        {
            finished(proxy);
        }

        void finished(com.zeroc.Ice.ObjectPrx proxy)
        {
            for(CompletableFuture<com.zeroc.Ice.ObjectPrx> f : _futures)
            {
                f.complete(proxy);
            }
            _futures.clear();
        }

        @Override
        public void run()
        {
            objectRequestTimedOut(this);
        }
    }

    public LookupI(LocatorRegistryI registry, LookupPrx lookup, com.zeroc.Ice.Properties properties)
    {
        _registry = registry;
        _lookup = lookup;
        _timeout = properties.getPropertyAsIntWithDefault("IceDiscovery.Timeout", 300);
        _retryCount = properties.getPropertyAsIntWithDefault("IceDiscovery.RetryCount", 3);
        _latencyMultiplier = properties.getPropertyAsIntWithDefault("IceDiscovery.LatencyMultiplier", 1);
        _domainId = properties.getProperty("IceDiscovery.DomainId");
        _timer = com.zeroc.IceInternal.Util.getInstance(lookup.ice_getCommunicator()).timer();
    }

    void setLookupReply(LookupReplyPrx lookupReply)
    {
        _lookupReply = lookupReply;
    }

    @Override
    public void findObjectById(String domainId, com.zeroc.Ice.Identity id, com.zeroc.IceDiscovery.LookupReplyPrx reply,
                               com.zeroc.Ice.Current c)
    {
        if(!domainId.equals(_domainId))
        {
            return; // Ignore.
        }

        com.zeroc.Ice.ObjectPrx proxy = _registry.findObject(id);
        if(proxy != null)
        {
            //
            // Reply to the mulicast request using the given proxy.
            //
            try
            {
               reply.foundObjectByIdAsync(id, proxy);
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                // Ignore
            }
        }
    }

    @Override
    public void findAdapterById(String domainId, String adapterId, com.zeroc.IceDiscovery.LookupReplyPrx reply,
                                com.zeroc.Ice.Current c)
    {
        if(!domainId.equals(_domainId))
        {
            return; // Ignore.
        }

        LocatorRegistryI.FindAdapterResult r = _registry.findAdapter(adapterId);
        if(r.returnValue != null)
        {
            //
            // Reply to the multicast request using the given proxy.
            //
            try
            {
                reply.foundAdapterByIdAsync(adapterId, r.returnValue, r.isReplicaGroup);
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                // Ignore
            }
        }
    }

    synchronized void findObject(CompletableFuture<com.zeroc.Ice.ObjectPrx> f, com.zeroc.Ice.Identity id)
    {
        ObjectRequest request = _objectRequests.get(id);
        if(request == null)
        {
            request = new ObjectRequest(id, _retryCount);
            _objectRequests.put(id, request);
        }

        if(request.addFuture(f))
        {
            try
            {
                _lookup.findObjectByIdAsync(_domainId, id, _lookupReply);
                request.scheduleTimer(_timeout);
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                request.finished(null);
                _objectRequests.remove(id);
            }
        }
    }

    synchronized void findAdapter(CompletableFuture<com.zeroc.Ice.ObjectPrx> f, String adapterId)
    {
        AdapterRequest request = _adapterRequests.get(adapterId);
        if(request == null)
        {
            request = new AdapterRequest(adapterId, _retryCount);
            _adapterRequests.put(adapterId, request);
        }

        if(request.addFuture(f))
        {
            try
            {
                _lookup.findAdapterByIdAsync(_domainId, adapterId, _lookupReply);
                request.scheduleTimer(_timeout);
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
                request.finished(null);
                _adapterRequests.remove(adapterId);
            }
        }
    }

    synchronized void foundObject(com.zeroc.Ice.Identity id, com.zeroc.Ice.ObjectPrx proxy)
    {
        ObjectRequest request = _objectRequests.get(id);
        if(request == null)
        {
            return;
        }

        request.response(proxy);
        request.cancelTimer();
        _objectRequests.remove(id);
    }

    synchronized void foundAdapter(String adapterId, com.zeroc.Ice.ObjectPrx proxy, boolean isReplicaGroup)
    {
        AdapterRequest request = _adapterRequests.get(adapterId);
        if(request == null)
        {
            return;
        }

        if(request.response(proxy, isReplicaGroup))
        {
            request.cancelTimer();
            _adapterRequests.remove(adapterId);
        }
    }

    synchronized void objectRequestTimedOut(ObjectRequest request)
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
                _lookup.findObjectByIdAsync(_domainId, request.getId(), _lookupReply);
                request.scheduleTimer(_timeout);
                return;
            }
            catch(com.zeroc.Ice.LocalException ex)
            {
            }
        }

        request.finished(null);
        _objectRequests.remove(request.getId());
    }

    synchronized void adapterRequestTimedOut(AdapterRequest request)
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
                _lookup.findAdapterByIdAsync(_domainId, request.getId(), _lookupReply);
                request.scheduleTimer(_timeout);
                return;
            }
            catch(com.zeroc.Ice.LocalException ex)
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

    private Map<com.zeroc.Ice.Identity, ObjectRequest> _objectRequests = new HashMap<>();
    private Map<String, AdapterRequest> _adapterRequests = new HashMap<>();
}
