// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceGrid
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Text;

    internal class Request
    {
        public Request(LocatorI locator, 
                       string operation, 
                       Ice.OperationMode mode, 
                       byte[] inParams,
                       Dictionary<string, string> context,
                       Ice.AMD_Object_ice_invoke amdCB)
        {
            _locator = locator;
            _operation = operation;
            _mode = mode;
            _inParams = inParams;
            _context = context;
            _amdCB = amdCB;
        }

        public void
        invoke(Ice.LocatorPrx l)
        {
            _locatorPrx = l;
            Request self = this;
            l.begin_ice_invoke(_operation, _mode, _inParams, _context).whenCompleted(
                (bool ok, byte[] outParams) =>
                {
                    _amdCB.ice_response(ok, outParams);
                },
                (Ice.Exception ex) =>
                {
                    _locator.invoke(_locatorPrx, self); // Retry with new locator proxy
                });
        }

        private readonly LocatorI _locator;
        private readonly string _operation;
        private readonly Ice.OperationMode _mode;
        private readonly Dictionary<string, string> _context;
        private readonly byte[] _inParams;
        private readonly Ice.AMD_Object_ice_invoke _amdCB;

        private Ice.LocatorPrx _locatorPrx;
    }

    internal class VoidLocatorI : LocatorDisp_
    {
        public override void 
        findObjectById_async(Ice.AMD_Locator_findObjectById amdCB, Ice.Identity id, Ice.Current current)
        {
            amdCB.ice_response(null);
        }
        
        public override void 
        findAdapterById_async(Ice.AMD_Locator_findAdapterById amdCB, String id, Ice.Current current)
        {
            amdCB.ice_response(null);
        }
        
        public override Ice.LocatorRegistryPrx 
        getRegistry(Ice.Current current)
        {
            return null;
        }
        
        public override IceGrid.RegistryPrx 
        getLocalRegistry(Ice.Current current)
        {
            return null;
        }
        
        public override IceGrid.QueryPrx 
        getLocalQuery(Ice.Current current)
        {
            return null;
        }
    };

    internal class LocatorI : Ice.BlobjectAsync, IceInternal.TimerTask
    {
        public
        LocatorI(LookupPrx lookup, Ice.Properties properties, string instanceName, IceGrid.LocatorPrx voidLocator)
        {
            _lookup = lookup;
            _timeout = properties.getPropertyAsIntWithDefault("IceGridDiscovery.Timeout", 300);
            _retryCount = properties.getPropertyAsIntWithDefault("IceGridDiscovery.RetryCount", 3);
            _retryDelay = properties.getPropertyAsIntWithDefault("IceGridDiscovery.RetryDelay", 2000);
            _timer = IceInternal.Util.getInstance(lookup.ice_getCommunicator()).timer();
            _instanceName = instanceName;
            _warned = false;
            _locator = lookup.ice_getCommunicator().getDefaultLocator();
            _voidLocator = voidLocator;
            _pendingRetryCount = 0;
        }

        public void
        setLookupReply(LookupReplyPrx lookupReply)
        {
            _lookupReply = lookupReply;
        }

        public override void
        ice_invoke_async(Ice.AMD_Object_ice_invoke amdCB, byte[] inParams, Ice.Current current)
        {
            lock(this)
            {
                invoke(null, new Request(this, current.operation, current.mode, inParams, current.ctx, amdCB));
            }
        }

        public void
        foundLocator(LocatorPrx locator)
        {
            lock(this)
            {
                if(locator == null ||
                   (_instanceName.Length > 0 && !locator.ice_getIdentity().category.Equals(_instanceName)))
                {
                    return;
                }

                //
                // If we already have a locator assigned, ensure the given locator
                // has the same identity, otherwise ignore it.
                //
                if(_locator != null && !locator.ice_getIdentity().category.Equals(_locator.ice_getIdentity().category))
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
                    List<Ice.Endpoint> newEndpoints = new List<Ice.Endpoint>(_locator.ice_getEndpoints());
                    foreach(Ice.Endpoint p in locator.ice_getEndpoints())
                    {
                        //
                        // Only add endpoints if not already in the locator proxy endpoints
                        //
                        bool found = false;
                        foreach(Ice.Endpoint q in newEndpoints)
                        {
                            if(p.Equals(q))
                            {
                                found = true;
                                break;
                            }
                        }
                        if(!found)
                        {
                            newEndpoints.Add(p);
                        }
                    }
                    _locator = (LocatorPrx) _locator.ice_endpoints(newEndpoints.ToArray());
                }
                else
                {
                    _locator = locator;
                    if(_instanceName.Length == 0)
                    {
                        _instanceName = _locator.ice_getIdentity().category; // Stick to the first locator
                    }
                }

                //
                // Send pending requests if any.
                //
                foreach(Request req in _pendingRequests)
                {
                    req.invoke(_locator);
                }
                _pendingRequests.Clear();
            }
        }

        public void
        invoke(Ice.LocatorPrx locator, Request request)
        {
            lock(this)
            {
                if(_locator != null && _locator != locator)
                {
                    request.invoke(_locator);
                }
                else if(IceInternal.Time.currentMonotonicTimeMillis() < _nextRetry)
                {
                    request.invoke(_voidLocator); // Don't retry to find a locator before the retry delay expires
                }
                else
                {
                    _locator = null;

                    _pendingRequests.Add(request);

                    if(_pendingRetryCount == 0) // No request in progress
                    {
                        _pendingRetryCount = _retryCount;
                        _lookup.begin_findLocator(_instanceName, _lookupReply); // Send multicast request.
                        _timer.schedule(this, _timeout);
                    }
                }
            }
        }

        public void
        runTimerTask()
        {
            lock(this)
            {
                if(--_pendingRetryCount > 0)
                {
                    _lookup.begin_findLocator(_instanceName, _lookupReply); // Send multicast request
                    _timer.schedule(this, _timeout);
                }
                else
                {
                    Debug.Assert(_pendingRequests.Count > 0, "Pending requests is not empty");
                    foreach(Request req in _pendingRequests)
                    {
                        req.invoke(_voidLocator);
                    }
                    _pendingRequests.Clear();
                    _nextRetry = IceInternal.Time.currentMonotonicTimeMillis() + _retryDelay;
                }
            }
        }

        private LookupPrx _lookup;
        private int _timeout;
        private IceInternal.Timer _timer;
        private int _retryCount;
        private int _retryDelay;

        private string _instanceName;
        private bool _warned;
        private LookupReplyPrx _lookupReply;
        private Ice.LocatorPrx _locator;
        private IceGrid.LocatorPrx _voidLocator;

        private int _pendingRetryCount;
        private List<Request> _pendingRequests = new List<Request>();
        private long _nextRetry;
    };

    internal class LookupReplyI : LookupReplyDisp_
    {
        public LookupReplyI(LocatorI locator)
        {
            _locator = locator;
        }

        public override void
        foundLocator(LocatorPrx locator, Ice.Current current)
        {
            _locator.foundLocator(locator);
        }

        private LocatorI _locator;
    }

    class DiscoveryPluginI : Ice.Plugin
    {
        public
        DiscoveryPluginI(Ice.Communicator communicator)
        {
           _communicator = communicator;
        }

        public void
        initialize()
        {
            Ice.Properties properties = _communicator.getProperties();

            bool ipv4 = properties.getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
            string address;
            if(ipv4)
            {
                address = properties.getPropertyWithDefault("IceGridDiscovery.Address", "239.255.0.1");
            }
            else
            {
                address = properties.getPropertyWithDefault("IceGridDiscovery.Address", "ff15::1");
            }
            int port = properties.getPropertyAsIntWithDefault("IceGridDiscovery.Port", 4061);
            string intf = properties.getProperty("IceGridDiscovery.Interface");

            if(properties.getProperty("IceGridDiscovery.Reply.Endpoints").Length == 0)
            {
                System.Text.StringBuilder s = new System.Text.StringBuilder();
                s.Append("udp");
                if(intf.Length > 0)
                {
                    s.Append(" -h \"");
                    s.Append(intf);
                    s.Append("\"");
                }
                properties.setProperty("IceGridDiscovery.Reply.Endpoints", s.ToString());
            }
            if(properties.getProperty("IceGridDiscovery.Locator.Endpoints").Length == 0)
            {
                properties.setProperty("IceGridDiscovery.Locator.AdapterId", Guid.NewGuid().ToString());
            }

            _replyAdapter = _communicator.createObjectAdapter("IceGridDiscovery.Reply");
            _locatorAdapter = _communicator.createObjectAdapter("IceGridDiscovery.Locator");

            // We don't want those adapters to be registered with the locator so clear their locator.
            _replyAdapter.setLocator(null);
            _locatorAdapter.setLocator(null);

            string lookupEndpoints = properties.getProperty("IceGridDiscovery.Lookup");
            if(lookupEndpoints.Length == 0)
            {
                System.Text.StringBuilder s = new System.Text.StringBuilder();
                s.Append("udp -h \"");
                s.Append(address);
                s.Append("\" -p ");
                s.Append(port);
                if(intf.Length > 0)
                {
                    s.Append(" --interface \"");
                    s.Append(intf);
                    s.Append("\"");
                }
                lookupEndpoints = s.ToString();
            }

            Ice.ObjectPrx lookupPrx = _communicator.stringToProxy("IceGrid/Lookup -d:" + lookupEndpoints);
            lookupPrx = lookupPrx.ice_collocationOptimized(false); // No colloc optimization for the multicast proxy!
            try
            {
                lookupPrx.ice_getConnection(); // Ensure we can establish a connection to the multicast proxy
            }
            catch (Ice.LocalException ex)
            {
                System.Text.StringBuilder s = new System.Text.StringBuilder();
                s.Append("unable to establish multicast connection, IceGrid discovery will be disabled:\n");
                s.Append("proxy = ");
                s.Append(lookupPrx.ToString());
                s.Append("\n");
                s.Append(ex);
                throw new Ice.PluginInitializationException(s.ToString());
            }

            LocatorPrx voidLo = LocatorPrxHelper.uncheckedCast(_locatorAdapter.addWithUUID(new VoidLocatorI()));
        
            string instanceName = properties.getProperty("IceGridDiscovery.InstanceName");
            Ice.Identity id = new Ice.Identity();
            id.name = "Locator";
            id.category = instanceName.Length > 0 ? instanceName : Guid.NewGuid().ToString();

            LocatorI locator = new LocatorI(LookupPrxHelper.uncheckedCast(lookupPrx), properties, instanceName, voidLo);
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
}
