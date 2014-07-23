// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGrid;

import java.util.List;
import java.util.Arrays;
import java.util.ArrayList;

class DiscoveryPluginI implements Ice.Plugin
{

    abstract private class Request
    {
        public
        Request(LocatorI locator)
        {
            _locator = locator;
        }

        abstract public void
        invoke(Ice.LocatorPrx locator);

        abstract public void
        response(Ice.ObjectPrx locator);

        protected LocatorI _locator;
        protected Ice.LocatorPrx _locatorPrx;
    };

    private class LocatorI extends Ice._LocatorDisp implements IceInternal.TimerTask
    {
        public
        LocatorI(LookupPrx lookup, Ice.Properties properties)
        {
            _lookup = lookup;
            _timeout = properties.getPropertyAsIntWithDefault("IceGridDiscovery.Timeout", 300) * 1000;
            _retryCount = properties.getPropertyAsIntWithDefault("IceGridDiscovery.RetryCount",3);
            _timer = IceInternal.Util.getInstance(lookup.ice_getCommunicator()).timer();
            _instanceName = properties.getProperty("IceGridDiscovery.InstanceName");
            _warned = false;
            _locator = lookup.ice_getCommunicator().getDefaultLocator();
            _pendingRetryCount = 0;
        }

        public void
        setLookupReply(LookupReplyPrx lookupReply)
        {
            _lookupReply = lookupReply;
        }

        public synchronized void
        findObjectById_async(Ice.AMD_Locator_findObjectById amdCB, Ice.Identity id, Ice.Current curr)
        {
            ((LocatorI)this).invoke(null, new ObjectRequest((LocatorI)this, id, amdCB));
        }

        public synchronized void
        findAdapterById_async(Ice.AMD_Locator_findAdapterById amdCB, String adapterId, Ice.Current curr)
        {
            ((LocatorI)this).invoke(null, new AdapterRequest((LocatorI)this, adapterId, amdCB));
        }


         public synchronized Ice.LocatorRegistryPrx
         getRegistry(Ice.Current current)
         {
            Ice.LocatorPrx locator;
            if(_locator != null)
            {
                ((LocatorI)this).queueRequest(null); // Search for locator if not already doing so.
                while(_pendingRetryCount > 0)
                {
                    try
                    {
                    wait();
                    }
                    catch(java.lang.InterruptedException ex)
                    {
                    }
                }
            }
            locator = _locator;
            return locator != null ? locator.getRegistry() : null;
         }

        public synchronized void
        foundLocator(LocatorPrx locator)
        {
            if(locator == null)
            {
                return;
            }

            //
            // If we already have a locator assigned, ensure the given locator
            // has the same identity, otherwise ignore it.
            //
            if(_locator != null && !locator.ice_getIdentity().category.equals(_locator.ice_getIdentity().category))
            {
                if(!_warned)
                {
                    _warned = true; // Only warn once

                    locator.ice_getCommunicator().getLogger().warning(
                        "received IceGrid locator with different instance name:\n" +
                        "using = `" + _locator.ice_getIdentity().category + "'\n" +
                        "received = `" + locator.ice_getIdentity().category + "'\n" +
                        "This is typically the case if multiple IceGrid registries with different " +
                        "nstance names are deployed and the property `IceGridDiscovery.InstanceName'" +
                        "is not set.");
                }
                return;
            }

            if(_pendingRetryCount > 0) // No need to retry, we found a locator
            {
                _timer.cancel(this);
                _pendingRetryCount = 0;
            }

            if(_locator != null)
            {
                //
                // We found another locator replica, append its endpoints to the
                // current locator proxy endpoints.
                //
                List<Ice.Endpoint> newEndpoints = new ArrayList<Ice.Endpoint>(Arrays.asList(_locator.ice_getEndpoints()));
                for(Ice.Endpoint p : locator.ice_getEndpoints())
                {
                    //
                    // Only add endpoints if not already in the locator proxy endpoints
                    //
                    boolean found = false;
                    for(Ice.Endpoint q : newEndpoints)
                    {
                        if (p.equals(q))
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
                _locator = (LocatorPrx)_locator.ice_endpoints(newEndpoints.toArray(new Ice.Endpoint[newEndpoints.size()]));
            }
            else
            {
                _locator = locator;
                if(_instanceName.isEmpty())
                {
                    _instanceName = _locator.ice_getIdentity().category;
                }
            }

            //
            // Send pending requests if any.
            //
            for(Request req : _pendingRequests)
            {
                req.invoke(_locator);
            }
            _pendingRequests.clear();
            notifyAll();
        }


        public synchronized void
        invoke(Ice.LocatorPrx locator, Request request)
        {

            if(_locator != null && !(_locator.equals(locator)))
            {

                request.invoke(_locator);
            }
            else
            {

                _locator = null;
                queueRequest(request);
            }
        }

        public void runTimerTask()
        {
            synchronized(this)
            {

                if(--_pendingRetryCount > 0)
                {

                    _lookup.begin_findLocator(_instanceName, _lookupReply); // Send multicast request.
                    _timer.schedule(this, _timeout);
                }
                else
                {
                    assert !_pendingRequests.isEmpty();
                    for(Request req : _pendingRequests)
                    {
                        req.response(null);
                    }
                    _pendingRequests.clear();
                    notifyAll();

                }
            }
        }

        private void
        queueRequest(Request request)
        {
            if(request != null)
            {

                _pendingRequests.add(request);
            }

            if(_pendingRetryCount == 0) // No request in progress
            {

                _pendingRetryCount = _retryCount;
                _lookup.begin_findLocator(_instanceName, _lookupReply); // Send multicast request.
                _timer.schedule(this, _timeout);
            }
        }

        private final LookupPrx _lookup;
        private final int _timeout;
        private final IceInternal.Timer _timer;
        private final int _retryCount;

        private String _instanceName;
        private boolean _warned;
        private LookupReplyPrx _lookupReply;
        private Ice.LocatorPrx _locator;

        private int _pendingRetryCount;
        private List<Request> _pendingRequests = new ArrayList<Request>();;
    };

    private class LookupReplyI extends _LookupReplyDisp
    {
        public LookupReplyI(LocatorI locator)
        {
            _locator = locator;
        }

        public void
        foundLocator(LocatorPrx locator, Ice.Current curr)
        {
            _locator.foundLocator(locator);
        }

        private final LocatorI _locator;
    };

    class ObjectRequest extends Request
    {
        public
        ObjectRequest(LocatorI locator, Ice.Identity id, Ice.AMD_Locator_findObjectById amdCB)
        {
            super(locator);
            _id = id;
            _amdCB = amdCB;
        }

        public void
        invoke(Ice.LocatorPrx l)
        {
            _locatorPrx = l;
            l.begin_findObjectById(_id,
                new Ice.Callback_Locator_findObjectById() {
                    public void
                    response(Ice.ObjectPrx proxy)
                    {
                        ObjectRequest.this.response(proxy);
                    }

                    public void
                    exception(Ice.UserException ex)
                    {
                        ObjectRequest.this.exception(ex);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        ObjectRequest.this.exception(ex);
                    }
                });
        }

        public void
        response(Ice.ObjectPrx prx)
        {
            _amdCB.ice_response(prx);
        }

        public void
        exception(Exception ex)
        {
            _locator.invoke(_locatorPrx, this);
        }

        private final Ice.Identity _id;
        private final Ice.AMD_Locator_findObjectById _amdCB;
    };

    class AdapterRequest extends Request {

        public
        AdapterRequest(LocatorI locator, String adapterId, Ice.AMD_Locator_findAdapterById amdCB) {
            super(locator);
            _adapterId = adapterId;
            _amdCB = amdCB;
        }

        public void
        invoke(Ice.LocatorPrx l)
        {
            _locatorPrx = l;
            l.begin_findAdapterById(_adapterId,
                new Ice.Callback_Locator_findAdapterById()
                {
                    public void
                    response(Ice.ObjectPrx proxy)
                    {
                        AdapterRequest.this.response(proxy);
                    }

                    public void
                    exception(Ice.UserException ex)
                    {
                        AdapterRequest.this.exception(ex);
                    }

                    public void
                    exception(Ice.LocalException ex)
                    {
                        AdapterRequest.this.exception(ex);
                    }
                });
        }

        public void
        response(Ice.ObjectPrx prx)
        {
            _amdCB.ice_response(prx);
        }

        public void
        exception(Exception ex)
        {
            _locator.invoke(_locatorPrx, this); // Retry with new locator proxy.
        }

        private final String _adapterId;
        private final Ice.AMD_Locator_findAdapterById _amdCB;
    };

    public
    DiscoveryPluginI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    public void
    initialize()
    {
        Ice.Properties properties = _communicator.getProperties();

        boolean ipv4 = properties.getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
        String address;
        if(ipv4)
        {
          address = properties.getPropertyWithDefault("IceGridDiscovery.Address", "239.255.0.1");
        }
        else
        {
            address = properties.getPropertyWithDefault("IceGridDiscovery.Address", "ff15::1");
        }
        int port = properties.getPropertyAsIntWithDefault("IceGridDiscovery.Port", 4061);
        String intf = properties.getProperty("IceGridDiscovery.Interface");

        if(properties.getProperty("IceGridDiscovery.Reply.Endpoints").isEmpty())
        {
            StringBuilder s = new StringBuilder();
            s.append("udp");
            if(!intf.isEmpty())
            {
                s.append(" -h \"").append(intf).append("\"");
            }
            properties.setProperty("IceGridDiscovery.Reply.Endpoints", s.toString());
        }
        if(properties.getProperty("IceGridDiscovery.Locator.Endpoints").isEmpty())
        {
            properties.setProperty("IceGridDiscovery.Locator.AdapterId", java.util.UUID.randomUUID().toString());
        }

        _replyAdapter = _communicator.createObjectAdapter("IceGridDiscovery.Reply");
        _locatorAdapter = _communicator.createObjectAdapter("IceGridDiscovery.Locator");

        // We don't want those adapters to be registered with the locator so clear their locator.
        _replyAdapter.setLocator(null);
        _locatorAdapter.setLocator(null);

        String lookupEndpoints = properties.getProperty("IceGridDiscovery.Lookup");
        if(lookupEndpoints.isEmpty())
        {
            StringBuilder s = new StringBuilder();
            s.append("udp -h \"").append(address).append("\" -p ").append(port);
            if(!intf.isEmpty())
            {
                s.append(" --interface \"").append(intf).append("\"");
            }
            lookupEndpoints = s.toString();
        }

        Ice.ObjectPrx lookupPrx = _communicator.stringToProxy("IceGridDiscovery/Lookup -d:" + lookupEndpoints);
        lookupPrx = lookupPrx.ice_collocationOptimized(false); // No collocation optimization for the multicast proxy!
        try
        {
            lookupPrx.ice_getConnection(); // Ensure we can establish a connection to the multicast proxy
        }
        catch(Ice.LocalException ex)
        {
            StringBuilder s = new StringBuilder();
            s.append("unable to establish multicast connection, IceGrid discovery will be disabled:\n");
            s.append("proxy = ").append(lookupPrx.toString()).append("\n");
            throw new Ice.PluginInitializationException(s.toString());
        }

        LocatorI locator = new LocatorI(LookupPrxHelper.uncheckedCast(lookupPrx), properties);
        _communicator.setDefaultLocator(Ice.LocatorPrxHelper.uncheckedCast(_locatorAdapter.addWithUUID(locator)));

        Ice.ObjectPrx lookupReply = _replyAdapter.addWithUUID(new LookupReplyI(locator)).ice_datagram();
        locator.setLookupReply(LookupReplyPrxHelper.uncheckedCast(lookupReply));

        _replyAdapter.activate();
        _locatorAdapter.activate();
    }

    public void
    destroy()
    {
        _replyAdapter.destroy();
        _locatorAdapter.destroy();
    }

    private Ice.Communicator _communicator;
    private Ice.ObjectAdapter _locatorAdapter;
    private Ice.ObjectAdapter _replyAdapter;
}
