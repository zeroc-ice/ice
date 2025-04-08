// Copyright (c) ZeroC, Inc.

package com.zeroc.IceLocatorDiscovery;

import com.zeroc.Ice.BlobjectAsync;
import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.CommunicatorDestroyedException;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.Endpoint;
import com.zeroc.Ice.EndpointInfo;
import com.zeroc.Ice.IPEndpointInfo;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.Locator;
import com.zeroc.Ice.LocatorPrx;
import com.zeroc.Ice.LocatorRegistryPrx;
import com.zeroc.Ice.Network;
import com.zeroc.Ice.NoEndpointException;
import com.zeroc.Ice.Object;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectAdapterDeactivatedException;
import com.zeroc.Ice.ObjectAdapterDestroyedException;
import com.zeroc.Ice.ObjectNotExistException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.OperationInterruptedException;
import com.zeroc.Ice.OperationMode;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.RequestFailedException;
import com.zeroc.Ice.Time;
import com.zeroc.Ice.UDPEndpointInfo;
import com.zeroc.Ice.UnknownException;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;
import java.util.concurrent.Future;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

class PluginI implements Plugin {
    private static class Request {
        Request(
                LocatorI locator,
                String operation,
                OperationMode mode,
                byte[] inParams,
                Map<String, String> context,
                CompletableFuture<Object.Ice_invokeResult> f) {
            _locator = locator;
            _operation = operation;
            _mode = mode;
            _inParams = inParams;
            _context = context;
            _future = f;
        }

        void invoke(LocatorPrx l) {
            if (_locatorPrx == null || !_locatorPrx.equals(l)) {
                _locatorPrx = l;
                try {
                    final CompletableFuture<Object.Ice_invokeResult> f =
                        l.ice_invokeAsync(_operation, _mode, _inParams, _context);
                    f.whenComplete(
                        (result, ex) -> {
                            if (ex != null) {
                                exception((LocalException) ex);
                            } else {
                                _future.complete(result);
                            }
                        });
                } catch (LocalException ex) {
                    exception(ex);
                }
            } else {
                assert (_exception != null); // Don't retry if the proxy didn't change
                exception(_exception);
            }
        }

        private void exception(LocalException ex) {
            try {
                throw ex;
            } catch (RequestFailedException exc) {
                _future.completeExceptionally(ex);
            } catch (UnknownException exc) {
                _future.completeExceptionally(ex);
            } catch (NoEndpointException exc) {
                _future.completeExceptionally(new ObjectNotExistException());
            } catch (ObjectAdapterDeactivatedException exc) {
                _future.completeExceptionally(new ObjectNotExistException());
            } catch (ObjectAdapterDestroyedException exc) {
                _future.completeExceptionally(new ObjectNotExistException());
            } catch (CommunicatorDestroyedException exc) {
                _future.completeExceptionally(new ObjectNotExistException());
            } catch (LocalException exc) {
                _exception = exc;
                _locator.invoke(_locatorPrx, Request.this); // Retry with new locator proxy
            }
        }

        private final LocatorI _locator;
        private LocalException _exception;
        private final String _operation;
        private final OperationMode _mode;
        private final Map<String, String> _context;
        private final byte[] _inParams;
        private final CompletableFuture<Object.Ice_invokeResult> _future;

        private LocatorPrx _locatorPrx;
    }

    private static class VoidLocatorI implements Locator {
        @Override
        public CompletionStage<ObjectPrx> findObjectByIdAsync(
                Identity id, Current current) {
            return CompletableFuture.completedFuture((ObjectPrx) null);
        }

        @Override
        public CompletionStage<ObjectPrx> findAdapterByIdAsync(
                String id, Current current) {
            return CompletableFuture.completedFuture((ObjectPrx) null);
        }

        @Override
        public LocatorRegistryPrx getRegistry(Current current) {
            return null;
        }
    }

    private static class LocatorI implements BlobjectAsync {
        LocatorI(
                LookupPrx lookup,
                Properties properties,
                String instanceName,
                LocatorPrx voidLocator) {
            _lookup = lookup;
            _timeout = properties.getIcePropertyAsInt("IceLocatorDiscovery.Timeout");
            if (_timeout < 0) {
                _timeout = 300;
            }
            _retryCount = properties.getIcePropertyAsInt("IceLocatorDiscovery.RetryCount");
            if (_retryCount < 0) {
                _retryCount = 0;
            }
            _retryDelay = properties.getIcePropertyAsInt("IceLocatorDiscovery.RetryDelay");
            if (_retryDelay < 0) {
                _retryDelay = 0;
            }
            _timer = lookup.ice_getCommunicator().getInstance().timer();
            _traceLevel = properties.getIcePropertyAsInt("IceLocatorDiscovery.Trace.Lookup");
            _instanceName = instanceName;
            _warned = false;
            _locator = lookup.ice_getCommunicator().getDefaultLocator();
            _voidLocator = voidLocator;
            _pendingRetryCount = 0;
            _pending = false;
            _failureCount = 0;
            _warnOnce = true;

            // Create one lookup proxy per endpoint from the given proxy. We want to send a
            // multicast
            // datagram on each endpoint.
            Endpoint[] single = new Endpoint[1];
            for (Endpoint endpt : lookup.ice_getEndpoints()) {
                single[0] = endpt;
                _lookups.put((LookupPrx) lookup.ice_endpoints(single), null);
            }
            assert (!_lookups.isEmpty());
        }

        public void setLookupReply(LookupReplyPrx lookupReply) {
            // Use a lookup reply proxy whose address matches the interface used to send multicast
            // datagrams.
            Endpoint[] single = new Endpoint[1];
            for (Map.Entry<LookupPrx, LookupReplyPrx> entry : _lookups.entrySet()) {
                UDPEndpointInfo info =
                    (UDPEndpointInfo)
                        entry.getKey().ice_getEndpoints()[0].getInfo();
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
                    // Fallback: just use the given lookup reply proxy if no matching endpoint
                    // found.
                    entry.setValue(lookupReply);
                }
            }
        }

        @Override
        public CompletionStage<Object.Ice_invokeResult> ice_invokeAsync(
                byte[] inParams, Current current) {
            CompletableFuture<Object.Ice_invokeResult> f = new CompletableFuture<>();
            invoke(
                null,
                new Request(this, current.operation, current.mode, inParams, current.ctx, f));
            return f;
        }

        public List<LocatorPrx> getLocators(String instanceName, int waitTime) {
            // Clear locators from previous search.
            synchronized (this) {
                _locators.clear();
            }

            // Find a locator
            invoke(null, null);

            // Wait for responses
            try {
                if (instanceName.isEmpty()) {
                    Thread.sleep(waitTime);
                } else {
                    synchronized (this) {
                        while (!_locators.containsKey(instanceName) && _pending) {
                            wait(waitTime);
                        }
                    }
                }
            } catch (InterruptedException ex) {
                throw new OperationInterruptedException(ex);
            }

            // Return found locators
            synchronized (this) {
                return new ArrayList<>(_locators.values());
            }
        }

        public synchronized void foundLocator(LocatorPrx locator) {
            if (locator == null) {
                if (_traceLevel > 2) {
                    _lookup.ice_getCommunicator()
                        .getLogger()
                        .trace("Lookup", "ignoring locator reply: (null locator)");
                }
                return;
            }

            if (!_instanceName.isEmpty()
                && !locator.ice_getIdentity().category.equals(_instanceName)) {
                if (_traceLevel > 2) {
                    StringBuffer s =
                        new StringBuffer(
                            "ignoring locator reply: instance name doesn't match\n");
                    s.append("expected = ").append(_instanceName);
                    s.append("received = ").append(locator.ice_getIdentity().category);
                    _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.toString());
                }
                return;
            }

            // If we already have a locator assigned, ensure the given locator has the same
            // identity, otherwise ignore it.
            if (!_pendingRequests.isEmpty()
                && _locator != null
                && !locator.ice_getIdentity()
                .category
                .equals(_locator.ice_getIdentity().category)) {
                if (!_warned) {
                    _warned = true; // Only warn once

                    locator.ice_getCommunicator()
                        .getLogger()
                        .warning(
                            "received Ice locator with different instance name:\n"
                                + "using = `"
                                + _locator.ice_getIdentity().category
                                + "'\n"
                                + "received = `"
                                + locator.ice_getIdentity().category
                                + "'\n"
                                + "This is typically the case if multiple Ice locators"
                                + " with different instance names are deployed and the"
                                + " property `IceLocatorDiscovery.InstanceName'is not"
                                + " set.");
                }
                return;
            }

            if (_pending) // No need to continue, we found a locator
                {
                    _future.cancel(false);
                    _future = null;
                    _pendingRetryCount = 0;
                    _pending = false;
                }

            if (_traceLevel > 0) {
                StringBuffer s = new StringBuffer("locator lookup succeeded:\nlocator = ");
                s.append(locator);
                if (!_instanceName.isEmpty()) {
                    s.append("\ninstance name = ").append(_instanceName);
                }
                _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.toString());
            }

            LocatorPrx l =
                _pendingRequests.isEmpty()
                    ? _locators.get(locator.ice_getIdentity().category)
                    : _locator;
            if (l != null) {
                // We found another locator replica, append its endpoints to the
                // current locator proxy endpoints.
                List<Endpoint> newEndpoints =
                    new ArrayList<>(Arrays.asList(l.ice_getEndpoints()));
                for (Endpoint p : locator.ice_getEndpoints()) {
                    // Only add endpoints if not already in the locator proxy endpoints
                    boolean found = false;
                    for (Endpoint q : newEndpoints) {
                        if (p.equals(q)) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        newEndpoints.add(p);
                    }
                }
                l =
                    (LocatorPrx)
                        l.ice_endpoints(
                            newEndpoints.toArray(
                                new Endpoint[newEndpoints.size()]));
            } else {
                l = locator;
            }

            if (_pendingRequests.isEmpty()) {
                _locators.put(locator.ice_getIdentity().category, l);
                notify();
            } else {
                _locator = l;
                if (_instanceName.isEmpty()) {
                    _instanceName =
                        _locator.ice_getIdentity().category; // Stick to the first locator
                }

                // Send pending requests if any.
                for (Request req : _pendingRequests) {
                    req.invoke(_locator);
                }
                _pendingRequests.clear();
            }
        }

        public synchronized void invoke(LocatorPrx locator, Request request) {
            if (request != null && _locator != null && _locator != locator) {
                request.invoke(_locator);
            } else if (request != null
                && Time.currentMonotonicTimeMillis() < _nextRetry) {
                request.invoke(
                    _voidLocator); // Don't retry to find a locator before the retry delay
                // expires
            } else {
                _locator = null;

                if (request != null) {
                    _pendingRequests.add(request);
                }

                if (!_pending) // No request in progress
                    {
                        _pending = true;
                        _pendingRetryCount = _retryCount;
                        _failureCount = 0;
                        try {
                            if (_traceLevel > 1) {
                                StringBuilder s = new StringBuilder("looking up locator:\nlookup = ");
                                s.append(_lookup);
                                if (!_instanceName.isEmpty()) {
                                    s.append("\ninstance name = ").append(_instanceName);
                                }
                                _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.toString());
                            }
                            for (Map.Entry<LookupPrx, LookupReplyPrx> entry : _lookups.entrySet()) {
                                entry.getKey()
                                    .findLocatorAsync(_instanceName, entry.getValue())
                                    .whenCompleteAsync(
                                        (v, ex) -> {
                                            if (ex != null) {
                                                exception(ex);
                                            }
                                        },
                                        entry.getKey()
                                            .ice_executor()); // Send multicast request.
                            }
                            _future =
                                _timer.schedule(
                                    _retryTask,
                                    _timeout,
                                    TimeUnit.MILLISECONDS);
                        } catch (LocalException ex) {
                            if (_traceLevel > 0) {
                                StringBuilder s =
                                    new StringBuilder("locator lookup failed:\nlookup = ");
                                s.append(_lookup);
                                if (!_instanceName.isEmpty()) {
                                    s.append("\ninstance name = ").append(_instanceName);
                                }
                                s.append("\n").append(ex);
                                _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.toString());
                            }

                            for (Request req : _pendingRequests) {
                                req.invoke(_voidLocator);
                            }
                            _pendingRequests.clear();
                            _pending = false;
                            _pendingRetryCount = 0;
                        }
                    }
            }
        }

        synchronized void exception(Throwable ex) {
            if (++_failureCount == _lookups.size() && _pending) {
                // All the lookup calls failed, cancel the timer and propagate the error to the
                // requests.
                _future.cancel(false);
                _future = null;
                _pendingRetryCount = 0;
                _pending = false;

                if (_warnOnce) {
                    StringBuilder builder = new StringBuilder();
                    builder.append("failed to lookup locator with lookup proxy `");
                    builder.append(_lookup);
                    builder.append("':\n");
                    builder.append(ex);
                    _lookup.ice_getCommunicator().getLogger().warning(builder.toString());
                    _warnOnce = false;
                }

                if (_traceLevel > 0) {
                    StringBuilder s = new StringBuilder("locator lookup failed:\nlookup = ");
                    s.append(_lookup);
                    if (!_instanceName.isEmpty()) {
                        s.append("\ninstance name = ").append(_instanceName);
                    }
                    s.append("\n").append(ex);
                    _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.toString());
                }

                if (_pendingRequests.isEmpty()) {
                    notify();
                } else {
                    for (Request req : _pendingRequests) {
                        req.invoke(_voidLocator);
                    }
                    _pendingRequests.clear();
                }
            }
        }

        private Runnable _retryTask =
            new Runnable() {
                @Override
                public void run() {
                    synchronized (LocatorI.this) {
                        if (!_pending) {
                            assert (_pendingRequests.isEmpty());
                            return; // Request failed
                        }

                        if (_pendingRetryCount > 0) {
                            --_pendingRetryCount;
                            try {
                                if (_traceLevel > 1) {
                                    StringBuilder s =
                                        new StringBuilder(
                                            "retrying locator lookup:\nlookup = ");
                                    s.append(_lookup);
                                    s.append("\nretry count = ").append(_retryCount);
                                    if (!_instanceName.isEmpty()) {
                                        s.append("\ninstance name = ").append(_instanceName);
                                    }
                                    _lookup.ice_getCommunicator()
                                        .getLogger()
                                        .trace("Lookup", s.toString());
                                }

                                _failureCount = 0;
                                for (Map.Entry<LookupPrx, LookupReplyPrx> entry :
                                            _lookups.entrySet()) {
                                    entry.getKey()
                                        .findLocatorAsync(_instanceName, entry.getValue())
                                        .whenCompleteAsync(
                                            (v, ex) -> {
                                                if (ex != null) {
                                                    exception(ex);
                                                }
                                            },
                                            entry.getKey()
                                                .ice_executor()); // Send multicast
                                    // request.
                                }
                                _future =
                                    _timer.schedule(
                                        _retryTask,
                                        _timeout,
                                        TimeUnit.MILLISECONDS);
                                return;
                            } catch (LocalException ex) {
                            }
                            _pendingRetryCount = 0;
                        }

                        assert (_pendingRetryCount == 0);
                        _pending = false;

                        if (_traceLevel > 0) {
                            StringBuilder s =
                                new StringBuilder("locator lookup timed out:\nlookup = ");
                            s.append(_lookup);
                            if (!_instanceName.isEmpty()) {
                                s.append("\ninstance name = ").append(_instanceName);
                            }
                            _lookup.ice_getCommunicator()
                                .getLogger()
                                .trace("Lookup", s.toString());
                        }

                        if (_pendingRequests.isEmpty()) {
                            notify();
                        } else {
                            for (Request req : _pendingRequests) {
                                req.invoke(_voidLocator);
                            }
                            _pendingRequests.clear();
                        }
                        _nextRetry =
                            Time.currentMonotonicTimeMillis() + _retryDelay;
                    }
                }
            };

        private final LookupPrx _lookup;
        private final Map<LookupPrx, LookupReplyPrx> _lookups = new HashMap<>();
        private int _timeout;
        private Future<?> _future;
        private final ScheduledExecutorService _timer;
        private final int _traceLevel;
        private int _retryCount;
        private int _retryDelay;

        private String _instanceName;
        private boolean _warned;
        private LocatorPrx _locator;
        private LocatorPrx _voidLocator;
        private Map<String, LocatorPrx> _locators = new HashMap<>();

        private boolean _pending;
        private int _pendingRetryCount;
        private int _failureCount;
        private boolean _warnOnce;
        private List<Request> _pendingRequests = new ArrayList<>();
        private long _nextRetry;
    }

    private class LookupReplyI implements LookupReply {
        LookupReplyI(LocatorI locator) {
            _locator = locator;
        }

        @Override
        public void foundLocator(LocatorPrx locator, Current curr) {
            _locator.foundLocator(locator);
        }

        private final LocatorI _locator;
    }

    public PluginI(Communicator communicator) {
        _communicator = communicator;
    }

    @Override
    public void initialize() {
        Properties properties = _communicator.getProperties();

        boolean ipv4 = properties.getIcePropertyAsInt("Ice.IPv4") > 0;
        boolean preferIPv6 = properties.getIcePropertyAsInt("Ice.PreferIPv6Address") > 0;
        String address = properties.getIceProperty("IceLocatorDiscovery.Address");
        if (address.isEmpty()) {
            address = ipv4 && !preferIPv6 ? "239.255.0.1" : "ff15::1";
        }
        int port = properties.getIcePropertyAsInt("IceLocatorDiscovery.Port");
        String intf = properties.getIceProperty("IceLocatorDiscovery.Interface");

        String lookupEndpoints = properties.getIceProperty("IceLocatorDiscovery.Lookup");
        if (lookupEndpoints.isEmpty()) {
            int protocol = ipv4 && !preferIPv6 ? Network.EnableIPv4 : Network.EnableIPv6;
            List<String> interfaces = Network.getInterfacesForMulticast(intf, protocol);
            for (String p : interfaces) {
                if (p != interfaces.get(0)) {
                    lookupEndpoints += ":";
                }
                lookupEndpoints +=
                    "udp -h \"" + address + "\" -p " + port + " --interface \"" + p + "\"";
            }
        }

        if (properties.getIceProperty("IceLocatorDiscovery.Reply.Endpoints").isEmpty()) {
            properties.setProperty(
                "IceLocatorDiscovery.Reply.Endpoints",
                "udp -h " + (intf.isEmpty() ? "*" : "\"" + intf + "\""));
        }

        if (properties.getIceProperty("IceLocatorDiscovery.Locator.Endpoints").isEmpty()) {
            properties.setProperty(
                "IceLocatorDiscovery.Locator.AdapterId",
                UUID.randomUUID().toString());
        }

        _replyAdapter = _communicator.createObjectAdapter("IceLocatorDiscovery.Reply");
        _locatorAdapter = _communicator.createObjectAdapter("IceLocatorDiscovery.Locator");

        // We don't want those adapters to be registered with the locator so clear their locator.
        _replyAdapter.setLocator(null);
        _locatorAdapter.setLocator(null);

        ObjectPrx lookupPrx =
            _communicator.stringToProxy("IceLocatorDiscovery/Lookup -d:" + lookupEndpoints);
        // No collocation optimization or router for the multicast proxy!
        lookupPrx = lookupPrx.ice_collocationOptimized(false).ice_router(null);

        LocatorPrx voidLoc =
            LocatorPrx.uncheckedCast(
                _locatorAdapter.addWithUUID(new VoidLocatorI()));

        String instanceName = properties.getIceProperty("IceLocatorDiscovery.InstanceName");
        Identity id = new Identity();
        id.name = "Locator";
        id.category =
            !instanceName.isEmpty() ? instanceName : UUID.randomUUID().toString();
        _locator =
            new LocatorI(LookupPrx.uncheckedCast(lookupPrx), properties, instanceName, voidLoc);
        _defaultLocator = _communicator.getDefaultLocator();
        _locatorPrx = LocatorPrx.uncheckedCast(_locatorAdapter.add(_locator, id));
        _communicator.setDefaultLocator(_locatorPrx);

        ObjectPrx lookupReply =
            _replyAdapter.addWithUUID(new LookupReplyI(_locator)).ice_datagram();
        _locator.setLookupReply(LookupReplyPrx.uncheckedCast(lookupReply));

        _replyAdapter.activate();
        _locatorAdapter.activate();
    }

    @Override
    public void destroy() {
        if (_replyAdapter != null) {
            _replyAdapter.destroy();
        }
        if (_locatorAdapter != null) {
            _locatorAdapter.destroy();
        }
        if (_communicator.getDefaultLocator().equals(_locatorPrx)) {
            // Restore original default locator proxy, if the user didn't change it in the meantime
            _communicator.setDefaultLocator(_defaultLocator);
        }
    }

    public List<LocatorPrx> getLocators(String instanceName, int waitTime) {
        return _locator.getLocators(instanceName, waitTime);
    }

    private final Communicator _communicator;
    private ObjectAdapter _locatorAdapter;
    private ObjectAdapter _replyAdapter;
    private LocatorI _locator;
    private LocatorPrx _locatorPrx;
    private LocatorPrx _defaultLocator;
}
