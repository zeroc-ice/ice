// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.time.Duration;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

final class LocatorInfo {
    interface GetEndpointsCallback {
        void setEndpoints(EndpointI[] endpoints, boolean cached);

        void setException(LocalException ex);
    }

    private static class RequestCallback {
        public void response(LocatorInfo locatorInfo, ObjectPrx proxy) {
            EndpointI[] endpoints = null;
            if (proxy != null) {
                Reference r = ((_ObjectPrxI) proxy)._getReference();
                if (_ref.isWellKnown()
                    && !Protocol.isSupported(_ref.getEncoding(), r.getEncoding())) {
                    //
                    // If a well-known proxy and the returned proxy encoding isn't supported, we're
                    // done: there's
                    // no compatible endpoint we can use.
                    //
                } else if (!r.isIndirect()) {
                    endpoints = r.getEndpoints();
                } else if (_ref.isWellKnown() && !r.isWellKnown()) {
                    //
                    // We're resolving the endpoints of a well-known object and the proxy returned
                    // by the locator is an indirect proxy. We now need to resolve the endpoints
                    // of this indirect proxy.
                    //
                    if (_ref.getInstance().traceLevels().location >= 1) {
                        locatorInfo.trace(
                            "retrieved adapter for well-known object from locator, "
                                + "adding to locator cache",
                            _ref,
                            r);
                    }
                    locatorInfo.getEndpoints(r, _ref, _ttl, _callback);
                    return;
                }
            }

            if (_ref.getInstance().traceLevels().location >= 1) {
                locatorInfo.getEndpointsTrace(_ref, endpoints, false);
            }
            if (_callback != null) {
                _callback.setEndpoints(endpoints == null ? new EndpointI[0] : endpoints, false);
            }
        }

        public void exception(LocatorInfo locatorInfo, Exception exc) {
            try {
                locatorInfo.getEndpointsException(_ref, exc); // This throws.
            } catch (LocalException ex) {
                if (_callback != null) {
                    _callback.setException(ex);
                }
            }
        }

        RequestCallback(Reference ref, Duration ttl, GetEndpointsCallback cb) {
            _ref = ref;
            _ttl = ttl;
            _callback = cb;
        }

        final Reference _ref;
        final Duration _ttl;
        final GetEndpointsCallback _callback;
    }

    private abstract class Request {
        public void addCallback(
                Reference ref, Reference wellKnownRef, Duration ttl, GetEndpointsCallback cb) {
            RequestCallback callback = new RequestCallback(ref, ttl, cb);
            synchronized (this) {
                if (!_response && _exception == null) {
                    _callbacks.add(callback);
                    if (wellKnownRef != null) {
                        // This request is to resolve the endpoints of a cached well-known object ref.
                        _wellKnownRefs.add(wellKnownRef);
                    }
                    if (!_sent) {
                        _sent = true;
                        send();
                    }
                    return;
                }
            }

            if (_response) {
                callback.response(_locatorInfo, _proxy);
            } else {
                assert (_exception != null);
                callback.exception(_locatorInfo, _exception);
            }
        }

        Request(LocatorInfo locatorInfo, Reference ref) {
            _locatorInfo = locatorInfo;
            _ref = ref;
            _sent = false;
            _response = false;
        }

        protected void response(ObjectPrx proxy) {
            synchronized (this) {
                _locatorInfo.finishRequest(_ref, _wellKnownRefs, proxy, false);
                _response = true;
                _proxy = proxy;
                notifyAll();
            }
            for (RequestCallback callback : _callbacks) {
                callback.response(_locatorInfo, proxy);
            }
        }

        protected void exception(Exception ex) {
            synchronized (this) {
                _locatorInfo.finishRequest(_ref, _wellKnownRefs, null, ex instanceof UserException);
                _exception = ex;
                notifyAll();
            }
            for (RequestCallback callback : _callbacks) {
                callback.exception(_locatorInfo, ex);
            }
        }

        protected abstract void send();

        protected final LocatorInfo _locatorInfo;
        protected final Reference _ref;

        private List<RequestCallback> _callbacks = new ArrayList<>();
        private List<Reference> _wellKnownRefs = new ArrayList<>();
        private boolean _sent;
        private boolean _response;
        private ObjectPrx _proxy;
        private Exception _exception;
    }

    private class ObjectRequest extends Request {
        public ObjectRequest(LocatorInfo locatorInfo, Reference reference) {
            super(locatorInfo, reference);
            assert (reference.isWellKnown());
        }

        @Override
        protected void send() {
            try {
                _locatorInfo
                    .getLocator()
                    .findObjectByIdAsync(_ref.getIdentity())
                    .whenComplete(
                        (ObjectPrx proxy, Throwable ex) -> {
                            if (ex != null) {
                                if (ex instanceof LocalException) {
                                    exception((LocalException) ex);
                                } else if (ex instanceof UserException) {
                                    exception((UserException) ex);
                                } else {
                                    exception(new UnknownException(ex));
                                }
                            } else {
                                response(proxy);
                            }
                        });
            } catch (Exception ex) {
                exception(ex);
            }
        }
    }

    private class AdapterRequest extends Request {
        public AdapterRequest(LocatorInfo locatorInfo, Reference reference) {
            super(locatorInfo, reference);
            assert (reference.isIndirect());
        }

        @Override
        protected void send() {
            try {
                _locatorInfo
                    .getLocator()
                    .findAdapterByIdAsync(_ref.getAdapterId())
                    .whenComplete(
                        (ObjectPrx proxy, Throwable ex) -> {
                            if (ex != null) {
                                if (ex instanceof LocalException) {
                                    exception((LocalException) ex);
                                } else if (ex instanceof UserException) {
                                    exception((UserException) ex);
                                } else {
                                    exception(new UnknownException(ex));
                                }
                            } else {
                                response(proxy);
                            }
                        });
            } catch (Exception ex) {
                exception(ex);
            }
        }
    }

    LocatorInfo(LocatorPrx locator, LocatorTable table, boolean background) {
        _locator = locator;
        _table = table;
        _background = background;
    }

    public synchronized void destroy() {
        _locatorRegistry = null;
        _table.clear();
    }

    @Override
    public boolean equals(java.lang.Object obj) {
        if (this == obj) {
            return true;
        }

        if (obj instanceof LocatorInfo) {
            return _locator.equals(((LocatorInfo) obj)._locator);
        }

        return false;
    }

    @Override
    public int hashCode() {
        return _locator.hashCode();
    }

    public LocatorPrx getLocator() {
        //
        // No synchronization necessary, _locator is immutable.
        //
        return _locator;
    }

    public LocatorRegistryPrx getLocatorRegistry() {
        synchronized (this) {
            if (_locatorRegistry != null) {
                return _locatorRegistry;
            }
        }

        //
        // Do not make locator calls from within sync.
        //
        LocatorRegistryPrx locatorRegistry = _locator.getRegistry();
        if (locatorRegistry == null) {
            return null;
        }

        synchronized (this) {
            //
            // The locator registry can't be located. We use ordered
            // endpoint selection in case the locator returned a proxy
            // with some endpoints which are preferred to be tried first.
            //
            _locatorRegistry =
                locatorRegistry
                    .ice_locator(null)
                    .ice_endpointSelection(EndpointSelectionType.Ordered);
            return _locatorRegistry;
        }
    }

    public void getEndpoints(Reference ref, Duration ttl, GetEndpointsCallback callback) {
        getEndpoints(ref, null, ttl, callback);
    }

    public void getEndpoints(
            Reference ref, Reference wellKnownRef, Duration ttl, GetEndpointsCallback callback) {
        assert (ref.isIndirect());
        EndpointI[] endpoints = null;
        Holder<Boolean> cached = new Holder<>();
        if (!ref.isWellKnown()) {
            endpoints = _table.getAdapterEndpoints(ref.getAdapterId(), ttl, cached);
            if (!cached.value) {
                if (_background && endpoints != null) {
                    getAdapterRequest(ref).addCallback(ref, wellKnownRef, ttl, null);
                } else {
                    getAdapterRequest(ref).addCallback(ref, wellKnownRef, ttl, callback);
                    return;
                }
            }
        } else {
            Reference r = _table.getObjectReference(ref.getIdentity(), ttl, cached);
            if (!cached.value) {
                if (_background && r != null) {
                    getObjectRequest(ref).addCallback(ref, null, ttl, null);
                } else {
                    getObjectRequest(ref).addCallback(ref, null, ttl, callback);
                    return;
                }
            }

            if (!r.isIndirect()) {
                endpoints = r.getEndpoints();
            } else if (!r.isWellKnown()) {
                if (ref.getInstance().traceLevels().location >= 1) {
                    trace("found adapter for well-known object in locator cache", ref, r);
                }
                getEndpoints(r, ref, ttl, callback);
                return;
            }
        }

        assert (endpoints != null);
        if (ref.getInstance().traceLevels().location >= 1) {
            getEndpointsTrace(ref, endpoints, true);
        }
        if (callback != null) {
            callback.setEndpoints(endpoints, true);
        }
    }

    public void clearCache(Reference ref) {
        assert (ref.isIndirect());

        if (!ref.isWellKnown()) {
            EndpointI[] endpoints = _table.removeAdapterEndpoints(ref.getAdapterId());

            if (endpoints != null && ref.getInstance().traceLevels().location >= 2) {
                trace("removed endpoints for adapter from locator cache", ref, endpoints);
            }
        } else {
            Reference r = _table.removeObjectReference(ref.getIdentity());
            if (r != null) {
                if (!r.isIndirect()) {
                    if (ref.getInstance().traceLevels().location >= 2) {
                        trace(
                            "removed endpoints for well-known object from locator cache",
                            ref,
                            r.getEndpoints());
                    }
                } else if (!r.isWellKnown()) {
                    if (ref.getInstance().traceLevels().location >= 2) {
                        trace("removed adapter for well-known object from locator cache", ref, r);
                    }
                    clearCache(r);
                }
            }
        }
    }

    private void trace(String msg, Reference ref, EndpointI[] endpoints) {
        assert (ref.isIndirect());

        StringBuilder s = new StringBuilder(128);
        s.append(msg);
        s.append("\n");
        if (!ref.isWellKnown()) {
            s.append("adapter = ");
            s.append(ref.getAdapterId());
            s.append("\n");
        } else {
            s.append("well-known proxy = ");
            s.append(ref.toString());
            s.append("\n");
        }

        s.append("endpoints = ");
        final int sz = endpoints.length;
        for (int i = 0; i < sz; i++) {
            s.append(endpoints[i].toString());
            if (i + 1 < sz) {
                s.append(':');
            }
        }

        ref.getInstance()
            .initializationData()
            .logger
            .trace(ref.getInstance().traceLevels().locationCat, s.toString());
    }

    private void trace(String msg, Reference ref, Reference resolved) {
        assert (ref.isWellKnown());

        StringBuilder s = new StringBuilder(128);
        s.append(msg);
        s.append("\n");
        s.append("well-known proxy = ");
        s.append(ref.toString());
        s.append("\n");
        s.append("adapter = ");
        s.append(resolved.getAdapterId());

        ref.getInstance()
            .initializationData()
            .logger
            .trace(ref.getInstance().traceLevels().locationCat, s.toString());
    }

    private void getEndpointsException(Reference ref, Exception exc) {
        assert (ref.isIndirect());

        try {
            throw exc;
        } catch (AdapterNotFoundException ex) {
            final Instance instance = ref.getInstance();
            if (instance.traceLevels().location >= 1) {
                StringBuilder s = new StringBuilder(128);
                s.append("adapter not found\n");
                s.append("adapter = ");
                s.append(ref.getAdapterId());
                instance.initializationData()
                    .logger
                    .trace(instance.traceLevels().locationCat, s.toString());
            }

            throw new NotRegisteredException("object adapter", ref.getAdapterId());
        } catch (ObjectNotFoundException ex) {
            final Instance instance = ref.getInstance();
            if (instance.traceLevels().location >= 1) {
                StringBuilder s = new StringBuilder(128);
                s.append("object not found\n");
                s.append("object = ");
                s.append(Util.identityToString(ref.getIdentity(), instance.toStringMode()));
                instance.initializationData()
                    .logger
                    .trace(instance.traceLevels().locationCat, s.toString());
            }

            final String id = Util.identityToString(ref.getIdentity(), instance.toStringMode());
            throw new NotRegisteredException("object", id);
        } catch (NotRegisteredException ex) {
            throw ex;
        } catch (LocalException ex) {
            final Instance instance = ref.getInstance();
            if (instance.traceLevels().location >= 1) {
                StringBuilder s = new StringBuilder(128);
                s.append("couldn't contact the locator to retrieve endpoints\n");
                if (ref.getAdapterId().length() > 0) {
                    s.append("adapter = ");
                    s.append(ref.getAdapterId());
                    s.append("\n");
                } else {
                    s.append("well-known proxy = ");
                    s.append(ref.toString());
                    s.append("\n");
                }
                s.append("reason = ").append(ex);
                instance.initializationData()
                    .logger
                    .trace(instance.traceLevels().locationCat, s.toString());
            }
            throw ex;
        } catch (Exception ex) {
            assert false;
        }
    }

    private void getEndpointsTrace(Reference ref, EndpointI[] endpoints, boolean cached) {
        if (endpoints != null && endpoints.length > 0) {
            if (cached) {
                if (ref.isWellKnown()) {
                    trace("found endpoints for well-known proxy in locator cache", ref, endpoints);
                } else {
                    trace("found endpoints for adapter in locator cache", ref, endpoints);
                }
            } else {
                if (ref.isWellKnown()) {
                    trace(
                        "retrieved endpoints for well-known proxy from locator, adding to locator cache",
                        ref,
                        endpoints);
                } else {
                    trace(
                        "retrieved endpoints for adapter from locator, adding to locator cache",
                        ref,
                        endpoints);
                }
            }
        } else {
            final Instance instance = ref.getInstance();
            StringBuilder s = new StringBuilder(128);
            s.append("no endpoints configured for ");
            if (ref.getAdapterId().length() > 0) {
                s.append("adapter\n");
                s.append("adapter = ");
                s.append(ref.getAdapterId());
                s.append("\n");
            } else {
                s.append("well-known object\n");
                s.append("well-known proxy = ");
                s.append(ref.toString());
                s.append("\n");
            }
            instance.initializationData()
                .logger
                .trace(instance.traceLevels().locationCat, s.toString());
        }
    }

    private synchronized Request getAdapterRequest(Reference ref) {
        if (ref.getInstance().traceLevels().location >= 1) {
            Instance instance = ref.getInstance();
            StringBuilder s = new StringBuilder(128);
            s.append("searching for adapter by id\n");
            s.append("adapter = ");
            s.append(ref.getAdapterId());
            instance.initializationData()
                .logger
                .trace(instance.traceLevels().locationCat, s.toString());
        }

        Request request = _adapterRequests.get(ref.getAdapterId());
        if (request != null) {
            return request;
        }
        request = new AdapterRequest(this, ref);
        _adapterRequests.put(ref.getAdapterId(), request);
        return request;
    }

    private synchronized Request getObjectRequest(Reference ref) {
        if (ref.getInstance().traceLevels().location >= 1) {
            Instance instance = ref.getInstance();
            StringBuilder s = new StringBuilder(128);
            s.append("searching for well-known object\n");
            s.append("well-known proxy = ");
            s.append(ref.toString());
            instance.initializationData()
                .logger
                .trace(instance.traceLevels().locationCat, s.toString());
        }

        Request request = _objectRequests.get(ref.getIdentity());
        if (request != null) {
            return request;
        }
        request = new ObjectRequest(this, ref);
        _objectRequests.put(ref.getIdentity(), request);
        return request;
    }

    private void finishRequest(
            Reference ref,
            List<Reference> wellKnownRefs,
            ObjectPrx proxy,
            boolean notRegistered) {
        if (proxy == null || ((_ObjectPrxI) proxy)._getReference().isIndirect()) {
            //
            // Remove the cached references of well-known objects for which we tried
            // to resolved the endpoints if these endpoints are empty.
            //
            for (Reference r : wellKnownRefs) {
                _table.removeObjectReference(r.getIdentity());
            }
        }

        if (!ref.isWellKnown()) {
            if (proxy != null && !((_ObjectPrxI) proxy)._getReference().isIndirect()) {
                // Cache the adapter endpoints.
                _table.addAdapterEndpoints(
                    ref.getAdapterId(), ((_ObjectPrxI) proxy)._getReference().getEndpoints());
            } else if (notRegistered) {
                // If the adapter isn't registered anymore, remove it from the cache.
                _table.removeAdapterEndpoints(ref.getAdapterId());
            }

            synchronized (this) {
                assert (_adapterRequests.get(ref.getAdapterId()) != null);
                _adapterRequests.remove(ref.getAdapterId());
            }
        } else {
            if (proxy != null && !((_ObjectPrxI) proxy)._getReference().isWellKnown()) {
                // Cache the well-known object reference.
                _table.addObjectReference(ref.getIdentity(), ((_ObjectPrxI) proxy)._getReference());
            } else if (notRegistered) {
                // If the well-known object isn't registered anymore, remove it from the cache.
                _table.removeObjectReference(ref.getIdentity());
            }

            synchronized (this) {
                assert (_objectRequests.get(ref.getIdentity()) != null);
                _objectRequests.remove(ref.getIdentity());
            }
        }
    }

    private final LocatorPrx _locator;
    private LocatorRegistryPrx _locatorRegistry;
    private final LocatorTable _table;
    private final boolean _background;

    private final Map<String, Request> _adapterRequests = new HashMap<>();
    private final Map<Identity, Request> _objectRequests = new HashMap<>();
}
