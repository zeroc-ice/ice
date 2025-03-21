// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.Internal;

public sealed class LocatorInfo : IEquatable<LocatorInfo>
{
    public interface GetEndpointsCallback
    {
        void setEndpoints(EndpointI[] endpoints, bool cached);

        void setException(Ice.LocalException ex);
    }

    private class RequestCallback
    {
        public void
        response(LocatorInfo locatorInfo, Ice.ObjectPrx proxy)
        {
            EndpointI[] endpoints = null;
            if (proxy != null)
            {
                Reference r = ((Ice.ObjectPrxHelperBase)proxy).iceReference();
                if (_ref.isWellKnown() && !Protocol.isSupported(_ref.getEncoding(), r.getEncoding()))
                {
                    //
                    // If a well-known proxy and the returned
                    // proxy encoding isn't supported, we're done:
                    // there's no compatible endpoint we can use.
                    //
                }
                else if (!r.isIndirect())
                {
                    endpoints = r.getEndpoints();
                }
                else if (_ref.isWellKnown() && !r.isWellKnown())
                {
                    //
                    // We're resolving the endpoints of a well-known object and the proxy returned
                    // by the locator is an indirect proxy. We now need to resolve the endpoints
                    // of this indirect proxy.
                    //
                    if (_ref.getInstance().traceLevels().location >= 1)
                    {
                        locatorInfo.trace(
                            "retrieved adapter for well-known object from locator, adding to locator cache",
                            _ref,
                            r);
                    }
                    locatorInfo.getEndpoints(r, _ref, _ttl, _callback);
                    return;
                }
            }

            if (_ref.getInstance().traceLevels().location >= 1)
            {
                locatorInfo.getEndpointsTrace(_ref, endpoints, false);
            }
            _callback?.setEndpoints(endpoints ?? [], false);
        }

        public void
        exception(LocatorInfo locatorInfo, Ice.Exception exc)
        {
            try
            {
                locatorInfo.getEndpointsException(_ref, exc); // This throws.
            }
            catch (Ice.LocalException ex)
            {
                _callback?.setException(ex);
            }
        }

        public
        RequestCallback(Reference @ref, TimeSpan ttl, GetEndpointsCallback cb)
        {
            _ref = @ref;
            _ttl = ttl;
            _callback = cb;
        }

        private readonly Reference _ref;
        private readonly TimeSpan _ttl;
        private readonly GetEndpointsCallback _callback;
    }

    private abstract class Request
    {
        public void
        addCallback(Reference @ref, Reference wellKnownRef, TimeSpan ttl, GetEndpointsCallback cb)
        {
            RequestCallback callback = new RequestCallback(@ref, ttl, cb);
            lock (_mutex)
            {
                if (!_response && _exception == null)
                {
                    _callbacks.Add(callback);
                    if (wellKnownRef != null)
                    {
                        // This request is to resolve the endpoints of a cached well-known object ref
                        _wellKnownRefs.Add(wellKnownRef);
                    }
                    if (!_sent)
                    {
                        _sent = true;
                        send();
                    }
                    return;
                }
            }

            if (_response)
            {
                callback.response(_locatorInfo, _proxy);
            }
            else
            {
                Debug.Assert(_exception != null);
                callback.exception(_locatorInfo, _exception);
            }
        }

        public Request(LocatorInfo locatorInfo, Reference @ref)
        {
            _locatorInfo = locatorInfo;
            _ref = @ref;
            _sent = false;
            _response = false;
        }

        public void
        response(Ice.ObjectPrx proxy)
        {
            lock (_mutex)
            {
                _locatorInfo.finishRequest(_ref, _wellKnownRefs, proxy, false);
                _response = true;
                _proxy = proxy;
                Monitor.PulseAll(_mutex);
            }
            foreach (RequestCallback callback in _callbacks)
            {
                callback.response(_locatorInfo, proxy);
            }
        }

        public void
        exception(Ice.Exception ex)
        {
            lock (_mutex)
            {
                _locatorInfo.finishRequest(_ref, _wellKnownRefs, null, ex is Ice.UserException);
                _exception = ex;
                Monitor.PulseAll(_mutex);
            }

            foreach (RequestCallback callback in _callbacks)
            {
                callback.exception(_locatorInfo, ex);
            }
        }

        protected abstract void send();

        protected readonly LocatorInfo _locatorInfo;
        protected readonly Reference _ref;

        private readonly List<RequestCallback> _callbacks = new List<RequestCallback>();
        private readonly List<Reference> _wellKnownRefs = new List<Reference>();
        private bool _sent;
        private bool _response;
        private Ice.ObjectPrx _proxy;
        private Ice.Exception _exception;
        private readonly object _mutex = new();
    }

    private class ObjectRequest : Request
    {
        public ObjectRequest(LocatorInfo locatorInfo, Reference reference)
            : base(locatorInfo, reference)
        {
        }

        protected override void
        send()
        {
            _ = Task.Run(
                async () =>
                {
                    try
                    {
                        var locator = _locatorInfo.getLocator();
                        var proxy = await locator.findObjectByIdAsync(_ref.getIdentity()).ConfigureAwait(false);
                        response(proxy);
                    }
                    catch (Ice.Exception ex)
                    {
                        exception(ex);
                    }
                });
        }
    }

    private class AdapterRequest : Request
    {
        public AdapterRequest(LocatorInfo locatorInfo, Reference reference)
            : base(locatorInfo, reference)
        {
        }

        protected override void
        send()
        {
            _ = Task.Run(
                async () =>
                {
                    try
                    {
                        var locator = _locatorInfo.getLocator();
                        var proxy = await locator.findAdapterByIdAsync(_ref.getAdapterId()).ConfigureAwait(false);
                        response(proxy);
                    }
                    catch (Ice.Exception ex)
                    {
                        exception(ex);
                    }
                });
        }
    }

    internal LocatorInfo(Ice.LocatorPrx locator, LocatorTable table, bool background)
    {
        _locator = locator;
        _table = table;
        _background = background;
    }

    public void destroy()
    {
        lock (_mutex)
        {
            _locatorRegistry = null;
            _table.clear();
        }
    }

    public static bool operator ==(LocatorInfo lhs, LocatorInfo rhs) => lhs is not null ? lhs.Equals(rhs) : rhs is null;

    public static bool operator !=(LocatorInfo lhs, LocatorInfo rhs) => !(lhs == rhs);

    public bool Equals(LocatorInfo other) =>
        ReferenceEquals(this, other) || (other is not null && _locator.Equals(other._locator));

    public override bool Equals(object obj) => Equals(obj as LocatorInfo);

    public override int GetHashCode() => _locator.GetHashCode();

    public Ice.LocatorPrx getLocator()
    {
        //
        // No synchronization necessary, _locator is immutable.
        //
        return _locator;
    }

    public Ice.LocatorRegistryPrx getLocatorRegistry()
    {
        lock (_mutex)
        {
            if (_locatorRegistry != null)
            {
                return _locatorRegistry;
            }
        }

        //
        // Do not make locator calls from within sync.
        //
        Ice.LocatorRegistryPrx locatorRegistry = _locator.getRegistry();
        if (locatorRegistry == null)
        {
            return null;
        }

        lock (_mutex)
        {
            //
            // The locator registry can't be located. We use ordered
            // endpoint selection in case the locator returned a proxy
            // with some endpoints which are preferred to be tried first.
            //
            _locatorRegistry = (Ice.LocatorRegistryPrx)locatorRegistry.ice_locator(null).ice_endpointSelection(
                Ice.EndpointSelectionType.Ordered);
            return _locatorRegistry;
        }
    }

    public void
    getEndpoints(Reference @ref, TimeSpan ttl, GetEndpointsCallback callback)
    {
        getEndpoints(@ref, null, ttl, callback);
    }

    public void
    getEndpoints(Reference @ref, Reference wellKnownRef, TimeSpan ttl, GetEndpointsCallback callback)
    {
        Debug.Assert(@ref.isIndirect());
        EndpointI[] endpoints = null;
        bool cached = false;
        if (!@ref.isWellKnown())
        {
            endpoints = _table.getAdapterEndpoints(@ref.getAdapterId(), ttl, out cached);
            if (!cached)
            {
                if (_background && endpoints != null)
                {
                    getAdapterRequest(@ref).addCallback(@ref, wellKnownRef, ttl, null);
                }
                else
                {
                    getAdapterRequest(@ref).addCallback(@ref, wellKnownRef, ttl, callback);
                    return;
                }
            }
        }
        else
        {
            Reference r = _table.getObjectReference(@ref.getIdentity(), ttl, out cached);
            if (!cached)
            {
                if (_background && r != null)
                {
                    getObjectRequest(@ref).addCallback(@ref, null, ttl, null);
                }
                else
                {
                    getObjectRequest(@ref).addCallback(@ref, null, ttl, callback);
                    return;
                }
            }

            if (!r.isIndirect())
            {
                endpoints = r.getEndpoints();
            }
            else if (!r.isWellKnown())
            {
                if (@ref.getInstance().traceLevels().location >= 1)
                {
                    trace("found adapter for well-known object in locator cache", @ref, r);
                }
                getEndpoints(r, @ref, ttl, callback);
                return;
            }
        }

        Debug.Assert(endpoints != null);
        if (@ref.getInstance().traceLevels().location >= 1)
        {
            getEndpointsTrace(@ref, endpoints, true);
        }
        callback?.setEndpoints(endpoints, true);
    }

    public void clearCache(Reference rf)
    {
        Debug.Assert(rf.isIndirect());
        if (!rf.isWellKnown())
        {
            EndpointI[] endpoints = _table.removeAdapterEndpoints(rf.getAdapterId());

            if (endpoints != null && rf.getInstance().traceLevels().location >= 2)
            {
                trace("removed endpoints for adapter from locator cache", rf, endpoints);
            }
        }
        else
        {
            Reference r = _table.removeObjectReference(rf.getIdentity());
            if (r != null)
            {
                if (!r.isIndirect())
                {
                    if (rf.getInstance().traceLevels().location >= 2)
                    {
                        trace("removed endpoints for well-known object from locator cache", rf, r.getEndpoints());
                    }
                }
                else if (!r.isWellKnown())
                {
                    if (rf.getInstance().traceLevels().location >= 2)
                    {
                        trace("removed adapter for well-known object from locator cache", rf, r);
                    }
                    clearCache(r);
                }
            }
        }
    }

    private void trace(string msg, Reference r, EndpointI[] endpoints)
    {
        System.Text.StringBuilder s = new System.Text.StringBuilder();
        s.Append(msg + "\n");
        if (r.getAdapterId().Length > 0)
        {
            s.Append("adapter = " + r.getAdapterId() + "\n");
        }
        else
        {
            s.Append("well-known proxy = " + r.ToString() + "\n");
        }

        s.Append("endpoints = ");
        int sz = endpoints.Length;
        for (int i = 0; i < sz; i++)
        {
            s.Append(endpoints[i].ToString());
            if (i + 1 < sz)
            {
                s.Append(':');
            }
        }

        r.getInstance().initializationData().logger.trace(r.getInstance().traceLevels().locationCat, s.ToString());
    }

    private void trace(string msg, Reference r, Reference resolved)
    {
        Debug.Assert(r.isWellKnown());

        System.Text.StringBuilder s = new System.Text.StringBuilder();
        s.Append(msg);
        s.Append('\n');
        s.Append("well-known proxy = ");
        s.Append(r.ToString());
        s.Append('\n');
        s.Append("adapter = ");
        s.Append(resolved.getAdapterId());

        r.getInstance().initializationData().logger.trace(r.getInstance().traceLevels().locationCat, s.ToString());
    }

    private void getEndpointsException(Reference @ref, System.Exception exc)
    {
        try
        {
            throw exc;
        }
        catch (Ice.AdapterNotFoundException)
        {
            Instance instance = @ref.getInstance();
            if (instance.traceLevels().location >= 1)
            {
                System.Text.StringBuilder s = new System.Text.StringBuilder();
                s.Append("adapter not found\n");
                s.Append("adapter = " + @ref.getAdapterId());
                instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.ToString());
            }

            throw new NotRegisteredException("object adapter", @ref.getAdapterId());
        }
        catch (Ice.ObjectNotFoundException)
        {
            Instance instance = @ref.getInstance();
            if (instance.traceLevels().location >= 1)
            {
                System.Text.StringBuilder s = new System.Text.StringBuilder();
                s.Append("object not found\n");
                s.Append("object = " + Ice.Util.identityToString(@ref.getIdentity(), instance.toStringMode()));
                instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.ToString());
            }

            throw new NotRegisteredException(
                "object",
                Ice.Util.identityToString(@ref.getIdentity(), instance.toStringMode()));
        }
        catch (Ice.NotRegisteredException)
        {
            throw;
        }
        catch (Ice.LocalException ex)
        {
            Instance instance = @ref.getInstance();
            if (instance.traceLevels().location >= 1)
            {
                System.Text.StringBuilder s = new System.Text.StringBuilder();
                s.Append("couldn't contact the locator to retrieve endpoints\n");
                if (@ref.getAdapterId().Length > 0)
                {
                    s.Append("adapter = " + @ref.getAdapterId() + "\n");
                }
                else
                {
                    s.Append("well-known proxy = " + @ref.ToString() + "\n");
                }
                s.Append("reason = " + ex);
                instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.ToString());
            }
            throw;
        }
        catch (System.Exception)
        {
            Debug.Assert(false);
        }
    }

    private void getEndpointsTrace(Reference @ref, EndpointI[] endpoints, bool cached)
    {
        if (endpoints != null && endpoints.Length > 0)
        {
            if (cached)
            {
                if (@ref.isWellKnown())
                {
                    trace("found endpoints for well-known proxy in locator cache", @ref, endpoints);
                }
                else
                {
                    trace("found endpoints for adapter in locator cache", @ref, endpoints);
                }
            }
            else
            {
                if (@ref.isWellKnown())
                {
                    trace(
                        "retrieved endpoints for well-known proxy from locator, adding to locator cache",
                        @ref,
                        endpoints);
                }
                else
                {
                    trace(
                        "retrieved endpoints for adapter from locator, adding to locator cache",
                        @ref,
                        endpoints);
                }
            }
        }
        else
        {
            Instance instance = @ref.getInstance();
            System.Text.StringBuilder s = new System.Text.StringBuilder();
            s.Append("no endpoints configured for ");
            if (@ref.getAdapterId().Length > 0)
            {
                s.Append("adapter\n");
                s.Append("adapter = " + @ref.getAdapterId());
            }
            else
            {
                s.Append("well-known object\n");
                s.Append("well-known proxy = " + @ref.ToString());
            }
            instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.ToString());
        }
    }

    private Request
    getAdapterRequest(Reference @ref)
    {
        if (@ref.getInstance().traceLevels().location >= 1)
        {
            Instance instance = @ref.getInstance();
            System.Text.StringBuilder s = new System.Text.StringBuilder();
            s.Append("searching for adapter by id\nadapter = ");
            s.Append(@ref.getAdapterId());
            instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.ToString());
        }

        lock (_mutex)
        {
            Request request;
            if (_adapterRequests.TryGetValue(@ref.getAdapterId(), out request))
            {
                return request;
            }

            request = new AdapterRequest(this, @ref);
            _adapterRequests.Add(@ref.getAdapterId(), request);
            return request;
        }
    }

    private Request
    getObjectRequest(Reference @ref)
    {
        if (@ref.getInstance().traceLevels().location >= 1)
        {
            Instance instance = @ref.getInstance();
            System.Text.StringBuilder s = new System.Text.StringBuilder();
            s.Append("searching for well-known object\nwell-known proxy = ");
            s.Append(@ref.ToString());
            instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.ToString());
        }

        lock (_mutex)
        {
            Request request;
            if (_objectRequests.TryGetValue(@ref.getIdentity(), out request))
            {
                return request;
            }

            request = new ObjectRequest(this, @ref);
            _objectRequests.Add(@ref.getIdentity(), request);
            return request;
        }
    }

    private void
    finishRequest(Reference @ref, List<Reference> wellKnownRefs, Ice.ObjectPrx proxy, bool notRegistered)
    {
        Ice.ObjectPrxHelperBase @base = proxy as Ice.ObjectPrxHelperBase;
        if (proxy == null || @base.iceReference().isIndirect())
        {
            //
            // Remove the cached references of well-known objects for which we tried
            // to resolved the endpoints if these endpoints are empty.
            //
            foreach (Reference r in wellKnownRefs)
            {
                _table.removeObjectReference(r.getIdentity());
            }
        }

        if (!@ref.isWellKnown())
        {
            if (proxy != null && !@base.iceReference().isIndirect())
            {
                // Cache the adapter endpoints.
                _table.addAdapterEndpoints(@ref.getAdapterId(), @base.iceReference().getEndpoints());
            }
            else if (notRegistered) // If the adapter isn't registered anymore, remove it from the cache.
            {
                _table.removeAdapterEndpoints(@ref.getAdapterId());
            }

            lock (_mutex)
            {
                Debug.Assert(_adapterRequests.ContainsKey(@ref.getAdapterId()));
                _adapterRequests.Remove(@ref.getAdapterId());
            }
        }
        else
        {
            if (proxy != null && !@base.iceReference().isWellKnown())
            {
                // Cache the well-known object reference.
                _table.addObjectReference(@ref.getIdentity(), @base.iceReference());
            }
            else if (notRegistered) // If the well-known object isn't registered anymore, remove it from the cache.
            {
                _table.removeObjectReference(@ref.getIdentity());
            }

            lock (_mutex)
            {
                Debug.Assert(_objectRequests.ContainsKey(@ref.getIdentity()));
                _objectRequests.Remove(@ref.getIdentity());
            }
        }
    }

    private readonly Ice.LocatorPrx _locator;
    private Ice.LocatorRegistryPrx _locatorRegistry;
    private readonly LocatorTable _table;
    private readonly bool _background;

    private readonly Dictionary<string, Request> _adapterRequests = new Dictionary<string, Request>();
    private readonly Dictionary<Ice.Identity, Request> _objectRequests = new Dictionary<Ice.Identity, Request>();
    private readonly object _mutex = new();
}

public sealed class LocatorManager
{
    private struct LocatorKey
    {
        public LocatorKey(Ice.LocatorPrx prx)
        {
            Reference r = ((Ice.ObjectPrxHelperBase)prx).iceReference();
            _id = r.getIdentity();
            _encoding = r.getEncoding();
        }

        public override bool Equals(object o)
        {
            LocatorKey k = (LocatorKey)o;
            if (!k._id.Equals(_id))
            {
                return false;
            }
            if (!k._encoding.Equals(_encoding))
            {
                return false;
            }
            return true;
        }

        public override int GetHashCode() => HashCode.Combine(_id, _encoding);

        private Ice.Identity _id;
        private Ice.EncodingVersion _encoding;
    }

    internal LocatorManager(Ice.Properties properties)
    {
        _table = new Dictionary<Ice.LocatorPrx, LocatorInfo>();
        _locatorTables = new Dictionary<LocatorKey, LocatorTable>();
        _background = properties.getIcePropertyAsInt("Ice.BackgroundLocatorCacheUpdates") > 0;
    }

    internal void destroy()
    {
        lock (_mutex)
        {
            foreach (LocatorInfo info in _table.Values)
            {
                info.destroy();
            }
            _table.Clear();
            _locatorTables.Clear();
        }
    }

    //
    // Returns locator info for a given locator. Automatically creates
    // the locator info if it doesn't exist yet.
    //
    public LocatorInfo get(Ice.LocatorPrx loc)
    {
        if (loc == null)
        {
            return null;
        }

        //
        // The locator can't be located.
        //
        Ice.LocatorPrx locator = Ice.LocatorPrxHelper.uncheckedCast(loc.ice_locator(null));

        //
        // TODO: reap unused locator info objects?
        //
        lock (_mutex)
        {
            LocatorInfo info = null;
            if (!_table.TryGetValue(locator, out info))
            {
                //
                // Rely on locator identity for the adapter table. We want to
                // have only one table per locator (not one per locator
                // proxy).
                //
                LocatorTable table = null;
                LocatorKey key = new LocatorKey(locator);
                if (!_locatorTables.TryGetValue(key, out table))
                {
                    table = new LocatorTable();
                    _locatorTables[key] = table;
                }

                info = new LocatorInfo(locator, table, _background);
                _table[locator] = info;
            }

            return info;
        }
    }

    private readonly Dictionary<Ice.LocatorPrx, LocatorInfo> _table;
    private readonly Dictionary<LocatorKey, LocatorTable> _locatorTables;
    private readonly bool _background;
    private readonly object _mutex = new();
}

internal sealed class LocatorTable
{
    internal LocatorTable()
    {
        _adapterEndpointsTable = new Dictionary<string, EndpointTableEntry>();
        _objectTable = new Dictionary<Ice.Identity, ReferenceTableEntry>();
    }

    internal void clear()
    {
        lock (_mutex)
        {
            _adapterEndpointsTable.Clear();
            _objectTable.Clear();
        }
    }

    internal EndpointI[] getAdapterEndpoints(string adapter, TimeSpan ttl, out bool cached)
    {
        if (ttl == TimeSpan.Zero) // Locator cache disabled.
        {
            cached = false;
            return null;
        }

        lock (_mutex)
        {
            EndpointTableEntry entry = null;
            if (_adapterEndpointsTable.TryGetValue(adapter, out entry))
            {
                cached = checkTTL(entry.time, ttl);
                return entry.endpoints;
            }
            cached = false;
            return null;
        }
    }

    internal void addAdapterEndpoints(string adapter, EndpointI[] endpoints)
    {
        lock (_mutex)
        {
            _adapterEndpointsTable[adapter] =
                new EndpointTableEntry(Time.currentMonotonicTimeMillis(), endpoints);
        }
    }

    internal EndpointI[] removeAdapterEndpoints(string adapter)
    {
        lock (_mutex)
        {
            EndpointTableEntry entry = null;
            if (_adapterEndpointsTable.TryGetValue(adapter, out entry))
            {
                _adapterEndpointsTable.Remove(adapter);
                return entry.endpoints;
            }
            return null;
        }
    }

    internal Reference getObjectReference(Ice.Identity id, TimeSpan ttl, out bool cached)
    {
        if (ttl == TimeSpan.Zero) // Locator cache disabled.
        {
            cached = false;
            return null;
        }

        lock (_mutex)
        {
            ReferenceTableEntry entry = null;
            if (_objectTable.TryGetValue(id, out entry))
            {
                cached = checkTTL(entry.time, ttl);
                return entry.reference;
            }
            cached = false;
            return null;
        }
    }

    internal void addObjectReference(Ice.Identity id, Reference reference)
    {
        lock (_mutex)
        {
            _objectTable[id] = new ReferenceTableEntry(Time.currentMonotonicTimeMillis(), reference);
        }
    }

    internal Reference removeObjectReference(Ice.Identity id)
    {
        lock (_mutex)
        {
            ReferenceTableEntry entry = null;
            if (_objectTable.TryGetValue(id, out entry))
            {
                _objectTable.Remove(id);
                return entry.reference;
            }
            return null;
        }
    }

    private bool checkTTL(long time, TimeSpan ttl)
    {
        Debug.Assert(ttl != TimeSpan.Zero);
        if (ttl < TimeSpan.Zero) // TTL = infinite
        {
            return true;
        }
        else
        {
            return Time.currentMonotonicTimeMillis() - time <= ttl.TotalMilliseconds;
        }
    }

    private sealed class EndpointTableEntry
    {
        public EndpointTableEntry(long time, EndpointI[] endpoints)
        {
            this.time = time;
            this.endpoints = endpoints;
        }

        public long time;
        public EndpointI[] endpoints;
    }

    private sealed class ReferenceTableEntry
    {
        public ReferenceTableEntry(long time, Reference reference)
        {
            this.time = time;
            this.reference = reference;
        }

        public long time;
        public Reference reference;
    }

    private readonly Dictionary<string, EndpointTableEntry> _adapterEndpointsTable;
    private readonly Dictionary<Ice.Identity, ReferenceTableEntry> _objectTable;
    private readonly object _mutex = new();
}
