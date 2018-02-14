// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceLocatorDiscovery;

import com.zeroc.IceInternal.Network;
import java.util.List;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

class PluginI implements Plugin
{
    private static class Request
    {
        Request(LocatorI locator,
                String operation,
                com.zeroc.Ice.OperationMode mode,
                byte[] inParams,
                java.util.Map<String, String> context,
                CompletableFuture<com.zeroc.Ice.Object.Ice_invokeResult> f)
        {
            _locator = locator;
            _operation = operation;
            _mode = mode;
            _inParams = inParams;
            _context = context;
            _future = f;
        }

        void invoke(com.zeroc.Ice.LocatorPrx l)
        {
            if(_locatorPrx == null || !_locatorPrx.equals(l))
            {
                _locatorPrx = l;
                try
                {
                    final CompletableFuture<com.zeroc.Ice.Object.Ice_invokeResult> f =
                        l.ice_invokeAsync(_operation, _mode, _inParams, _context);
                    f.whenComplete((result, ex) ->
                                   {
                                       if(ex != null)
                                       {
                                           exception((com.zeroc.Ice.LocalException)ex);
                                       }
                                       else
                                       {
                                           _future.complete(result);
                                       }
                                   });
                }
                catch(com.zeroc.Ice.LocalException ex)
                {
                    exception(ex);
                }
            }
            else
            {
                assert(_exception != null); // Don't retry if the proxy didn't change
                exception(_exception);
            }
        }

        private void exception(com.zeroc.Ice.LocalException ex)
        {
            try
            {
                throw ex;
            }
            catch(com.zeroc.Ice.RequestFailedException exc)
            {
                _future.completeExceptionally(ex);
            }
            catch(com.zeroc.Ice.UnknownException exc)
            {
                _future.completeExceptionally(ex);
            }
            catch(com.zeroc.Ice.NoEndpointException exc)
            {
                _future.completeExceptionally(new com.zeroc.Ice.ObjectNotExistException());
            }
            catch(com.zeroc.Ice.ObjectAdapterDeactivatedException exc)
            {
                _future.completeExceptionally(new com.zeroc.Ice.ObjectNotExistException());
            }
            catch(com.zeroc.Ice.CommunicatorDestroyedException exc)
            {
                _future.completeExceptionally(new com.zeroc.Ice.ObjectNotExistException());
            }
            catch(com.zeroc.Ice.LocalException exc)
            {
                _exception = exc;
                _locator.invoke(_locatorPrx, Request.this); // Retry with new locator proxy
            }
        }

        private final LocatorI _locator;
        private com.zeroc.Ice.LocalException _exception = null;
        private final String _operation;
        private final com.zeroc.Ice.OperationMode _mode;
        private final java.util.Map<String, String> _context;
        private final byte[] _inParams;
        private final CompletableFuture<com.zeroc.Ice.Object.Ice_invokeResult> _future;

        private com.zeroc.Ice.LocatorPrx _locatorPrx;
    }

    static private class VoidLocatorI implements com.zeroc.Ice.Locator
    {
        @Override
        public CompletionStage<com.zeroc.Ice.ObjectPrx> findObjectByIdAsync(com.zeroc.Ice.Identity id,
                                                                            com.zeroc.Ice.Current current)
        {
            return CompletableFuture.completedFuture((com.zeroc.Ice.ObjectPrx)null);
        }

        @Override
        public CompletionStage<com.zeroc.Ice.ObjectPrx> findAdapterByIdAsync(String id, com.zeroc.Ice.Current current)
        {
            return CompletableFuture.completedFuture((com.zeroc.Ice.ObjectPrx)null);
        }

        @Override
        public com.zeroc.Ice.LocatorRegistryPrx getRegistry(com.zeroc.Ice.Current current)
        {
            return null;
        }
    }

    private static class LocatorI implements com.zeroc.Ice.BlobjectAsync
    {
        LocatorI(String name, LookupPrx lookup, com.zeroc.Ice.Properties properties, String instanceName,
                 com.zeroc.Ice.LocatorPrx voidLocator)
        {
            _lookup = lookup;
            _timeout = properties.getPropertyAsIntWithDefault(name + ".Timeout", 300);
            _retryCount = properties.getPropertyAsIntWithDefault(name + ".RetryCount", 3);
            _retryDelay = properties.getPropertyAsIntWithDefault(name + ".RetryDelay", 2000);
            _timer = com.zeroc.IceInternal.Util.getInstance(lookup.ice_getCommunicator()).timer();
            _traceLevel = properties.getPropertyAsInt(name + ".Trace.Lookup");
            _instanceName = instanceName;
            _warned = false;
            _locator = lookup.ice_getCommunicator().getDefaultLocator();
            _voidLocator = voidLocator;
            _pendingRetryCount = 0;
            _failureCount = 0;
            _warnOnce = true;

            //
            // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast
            // datagram on each endpoint.
            //
            com.zeroc.Ice.Endpoint[] single = new com.zeroc.Ice.Endpoint[1];
            for(com.zeroc.Ice.Endpoint endpt : lookup.ice_getEndpoints())
            {
                single[0] = endpt;
                _lookups.put((LookupPrx)lookup.ice_endpoints(single), null);
            }
            assert(!_lookups.isEmpty());
        }

        public void setLookupReply(LookupReplyPrx lookupReply)
        {
            //
            // Use a lookup reply proxy whose adress matches the interface used to send multicast datagrams.
            //
            com.zeroc.Ice.Endpoint[] single = new com.zeroc.Ice.Endpoint[1];
            for(Map.Entry<LookupPrx, LookupReplyPrx> entry : _lookups.entrySet())
            {
                com.zeroc.Ice.UDPEndpointInfo info =
                    (com.zeroc.Ice.UDPEndpointInfo)entry.getKey().ice_getEndpoints()[0].getInfo();
                if(!info.mcastInterface.isEmpty())
                {
                    for(com.zeroc.Ice.Endpoint q : lookupReply.ice_getEndpoints())
                    {
                        com.zeroc.Ice.EndpointInfo r = q.getInfo();
                        if(r instanceof com.zeroc.Ice.IPEndpointInfo &&
                           ((com.zeroc.Ice.IPEndpointInfo)r).host.equals(info.mcastInterface))
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
        public CompletionStage<com.zeroc.Ice.Object.Ice_invokeResult> ice_invokeAsync(byte[] inParams,
                                                                                      com.zeroc.Ice.Current current)
        {
            CompletableFuture<com.zeroc.Ice.Object.Ice_invokeResult> f = new CompletableFuture<>();
            invoke(null, new Request(this, current.operation, current.mode, inParams, current.ctx, f));
            return f;
        }

        public List<com.zeroc.Ice.LocatorPrx> getLocators(String instanceName, int waitTime)
        {
            //
            // Clear locators from previous search.
            //
            synchronized(this)
            {
                _locators.clear();
            }

            //
            // Find a locator
            //
            invoke(null, null);

            //
            // Wait for responses
            //
            try
            {
                if(instanceName.isEmpty())
                {
                    Thread.sleep(waitTime);
                }
                else
                {
                    synchronized(this)
                    {
                        while(!_locators.containsKey(instanceName) && _pendingRetryCount > 0)
                        {
                            wait(waitTime);
                        }
                    }
                }
            }
            catch(java.lang.InterruptedException ex)
            {
                throw new com.zeroc.Ice.OperationInterruptedException();
            }

            //
            // Return found locators
            //
            synchronized(this)
            {
                return new ArrayList<>(_locators.values());
            }
        }

        public synchronized void foundLocator(com.zeroc.Ice.LocatorPrx locator)
        {
            if(locator == null ||
               (!_instanceName.isEmpty() && !locator.ice_getIdentity().category.equals(_instanceName)))
            {
                if(_traceLevel > 2)
                {
                    StringBuffer s = new StringBuffer("ignoring locator reply: instance name doesn't match\n");
                    s.append("expected = ").append(_instanceName);
                    s.append("received = ").append(locator.ice_getIdentity().category);
                    _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.toString());
                }
                return;
            }

            //
            // If we already have a locator assigned, ensure the given locator
            // has the same identity, otherwise ignore it.
            //
            if(!_pendingRequests.isEmpty() &&
               _locator != null && !locator.ice_getIdentity().category.equals(_locator.ice_getIdentity().category))
            {
                if(!_warned)
                {
                    _warned = true; // Only warn once

                    locator.ice_getCommunicator().getLogger().warning(
                        "received Ice locator with different instance name:\n" +
                        "using = `" + _locator.ice_getIdentity().category + "'\n" +
                        "received = `" + locator.ice_getIdentity().category + "'\n" +
                        "This is typically the case if multiple Ice locators with different " +
                        "instance names are deployed and the property `IceLocatorDiscovery.InstanceName'" +
                        "is not set.");
                }
                return;
            }

            if(_pendingRetryCount > 0) // No need to retry, we found a locator
            {
                _future.cancel(false);
                _future = null;

                _pendingRetryCount = 0;
            }

            if(_traceLevel > 0)
            {
                StringBuffer s = new StringBuffer("locator lookup succeeded:\nlocator = ");
                s.append(locator);
                if(!_instanceName.isEmpty())
                {
                    s.append("\ninstance name = ").append(_instanceName);
                }
                _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.toString());
            }

            com.zeroc.Ice.LocatorPrx l =
                _pendingRequests.isEmpty() ? _locators.get(locator.ice_getIdentity().category) : _locator;
            if(l != null)
            {
                //
                // We found another locator replica, append its endpoints to the
                // current locator proxy endpoints.
                //
                List<com.zeroc.Ice.Endpoint> newEndpoints = new ArrayList<>(Arrays.asList(l.ice_getEndpoints()));
                for(com.zeroc.Ice.Endpoint p : locator.ice_getEndpoints())
                {
                    //
                    // Only add endpoints if not already in the locator proxy endpoints
                    //
                    boolean found = false;
                    for(com.zeroc.Ice.Endpoint q : newEndpoints)
                    {
                        if(p.equals(q))
                        {
                            found = true;
                            break;
                        }
                    }
                    if(!found)
                    {
                        newEndpoints.add(p);
                    }

                }
                l = (com.zeroc.Ice.LocatorPrx)l.ice_endpoints(
                    newEndpoints.toArray(new com.zeroc.Ice.Endpoint[newEndpoints.size()]));
            }
            else
            {
                l = locator;
            }

            if(_pendingRequests.isEmpty())
            {
                _locators.put(locator.ice_getIdentity().category, l);
                notify();
            }
            else
            {
                _locator = l;
                if(_instanceName.isEmpty())
                {
                    _instanceName = _locator.ice_getIdentity().category; // Stick to the first locator
                }

                //
                // Send pending requests if any.
                //
                for(Request req : _pendingRequests)
                {
                    req.invoke(_locator);
                }
                _pendingRequests.clear();
            }
        }

        public synchronized void invoke(com.zeroc.Ice.LocatorPrx locator, Request request)
        {
            if(request != null && _locator != null && _locator != locator)
            {
                request.invoke(_locator);
            }
            else if(request != null && com.zeroc.IceInternal.Time.currentMonotonicTimeMillis() < _nextRetry)
            {
                request.invoke(_voidLocator); // Don't retry to find a locator before the retry delay expires
            }
            else
            {
                _locator = null;

                if(request != null)
                {
                    _pendingRequests.add(request);
                }

                if(_pendingRetryCount == 0) // No request in progress
                {
                    _failureCount = 0;
                    _pendingRetryCount = _retryCount;
                    try
                    {
                        if(_traceLevel > 1)
                        {
                            StringBuilder s = new StringBuilder("looking up locator:\nlookup = ");
                            s.append(_lookup);
                            if(!_instanceName.isEmpty())
                            {
                                s.append("\ninstance name = ").append(_instanceName);
                            }
                            _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.toString());
                        }
                        for(Map.Entry<LookupPrx, LookupReplyPrx> entry : _lookups.entrySet())
                        {
                            entry.getKey().findLocatorAsync(_instanceName, entry.getValue()).whenComplete((v, ex) -> {
                                if(ex != null)
                                {
                                    exception(ex);
                                }
                            }); // Send multicast request.
                        }
                        _future = _timer.schedule(_retryTask, _timeout, java.util.concurrent.TimeUnit.MILLISECONDS);
                    }
                    catch(com.zeroc.Ice.LocalException ex)
                    {
                        if(_traceLevel > 0)
                        {
                            StringBuilder s = new StringBuilder("locator lookup failed:\nlookup = ");
                            s.append(_lookup);
                            if(!_instanceName.isEmpty())
                            {
                                s.append("\ninstance name = ").append(_instanceName);
                            }
                            s.append("\n").append(ex);
                            _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.toString());
                        }

                        for(Request req : _pendingRequests)
                        {
                            req.invoke(_voidLocator);
                        }
                        _pendingRequests.clear();
                        _pendingRetryCount = 0;
                    }
                }
            }
        }

        synchronized void exception(Throwable ex)
        {
            if(++_failureCount == _lookups.size() && _pendingRetryCount > 0)
            {
                //
                // All the lookup calls failed, cancel the timer and propagate the error to the requests.
                //
                _future.cancel(false);
                _future = null;

                _pendingRetryCount = 0;

                if(_warnOnce)
                {
                    StringBuilder builder = new StringBuilder();
                    builder.append("failed to lookup locator with lookup proxy `");
                    builder.append(_lookup);
                    builder.append("':\n");
                    builder.append(ex);
                    _lookup.ice_getCommunicator().getLogger().warning(builder.toString());
                    _warnOnce = false;
                }

                if(_traceLevel > 0)
                {
                    StringBuilder s = new StringBuilder("locator lookup failed:\nlookup = ");
                    s.append(_lookup);
                    if(!_instanceName.isEmpty())
                    {
                        s.append("\ninstance name = ").append(_instanceName);
                    }
                    s.append("\n").append(ex);
                    _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.toString());
                }

                if(_pendingRequests.isEmpty())
                {
                    notify();
                }
                else
                {
                    for(Request req : _pendingRequests)
                    {
                        req.invoke(_voidLocator);
                    }
                    _pendingRequests.clear();
                }
            }
        }

        private Runnable _retryTask = new Runnable()
        {
            @Override
            public void run()
            {
                synchronized(LocatorI.this)
                {
                    if(--_pendingRetryCount > 0)
                    {
                        try
                        {
                            if(_traceLevel > 1)
                            {
                                StringBuilder s = new StringBuilder("retrying locator lookup:\nlookup = ");
                                s.append(_lookup);
                                s.append("retry count = ").append(_retryCount);
                                if(!_instanceName.isEmpty())
                                {
                                    s.append("\ninstance name = ").append(_instanceName);
                                }
                                _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.toString());
                            }

                            _failureCount = 0;
                            for(Map.Entry<LookupPrx, LookupReplyPrx> entry : _lookups.entrySet())
                            {
                                entry.getKey().findLocatorAsync(_instanceName, entry.getValue()).whenComplete((v, ex) -> {
                                    if(ex != null)
                                    {
                                        exception(ex);
                                    }
                                }); // Send multicast request.
                            }
                            _future = _timer.schedule(_retryTask, _timeout, java.util.concurrent.TimeUnit.MILLISECONDS);
                            return;
                        }
                        catch(com.zeroc.Ice.LocalException ex)
                        {
                        }
                        _pendingRetryCount = 0;
                    }

                    if(_traceLevel > 0)
                    {
                        StringBuilder s = new StringBuilder("locator lookup timed out:\nlookup = ");
                        s.append(_lookup);
                        if(!_instanceName.isEmpty())
                        {
                            s.append("\ninstance name = ").append(_instanceName);
                        }
                        _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.toString());
                    }

                    if(_pendingRequests.isEmpty())
                    {
                        notify();
                    }
                    else
                    {
                        for(Request req : _pendingRequests)
                        {
                            req.invoke(_voidLocator);
                        }
                        _pendingRequests.clear();
                    }
                    _nextRetry = com.zeroc.IceInternal.Time.currentMonotonicTimeMillis() + _retryDelay;
                }

            }
        };

        private final LookupPrx _lookup;
        private final Map<LookupPrx, LookupReplyPrx> _lookups = new java.util.HashMap<>();
        private final int _timeout;
        private java.util.concurrent.Future<?> _future;
        private final java.util.concurrent.ScheduledExecutorService _timer;
        private final int _traceLevel;
        private final int _retryCount;
        private final int _retryDelay;

        private String _instanceName;
        private boolean _warned;
        private com.zeroc.Ice.LocatorPrx _locator;
        private com.zeroc.Ice.LocatorPrx _voidLocator;
        private Map<String, com.zeroc.Ice.LocatorPrx> _locators = new HashMap<>();

        private int _pendingRetryCount;
        private int _failureCount;
        private boolean _warnOnce;
        private List<Request> _pendingRequests = new ArrayList<>();
        private long _nextRetry;
    }

    private class LookupReplyI implements LookupReply
    {
        LookupReplyI(LocatorI locator)
        {
            _locator = locator;
        }

        @Override
        public void foundLocator(com.zeroc.Ice.LocatorPrx locator, com.zeroc.Ice.Current curr)
        {
            _locator.foundLocator(locator);
        }

        private final LocatorI _locator;
    }

    public PluginI(String name, com.zeroc.Ice.Communicator communicator)
    {
        _name = name;
        _communicator = communicator;
    }

    @Override
    public void initialize()
    {
        com.zeroc.Ice.Properties properties = _communicator.getProperties();

        boolean ipv4 = properties.getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
        boolean preferIPv6 = properties.getPropertyAsInt("Ice.PreferIPv6Address") > 0;
        String address;
        if(ipv4 && !preferIPv6)
        {
            address = properties.getPropertyWithDefault(_name + ".Address", "239.255.0.1");
        }
        else
        {
            address = properties.getPropertyWithDefault(_name + ".Address", "ff15::1");
        }
        int port = properties.getPropertyAsIntWithDefault(_name + ".Port", 4061);
        String intf = properties.getProperty(_name + ".Interface");

        String lookupEndpoints = properties.getProperty(_name + ".Lookup");
        if(lookupEndpoints.isEmpty())
        {
            int protocol = ipv4 && !preferIPv6 ? Network.EnableIPv4 : Network.EnableIPv6;
            java.util.List<String> interfaces = Network.getInterfacesForMulticast(intf, protocol);
            for(String p : interfaces)
            {
                if(p != interfaces.get(0))
                {
                    lookupEndpoints += ":";
                }
                lookupEndpoints += "udp -h \"" + address + "\" -p " + port + " --interface \"" + p + "\"";
            }
        }

        if(properties.getProperty(_name + ".Reply.Endpoints").isEmpty())
        {
            properties.setProperty(_name + ".Reply.Endpoints", "udp -h " + (intf.isEmpty() ? "*" : "\"" + intf + "\""));
        }

        if(properties.getProperty(_name + ".Locator.Endpoints").isEmpty())
        {
            properties.setProperty(_name + ".Locator.AdapterId", java.util.UUID.randomUUID().toString());
        }

        _replyAdapter = _communicator.createObjectAdapter(_name + ".Reply");
        _locatorAdapter = _communicator.createObjectAdapter(_name + ".Locator");

        // We don't want those adapters to be registered with the locator so clear their locator.
        _replyAdapter.setLocator(null);
        _locatorAdapter.setLocator(null);

        com.zeroc.Ice.ObjectPrx lookupPrx =
            _communicator.stringToProxy("IceLocatorDiscovery/Lookup -d:" + lookupEndpoints);
        // No collocation optimization or router for the multicast proxy!
        lookupPrx = lookupPrx.ice_collocationOptimized(false).ice_router(null);

        com.zeroc.Ice.LocatorPrx voidLoc =
            com.zeroc.Ice.LocatorPrx.uncheckedCast(_locatorAdapter.addWithUUID(new VoidLocatorI()));

        String instanceName = properties.getProperty(_name + ".InstanceName");
        com.zeroc.Ice.Identity id = new com.zeroc.Ice.Identity();
        id.name = "Locator";
        id.category = !instanceName.isEmpty() ? instanceName : java.util.UUID.randomUUID().toString();
        _locator = new LocatorI(_name, LookupPrx.uncheckedCast(lookupPrx), properties, instanceName, voidLoc);
        _defaultLocator = _communicator.getDefaultLocator();
        _locatorPrx = com.zeroc.Ice.LocatorPrx.uncheckedCast(_locatorAdapter.addWithUUID(_locator));
        _communicator.setDefaultLocator(_locatorPrx);

        com.zeroc.Ice.ObjectPrx lookupReply = _replyAdapter.addWithUUID(new LookupReplyI(_locator)).ice_datagram();
        _locator.setLookupReply(LookupReplyPrx.uncheckedCast(lookupReply));

        _replyAdapter.activate();
        _locatorAdapter.activate();
    }

    @Override
    public void destroy()
    {
        if(_replyAdapter != null)
        {
            _replyAdapter.destroy();
        }
        if(_locatorAdapter != null)
        {
            _locatorAdapter.destroy();
        }
        if(_communicator.getDefaultLocator().equals(_locatorPrx))
        {
            // Restore original default locator proxy, if the user didn't change it in the meantime
            _communicator.setDefaultLocator(_defaultLocator);
        }
    }

    public List<com.zeroc.Ice.LocatorPrx>
    getLocators(String instanceName, int waitTime)
    {
        return _locator.getLocators(instanceName, waitTime);
    }

    private String _name;
    private com.zeroc.Ice.Communicator _communicator;
    private com.zeroc.Ice.ObjectAdapter _locatorAdapter;
    private com.zeroc.Ice.ObjectAdapter _replyAdapter;
    private LocatorI _locator;
    private com.zeroc.Ice.LocatorPrx _locatorPrx;
    private com.zeroc.Ice.LocatorPrx _defaultLocator;
}
