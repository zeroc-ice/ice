//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceLocatorDiscovery
{
    using System;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Threading;
    using System.Threading.Tasks;
    using System.Text;

    public sealed class PluginFactory : Ice.PluginFactory
    {
        public Ice.Plugin
        create(Ice.Communicator communicator, string name, string[] args)
        {
            return new PluginI(name, communicator);
        }
    }

    public interface Plugin : Ice.Plugin
    {
        List<Ice.LocatorPrx> getLocators(string instanceName, int waitTime);
    }

    internal class Request : TaskCompletionSource<Ice.Object_Ice_invokeResult>
    {
        public Request(LocatorI locator,
                       string operation,
                       Ice.OperationMode mode,
                       byte[] inParams,
                       Dictionary<string, string> context)
        {
            _locator = locator;
            _operation = operation;
            _mode = mode;
            _inParams = inParams;
            _context = context;
        }

        public void
        invoke(Ice.LocatorPrx l)
        {
            if(_locatorPrx == null || !_locatorPrx.Equals(l))
            {
                _locatorPrx = l;
                l.ice_invokeAsync(_operation, _mode, _inParams, _context).ContinueWith(
                    (task) =>
                    {
                        try
                        {
                            SetResult(task.Result);
                        }
                        catch(AggregateException ae)
                        {
                            exception(ae.InnerException);
                        }
                    });
            }
            else
            {
                Debug.Assert(_exception != null);
                throw _exception;
            }
        }

        private void
        exception(Exception ex)
        {
            try
            {
                throw ex;
            }
            catch(Ice.RequestFailedException exc)
            {
                SetException(exc);
            }
            catch(Ice.UnknownException exc)
            {
                SetException(exc);
            }
            catch(Ice.NoEndpointException)
            {
                SetException(new Ice.ObjectNotExistException());
            }
            catch(Ice.ObjectAdapterDeactivatedException)
            {
                SetException(new Ice.ObjectNotExistException());
            }
            catch(Ice.CommunicatorDestroyedException)
            {
                SetException(new Ice.ObjectNotExistException());
            }
            catch(Exception exc)
            {
                _exception = exc;
                _locator.invoke(_locatorPrx, this); // Retry with new locator proxy
            }
        }

        private readonly LocatorI _locator;
        private readonly string _operation;
        private readonly Ice.OperationMode _mode;
        private readonly Dictionary<string, string> _context;
        private readonly byte[] _inParams;

        private Ice.LocatorPrx _locatorPrx;
        private Exception _exception;
    }

    internal class VoidLocatorI : Ice.LocatorDisp_
    {
        public override Task<Ice.ObjectPrx>
        findObjectByIdAsync(Ice.Identity id, Ice.Current current)
        {
            return null;
        }

        public override Task<Ice.ObjectPrx>
        findAdapterByIdAsync(string id, Ice.Current current)
        {
            return null;
        }

        public override Ice.LocatorRegistryPrx
        getRegistry(Ice.Current current)
        {
            return null;
        }
    };

    internal class LocatorI : Ice.BlobjectAsync, IceInternal.TimerTask
    {
        public
        LocatorI(string name, LookupPrx lookup, Ice.Properties properties, string instanceName,
                 Ice.LocatorPrx voidLocator)
        {
            _lookup = lookup;
            _timeout = properties.getPropertyAsIntWithDefault(name + ".Timeout", 300);
            _retryCount = properties.getPropertyAsIntWithDefault(name + ".RetryCount", 3);
            _retryDelay = properties.getPropertyAsIntWithDefault(name + ".RetryDelay", 2000);
            _timer = IceInternal.Util.getInstance(lookup.ice_getCommunicator()).timer();
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
            var single = new Ice.Endpoint[1];
            foreach(var endpt in lookup.ice_getEndpoints())
            {
                single[0] = endpt;
                _lookups[(LookupPrx)lookup.ice_endpoints(single)] = null;
            }
            Debug.Assert(_lookups.Count > 0);
        }

        public void
        setLookupReply(LookupReplyPrx lookupReply)
        {
            //
            // Use a lookup reply proxy whose adress matches the interface used to send multicast datagrams.
            //
            var single = new Ice.Endpoint[1];
            foreach(var key in new List<LookupPrx>(_lookups.Keys))
            {
                var info = (Ice.UDPEndpointInfo)key.ice_getEndpoints()[0].getInfo();
                if(info.mcastInterface.Length > 0)
                {
                    foreach(var q in lookupReply.ice_getEndpoints())
                    {
                        var r = q.getInfo();
                        if(r is Ice.IPEndpointInfo && ((Ice.IPEndpointInfo)r).host.Equals(info.mcastInterface))
                        {
                            single[0] = q;
                            _lookups[key] = (LookupReplyPrx)lookupReply.ice_endpoints(single);
                        }
                    }
                }

                if(_lookups[key] == null)
                {
                    // Fallback: just use the given lookup reply proxy if no matching endpoint found.
                    _lookups[key] = lookupReply;
                }
            }
        }

        public override Task<Ice.Object_Ice_invokeResult>
        ice_invokeAsync(byte[] inParams, Ice.Current current)
        {
            lock(this)
            {
                var request = new Request(this, current.operation, current.mode, inParams, current.ctx);
                invoke(null, request);
                return request.Task;
            }
        }

        public List<Ice.LocatorPrx>
        getLocators(String instanceName, int waitTime)
        {
            //
            // Clear locators from previous search.
            //
            lock(this)
            {
                _locators.Clear();
            }

            //
            // Find a locator
            //
            invoke(null, null);

            //
            // Wait for responses
            //
            if(instanceName.Length == 0)
            {
                Thread.Sleep(waitTime);
            }
            else
            {
                lock(this)
                {
                    while(!_locators.ContainsKey(instanceName) && _pendingRetryCount > 0)
                    {
                        Monitor.Wait(this, waitTime);
                    }
                }
            }

            //
            // Return found locators
            //
            lock(this)
            {
                return new List<Ice.LocatorPrx>(_locators.Values);
            }
        }

        public void
        foundLocator(Ice.LocatorPrx locator)
        {
            lock(this)
            {
                if(locator == null ||
                   (_instanceName.Length > 0 && !locator.ice_getIdentity().category.Equals(_instanceName)))
                {
                    if(_traceLevel > 2)
                    {
                        StringBuilder s = new StringBuilder("ignoring locator reply: instance name doesn't match\n");
                        s.Append("expected = ").Append(_instanceName);
                        s.Append("received = ").Append(locator.ice_getIdentity().category);
                        _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.ToString());
                    }
                    return;
                }

                //
                // If we already have a locator assigned, ensure the given locator
                // has the same identity, otherwise ignore it.
                //
                if(_pendingRequests.Count > 0 &&
                   _locator != null && !locator.ice_getIdentity().category.Equals(_locator.ice_getIdentity().category))
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
                    _timer.cancel(this);
                    _pendingRetryCount = 0;
                }

                if(_traceLevel > 0)
                {
                    StringBuilder s = new StringBuilder("locator lookup succeeded:\nlocator = ");
                    s.Append(locator);
                    if(_instanceName.Length == 0)
                    {
                        s.Append("\ninstance name = ").Append(_instanceName);
                    }
                    _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.ToString());
                }

                Ice.LocatorPrx l = null;
                if(_pendingRequests.Count == 0)
                {
                    _locators.TryGetValue(locator.ice_getIdentity().category, out _locator);
                }
                else
                {
                    l = _locator;
                }
                if(l != null)
                {
                    //
                    // We found another locator replica, append its endpoints to the
                    // current locator proxy endpoints.
                    //
                    List<Ice.Endpoint> newEndpoints = new List<Ice.Endpoint>(l.ice_getEndpoints());
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
                    l = (Ice.LocatorPrx)l.ice_endpoints(newEndpoints.ToArray());
                }
                else
                {
                    l = locator;
                }

                if(_pendingRequests.Count == 0)
                {
                    _locators[locator.ice_getIdentity().category] = l;
                    Monitor.Pulse(this);
                }
                else
                {
                    _locator = l;
                    if(_instanceName.Length == 0)
                    {
                        _instanceName = _locator.ice_getIdentity().category; // Stick to the first locator
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
        }

        public void
        invoke(Ice.LocatorPrx locator, Request request)
        {
            lock(this)
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
                        _pendingRequests.Add(request);
                    }

                    if(_pendingRetryCount == 0) // No request in progress
                    {
                        _pendingRetryCount = _retryCount;
                        _failureCount = 0;
                        try
                        {
                            if(_traceLevel > 1)
                            {
                                StringBuilder s = new StringBuilder("looking up locator:\nlookup = ");
                                s.Append(_lookup);
                                if(_instanceName.Length == 0)
                                {
                                    s.Append("\ninstance name = ").Append(_instanceName);
                                }
                                _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.ToString());
                            }

                            foreach(var l in _lookups)
                            {
                                l.Key.findLocatorAsync(_instanceName, l.Value).ContinueWith(t => {
                                    try
                                    {
                                        t.Wait();
                                    }
                                    catch(AggregateException ex)
                                    {
                                        exception(ex.InnerException);
                                    }
                                }, l.Key.ice_scheduler()); // Send multicast request.
                            }
                            _timer.schedule(this, _timeout);
                        }
                        catch(Ice.LocalException ex)
                        {
                            if(_traceLevel > 0)
                            {
                                StringBuilder s = new StringBuilder("locator lookup failed:\nlookup = ");
                                s.Append(_lookup);
                                if(_instanceName.Length == 0)
                                {
                                    s.Append("\ninstance name = ").Append(_instanceName);
                                }
                                s.Append("\n").Append(ex);
                                _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.ToString());
                            }

                            foreach(Request req in _pendingRequests)
                            {
                                req.invoke(_voidLocator);
                            }
                            _pendingRequests.Clear();
                            _pendingRetryCount = 0;
                        }
                    }
                }
            }
        }

        void exception(Exception ex)
        {
            lock(this)
            {
                if(++_failureCount == _lookups.Count && _pendingRetryCount > 0)
                {
                    //
                    // All the lookup calls failed, cancel the timer and propagate the error to the requests.
                    //
                    _timer.cancel(this);

                    _pendingRetryCount = 0;

                    if(_warnOnce)
                    {
                        StringBuilder builder = new StringBuilder();
                        builder.Append("failed to lookup locator with lookup proxy `");
                        builder.Append(_lookup);
                        builder.Append("':\n");
                        builder.Append(ex);
                        _lookup.ice_getCommunicator().getLogger().warning(builder.ToString());
                        _warnOnce = false;
                    }

                    if(_traceLevel > 0)
                    {
                        StringBuilder s = new StringBuilder("locator lookup failed:\nlookup = ");
                        s.Append(_lookup);
                        if(_instanceName.Length == 0)
                        {
                            s.Append("\ninstance name = ").Append(_instanceName);
                        }
                        s.Append("\n").Append(ex);
                        _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.ToString());
                    }

                    if(_pendingRequests.Count == 0)
                    {
                        Monitor.Pulse(this);
                    }
                    else
                    {
                        foreach(Request req in _pendingRequests)
                        {
                            req.invoke(_voidLocator);
                        }
                        _pendingRequests.Clear();
                    }
                }
            }
        }

        public void runTimerTask()
        {
            lock(this)
            {
                if(--_pendingRetryCount > 0)
                {
                    try
                    {
                        if(_traceLevel > 1)
                        {
                            StringBuilder s = new StringBuilder("retrying locator lookup:\nlookup = ");
                            s.Append(_lookup);
                            s.Append("\nretry count = ").Append(_retryCount);
                            if(_instanceName.Length == 0)
                            {
                                s.Append("\ninstance name = ").Append(_instanceName);
                            }
                            _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.ToString());
                        }

                        foreach(var l in _lookups)
                        {
                            l.Key.findLocatorAsync(_instanceName, l.Value).ContinueWith(t => {
                                    try
                                    {
                                        t.Wait();
                                    }
                                    catch(AggregateException ex)
                                    {
                                        exception(ex.InnerException);
                                    }
                                }, l.Key.ice_scheduler()); // Send multicast request.
                        }
                        _timer.schedule(this, _timeout);
                        return;
                    }
                    catch(Ice.LocalException)
                    {
                    }
                    _pendingRetryCount = 0;
                }

                if(_traceLevel > 0)
                {
                    StringBuilder s = new StringBuilder("locator lookup timed out:\nlookup = ");
                    s.Append(_lookup);
                    if(_instanceName.Length == 0)
                    {
                        s.Append("\ninstance name = ").Append(_instanceName);
                    }
                    _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.ToString());
                }

                if(_pendingRequests.Count == 0)
                {
                    Monitor.Pulse(this);
                }
                else
                {
                    foreach(Request req in _pendingRequests)
                    {
                        req.invoke(_voidLocator);
                    }
                    _pendingRequests.Clear();
                }
                _nextRetry = IceInternal.Time.currentMonotonicTimeMillis() + _retryDelay;
            }
        }

        private LookupPrx _lookup;
        private Dictionary<LookupPrx, LookupReplyPrx> _lookups = new Dictionary<LookupPrx, LookupReplyPrx>();
        private int _timeout;
        private IceInternal.Timer _timer;
        private int _traceLevel;
        private int _retryCount;
        private int _retryDelay;

        private string _instanceName;
        private bool _warned;
        private Ice.LocatorPrx _locator;
        private Ice.LocatorPrx _voidLocator;
        private Dictionary<string, Ice.LocatorPrx> _locators = new Dictionary<string, Ice.LocatorPrx>();

        private int _pendingRetryCount;
        private int _failureCount;
        private bool _warnOnce = true;
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
        foundLocator(Ice.LocatorPrx locator, Ice.Current current)
        {
            _locator.foundLocator(locator);
        }

        private LocatorI _locator;
    }

    internal class PluginI : Ice.Plugin
    {
        public
        PluginI(string name, Ice.Communicator communicator)
        {
            _name = name;
            _communicator = communicator;
        }

        public void
        initialize()
        {
            Ice.Properties properties = _communicator.getProperties();

            bool ipv4 = properties.getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
            bool preferIPv6 = properties.getPropertyAsInt("Ice.PreferIPv6Address") > 0;
            string address;
            if(ipv4 && !preferIPv6)
            {
                address = properties.getPropertyWithDefault(_name + ".Address", "239.255.0.1");
            }
            else
            {
                address = properties.getPropertyWithDefault(_name + ".Address", "ff15::1");
            }
            int port = properties.getPropertyAsIntWithDefault(_name + ".Port", 4061);
            string intf = properties.getProperty(_name + ".Interface");

            string lookupEndpoints = properties.getProperty(_name + ".Lookup");
            if(lookupEndpoints.Length == 0)
            {
                int protocol = ipv4 && !preferIPv6 ? IceInternal.Network.EnableIPv4 : IceInternal.Network.EnableIPv6;
                var interfaces = IceInternal.Network.getInterfacesForMulticast(intf, protocol);
                foreach(string p in interfaces)
                {
                    if(p != interfaces[0])
                    {
                        lookupEndpoints += ":";
                    }
                    lookupEndpoints += "udp -h \"" + address + "\" -p " + port + " --interface \"" + p + "\"";
                }
            }

            if(properties.getProperty(_name + ".Reply.Endpoints").Length == 0)
            {
                properties.setProperty(_name + ".Reply.Endpoints",
                                       "udp -h " + (intf.Length == 0 ? "*" : "\"" + intf + "\""));
            }

            if(properties.getProperty(_name + ".Locator.Endpoints").Length == 0)
            {
                properties.setProperty(_name + ".Locator.AdapterId", Guid.NewGuid().ToString());
            }

            _replyAdapter = _communicator.createObjectAdapter(_name + ".Reply");
            _locatorAdapter = _communicator.createObjectAdapter(_name + ".Locator");

            // We don't want those adapters to be registered with the locator so clear their locator.
            _replyAdapter.setLocator(null);
            _locatorAdapter.setLocator(null);

            Ice.ObjectPrx lookupPrx = _communicator.stringToProxy("IceLocatorDiscovery/Lookup -d:" + lookupEndpoints);
            // No colloc optimization or router for the multicast proxy!
            lookupPrx = lookupPrx.ice_collocationOptimized(false).ice_router(null);

            Ice.LocatorPrx voidLo = Ice.LocatorPrxHelper.uncheckedCast(_locatorAdapter.addWithUUID(new VoidLocatorI()));

            string instanceName = properties.getProperty(_name + ".InstanceName");
            Ice.Identity id = new Ice.Identity();
            id.name = "Locator";
            id.category = instanceName.Length > 0 ? instanceName : Guid.NewGuid().ToString();

            _defaultLocator = _communicator.getDefaultLocator();
            _locator = new LocatorI(_name, LookupPrxHelper.uncheckedCast(lookupPrx), properties, instanceName, voidLo);
            _locatorPrx = Ice.LocatorPrxHelper.uncheckedCast(_locatorAdapter.addWithUUID(_locator));
            _communicator.setDefaultLocator(_locatorPrx);

            Ice.ObjectPrx lookupReply = _replyAdapter.addWithUUID(new LookupReplyI(_locator)).ice_datagram();
            _locator.setLookupReply(LookupReplyPrxHelper.uncheckedCast(lookupReply));

            _replyAdapter.activate();
            _locatorAdapter.activate();
        }

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
            if(_communicator.getDefaultLocator().Equals(_locatorPrx))
            {
                // Restore original default locator proxy, if the user didn't change it in the meantime
                _communicator.setDefaultLocator(_defaultLocator);
            }
        }

        List<Ice.LocatorPrx>
        getLocators(string instanceName, int waitTime)
        {
            return _locator.getLocators(instanceName, waitTime);
        }

        private string _name;
        private Ice.Communicator _communicator;
        private Ice.ObjectAdapter _locatorAdapter;
        private Ice.ObjectAdapter _replyAdapter;
        private LocatorI _locator;
        private Ice.LocatorPrx _locatorPrx;
        private Ice.LocatorPrx _defaultLocator;
    }

    public class Util
    {
        public static void
        registerIceLocatorDiscovery(bool loadOnInitialize)
        {
            Ice.Util.registerPluginFactory("IceLocatorDiscovery", new PluginFactory(), loadOnInitialize);
        }
    }
}
