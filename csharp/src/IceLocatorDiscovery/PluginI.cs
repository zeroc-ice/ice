// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Text;

namespace IceLocatorDiscovery;

public sealed class PluginFactory : Ice.PluginFactory
{
    public Ice.Plugin
    create(Ice.Communicator communicator, string name, string[] args) => new PluginI(communicator);
}

public interface Plugin : Ice.Plugin
{
    List<Ice.LocatorPrx> getLocators(string instanceName, int waitTime);
}

internal class Request : TaskCompletionSource<Ice.Object_Ice_invokeResult>
{
    public Request(
        LocatorI locator,
        string operation,
        Ice.OperationMode mode,
        byte[] inParams,
        Dictionary<string, string> context)
        : base(TaskCreationOptions.RunContinuationsAsynchronously)
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
        if (_locatorPrx == null || !_locatorPrx.Equals(l))
        {
            _locatorPrx = l;
            _ = performInvokeAsync(l);
        }
        else
        {
            Debug.Assert(_exception != null);
            throw _exception;
        }

        async Task performInvokeAsync(Ice.LocatorPrx locator)
        {
            try
            {
                var result =
                    await locator.ice_invokeAsync(_operation, _mode, _inParams, _context).ConfigureAwait(false);
                SetResult(result);
            }
            catch (Ice.RequestFailedException exc)
            {
                SetException(exc);
            }
            catch (Ice.UnknownException exc)
            {
                SetException(exc);
            }
            catch (Ice.NoEndpointException)
            {
                SetException(new Ice.ObjectNotExistException());
            }
            catch (Ice.ObjectAdapterDeactivatedException)
            {
                SetException(new Ice.ObjectNotExistException());
            }
            catch (Ice.ObjectAdapterDestroyedException)
            {
                SetException(new Ice.ObjectNotExistException());
            }
            catch (Ice.CommunicatorDestroyedException)
            {
                SetException(new Ice.ObjectNotExistException());
            }
            catch (Exception exc)
            {
                _exception = exc;
                _locator.invoke(_locatorPrx, this); // Retry with new locator proxy
            }
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
        return Task.FromResult<Ice.ObjectPrx>(null);
    }

    public override Task<Ice.ObjectPrx>
    findAdapterByIdAsync(string id, Ice.Current current)
    {
        return Task.FromResult<Ice.ObjectPrx>(null);
    }

    public override Ice.LocatorRegistryPrx
    getRegistry(Ice.Current current)
    {
        return null;
    }
}

internal class LocatorI : Ice.BlobjectAsync, Ice.Internal.TimerTask
{
    public
    LocatorI(LookupPrx lookup, Ice.Properties properties, string instanceName, Ice.LocatorPrx voidLocator)
    {
        _lookup = lookup;
        _timeout = properties.getIcePropertyAsInt("IceLocatorDiscovery.Timeout");
        if (_timeout < 0)
        {
            _timeout = 300;
        }
        _retryCount = properties.getIcePropertyAsInt("IceLocatorDiscovery.RetryCount");
        if (_retryCount < 0)
        {
            _retryCount = 0;
        }
        _retryDelay = properties.getIcePropertyAsInt("IceLocatorDiscovery.RetryDelay");
        if (_retryDelay < 0)
        {
            _retryDelay = 0;
        }
        _timer = Ice.Internal.Util.getInstance(lookup.ice_getCommunicator()).timer();
        _traceLevel = properties.getIcePropertyAsInt("IceLocatorDiscovery.Trace.Lookup");
        _instanceName = instanceName;
        _warned = false;
        _locator = lookup.ice_getCommunicator().getDefaultLocator();
        _voidLocator = voidLocator;
        _pending = false;
        _pendingRetryCount = 0;
        _failureCount = 0;
        _warnOnce = true;

        //
        // Create one lookup proxy per endpoint from the given proxy. We want to send a multicast
        // datagram on each endpoint.
        //
        var single = new Ice.Endpoint[1];
        foreach (Ice.Endpoint endpoint in lookup.ice_getEndpoints())
        {
            single[0] = endpoint;
            _lookups[(LookupPrx)lookup.ice_endpoints(single)] = null;
        }
        Debug.Assert(_lookups.Count > 0);
    }

    public void
    setLookupReply(LookupReplyPrx lookupReply)
    {
        //
        // Use a lookup reply proxy whose address matches the interface used to send multicast datagrams.
        //
        var single = new Ice.Endpoint[1];
        foreach (var key in new List<LookupPrx>(_lookups.Keys))
        {
            var info = (Ice.UDPEndpointInfo)key.ice_getEndpoints()[0].getInfo();
            if (info.mcastInterface.Length > 0)
            {
                foreach (var q in lookupReply.ice_getEndpoints())
                {
                    var r = q.getInfo();
                    if (r is Ice.IPEndpointInfo &&
                        ((Ice.IPEndpointInfo)r).host.Equals(info.mcastInterface, StringComparison.Ordinal))
                    {
                        single[0] = q;
                        _lookups[key] = (LookupReplyPrx)lookupReply.ice_endpoints(single);
                    }
                }
            }

            if (_lookups[key] == null)
            {
                // Fallback: just use the given lookup reply proxy if no matching endpoint found.
                _lookups[key] = lookupReply;
            }
        }
    }

    public override Task<Ice.Object_Ice_invokeResult>
    ice_invokeAsync(byte[] inParams, Ice.Current current)
    {
        lock (_mutex)
        {
            var request = new Request(this, current.operation, current.mode, inParams, current.ctx);
            invoke(null, request);
            return request.Task;
        }
    }

    public List<Ice.LocatorPrx> getLocators(string instanceName, int waitTime)
    {
        //
        // Clear locators from previous search.
        //
        lock (_mutex)
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
        if (instanceName.Length == 0)
        {
            Thread.Sleep(waitTime);
        }
        else
        {
            lock (_mutex)
            {
                while (!_locators.ContainsKey(instanceName) && _pending)
                {
                    Monitor.Wait(_mutex, waitTime);
                }
            }
        }

        //
        // Return found locators
        //
        lock (_mutex)
        {
            return new List<Ice.LocatorPrx>(_locators.Values);
        }
    }

    public void
    foundLocator(Ice.LocatorPrx locator)
    {
        lock (_mutex)
        {
            if (locator == null)
            {
                if (_traceLevel > 2)
                {
                    _lookup.ice_getCommunicator().getLogger().trace(
                        "Lookup",
                        "ignoring locator reply: (null locator)");
                }
                return;
            }

            if (_instanceName.Length > 0 &&
                !locator.ice_getIdentity().category.Equals(_instanceName, StringComparison.Ordinal))
            {
                if (_traceLevel > 2)
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
            if (_pendingRequests.Count > 0 &&
               _locator != null &&
               !locator.ice_getIdentity().category.Equals(_locator.ice_getIdentity().category, StringComparison.Ordinal))
            {
                if (!_warned)
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

            if (_pending) // No need to continue, we found a locator
            {
                _timer.cancel(this);
                _pendingRetryCount = 0;
                _pending = false;
            }

            if (_traceLevel > 0)
            {
                StringBuilder s = new StringBuilder("locator lookup succeeded:\nlocator = ");
                s.Append(locator);
                if (_instanceName.Length > 0)
                {
                    s.Append("\ninstance name = ").Append(_instanceName);
                }
                _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.ToString());
            }

            Ice.LocatorPrx l = null;
            if (_pendingRequests.Count == 0)
            {
                _locators.TryGetValue(locator.ice_getIdentity().category, out _locator);
            }
            else
            {
                l = _locator;
            }
            if (l != null)
            {
                //
                // We found another locator replica, append its endpoints to the
                // current locator proxy endpoints.
                //
                List<Ice.Endpoint> newEndpoints = new List<Ice.Endpoint>(l.ice_getEndpoints());
                foreach (Ice.Endpoint p in locator.ice_getEndpoints())
                {
                    //
                    // Only add endpoints if not already in the locator proxy endpoints
                    //
                    bool found = false;
                    foreach (Ice.Endpoint q in newEndpoints)
                    {
                        if (p.Equals(q))
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
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

            if (_pendingRequests.Count == 0)
            {
                _locators[locator.ice_getIdentity().category] = l;
                Monitor.Pulse(_mutex);
            }
            else
            {
                _locator = l;
                if (_instanceName.Length == 0)
                {
                    _instanceName = _locator.ice_getIdentity().category; // Stick to the first locator
                }

                //
                // Send pending requests if any.
                //
                foreach (Request req in _pendingRequests)
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
        lock (_mutex)
        {
            if (request != null && _locator != null && _locator != locator)
            {
                request.invoke(_locator);
            }
            else if (request != null && Ice.Internal.Time.currentMonotonicTimeMillis() < _nextRetry)
            {
                request.invoke(_voidLocator); // Don't retry to find a locator before the retry delay expires
            }
            else
            {
                _locator = null;

                if (request != null)
                {
                    _pendingRequests.Add(request);
                }

                if (!_pending) // No request in progress
                {
                    _pending = true;
                    _pendingRetryCount = _retryCount;
                    _failureCount = 0;
                    try
                    {
                        if (_traceLevel > 1)
                        {
                            StringBuilder s = new StringBuilder("looking up locator:\nlookup = ");
                            s.Append(_lookup);
                            if (_instanceName.Length > 0)
                            {
                                s.Append("\ninstance name = ").Append(_instanceName);
                            }
                            _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.ToString());
                        }

                        foreach (var l in _lookups)
                        {
                            _ = preformFindLocatorAsync(l.Key, l.Value);
                        }
                        _timer.schedule(this, _timeout);
                    }
                    catch (Ice.LocalException ex)
                    {
                        if (_traceLevel > 0)
                        {
                            StringBuilder s = new StringBuilder("locator lookup failed:\nlookup = ");
                            s.Append(_lookup);
                            if (_instanceName.Length > 0)
                            {
                                s.Append("\ninstance name = ").Append(_instanceName);
                            }
                            s.Append('\n').Append(ex);
                            _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.ToString());
                        }

                        foreach (Request req in _pendingRequests)
                        {
                            req.invoke(_voidLocator);
                        }
                        _pendingRequests.Clear();
                        _pendingRetryCount = 0;
                        _pending = false;
                    }
                }
            }
        }

        async Task preformFindLocatorAsync(LookupPrx lookupPrx, LookupReplyPrx lookupReplyPrx)
        {
            // Send multicast request.
            try
            {
                await lookupPrx.findLocatorAsync(_instanceName, lookupReplyPrx).ConfigureAwait(false);
            }
            catch (System.Exception ex)
            {
                exception(ex);
            }
        }
    }

    private void exception(Exception ex)
    {
        lock (_mutex)
        {
            if (++_failureCount == _lookups.Count && _pending)
            {
                //
                // All the lookup calls failed, cancel the timer and propagate the error to the requests.
                //
                _timer.cancel(this);
                _pendingRetryCount = 0;
                _pending = false;

                if (_warnOnce)
                {
                    StringBuilder builder = new StringBuilder();
                    builder.Append("failed to lookup locator with lookup proxy `");
                    builder.Append(_lookup);
                    builder.Append("':\n");
                    builder.Append(ex);
                    _lookup.ice_getCommunicator().getLogger().warning(builder.ToString());
                    _warnOnce = false;
                }

                if (_traceLevel > 0)
                {
                    StringBuilder s = new StringBuilder("locator lookup failed:\nlookup = ");
                    s.Append(_lookup);
                    if (_instanceName.Length > 0)
                    {
                        s.Append("\ninstance name = ").Append(_instanceName);
                    }
                    s.Append('\n').Append(ex);
                    _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.ToString());
                }

                if (_pendingRequests.Count == 0)
                {
                    Monitor.Pulse(_mutex);
                }
                else
                {
                    foreach (Request req in _pendingRequests)
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
        lock (_mutex)
        {
            if (!_pending)
            {
                Debug.Assert(_pendingRequests.Count == 0);
                return; // Request failed
            }

            if (_pendingRetryCount > 0)
            {
                --_pendingRetryCount;
                try
                {
                    if (_traceLevel > 1)
                    {
                        StringBuilder s = new StringBuilder("retrying locator lookup:\nlookup = ");
                        s.Append(_lookup);
                        s.Append("\nretry count = ").Append(_retryCount);
                        if (_instanceName.Length > 0)
                        {
                            s.Append("\ninstance name = ").Append(_instanceName);
                        }
                        _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.ToString());
                    }

                    foreach (var l in _lookups)
                    {
                        l.Key.findLocatorAsync(_instanceName, l.Value).ContinueWith(
                            t =>
                            {
                                try
                                {
                                    t.Wait();
                                }
                                catch (AggregateException ex)
                                {
                                    exception(ex.InnerException);
                                }
                            },
                            l.Key.ice_scheduler()); // Send multicast request.
                    }
                    _timer.schedule(this, _timeout);
                    return;
                }
                catch (Ice.LocalException)
                {
                }
                _pendingRetryCount = 0;
            }

            Debug.Assert(_pendingRetryCount == 0);
            _pending = false;

            if (_traceLevel > 0)
            {
                StringBuilder s = new StringBuilder("locator lookup timed out:\nlookup = ");
                s.Append(_lookup);
                if (_instanceName.Length > 0)
                {
                    s.Append("\ninstance name = ").Append(_instanceName);
                }
                _lookup.ice_getCommunicator().getLogger().trace("Lookup", s.ToString());
            }

            if (_pendingRequests.Count == 0)
            {
                Monitor.Pulse(_mutex);
            }
            else
            {
                foreach (Request req in _pendingRequests)
                {
                    req.invoke(_voidLocator);
                }
                _pendingRequests.Clear();
            }
            _nextRetry = Ice.Internal.Time.currentMonotonicTimeMillis() + _retryDelay;
        }
    }

    private LookupPrx _lookup;
    private Dictionary<LookupPrx, LookupReplyPrx> _lookups = new Dictionary<LookupPrx, LookupReplyPrx>();
    private int _timeout;
    private Ice.Internal.Timer _timer;
    private int _traceLevel;
    private int _retryCount;
    private int _retryDelay;

    private string _instanceName;
    private bool _warned;
    private Ice.LocatorPrx _locator;
    private Ice.LocatorPrx _voidLocator;
    private Dictionary<string, Ice.LocatorPrx> _locators = new Dictionary<string, Ice.LocatorPrx>();

    private bool _pending;
    private int _pendingRetryCount;
    private int _failureCount;
    private bool _warnOnce = true;
    private List<Request> _pendingRequests = new List<Request>();
    private long _nextRetry;
    private readonly object _mutex = new();
}

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
    PluginI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    public void
    initialize()
    {
        Ice.Properties properties = _communicator.getProperties();

        bool ipv4 = properties.getIcePropertyAsInt("Ice.IPv4") > 0;
        bool preferIPv6 = properties.getIcePropertyAsInt("Ice.PreferIPv6Address") > 0;
        string address = properties.getIceProperty("IceLocatorDiscovery.Address");
        if (address.Length == 0)
        {
            address = ipv4 && !preferIPv6 ? "239.255.0.1" : "ff15::1";
        }
        int port = properties.getIcePropertyAsInt("IceLocatorDiscovery.Port");
        string intf = properties.getIceProperty("IceLocatorDiscovery.Interface");

        string lookupEndpoints = properties.getIceProperty("IceLocatorDiscovery.Lookup");
        if (lookupEndpoints.Length == 0)
        {
            int protocol = ipv4 && !preferIPv6 ? Ice.Internal.Network.EnableIPv4 : Ice.Internal.Network.EnableIPv6;
            var interfaces = Ice.Internal.Network.getInterfacesForMulticast(intf, protocol);
            foreach (string p in interfaces)
            {
                if (p != interfaces[0])
                {
                    lookupEndpoints += ":";
                }
                lookupEndpoints += "udp -h \"" + address + "\" -p " + port + " --interface \"" + p + "\"";
            }
        }

        if (properties.getIceProperty("IceLocatorDiscovery.Reply.Endpoints").Length == 0)
        {
            properties.setProperty(
                "IceLocatorDiscovery.Reply.Endpoints",
                "udp -h " + (intf.Length == 0 ? "*" : "\"" + intf + "\""));
        }

        if (properties.getIceProperty("IceLocatorDiscovery.Locator.Endpoints").Length == 0)
        {
            properties.setProperty("IceLocatorDiscovery.Locator.AdapterId", Guid.NewGuid().ToString());
        }

        _replyAdapter = _communicator.createObjectAdapter("IceLocatorDiscovery.Reply");
        _locatorAdapter = _communicator.createObjectAdapter("IceLocatorDiscovery.Locator");

        // We don't want those adapters to be registered with the locator so clear their locator.
        _replyAdapter.setLocator(null);
        _locatorAdapter.setLocator(null);

        Ice.ObjectPrx lookupPrx = _communicator.stringToProxy("IceLocatorDiscovery/Lookup -d:" + lookupEndpoints);
        // No colloc optimization or router for the multicast proxy!
        lookupPrx = lookupPrx.ice_collocationOptimized(false).ice_router(null);

        Ice.LocatorPrx voidLo = Ice.LocatorPrxHelper.uncheckedCast(_locatorAdapter.addWithUUID(new VoidLocatorI()));

        string instanceName = properties.getIceProperty("IceLocatorDiscovery.InstanceName");
        var id = new Ice.Identity("Locator", instanceName.Length > 0 ? instanceName : Guid.NewGuid().ToString());

        _defaultLocator = _communicator.getDefaultLocator();
        _locator = new LocatorI(LookupPrxHelper.uncheckedCast(lookupPrx), properties, instanceName, voidLo);
        _locatorPrx = Ice.LocatorPrxHelper.uncheckedCast(_locatorAdapter.add(_locator, id));
        _communicator.setDefaultLocator(_locatorPrx);

        Ice.ObjectPrx lookupReply = _replyAdapter.addWithUUID(new LookupReplyI(_locator)).ice_datagram();
        _locator.setLookupReply(LookupReplyPrxHelper.uncheckedCast(lookupReply));

        _replyAdapter.activate();
        _locatorAdapter.activate();
    }

    public void
    destroy()
    {
        if (_replyAdapter != null)
        {
            _replyAdapter.destroy();
        }
        if (_locatorAdapter != null)
        {
            _locatorAdapter.destroy();
        }
        if (_communicator.getDefaultLocator().Equals(_locatorPrx))
        {
            // Restore original default locator proxy, if the user didn't change it in the meantime
            _communicator.setDefaultLocator(_defaultLocator);
        }
    }

    private List<Ice.LocatorPrx>
    getLocators(string instanceName, int waitTime)
    {
        return _locator.getLocators(instanceName, waitTime);
    }

    private Ice.Communicator _communicator;
    private Ice.ObjectAdapter _locatorAdapter;
    private Ice.ObjectAdapter _replyAdapter;
    private LocatorI _locator;
    private Ice.LocatorPrx _locatorPrx;
    private Ice.LocatorPrx _defaultLocator;
}

public static class Util
{
    public static void
    registerIceLocatorDiscovery(bool loadOnInitialize)
    {
        Ice.Util.registerPluginFactory("IceLocatorDiscovery", new PluginFactory(), loadOnInitialize);
    }
}
