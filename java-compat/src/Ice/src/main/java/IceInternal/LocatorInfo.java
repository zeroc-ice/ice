// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class LocatorInfo
{
    interface GetEndpointsCallback
    {
        void setEndpoints(EndpointI[] endpoints, boolean cached);
        void setException(Ice.LocalException ex);
    }

    private static class RequestCallback
    {
        public void
        response(LocatorInfo locatorInfo, Ice.ObjectPrx proxy)
        {
            EndpointI[] endpoints = null;
            if(proxy != null)
            {
                Reference r = ((Ice.ObjectPrxHelperBase)proxy).__reference();
                if(_ref.isWellKnown() && !Protocol.isSupported(_ref.getEncoding(), r.getEncoding()))
                {
                    //
                    // If a well-known proxy and the returned proxy
                    // encoding isn't supported, we're done: there's
                    // no compatible endpoint we can use.
                    //
                }
                else if(!r.isIndirect())
                {
                    endpoints = r.getEndpoints();
                }
                else if(_ref.isWellKnown() && !r.isWellKnown())
                {
                    //
                    // We're resolving the endpoints of a well-known object and the proxy returned
                    // by the locator is an indirect proxy. We now need to resolve the endpoints
                    // of this indirect proxy.
                    //
                    locatorInfo.getEndpoints(r, _ref, _ttl, _callback);
                    return;
                }
            }

            if(_ref.getInstance().traceLevels().location >= 1)
            {
                locatorInfo.getEndpointsTrace(_ref, endpoints, false);
            }
            if(_callback != null)
            {
                _callback.setEndpoints(endpoints == null ? new EndpointI[0] : endpoints, false);
            }
        }

        public void
        exception(LocatorInfo locatorInfo, Exception exc)
        {
            try
            {
                locatorInfo.getEndpointsException(_ref, exc); // This throws.
            }
            catch(Ice.LocalException ex)
            {
                if(_callback != null)
                {
                    _callback.setException(ex);
                }
            }
        }

        RequestCallback(Reference ref, int ttl, GetEndpointsCallback cb)
        {
            _ref = ref;
            _ttl = ttl;
            _callback = cb;
        }

        final Reference _ref;
        final int _ttl;
        final GetEndpointsCallback _callback;
    }

    private abstract class Request
    {
        public void
        addCallback(Reference ref, Reference wellKnownRef, int ttl, GetEndpointsCallback cb)
        {
            RequestCallback callback = new RequestCallback(ref, ttl, cb);
            synchronized(this)
            {
                if(!_response && _exception == null)
                {
                    _callbacks.add(callback);
                    if(wellKnownRef != null) // This request is to resolve the endpoints of a cached well-known object ref
                    {
                        _wellKnownRefs.add(wellKnownRef);
                    }
                    if(!_sent)
                    {
                        _sent = true;
                        send();
                    }
                    return;
                }
            }

            if(_response)
            {
                callback.response(_locatorInfo, _proxy);
            }
            else
            {
                assert(_exception != null);
                callback.exception(_locatorInfo, _exception);
            }
        }

        Request(LocatorInfo locatorInfo, Reference ref)
        {
            _locatorInfo = locatorInfo;
            _ref = ref;
            _sent = false;
            _response = false;
        }

        protected void
        response(Ice.ObjectPrx proxy)
        {
            synchronized(this)
            {
                _locatorInfo.finishRequest(_ref, _wellKnownRefs, proxy, false);
                _response = true;
                _proxy = proxy;
                notifyAll();
            }
            for(RequestCallback callback : _callbacks)
            {
                callback.response(_locatorInfo, proxy);
            }
        }

        protected void
        exception(Exception ex)
        {
            synchronized(this)
            {
                _locatorInfo.finishRequest(_ref, _wellKnownRefs, null, ex instanceof Ice.UserException);
                _exception = ex;
                notifyAll();
            }
            for(RequestCallback callback : _callbacks)
            {
                callback.exception(_locatorInfo, ex);
            }
        }

        protected abstract void send();

        final protected LocatorInfo _locatorInfo;
        final protected Reference _ref;

        private java.util.List<RequestCallback> _callbacks = new java.util.ArrayList<RequestCallback>();
        private java.util.List<Reference> _wellKnownRefs = new java.util.ArrayList<Reference>();
        private boolean _sent;
        private boolean _response;
        private Ice.ObjectPrx _proxy;
        private Exception _exception;
    }

    private class ObjectRequest extends Request
    {
        public ObjectRequest(LocatorInfo locatorInfo, Reference reference)
        {
            super(locatorInfo, reference);
            assert(reference.isWellKnown());
        }

        @Override
        protected void
        send()
        {
            try
            {
                _locatorInfo.getLocator().begin_findObjectById(
                    _ref.getIdentity(),
                    new Ice.Callback_Locator_findObjectById()
                    {
                        @Override
                        public void
                        response(Ice.ObjectPrx proxy)
                        {
                            ObjectRequest.this.response(proxy);
                        }

                        @Override
                        public void
                        exception(Ice.UserException ex)
                        {
                            ObjectRequest.this.exception(ex);
                        }

                        @Override
                        public void
                        exception(Ice.LocalException ex)
                        {
                            ObjectRequest.this.exception(ex);
                        }
                    });
            }
            catch(Exception ex)
            {
                exception(ex);
            }
        }
    }

    private class AdapterRequest extends Request
    {
        public AdapterRequest(LocatorInfo locatorInfo, Reference reference)
        {
            super(locatorInfo, reference);
            assert(reference.isIndirect());
        }

        @Override
        protected void
        send()
        {
            try
            {
                _locatorInfo.getLocator().begin_findAdapterById(
                    _ref.getAdapterId(),
                    new Ice.Callback_Locator_findAdapterById()
                    {
                        @Override
                        public void
                        response(Ice.ObjectPrx proxy)
                        {
                            AdapterRequest.this.response(proxy);
                        }

                        @Override
                        public void
                        exception(Ice.UserException ex)
                        {
                            AdapterRequest.this.exception(ex);
                        }

                        @Override
                        public void
                        exception(Ice.LocalException ex)
                        {
                            AdapterRequest.this.exception(ex);
                        }
                    });
            }
            catch(Exception ex)
            {
                exception(ex);
            }
        }
    }

    LocatorInfo(Ice.LocatorPrx locator, LocatorTable table, boolean background)
    {
        _locator = locator;
        _table = table;
        _background = background;
    }

    synchronized public void
    destroy()
    {
        _locatorRegistry = null;
        _table.clear();
    }

    @Override
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

    @Override
    public int
    hashCode()
    {
         return _locator.hashCode();
    }

    public Ice.LocatorPrx
    getLocator()
    {
        //
        // No synchronization necessary, _locator is immutable.
        //
        return _locator;
    }

    public Ice.LocatorRegistryPrx
    getLocatorRegistry()
    {
        synchronized(this)
        {
            if(_locatorRegistry != null)
            {
                return _locatorRegistry;
            }
        }

        //
        // Do not make locator calls from within sync.
        //
        Ice.LocatorRegistryPrx locatorRegistry = _locator.getRegistry();
        if(locatorRegistry == null)
        {
            return null;
        }

        synchronized(this)
        {
            //
            // The locator registry can't be located. We use ordered
            // endpoint selection in case the locator returned a proxy
            // with some endpoints which are prefered to be tried first.
            //
            _locatorRegistry = (Ice.LocatorRegistryPrx)locatorRegistry.ice_locator(null).ice_endpointSelection(
                Ice.EndpointSelectionType.Ordered);
            return _locatorRegistry;
        }
    }

    public void
    getEndpoints(Reference ref, int ttl, GetEndpointsCallback callback)
    {
        getEndpoints(ref, null, ttl, callback);
    }

    public void
    getEndpoints(Reference ref, Reference wellKnownRef, int ttl, GetEndpointsCallback callback)
    {
        assert(ref.isIndirect());
        EndpointI[] endpoints = null;
        Ice.Holder<Boolean> cached = new Ice.Holder<Boolean>();
        if(!ref.isWellKnown())
        {
            endpoints = _table.getAdapterEndpoints(ref.getAdapterId(), ttl, cached);
            if(!cached.value)
            {
                if(_background && endpoints != null)
                {
                    getAdapterRequest(ref).addCallback(ref, wellKnownRef, ttl, null);
                }
                else
                {
                    getAdapterRequest(ref).addCallback(ref, wellKnownRef, ttl, callback);
                    return;
                }
            }
        }
        else
        {
            Reference r = _table.getObjectReference(ref.getIdentity(), ttl, cached);
            if(!cached.value)
            {
                if(_background && r != null)
                {
                    getObjectRequest(ref).addCallback(ref, null, ttl, null);
                }
                else
                {
                    getObjectRequest(ref).addCallback(ref, null, ttl, callback);
                    return;
                }
            }

            if(!r.isIndirect())
            {
                endpoints = r.getEndpoints();
            }
            else if(!r.isWellKnown())
            {
                getEndpoints(r, ref, ttl, callback);
                return;
            }
        }

        assert(endpoints != null);
        if(ref.getInstance().traceLevels().location >= 1)
        {
            getEndpointsTrace(ref, endpoints, true);
        }
        if(callback != null)
        {
            callback.setEndpoints(endpoints, true);
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
            Reference r = _table.removeObjectReference(ref.getIdentity());
            if(r != null)
            {
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

        StringBuilder s = new StringBuilder(128);
        s.append(msg);
        s.append("\n");
        if(!ref.isWellKnown())
        {
            s.append("adapter = ");
            s.append(ref.getAdapterId());
            s.append("\n");
        }
        else
        {
            s.append("object = ");
            s.append(Ice.Util.identityToString(ref.getIdentity()));
            s.append("\n");
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
                StringBuilder s = new StringBuilder(128);
                s.append("adapter not found\n");
                s.append("adapter = ");
                s.append(ref.getAdapterId());
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
                StringBuilder s = new StringBuilder(128);
                s.append("object not found\n");
                s.append("object = ");
                s.append(Ice.Util.identityToString(ref.getIdentity()));
                instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.toString());
            }

            Ice.NotRegisteredException e = new Ice.NotRegisteredException();
            e.kindOfObject = "object";
            e.id = Ice.Util.identityToString(ref.getIdentity());
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
                StringBuilder s = new StringBuilder(128);
                s.append("couldn't contact the locator to retrieve adapter endpoints\n");
                if(ref.getAdapterId().length() > 0)
                {
                    s.append("adapter = ");
                    s.append(ref.getAdapterId());
                    s.append("\n");
                }
                else
                {
                    s.append("object = ");
                    s.append(Ice.Util.identityToString(ref.getIdentity()));
                    s.append("\n");
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
            StringBuilder s = new StringBuilder(128);
            s.append("no endpoints configured for ");
            if(ref.getAdapterId().length() > 0)
            {
                s.append("adapter\n");
                s.append("adapter = ");
                s.append(ref.getAdapterId());
                s.append("\n");
            }
            else
            {
                s.append("object\n");
                s.append("object = ");
                s.append(Ice.Util.identityToString(ref.getIdentity()));
                s.append("\n");
            }
            instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.toString());
        }
    }

    synchronized private Request
    getAdapterRequest(Reference ref)
    {
        if(ref.getInstance().traceLevels().location >= 1)
        {
            Instance instance = ref.getInstance();
            StringBuilder s = new StringBuilder(128);
            s.append("searching for adapter by id\n");
            s.append("adapter = ");
            s.append(ref.getAdapterId());
            instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.toString());
        }

        Request request = _adapterRequests.get(ref.getAdapterId());
        if(request != null)
        {
            return request;
        }
        request = new AdapterRequest(this, ref);
        _adapterRequests.put(ref.getAdapterId(), request);
        return request;
    }

    synchronized private Request
    getObjectRequest(Reference ref)
    {
        if(ref.getInstance().traceLevels().location >= 1)
        {
            Instance instance = ref.getInstance();
            StringBuilder s = new StringBuilder(128);
            s.append("searching for object by id\n");
            s.append("object = ");
            s.append(Ice.Util.identityToString(ref.getIdentity()));
            instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.toString());
        }

        Request request = _objectRequests.get(ref.getIdentity());
        if(request != null)
        {
            return request;
        }
        request = new ObjectRequest(this, ref);
        _objectRequests.put(ref.getIdentity(), request);
        return request;
    }

    private void
    finishRequest(Reference ref, java.util.List<Reference> wellKnownRefs, Ice.ObjectPrx proxy, boolean notRegistered)
    {
        if(proxy == null || ((Ice.ObjectPrxHelperBase)proxy).__reference().isIndirect())
        {
            //
            // Remove the cached references of well-known objects for which we tried
            // to resolved the endpoints if these endpoints are empty.
            //
            for(Reference r : wellKnownRefs)
            {
                _table.removeObjectReference(r.getIdentity());
            }
        }

        if(!ref.isWellKnown())
        {
            if(proxy != null && !((Ice.ObjectPrxHelperBase)proxy).__reference().isIndirect())
            {
                // Cache the adapter endpoints.
                _table.addAdapterEndpoints(ref.getAdapterId(),
                                           ((Ice.ObjectPrxHelperBase)proxy).__reference().getEndpoints());
            }
            else if(notRegistered) // If the adapter isn't registered anymore, remove it from the cache.
            {
                _table.removeAdapterEndpoints(ref.getAdapterId());
            }

            synchronized(this)
            {
                assert(_adapterRequests.get(ref.getAdapterId()) != null);
                _adapterRequests.remove(ref.getAdapterId());
            }
        }
        else
        {
            if(proxy != null && !((Ice.ObjectPrxHelperBase)proxy).__reference().isWellKnown())
            {
                // Cache the well-known object reference.
                _table.addObjectReference(ref.getIdentity(), ((Ice.ObjectPrxHelperBase)proxy).__reference());
            }
            else if(notRegistered) // If the well-known object isn't registered anymore, remove it from the cache.
            {
                _table.removeObjectReference(ref.getIdentity());
            }

            synchronized(this)
            {
                assert(_objectRequests.get(ref.getIdentity()) != null);
                _objectRequests.remove(ref.getIdentity());
            }
        }
    }

    private final Ice.LocatorPrx _locator;
    private Ice.LocatorRegistryPrx _locatorRegistry;
    private final LocatorTable _table;
    private final boolean _background;

    private java.util.Map<String, Request> _adapterRequests = new java.util.HashMap<String, Request>();
    private java.util.Map<Ice.Identity, Request> _objectRequests = new java.util.HashMap<Ice.Identity, Request>();
}
