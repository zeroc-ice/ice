// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class LocatorInfo
{
    private interface RequestCallback
    {
        public void response(LocatorInfo locatorInfo, Ice.ObjectPrx proxy);
        public void exception(LocatorInfo locatorInfo, Exception ex);
    };

    private abstract class Request
    {
        synchronized public void 
        addCallback(RequestCallback callback)
        {
            if(_response)
            {
                callback.response(_locatorInfo, _proxy);
                return;
            }
            else if(_exception != null)
            {
                callback.exception(_locatorInfo, _exception);
                return;
            }

            _callbacks.add(callback);

            if(!_sent)
            {
                _sent = true;
                try
                {
                    send();
                }
                catch(Exception ex)
                {
                    exception(ex);
                }
            }
        }

        synchronized public Ice.ObjectPrx 
        getProxy()
            throws Exception
        {
            if(_response)
            {
                return _proxy;
            }
            else if(_exception != null)
            {
                throw _exception;
            }

            if(!_sent)
            {
                _sent = true;
                try
                {
                    send();
                }
                catch(Exception ex)
                {
                    exception(ex);
                }
            }
    
            while(!_response && _exception == null)
            {
                try
                {
                    wait();
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }
    
            if(_exception != null)
            {
                throw _exception;
            }
            assert(_response);
            return _proxy;
        }

        protected 
        Request(LocatorInfo locatorInfo)
        {
            _locatorInfo = locatorInfo;
            _sent = false;
            _response = false;
        }

        synchronized protected void 
        response(Ice.ObjectPrx proxy)
        {
            _response = true;
            _proxy = proxy;
            for(RequestCallback c : _callbacks)
            {
                c.response(_locatorInfo, proxy);
            }
            notifyAll();
        }

        synchronized protected void 
        exception(Exception ex)
        {
            _exception = ex;
            for(RequestCallback c : _callbacks)
            {
                c.exception(_locatorInfo, ex);
            }
            notifyAll();
        }

        protected abstract void send();

        final protected LocatorInfo _locatorInfo;

        private java.util.List<RequestCallback> _callbacks = new java.util.ArrayList<RequestCallback>();
        private boolean _sent;
        private boolean _response;
        private Ice.ObjectPrx _proxy;
        private Exception _exception;
    };

    interface GetEndpointsCallback
    {
        void setEndpoints(EndpointI[] endpoints, boolean cached);
        void setException(Ice.LocalException ex);
    }

    private class ObjectRequest extends Request
    {
        public ObjectRequest(LocatorInfo locatorInfo, Ice.Identity id)
        {
            super(locatorInfo);
            _id = id;
        }

        protected void 
        send()
        {
            _locatorInfo.getLocator().findObjectById_async(
                new Ice.AMI_Locator_findObjectById()
                {
                    public void
                    ice_response(Ice.ObjectPrx proxy)
                    {
                        _locatorInfo.removeObjectRequest(_id);
                        response(proxy);
                    }

                    public void
                    ice_exception(Ice.UserException ex)
                    {
                        _locatorInfo.removeObjectRequest(_id);
                        exception(ex);
                    }
                    
                    public void
                    ice_exception(Ice.LocalException ex)
                    {
                        _locatorInfo.removeObjectRequest(_id);
                        exception(ex);
                    }
                },
                _id);
        }

        private final Ice.Identity _id;
    };

    private class AdapterRequest extends Request
    {
        public AdapterRequest(LocatorInfo locatorInfo, String id)
        {
            super(locatorInfo);
            _id = id;
        }
    
        protected void
        send()
        {
            _locatorInfo.getLocator().findAdapterById_async(
                new Ice.AMI_Locator_findAdapterById()
                {
                    public void
                    ice_response(Ice.ObjectPrx proxy)
                    {
                        _locatorInfo.removeAdapterRequest(_id);
                        response(proxy);
                    }

                    public void
                    ice_exception(Ice.UserException ex)
                    {
                        _locatorInfo.removeAdapterRequest(_id);
                        exception(ex);
                    }
                    
                    public void
                    ice_exception(Ice.LocalException ex)
                    {
                        _locatorInfo.removeAdapterRequest(_id);
                        exception(ex);
                    }
                },
                _id);
        }
        
        private final String _id;
    };

    private class ObjectRequestCallback implements RequestCallback
    {
        public void
        response(LocatorInfo locatorInfo, Ice.ObjectPrx object)
        {
            locatorInfo.getWellKnownObjectEndpoints(_reference, object, _ttl, false, _callback);
        }
        
        public void
        exception(LocatorInfo locatorInfo, Exception ex)
        {
            if(ex instanceof Ice.CollocationOptimizationException)
            {
                try
                {
                    Ice.BooleanHolder cached = new Ice.BooleanHolder();
                    _callback.setEndpoints(locatorInfo.getEndpoints(_reference, _ttl, cached), cached.value);
                }
                catch(Ice.LocalException e)
                {
                    _callback.setException(e);
                }
            }
            else
            {
                locatorInfo.getEndpointsException(_reference, ex, _callback);
            }
        }
    
        public 
        ObjectRequestCallback(Reference ref, int ttl, GetEndpointsCallback cb)
        {
            _reference = ref;
            _ttl = ttl;
            _callback = cb;
        }
    
        private final Reference _reference;
        private final int _ttl;
        private final GetEndpointsCallback _callback;
    };

    private class AdapterRequestCallback implements RequestCallback
    {
        public void
        response(LocatorInfo locatorInfo, Ice.ObjectPrx object)
        {
            EndpointI[] endpoints = null;
            if(object != null)
            {
                endpoints = ((Ice.ObjectPrxHelperBase)object).__reference().getEndpoints();
                if(endpoints.length > 0)
                {
                    locatorInfo.getTable().addAdapterEndpoints(_reference.getAdapterId(), endpoints);
                }
            }
            
            if(_reference.getInstance().traceLevels().location >= 1)
            {
                locatorInfo.getEndpointsTrace(_reference, endpoints, false);
            }
            
            if(endpoints == null)
            {
                _callback.setEndpoints(new EndpointI[0], false);
            }
            else
            {
                _callback.setEndpoints(endpoints, false);
            }
        }
        
        public void
        exception(LocatorInfo locatorInfo, Exception ex)
        {
            if(ex instanceof Ice.CollocationOptimizationException)
            {
                try
                {
                    Ice.BooleanHolder cached = new Ice.BooleanHolder();
                    _callback.setEndpoints(getEndpoints(_reference, _ttl, cached), cached.value);
                }
                catch(Ice.LocalException e)
                {
                    _callback.setException(e);
                }
            }
            else
            {
                getEndpointsException(_reference, ex, _callback);
            }
        }

        public 
        AdapterRequestCallback(Reference ref, int ttl, GetEndpointsCallback callback)
        {
            _reference = ref;
            _ttl = ttl;
            _callback = callback;
        }

        private final Reference _reference;
        private final int _ttl;
        private final GetEndpointsCallback _callback;
    };

    LocatorInfo(Ice.LocatorPrx locator, LocatorTable table)
    {
        _locator = locator;
        _table = table;
    }

    synchronized public void
    destroy()
    {
        _locatorRegistry = null;
        _table.clear();
    }

    public boolean
    equals(java.lang.Object obj)
    {
        if(this == obj)
        {
            return true;
        }

        if(obj instanceof LocatorInfo)
        {
            return _locator.equals(((LocatorInfo)obj)._locator);
        }

        return false;
    }

    public Ice.LocatorPrx
    getLocator()
    {
        //
        // No synchronization necessary, _locator is immutable.
        //
        return _locator;
    }

    public synchronized Ice.LocatorRegistryPrx
    getLocatorRegistry()
    {
        if(_locatorRegistry == null) // Lazy initialization
        {
            _locatorRegistry = _locator.getRegistry();

            //
            // The locator registry can't be located.
            //
            _locatorRegistry = Ice.LocatorRegistryPrxHelper.uncheckedCast(_locatorRegistry.ice_locator(null));
        }
        
        return _locatorRegistry;
    }

    public EndpointI[]
    getEndpoints(Reference ref, int ttl, Ice.BooleanHolder cached)
    {
        assert(ref.isIndirect());

        EndpointI[] endpoints = null;
        Ice.ObjectPrx object = null;
        cached.value = true;
        String adapterId = ref.getAdapterId();
        Ice.Identity identity = ref.getIdentity();
        
        try
        {
            if(adapterId.length() > 0)
            {
                endpoints = _table.getAdapterEndpoints(adapterId, ttl);
                if(endpoints == null)
                {
                    cached.value = false;
                    
                    if(ref.getInstance().traceLevels().location >= 1)
                    {
                        StringBuffer s = new StringBuffer();
                        s.append("searching for adapter by id\n");
                        s.append("adapter = " + adapterId);
                        ref.getInstance().initializationData().logger.trace(
                            ref.getInstance().traceLevels().locationCat, s.toString());
                    }

                    //
                    // Search the adapter in the location service if we didn't
                    // find it in the cache.
                    //
                    Request request = getAdapterRequest(adapterId);
                    object = request.getProxy();
                    if(object != null)
                    {
                        endpoints = ((Ice.ObjectPrxHelperBase)object).__reference().getEndpoints();
                        
                        if(endpoints.length > 0)
                        {
                            _table.addAdapterEndpoints(adapterId, endpoints);
                        }
                    }
                }
            }
            else
            {
                boolean objectCached = true;
                object = _table.getProxy(identity, ttl);
                if(object == null)
                {
                    objectCached = false;

                    if(ref.getInstance().traceLevels().location >= 1)
                    {
                        StringBuffer s = new StringBuffer();
                        s.append("searching for object by id\n");
                        s.append("object = " + ref.getInstance().identityToString(identity));
                        ref.getInstance().initializationData().logger.trace(
                            ref.getInstance().traceLevels().locationCat, s.toString());
                    }

                    Request request = getObjectRequest(identity);
                    object = request.getProxy();
                }
                
                boolean endpointsCached = true;
                if(object != null)
                {
                    Reference r = ((Ice.ObjectPrxHelperBase)object).__reference();
                    if(!r.isIndirect())
                    {
                        endpointsCached = false;
                        endpoints = r.getEndpoints();
                    }
                    else if(!r.isWellKnown())
                    {
                        Ice.BooleanHolder c = new Ice.BooleanHolder();
                        endpoints = getEndpoints(r, ttl, c);
                        endpointsCached = c.value;
                    }
                }
                
                if(!objectCached && endpoints != null && endpoints.length > 0)
                {
                    _table.addProxy(identity, object);
                }

                cached.value = objectCached || endpointsCached;
            }
        }
        catch(Exception ex)
        {
            getEndpointsException(ref, ex);
        }

        if(ref.getInstance().traceLevels().location >= 1)
        {
            getEndpointsTrace(ref, endpoints, cached.value);
        }

        return endpoints == null ? new EndpointI[0] : endpoints;
    }

    public void
    getEndpoints(final Reference ref, final int ttl, final GetEndpointsCallback callback)
    {
        final String adapterId = ref.getAdapterId();
        final Ice.Identity identity = ref.getIdentity();
        final Instance instance = ref.getInstance();
        if(adapterId.length() > 0)
        {
            EndpointI[] endpoints = _table.getAdapterEndpoints(adapterId, ttl);
            if(endpoints == null)
            {
                if(instance.traceLevels().location >= 1)
                {
                    StringBuffer s = new StringBuffer();
                    s.append("searching for adapter by id\n");
                    s.append("adapter = " + adapterId);
                    instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.toString());
                }

                //
                // Search the adapter in the location service if we didn't
                // find it in the cache.
                //
                Request request = getAdapterRequest(adapterId);
                request.addCallback(new AdapterRequestCallback(ref, ttl, callback));
                return;
            }
            else
            {
                if(instance.traceLevels().location >= 1)
                {
                    getEndpointsTrace(ref, endpoints, true);
                }
                callback.setEndpoints(endpoints, true);
                return;
            }
        }
        else
        {
            Ice.ObjectPrx object = _table.getProxy(identity, ttl);
            if(object == null)
            {
                if(instance.traceLevels().location >= 1)
                {
                    StringBuffer s = new StringBuffer();
                    s.append("searching for object by id\n");
                    s.append("object = " + instance.identityToString(identity));
                    instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.toString());
                }

                Request request = getObjectRequest(identity);
                request.addCallback(new ObjectRequestCallback(ref, ttl, callback));
                return;
            }
            else
            {
                getWellKnownObjectEndpoints(ref, object, ttl, true, callback);
                return;
            }
        }
    }

    public void
    clearObjectCache(Reference ref)
    {
        assert(ref.isIndirect());
        if(ref.isWellKnown())
        {
            Ice.ObjectPrx object = _table.removeProxy(ref.getIdentity());
            if(object != null)
            {
                Reference r = ((Ice.ObjectPrxHelperBase)object).__reference();
                if(!r.isIndirect())
                {
                    if(ref.getInstance().traceLevels().location >= 2)
                    {
                        trace("removed endpoints from locator table", ref, r.getEndpoints());
                    }
                }
                else if(!r.isWellKnown())
                {
                    clearCache(r);
                }
            }
        }
    }
    
    public void
    clearCache(Reference ref)
    {
        assert(ref.isIndirect());

        if(!ref.isWellKnown())
        {
            EndpointI[] endpoints = _table.removeAdapterEndpoints(ref.getAdapterId());

            if(endpoints != null && ref.getInstance().traceLevels().location >= 2)
            {
                trace("removed endpoints from locator table\n", ref, endpoints);
            }
        }
        else
        {
            Ice.ObjectPrx object = _table.removeProxy(ref.getIdentity());
            if(object != null)
            {
                Reference r = ((Ice.ObjectPrxHelperBase)object).__reference();
                if(!r.isIndirect())
                {
                    if(ref.getInstance().traceLevels().location >= 2)
                    {
                        trace("removed endpoints from locator table", ref, r.getEndpoints());
                    }
                }
                else if(!r.isWellKnown())
                {
                    clearCache(r);
                }
            }
        }
    }

    private void
    trace(String msg, Reference ref, EndpointI[] endpoints)
    {
        assert(ref.isIndirect());

        StringBuffer s = new StringBuffer();
        s.append(msg + "\n");
        if(!ref.isWellKnown())
        {
            s.append("adapter = " + ref.getAdapterId() + "\n");
        }
        else
        {
            s.append("object = " + ref.getInstance().identityToString(ref.getIdentity()) + "\n");
        }

        s.append("endpoints = ");
        final int sz = endpoints.length;
        for(int i = 0; i < sz; i++)
        {
            s.append(endpoints[i].toString());
            if(i + 1 < sz)
            {
                s.append(":");
            }
        }

        ref.getInstance().initializationData().logger.trace(ref.getInstance().traceLevels().locationCat, s.toString());
    }

    private void
    getEndpointsException(Reference ref, Exception exc)
    {
        assert(ref.isIndirect());

        try
        {
            throw exc;
        }
        catch(Ice.AdapterNotFoundException ex)
        {
            final Instance instance = ref.getInstance();
            if(instance.traceLevels().location >= 1)
            {
                StringBuffer s = new StringBuffer();
                s.append("adapter not found\n");
                s.append("adapter = " + ref.getAdapterId());
                instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.toString());
            }

            Ice.NotRegisteredException e = new Ice.NotRegisteredException();
            e.kindOfObject = "object adapter";
            e.id = ref.getAdapterId();
            throw e;
        }
        catch(Ice.ObjectNotFoundException ex)
        {
            final Instance instance = ref.getInstance();
            if(instance.traceLevels().location >= 1)
            {
                StringBuffer s = new StringBuffer();
                s.append("object not found\n");
                s.append("object = " + instance.identityToString(ref.getIdentity()));
                instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.toString());
            }

            Ice.NotRegisteredException e = new Ice.NotRegisteredException();
            e.kindOfObject = "object";
            e.id = instance.identityToString(ref.getIdentity());
            throw e;
        }
        catch(Ice.NotRegisteredException ex)
        {
            throw ex;
        }
        catch(Ice.LocalException ex)
        {
            final Instance instance = ref.getInstance();
            if(instance.traceLevels().location >= 1)
            {
                StringBuffer s = new StringBuffer();
                s.append("couldn't contact the locator to retrieve adapter endpoints\n");
                if(ref.getAdapterId().length() > 0)
                {
                    s.append("adapter = " + ref.getAdapterId() + "\n");
                }
                else
                {
                    s.append("object = " + instance.identityToString(ref.getIdentity()) + "\n");
                }
                s.append("reason = " + ex);
                instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.toString());
            }
            throw ex;
        }
        catch(Exception ex)
        {
            assert(false);
        }
    }

    private void
    getEndpointsException(Reference ref, Exception exc, GetEndpointsCallback callback)
    {
        try
        {
            getEndpointsException(ref, exc);
        }
        catch(Ice.LocalException ex)
        {
            callback.setException(ex);
        }
        catch(Exception ex)
        {
            assert(false);
        }
    }

    private void
    getWellKnownObjectEndpoints(final Reference ref, 
                                final Ice.ObjectPrx object, 
                                final int ttl, 
                                final boolean objectCached, 
                                final GetEndpointsCallback callback)
    {
        EndpointI[] endpoints = null;
        if(object != null)
        {
            Reference r = ((Ice.ObjectPrxHelperBase)object).__reference();
            if(!r.isIndirect())
            {
                endpoints = r.getEndpoints();
            }
            else if(!r.isWellKnown())
            {
                getEndpoints(r, ttl, new GetEndpointsCallback()
                {
                    public void
                    setEndpoints(EndpointI[] endpoints, boolean endpointsCached)
                    {
                        if(!objectCached && endpoints != null && endpoints.length > 0)
                        {
                            _table.addProxy(ref.getIdentity(), object);
                        }
                        
                        if(ref.getInstance().traceLevels().location >= 1)
                        {
                            getEndpointsTrace(ref, endpoints, objectCached || endpointsCached);
                        }
                        
                        callback.setEndpoints(endpoints, objectCached || endpointsCached);
                    }
                    
                    public void
                    setException(Ice.LocalException ex)
                    {
                        callback.setException(ex);
                    }
                    });
                return;
            }
        }
        
        if(!objectCached && endpoints != null && endpoints.length > 0)
        {
            _table.addProxy(ref.getIdentity(), object);
        }

        if(ref.getInstance().traceLevels().location >= 1)
        {
            getEndpointsTrace(ref, endpoints, objectCached);
        }

        if(endpoints == null)
        {
            callback.setEndpoints(new EndpointI[0], false);
        }
        else
        {
            callback.setEndpoints(endpoints, objectCached);
        }
    }
 
    private void
    getEndpointsTrace(Reference ref, EndpointI[] endpoints, boolean cached)
    {
        if(endpoints != null && endpoints.length > 0)
        {
            if(cached)
            {
                trace("found endpoints in locator table", ref, endpoints);
            }
            else
            {
                trace("retrieved endpoints from locator, adding to locator table", ref, endpoints);
            }
        }
        else
        {
            final Instance instance = ref.getInstance();
            StringBuffer s = new StringBuffer();
            s.append("no endpoints configured for ");
            if(ref.getAdapterId().length() > 0)
            {
                s.append("adapter\n");
                s.append("adapter = " + ref.getAdapterId() + "\n");
            }
            else
            {
                s.append("object\n");
                s.append("object = " + instance.identityToString(ref.getIdentity()) + "\n");
            }
            instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.toString());
        }
    }

    synchronized private Request
    getAdapterRequest(String id)
    {
        Request request = _adapterRequests.get(id);
        if(request != null)
        {
            return request;
        }
        
        request = new AdapterRequest(this, id);
        _adapterRequests.put(id, request);
        return request;
    }

    synchronized private void
    removeAdapterRequest(String id)
    {
        assert(_adapterRequests.get(id) != null);
        _adapterRequests.remove(id);
    }

    synchronized private Request
    getObjectRequest(Ice.Identity id)
    {
        Request request = _objectRequests.get(id);
        if(request != null)
        {
            return request;
        }
        
        request = new ObjectRequest(this, id);
        _objectRequests.put(id, request);
        return request;
    }

    synchronized private void
    removeObjectRequest(Ice.Identity id)
    {
        assert(_objectRequests.get(id) != null);
        _objectRequests.remove(id);
    }

    LocatorTable getTable()
    {
        return _table;
    }

    private final Ice.LocatorPrx _locator;
    private Ice.LocatorRegistryPrx _locatorRegistry;
    private final LocatorTable _table;

    private java.util.Map<String, Request> _adapterRequests = new java.util.HashMap<String, Request>();
    private java.util.Map<Ice.Identity, Request> _objectRequests = new java.util.HashMap<Ice.Identity, Request>();
}
