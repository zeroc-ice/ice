// Copyright (c) ZeroC, Inc.

package com.zeroc.IceDiscovery;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.Endpoint;
import com.zeroc.Ice.EndpointInfo;
import com.zeroc.Ice.IPEndpointInfo;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.UDPEndpointInfo;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Future;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

class LookupI implements Lookup {
    private abstract class Request<T, Ret> implements Runnable {
        Request(T id, int retryCount) {
            _id = id;
            _requestId = UUID.randomUUID().toString();
            _retryCount = retryCount;
        }

        T getId() {
            return _id;
        }

        boolean addFuture(CompletableFuture<Ret> f) {
            _futures.add(f);
            return _futures.size() == 1;
        }

        boolean retry() {
            return --_retryCount >= 0;
        }

        void invoke(String domainId, Map<LookupPrx, LookupReplyPrx> lookups) {
            _lookupCount = lookups.size();
            _failureCount = 0;
            final Identity id = new Identity(_requestId, "");
            for (Map.Entry<LookupPrx, LookupReplyPrx> entry : lookups.entrySet()) {
                invokeWithLookup(
                    domainId,
                    entry.getKey(),
                    LookupReplyPrx.uncheckedCast(entry.getValue().ice_identity(id)));
            }
        }

        boolean exception() {
            if (++_failureCount == _lookupCount) {
                finished(null);
                return true;
            }
            return false;
        }

        String getRequestId() {
            return _requestId;
        }

        void scheduleTimer(long timeout) {
            _future = _timer.schedule(this, timeout, TimeUnit.MILLISECONDS);
        }

        void cancelTimer() {
            assert _future != null;
            _future.cancel(false);
            _future = null;
        }

        abstract void finished(ObjectPrx proxy);

        protected abstract void invokeWithLookup(
                String domainId, LookupPrx lookup, LookupReplyPrx lookupReply);

        private final String _requestId;

        protected int _retryCount;
        protected int _lookupCount;
        protected int _failureCount;
        protected List<CompletableFuture<Ret>> _futures = new ArrayList<>();
        protected T _id;
        protected Future<?> _future;
    }

    private class AdapterRequest extends Request<String, ObjectPrx> {
        AdapterRequest(String id, int retryCount) {
            super(id, retryCount);
            _start = System.nanoTime();
            _latency = 0;
        }

        @Override
        boolean retry() {
            return _proxies.isEmpty() && --_retryCount >= 0;
        }

        boolean response(ObjectPrx proxy, boolean isReplicaGroup) {
            if (isReplicaGroup) {
                _proxies.add(proxy);
                if (_latency == 0) {
                    _latency =
                        (long) ((System.nanoTime() - _start) * _latencyMultiplier / 100000.0);
                    if (_latency == 0) {
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
        void finished(ObjectPrx proxy) {
            if (proxy != null || _proxies.isEmpty()) {
                sendResponse(proxy);
            } else if (_proxies.size() == 1) {
                sendResponse(_proxies.toArray(new ObjectPrx[1])[0]);
            } else {
                List<Endpoint> endpoints = new ArrayList<>();
                ObjectPrx result = null;
                for (ObjectPrx prx : _proxies) {
                    if (result == null) {
                        result = prx;
                    }
                    endpoints.addAll(Arrays.asList(prx.ice_getEndpoints()));
                }
                sendResponse(
                    result.ice_endpoints(
                        endpoints.toArray(new Endpoint[endpoints.size()])));
            }
        }

        @Override
        public void run() {
            adapterRequestTimedOut(this);
        }

        @Override
        protected void invokeWithLookup(
                String domainId, LookupPrx lookup, LookupReplyPrx lookupReply) {
            lookup.findAdapterByIdAsync(domainId, _id, lookupReply)
                .whenCompleteAsync(
                    (v, ex) -> {
                        if (ex != null) {
                            adapterRequestException(AdapterRequest.this, ex);
                        }
                    },
                    lookup.ice_executor());
        }

        private void sendResponse(ObjectPrx proxy) {
            for (CompletableFuture<ObjectPrx> f : _futures) {
                f.complete(proxy);
            }
            _futures.clear();
        }

        // We use a set because the same IceDiscovery plugin might return multiple times
        // the same proxy if it's accessible through multiple network interfaces and if we
        // also sent the request to multiple interfaces.
        private Set<ObjectPrx> _proxies = new HashSet<>();
        private long _start;
        private long _latency;
    }

    private class ObjectRequest extends Request<Identity, ObjectPrx> {
        ObjectRequest(Identity id, int retryCount) {
            super(id, retryCount);
        }

        void response(ObjectPrx proxy) {
            finished(proxy);
        }

        @Override
        void finished(ObjectPrx proxy) {
            for (CompletableFuture<ObjectPrx> f : _futures) {
                f.complete(proxy);
            }
            _futures.clear();
        }

        @Override
        public void run() {
            objectRequestTimedOut(this);
        }

        @Override
        protected void invokeWithLookup(
                String domainId, LookupPrx lookup, LookupReplyPrx lookupReply) {
            lookup.findObjectByIdAsync(domainId, _id, lookupReply)
                .whenCompleteAsync(
                    (v, ex) -> {
                        if (ex != null) {
                            objectRequestException(ObjectRequest.this, ex);
                        }
                    },
                    lookup.ice_executor());
        }
    }

    public LookupI(
            LocatorRegistryI registry, LookupPrx lookup, Properties properties) {
        _registry = registry;
        _lookup = lookup;
        _timeout = properties.getIcePropertyAsInt("IceDiscovery.Timeout");
        _retryCount = properties.getIcePropertyAsInt("IceDiscovery.RetryCount");
        _latencyMultiplier = properties.getIcePropertyAsInt("IceDiscovery.LatencyMultiplier");
        _domainId = properties.getIceProperty("IceDiscovery.DomainId");
        _timer = lookup.ice_getCommunicator().getInstance().timer();

        Endpoint[] single = new Endpoint[1];
        for (Endpoint endpt : lookup.ice_getEndpoints()) {
            single[0] = endpt;
            _lookups.put((LookupPrx) lookup.ice_endpoints(single), null);
        }
        assert (!_lookups.isEmpty());
    }

    void setLookupReply(LookupReplyPrx lookupReply) {
        // Use a lookup reply proxy whose address matches the interface used to send multicast
        // datagrams.
        Endpoint[] single = new Endpoint[1];
        for (Map.Entry<LookupPrx, LookupReplyPrx> entry : _lookups.entrySet()) {
            UDPEndpointInfo info =
                (UDPEndpointInfo) entry.getKey().ice_getEndpoints()[0].getInfo();
            if (!info.mcastInterface.isEmpty()) {
                for (Endpoint q : lookupReply.ice_getEndpoints()) {
                    EndpointInfo r = q.getInfo();
                    if (r instanceof IPEndpointInfo
                        && ((IPEndpointInfo) r)
                        .host.equals(info.mcastInterface)) {
                        single[0] = q;
                        entry.setValue((LookupReplyPrx) lookupReply.ice_endpoints(single));
                    }
                }
            }

            if (entry.getValue() == null) {
                // Fallback: just use the given lookup reply proxy if no matching endpoint found.
                entry.setValue(lookupReply);
            }
        }
    }

    @Override
    public void findObjectById(
            String domainId,
            Identity id,
            LookupReplyPrx reply,
            Current c) {
        if (!domainId.equals(_domainId)) {
            return; // Ignore.
        }

        ObjectPrx proxy = _registry.findObject(id);
        if (proxy != null) {
            // Reply to the multicast request using the given proxy.
            try {
                reply.foundObjectByIdAsync(id, proxy);
            } catch (LocalException ex) {
                // Ignore
            }
        }
    }

    @Override
    public void findAdapterById(
            String domainId,
            String adapterId,
            LookupReplyPrx reply,
            Current c) {
        if (!domainId.equals(_domainId)) {
            return; // Ignore.
        }

        LocatorRegistryI.FindAdapterResult r = _registry.findAdapter(adapterId);
        if (r.returnValue != null) {
            // Reply to the multicast request using the given proxy.
            try {
                reply.foundAdapterByIdAsync(adapterId, r.returnValue, r.isReplicaGroup);
            } catch (LocalException ex) {
                // Ignore
            }
        }
    }

    synchronized void findObject(
            CompletableFuture<ObjectPrx> f, Identity id) {
        ObjectRequest request = _objectRequests.get(id);
        if (request == null) {
            request = new ObjectRequest(id, _retryCount);
            _objectRequests.put(id, request);
        }

        if (request.addFuture(f)) {
            try {
                request.invoke(_domainId, _lookups);
                request.scheduleTimer(_timeout);
            } catch (LocalException ex) {
                request.finished(null);
                _objectRequests.remove(id);
            }
        }
    }

    synchronized void findAdapter(CompletableFuture<ObjectPrx> f, String adapterId) {
        AdapterRequest request = _adapterRequests.get(adapterId);
        if (request == null) {
            request = new AdapterRequest(adapterId, _retryCount);
            _adapterRequests.put(adapterId, request);
        }

        if (request.addFuture(f)) {
            try {
                request.invoke(_domainId, _lookups);
                request.scheduleTimer(_timeout);
            } catch (LocalException ex) {
                request.finished(null);
                _adapterRequests.remove(adapterId);
            }
        }
    }

    synchronized void foundObject(
            Identity id, String requestId, ObjectPrx proxy) {
        ObjectRequest request = _objectRequests.get(id);
        if (request != null
            && request.getRequestId().equals(requestId)) // Ignore responses from old requests
            {
                request.response(proxy);
                request.cancelTimer();
                _objectRequests.remove(id);
            }
    }

    synchronized void foundAdapter(
            String adapterId,
            String requestId,
            ObjectPrx proxy,
            boolean isReplicaGroup) {
        AdapterRequest request = _adapterRequests.get(adapterId);
        if (request != null
            && request.getRequestId().equals(requestId)) // Ignore responses from old requests
            {
                if (request.response(proxy, isReplicaGroup)) {
                    request.cancelTimer();
                    _adapterRequests.remove(adapterId);
                }
            }
    }

    synchronized void objectRequestTimedOut(ObjectRequest request) {
        ObjectRequest r = _objectRequests.get(request.getId());
        if (r == null || request != r) {
            return;
        }

        if (request.retry()) {
            try {
                request.invoke(_domainId, _lookups);
                request.scheduleTimer(_timeout);
                return;
            } catch (LocalException ex) {}
        }

        request.finished(null);
        _objectRequests.remove(request.getId());
    }

    synchronized void objectRequestException(ObjectRequest request, Throwable ex) {
        ObjectRequest r = _objectRequests.get(request.getId());
        if (r == null || r != request) {
            return;
        }

        if (request.exception()) {
            if (_warnOnce) {
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

    synchronized void adapterRequestTimedOut(AdapterRequest request) {
        AdapterRequest r = _adapterRequests.get(request.getId());
        if (r == null || r != request) {
            return;
        }

        if (request.retry()) {
            try {
                request.invoke(_domainId, _lookups);
                request.scheduleTimer(_timeout);
                return;
            } catch (LocalException ex) {}
        }

        request.finished(null);
        _adapterRequests.remove(request.getId());
    }

    synchronized void adapterRequestException(AdapterRequest request, Throwable ex) {
        AdapterRequest r = _adapterRequests.get(request.getId());
        if (r == null || r != request) {
            return;
        }

        if (request.exception()) {
            if (_warnOnce) {
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

    private final LocatorRegistryI _registry;
    private final LookupPrx _lookup;
    private final Map<LookupPrx, LookupReplyPrx> _lookups = new HashMap<>();
    private final int _timeout;
    private final int _retryCount;
    private final int _latencyMultiplier;
    private final String _domainId;

    private final ScheduledExecutorService _timer;
    private boolean _warnOnce = true;

    private final Map<Identity, ObjectRequest> _objectRequests = new HashMap<>();
    private final Map<String, AdapterRequest> _adapterRequests = new HashMap<>();
}
