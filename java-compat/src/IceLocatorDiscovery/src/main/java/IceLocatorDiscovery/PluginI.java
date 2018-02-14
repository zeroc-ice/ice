// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceLocatorDiscovery;

import java.util.List;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.Map;

class PluginI implements Plugin
{
    private static class Request
    {
        Request(LocatorI locator,
                String operation,
                Ice.OperationMode mode,
                byte[] inParams,
                java.util.Map<String, String> context,
                Ice.AMD_Object_ice_invoke amdCB)
        {
            _locator = locator;
            _operation = operation;
            _mode = mode;
            _inParams = inParams;
            _context = context;
            _amdCB = amdCB;
        }

        void
        invoke(Ice.LocatorPrx l)
        {
            if(_locatorPrx == null || !_locatorPrx.equals(l))
            {
                _locatorPrx = l;
                try
                {
                    l.begin_ice_invoke(_operation, _mode, _inParams, _context,
                        new Ice.Callback_Object_ice_invoke()
                        {
                            @Override
                            public void
                            response(boolean ok, byte[] outParams)
                            {
                                _amdCB.ice_response(ok, outParams);
                            }

                            @Override
                            public void
                            exception(Ice.LocalException ex)
                            {
                                Request.this.exception(ex);
                            }
                        });
                }
                catch(Ice.LocalException ex)
                {
                    exception(ex);
                }
            }
            else
            {
                assert(_exception != null); // Don't retry if the proxy didn't change
                _amdCB.ice_exception(_exception);
            }
        }

        private void
        exception(Ice.LocalException ex)
        {
            try
            {
                throw ex;
            }
            catch(Ice.RequestFailedException exc)
            {
                _amdCB.ice_exception(ex);
            }
            catch(Ice.UnknownException exc)
            {
                _amdCB.ice_exception(ex);
            }
            catch(Ice.NoEndpointException exc)
            {
                _amdCB.ice_exception(new Ice.ObjectNotExistException());
            }
            catch(Ice.ObjectAdapterDeactivatedException exc)
            {
                _amdCB.ice_exception(new Ice.ObjectNotExistException());
            }
            catch(Ice.CommunicatorDestroyedException exc)
            {
                _amdCB.ice_exception(new Ice.ObjectNotExistException());
            }
            catch(Ice.LocalException exc)
            {
                _exception = exc;
                _locator.invoke(_locatorPrx, Request.this); // Retry with new locator proxy
            }
        }

        private final LocatorI _locator;
        private final String _operation;
        private final Ice.OperationMode _mode;
        private final java.util.Map<String, String> _context;
        private final byte[] _inParams;
        private final Ice.AMD_Object_ice_invoke _amdCB;

        private Ice.LocatorPrx _locatorPrx = null;
        private Ice.LocalException _exception = null;
    };

    static private class VoidLocatorI extends Ice._LocatorDisp
    {
        @Override
        public void
        findObjectById_async(Ice.AMD_Locator_findObjectById amdCB, Ice.Identity id, Ice.Current current)
        {
            amdCB.ice_response(null);
        }

        @Override
        public void
        findAdapterById_async(Ice.AMD_Locator_findAdapterById amdCB, String id, Ice.Current current)
        {
            amdCB.ice_response(null);
        }

        @Override
        public Ice.LocatorRegistryPrx
        getRegistry(Ice.Current current)
        {
            return null;
        }
    };

    private static class LocatorI extends Ice.BlobjectAsync
    {
        LocatorI(String name, LookupPrx lookup, Ice.Properties properties, String instanceName,
                 Ice.LocatorPrx voidLocator)
        {
            _lookup = lookup;
            _timeout = properties.getPropertyAsIntWithDefault(name + ".Timeout", 300);
            _retryCount = properties.getPropertyAsIntWithDefault(name + ".RetryCount", 3);
            _retryDelay = properties.getPropertyAsIntWithDefault(name + ".RetryDelay", 2000);
            _timer = IceInternal.Util.getInstance(lookup.ice_getCommunicator()).timer();
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
            Ice.Endpoint[] single = new Ice.Endpoint[1];
            for(Ice.Endpoint endpt : lookup.ice_getEndpoints())
            {
                single[0] = endpt;
                _lookups.put((LookupPrx)lookup.ice_endpoints(single), null);
            }
            assert(!_lookups.isEmpty());
        }

        public void
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
        public synchronized void
        ice_invoke_async(Ice.AMD_Object_ice_invoke amdCB, byte[] inParams, Ice.Current current)
        {
            invoke(null, new Request(this, current.operation, current.mode, inParams, current.ctx, amdCB));
        }

        public List<Ice.LocatorPrx>
        getLocators(String instanceName, int waitTime)
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
                throw new Ice.OperationInterruptedException();
            }

            //
            // Return found locators
            //
            synchronized(this)
            {
                return new ArrayList<>(_locators.values());
            }
        }

        public synchronized void
        foundLocator(Ice.LocatorPrx locator)
        {
            if(locator == null ||
               (!_instanceName.isEmpty() && !locator.ice_getIdentity().category.equals(_instanceName)))
            {
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

            Ice.LocatorPrx l =
                _pendingRequests.isEmpty() ? _locators.get(locator.ice_getIdentity().category) : _locator;
            if(l != null)
            {
                //
                // We found another locator replica, append its endpoints to the
                // current locator proxy endpoints.
                //
                List<Ice.Endpoint> newEndpoints = new ArrayList<Ice.Endpoint>(Arrays.asList(l.ice_getEndpoints()));
                for(Ice.Endpoint p : locator.ice_getEndpoints())
                {
                    //
                    // Only add endpoints if not already in the locator proxy endpoints
                    //
                    boolean found = false;
                    for(Ice.Endpoint q : newEndpoints)
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
                l = (Ice.LocatorPrx)l.ice_endpoints(newEndpoints.toArray(new Ice.Endpoint[newEndpoints.size()]));
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

        public synchronized void
        invoke(Ice.LocatorPrx locator, Request request)
        {
            if(request != null && _locator != null && _locator != locator)
            {
                request.invoke(_locator);
            }
            else if(request != null && IceInternal.Time.currentMonotonicTimeMillis() < _nextRetry)
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
                    _pendingRetryCount = _retryCount;
                    _failureCount = 0;
                    try
                    {
                        for(Map.Entry<LookupPrx, LookupReplyPrx> entry : _lookups.entrySet())
                        {
                            entry.getKey().begin_findLocator(_instanceName, entry.getValue(), new Ice.Callback() {
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
                                        exception(ex);
                                    }
                                }
                            }); // Send multicast request
                        }
                        _future = _timer.schedule(_retryTask, _timeout, java.util.concurrent.TimeUnit.MILLISECONDS);
                    }
                    catch(Ice.LocalException ex)
                    {
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

        synchronized void
        exception(Ice.LocalException ex)
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
                            _failureCount = 0;
                            for(Map.Entry<LookupPrx, LookupReplyPrx> entry : _lookups.entrySet())
                            {
                                entry.getKey().begin_findLocator(_instanceName, entry.getValue(), new Ice.Callback() {
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
                                            exception(ex);
                                        }
                                    }
                                }); // Send multicast request
                            }
                            _future = _timer.schedule(_retryTask, _timeout, java.util.concurrent.TimeUnit.MILLISECONDS);
                            return;
                        }
                        catch(Ice.LocalException ex)
                        {
                        }
                        _pendingRetryCount = 0;
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
                    _nextRetry = IceInternal.Time.currentMonotonicTimeMillis() + _retryDelay;
                }

            }
        };

        private final LookupPrx _lookup;
        private final java.util.Map<LookupPrx, LookupReplyPrx> _lookups = new java.util.HashMap<>();
        private final int _timeout;
        private java.util.concurrent.Future<?> _future;
        private final java.util.concurrent.ScheduledExecutorService _timer;
        private final int _retryCount;
        private final int _retryDelay;

        private String _instanceName;
        private boolean _warned;
        private Ice.LocatorPrx _locator;
        private Ice.LocatorPrx _voidLocator;
        private Map<String, Ice.LocatorPrx> _locators = new java.util.HashMap<>();

        private int _pendingRetryCount;
        private int _failureCount;
        private boolean _warnOnce;
        private List<Request> _pendingRequests = new ArrayList<Request>();
        private long _nextRetry;
    };

    private class LookupReplyI extends _LookupReplyDisp
    {
        LookupReplyI(LocatorI locator)
        {
            _locator = locator;
        }

        @Override
        public void
        foundLocator(Ice.LocatorPrx locator, Ice.Current curr)
        {
            _locator.foundLocator(locator);
        }

        private final LocatorI _locator;
    };

    public
    PluginI(String name, Ice.Communicator communicator)
    {
        _name = name;
        _communicator = communicator;
    }

    @Override
    public void
    initialize()
    {
        Ice.Properties properties = _communicator.getProperties();

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
            int protocol = ipv4 && !preferIPv6 ? IceInternal.Network.EnableIPv4 : IceInternal.Network.EnableIPv6;
            java.util.List<String> interfaces = IceInternal.Network.getInterfacesForMulticast(intf, protocol);
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

        Ice.ObjectPrx lookupPrx = _communicator.stringToProxy("IceLocatorDiscovery/Lookup -d:" + lookupEndpoints);
        // No collocation optimization or router for the multicast proxy!
        lookupPrx = lookupPrx.ice_collocationOptimized(false).ice_router(null);

        Ice.LocatorPrx voidLoc = Ice.LocatorPrxHelper.uncheckedCast(_locatorAdapter.addWithUUID(new VoidLocatorI()));

        String instanceName = properties.getProperty(_name + ".InstanceName");
        Ice.Identity id = new Ice.Identity();
        id.name = "Locator";
        id.category = !instanceName.isEmpty() ? instanceName : java.util.UUID.randomUUID().toString();
        _locator = new LocatorI(_name, LookupPrxHelper.uncheckedCast(lookupPrx), properties, instanceName, voidLoc);
        _defaultLocator = _communicator.getDefaultLocator();
        _locatorPrx = Ice.LocatorPrxHelper.uncheckedCast(_locatorAdapter.addWithUUID(_locator));
        _communicator.setDefaultLocator(_locatorPrx);

        Ice.ObjectPrx lookupReply = _replyAdapter.addWithUUID(new LookupReplyI(_locator)).ice_datagram();
        _locator.setLookupReply(LookupReplyPrxHelper.uncheckedCast(lookupReply));

        _replyAdapter.activate();
        _locatorAdapter.activate();
    }

    @Override
    public void
    destroy()
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

    @Override
    public List<Ice.LocatorPrx>
    getLocators(String instanceName, int waitTime)
    {
        return _locator.getLocators(instanceName, waitTime);
    }

    private String _name;
    private Ice.Communicator _communicator;
    private Ice.ObjectAdapter _locatorAdapter;
    private Ice.ObjectAdapter _replyAdapter;
    private LocatorI _locator;
    private Ice.LocatorPrx _locatorPrx;
    private Ice.LocatorPrx _defaultLocator;
}
