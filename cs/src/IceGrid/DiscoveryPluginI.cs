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


    abstract class Request
    {
        protected Request(LocatorI locator)
        {
            locator_ = locator;
        }
        abstract public void
        invoke(Ice.LocatorPrx locatorPrx);

        abstract public void
        response(Ice.ObjectPrx objectPrx);

        protected LocatorI locator_;
        protected Ice.LocatorPrx locatorPrx_;
    }

    class LocatorI : Ice.LocatorDisp_, IceInternal.TimerTask
    {
        public
        LocatorI(LookupPrx lookup, Ice.Properties properties)
        {
            _lookup = lookup;
            _timeout = properties.getPropertyAsIntWithDefault("IceGridDiscovery.Timeout", 300);
            _retryCount = properties.getPropertyAsIntWithDefault("IceGridDiscovery.RetryCount", 3);
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

        public override void
        findObjectById_async(Ice.AMD_Locator_findObjectById amdCB, Ice.Identity id, Ice.Current curr)
        {
            lock(this)
            {
                ((LocatorI)this).invoke(null, new ObjectRequest((LocatorI)this, id, amdCB));
            }
        }

        public override void
        findAdapterById_async(Ice.AMD_Locator_findAdapterById amdCB, string adapterId, Ice.Current curr)
        {
            lock(this)
            {
                ((LocatorI)this).invoke(null, new AdapterRequest((LocatorI)this, adapterId, amdCB));
            }
        }

        public override Ice.LocatorRegistryPrx
        getRegistry(Ice.Current current)
        {
            lock(this)
            {
                Ice.LocatorPrx locator;
                if(_locator != null)
                {
                    ((LocatorI)this).queueRequest(null); // Search for locator if not already doing so.
                    while(_pendingRetryCount > 0)
                    {
                        System.Threading.Monitor.Wait(this);
                    }
                }
                locator = _locator;
                return locator != null ? locator.getRegistry() : null;
            }
        }

        public void
        foundLocator(LocatorPrx locator)
        {
            lock(this)
            {
                if(locator == null)
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
                            if (p.Equals(q))
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
                        _instanceName = _locator.ice_getIdentity().category;
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
                System.Threading.Monitor.PulseAll(this);
            }
        }

        public void
        invoke(Ice.LocatorPrx locator, Request request)
        {
            lock(this)
            {
                if(_locator != null && _locator.Equals(locator))
                {
                    request.invoke(_locator);
                }
                else
                {
                    _locator = null;
                    queueRequest(request);
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
                        req.response(null);
                    }
                    _pendingRequests.Clear();
                    System.Threading.Monitor.PulseAll(this);
                }
            }
        }

        private void
        queueRequest(Request request)
        {
            if(request != null)
            {
                _pendingRequests.Add(request);
            }

            if(_pendingRetryCount == 0) // No request in progress
            {
                _pendingRetryCount = _retryCount;
                _lookup.begin_findLocator(_instanceName, _lookupReply); // Send multicast request
                _timer.schedule(this, _timeout);
            }
        }

        private LookupPrx _lookup;
        private int _timeout;
        private IceInternal.Timer _timer;
        private int _retryCount;

        private string _instanceName;
        private bool _warned;
        private LookupReplyPrx _lookupReply;
        private Ice.LocatorPrx _locator;
        private int _pendingRetryCount;
        private List<Request> _pendingRequests = new List<Request>();
    }

    class LookupReplyI : LookupReplyDisp_
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

    class ObjectRequest : Request
    {
        public
        ObjectRequest(LocatorI locator, Ice.Identity id, Ice.AMD_Locator_findObjectById amdCB) : base(locator)
        {
            _id = id;
            _amdCB = amdCB;
        }

        override public void
        invoke(Ice.LocatorPrx l)
        {
            l.begin_findObjectById(_id).whenCompleted(this.response, this.exception);
        }

        override public void
        response(Ice.ObjectPrx prx)
        {
            _amdCB.ice_response(prx);
        }

        public void
        exception(Ice.Exception ex)
        {
            locator_.invoke(locatorPrx_, this);
        }

        private Ice.Identity _id;
        private Ice.AMD_Locator_findObjectById _amdCB;
    }

    class AdapterRequest : Request
    {
        public
        AdapterRequest(LocatorI locator, string adapterId, Ice.AMD_Locator_findAdapterById amdCB) : base(locator)
        {
            _adapterId = adapterId;
            _amdCB = amdCB;
        }

        override public void
        invoke(Ice.LocatorPrx l)
        {
            l.begin_findAdapterById(_adapterId).whenCompleted(this.response, this.exception);
        }

        override public void
        response(Ice.ObjectPrx prx)
        {
            _amdCB.ice_response(prx);
        }

        public void
        exception(Ice.Exception ex)
        {
            locator_.invoke(locatorPrx_, this); // Retry with new locator proxy.
        }

        private string _adapterId;
        private Ice.AMD_Locator_findAdapterById _amdCB;
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

            Ice.ObjectPrx lookupPrx = _communicator.stringToProxy("IceGridDiscovery/Lookup -d:" + lookupEndpoints);
            lookupPrx = lookupPrx.ice_collocationOptimized(false); // No collocation optimization for the multicast proxy!
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
                throw new Ice.PluginInitializationException(s.ToString());
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
}
