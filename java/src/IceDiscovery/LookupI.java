// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
    abstract private class Request<T, AmdCB> implements IceInternal.TimerTask
    {
        public Request(T id, int retryCount)
        {
            _id = id;
            _nRetry = retryCount;
        }

        public T 
        getId()
        {
            return _id;
        }
        
        public boolean
        addCallback(AmdCB cb)
        {
            _callbacks.add(cb);
            return _callbacks.size() == 1;
        }

        public boolean
        retry()
        {
            return --_nRetry >= 0;
        }

        protected int _nRetry;
        protected List<AmdCB> _callbacks = new ArrayList<AmdCB>();
        private T _id;
    };

    private class AdapterRequest extends Request<String, Ice.AMD_Locator_findAdapterById>
    {
        public AdapterRequest(String id, int retryCount)
        {
            super(id, retryCount);
            _start = System.nanoTime();
            _latency = 0;
        }

        public boolean
        retry()
        {
            return _proxies.size() == 0 && --_nRetry >= 0;
        }
        
        public boolean
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
                    _timer.cancel(this);
                    _timer.schedule(this, _latency);
                }
                return false;
            }
            finished(proxy);
            return true;
        }

        public void
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
        
        public void 
        runTimerTask()
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
        public 
        ObjectRequest(Ice.Identity id, int retryCount)
        {
            super(id, retryCount);
        }

        public void 
        response(Ice.ObjectPrx proxy)
        {
            finished(proxy);
        }
        
        public void 
        finished(Ice.ObjectPrx proxy)
        {
            for(Ice.AMD_Locator_findObjectById cb : _callbacks)
            {
                cb.ice_response(proxy);
            }
            _callbacks.clear();
        }

        public void runTimerTask()
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
            reply.begin_foundObjectById(id, proxy);
        }
    }

    public void 
    findAdapterById(String domainId, String adapterId, IceDiscovery.LookupReplyPrx reply, Ice.Current c)
    {
        if(!domainId.equals(_domainId))
        {
            return; // Ignore.
        }
        
        Ice.BooleanHolder isReplicaGroup = new Ice.BooleanHolder();
        Ice.ObjectPrx proxy = _registry.findAdapter(adapterId, isReplicaGroup);
        if(proxy != null)
        {
            //
            // Reply to the multicast request using the given proxy.
            //
            reply.begin_foundAdapterById(adapterId, proxy, isReplicaGroup.value);
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
            _lookup.findObjectById(_domainId, id, _lookupReply);
            _timer.schedule(request, _timeout);
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
            _lookup.findAdapterById(_domainId, adapterId, _lookupReply);
            _timer.schedule(request, _timeout);
        }
    }

    synchronized void
    foundObject(Ice.Identity id, Ice.ObjectPrx proxy)
    {
        ObjectRequest request = _objectRequests.get(id);
        if(request == null)
        {
            return;
        }
        
        request.response(proxy);
        _timer.cancel(request);
        _objectRequests.remove(id);
    }

    synchronized void
    foundAdapter(String adapterId, Ice.ObjectPrx proxy, boolean isReplicaGroup)
    {
        AdapterRequest request = _adapterRequests.get(adapterId);
        if(request == null)
        {
            return;
        }
        
        if(request.response(proxy, isReplicaGroup))
        {
            _timer.cancel(request);
            _adapterRequests.remove(adapterId);
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
            _lookup.findObjectById(_domainId, request.getId(), _lookupReply);
            _timer.schedule(request, _timeout);
        }
        else
        {
            request.finished(null);
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
            _lookup.findAdapterById(_domainId, request.getId(), _lookupReply);
            _timer.schedule(request, _timeout);
        }
        else
        {
            request.finished(null);
            _adapterRequests.remove(request.getId());
        }
    }

    private LocatorRegistryI _registry;
    private final LookupPrx _lookup;
    private LookupReplyPrx _lookupReply;
    private final int _timeout;
    private final int _retryCount;
    private final int _latencyMultiplier;
    private final String _domainId;

    private final IceInternal.Timer _timer;

    private Map<Ice.Identity, ObjectRequest> _objectRequests = new HashMap<Ice.Identity, ObjectRequest>();
    private Map<String, AdapterRequest> _adapterRequests = new HashMap<String, AdapterRequest>();

}

