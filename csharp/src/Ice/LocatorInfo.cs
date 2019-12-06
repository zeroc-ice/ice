//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using Ice;

namespace IceInternal
{
    public sealed class LocatorInfo
    {
        public interface GetEndpointsCallback
        {
            void setEndpoints(EndpointI[] endpoints, bool cached);
            void setException(Ice.LocalException ex);
        }

        private class RequestCallback
        {
            public void
            response(LocatorInfo locatorInfo, Ice.IObjectPrx proxy)
            {
                EndpointI[] endpoints = null;
                if (proxy != null)
                {
                    Reference r = proxy.IceReference;
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
                        if (_ref.getCommunicator().traceLevels().location >= 1)
                        {
                            locatorInfo.trace("retrieved adapter for well-known object from locator, " +
                                              "adding to locator cache", _ref, r);
                        }
                        locatorInfo.getEndpoints(r, _ref, _ttl, _callback);
                        return;
                    }
                }

                if (_ref.getCommunicator().traceLevels().location >= 1)
                {
                    locatorInfo.getEndpointsTrace(_ref, endpoints, false);
                }
                if (_callback != null)
                {
                    _callback.setEndpoints(endpoints == null ? System.Array.Empty<EndpointI>() : endpoints, false);
                }
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
                    if (_callback != null)
                    {
                        _callback.setException(ex);
                    }
                }
            }

            public
            RequestCallback(Reference reference, int ttl, GetEndpointsCallback cb)
            {
                _ref = reference;
                _ttl = ttl;
                _callback = cb;
            }

            private readonly Reference _ref;
            private readonly int _ttl;
            private readonly GetEndpointsCallback _callback;
        }

        private abstract class Request
        {
            public void
            addCallback(Reference reference, Reference wellKnownRef, int ttl, GetEndpointsCallback cb)
            {
                RequestCallback callback = new RequestCallback(reference, ttl, cb);
                lock (this)
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

            public Request(LocatorInfo locatorInfo, Reference reference)
            {
                _locatorInfo = locatorInfo;
                _ref = reference;
                _sent = false;
                _response = false;
            }

            public void
            response(Ice.IObjectPrx proxy)
            {
                lock (this)
                {
                    _locatorInfo.finishRequest(_ref, _wellKnownRefs, proxy, false);
                    _response = true;
                    _proxy = proxy;
                    Monitor.PulseAll(this);
                }
                foreach (RequestCallback callback in _callbacks)
                {
                    callback.response(_locatorInfo, proxy);
                }
            }

            public void
            exception(Ice.Exception ex)
            {
                lock (this)
                {
                    _locatorInfo.finishRequest(_ref, _wellKnownRefs, null, ex is Ice.UserException);
                    _exception = ex;
                    Monitor.PulseAll(this);
                }
                foreach (RequestCallback callback in _callbacks)
                {
                    callback.exception(_locatorInfo, ex);
                }
            }

            protected abstract void send();

            protected readonly LocatorInfo _locatorInfo;
            protected readonly Reference _ref;

            private List<RequestCallback> _callbacks = new List<RequestCallback>();
            private List<Reference> _wellKnownRefs = new List<Reference>();
            private bool _sent;
            private bool _response;
            private Ice.IObjectPrx _proxy;
            private Ice.Exception _exception;
        }

        private class ObjectRequest : Request
        {
            public ObjectRequest(LocatorInfo locatorInfo, Reference reference) : base(locatorInfo, reference)
            {
            }

            protected override void
            send()
            {
                try
                {
                    _locatorInfo.getLocator().findObjectByIdAsync(_ref.getIdentity()).ContinueWith(
                        (Task<Ice.IObjectPrx> p) =>
                        {
                            try
                            {
                                response(p.Result);
                            }
                            catch (System.AggregateException ex)
                            {
                                exception(ex.InnerException as Ice.Exception);
                            }
                        });
                }
                catch (Ice.Exception ex)
                {
                    exception(ex);
                }
            }
        }

        private class AdapterRequest : Request
        {
            public AdapterRequest(LocatorInfo locatorInfo, Reference reference) : base(locatorInfo, reference)
            {
            }

            protected override void
            send()
            {
                try
                {
                    _locatorInfo.getLocator().findAdapterByIdAsync(_ref.getAdapterId()).ContinueWith(
                        (Task<Ice.IObjectPrx> p) =>
                        {
                            try
                            {
                                response(p.Result);
                            }
                            catch (System.AggregateException ex)
                            {
                                exception(ex.InnerException as Ice.Exception);
                            }
                        });
                }
                catch (Ice.Exception ex)
                {
                    exception(ex);
                }
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
            lock (this)
            {
                _locatorRegistry = null;
                _table.clear();
            }
        }

        public override bool Equals(object obj)
        {
            if (ReferenceEquals(this, obj))
            {
                return true;
            }

            LocatorInfo rhs = obj as LocatorInfo;
            return rhs == null ? false : _locator.Equals(rhs._locator);
        }

        public override int GetHashCode()
        {
            return _locator.GetHashCode();
        }

        public Ice.LocatorPrx getLocator()
        {
            //
            // No synchronization necessary, _locator is immutable.
            //
            return _locator;
        }

        public Ice.LocatorRegistryPrx getLocatorRegistry()
        {
            lock (this)
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

            lock (this)
            {
                //
                // The locator registry can't be located. We use ordered
                // endpoint selection in case the locator returned a proxy
                // with some endpoints which are prefered to be tried first.
                //
                _locatorRegistry = locatorRegistry.Clone(clearLocator: true, endpointSelectionType: EndpointSelectionType.Ordered);
                return _locatorRegistry;
            }
        }

        public void
        getEndpoints(Reference reference, int ttl, GetEndpointsCallback callback)
        {
            getEndpoints(reference, null, ttl, callback);
        }

        public void
        getEndpoints(Reference reference, Reference wellKnownRef, int ttl, GetEndpointsCallback callback)
        {
            Debug.Assert(reference.isIndirect());
            EndpointI[] endpoints = null;
            bool cached = false;
            if (!reference.isWellKnown())
            {
                endpoints = _table.getAdapterEndpoints(reference.getAdapterId(), ttl, out cached);
                if (!cached)
                {
                    if (_background && endpoints != null)
                    {
                        getAdapterRequest(reference).addCallback(reference, wellKnownRef, ttl, null);
                    }
                    else
                    {
                        getAdapterRequest(reference).addCallback(reference, wellKnownRef, ttl, callback);
                        return;
                    }
                }
            }
            else
            {
                Reference r = _table.getObjectReference(reference.getIdentity(), ttl, out cached);
                if (!cached)
                {
                    if (_background && r != null)
                    {
                        getObjectRequest(reference).addCallback(reference, null, ttl, null);
                    }
                    else
                    {
                        getObjectRequest(reference).addCallback(reference, null, ttl, callback);
                        return;
                    }
                }

                if (!r.isIndirect())
                {
                    endpoints = r.getEndpoints();
                }
                else if (!r.isWellKnown())
                {
                    if (reference.getCommunicator().traceLevels().location >= 1)
                    {
                        trace("found adapter for well-known object in locator cache", reference, r);
                    }
                    getEndpoints(r, reference, ttl, callback);
                    return;
                }
            }

            Debug.Assert(endpoints != null);
            if (reference.getCommunicator().traceLevels().location >= 1)
            {
                getEndpointsTrace(reference, endpoints, true);
            }
            if (callback != null)
            {
                callback.setEndpoints(endpoints, true);
            }
        }

        public void clearCache(Reference rf)
        {
            Debug.Assert(rf.isIndirect());
            if (!rf.isWellKnown())
            {
                EndpointI[] endpoints = _table.removeAdapterEndpoints(rf.getAdapterId());

                if (endpoints != null && rf.getCommunicator().traceLevels().location >= 2)
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
                        if (rf.getCommunicator().traceLevels().location >= 2)
                        {
                            trace("removed endpoints for well-known object from locator cache", rf, r.getEndpoints());
                        }
                    }
                    else if (!r.isWellKnown())
                    {
                        if (rf.getCommunicator().traceLevels().location >= 2)
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
                    s.Append(":");
                }
            }

            r.getCommunicator().initializationData().logger.trace(r.getCommunicator().traceLevels().locationCat, s.ToString());
        }

        private void trace(string msg, Reference r, Reference resolved)
        {
            Debug.Assert(r.isWellKnown());

            System.Text.StringBuilder s = new System.Text.StringBuilder();
            s.Append(msg);
            s.Append("\n");
            s.Append("well-known proxy = ");
            s.Append(r.ToString());
            s.Append("\n");
            s.Append("adapter = ");
            s.Append(resolved.getAdapterId());

            r.getCommunicator().initializationData().logger.trace(r.getCommunicator().traceLevels().locationCat, s.ToString());
        }

        private void getEndpointsException(Reference reference, System.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch (Ice.AdapterNotFoundException ex)
            {
                var communicator = reference.getCommunicator();
                if (communicator.traceLevels().location >= 1)
                {
                    System.Text.StringBuilder s = new System.Text.StringBuilder();
                    s.Append("adapter not found\n");
                    s.Append("adapter = " + reference.getAdapterId());
                    communicator.initializationData().logger.trace(communicator.traceLevels().locationCat, s.ToString());
                }

                Ice.NotRegisteredException e = new Ice.NotRegisteredException(ex);
                e.kindOfObject = "object adapter";
                e.id = reference.getAdapterId();
                throw e;
            }
            catch (Ice.ObjectNotFoundException ex)
            {
                var communicator = reference.getCommunicator();
                if (communicator.traceLevels().location >= 1)
                {
                    System.Text.StringBuilder s = new System.Text.StringBuilder();
                    s.Append("object not found\n");
                    s.Append("object = " + Ice.Util.identityToString(reference.getIdentity(), communicator.toStringMode()));
                    communicator.initializationData().logger.trace(communicator.traceLevels().locationCat, s.ToString());
                }

                Ice.NotRegisteredException e = new Ice.NotRegisteredException(ex);
                e.kindOfObject = "object";
                e.id = Ice.Util.identityToString(reference.getIdentity(), communicator.toStringMode());
                throw e;
            }
            catch (Ice.NotRegisteredException)
            {
                throw;
            }
            catch (Ice.LocalException ex)
            {
                var communicator = reference.getCommunicator();
                if (communicator.traceLevels().location >= 1)
                {
                    System.Text.StringBuilder s = new System.Text.StringBuilder();
                    s.Append("couldn't contact the locator to retrieve endpoints\n");
                    if (reference.getAdapterId().Length > 0)
                    {
                        s.Append("adapter = " + reference.getAdapterId() + "\n");
                    }
                    else
                    {
                        s.Append("well-known proxy = " + reference.ToString() + "\n");
                    }
                    s.Append("reason = " + ex);
                    communicator.initializationData().logger.trace(communicator.traceLevels().locationCat, s.ToString());
                }
                throw;
            }
            catch (System.Exception)
            {
                Debug.Assert(false);
            }
        }

        private void getEndpointsTrace(Reference reference, EndpointI[] endpoints, bool cached)
        {
            if (endpoints != null && endpoints.Length > 0)
            {
                if (cached)
                {
                    if (reference.isWellKnown())
                    {
                        trace("found endpoints for well-known proxy in locator cache", reference, endpoints);
                    }
                    else
                    {
                        trace("found endpoints for adapter in locator cache", reference, endpoints);
                    }
                }
                else
                {
                    if (reference.isWellKnown())
                    {
                        trace("retrieved endpoints for well-known proxy from locator, adding to locator cache",
                              reference, endpoints);
                    }
                    else
                    {
                        trace("retrieved endpoints for adapter from locator, adding to locator cache",
                              reference, endpoints);
                    }
                }
            }
            else
            {
                var communicator = reference.getCommunicator();
                System.Text.StringBuilder s = new System.Text.StringBuilder();
                s.Append("no endpoints configured for ");
                if (reference.getAdapterId().Length > 0)
                {
                    s.Append("adapter\n");
                    s.Append("adapter = " + reference.getAdapterId());
                }
                else
                {
                    s.Append("well-known object\n");
                    s.Append("well-known proxy = " + reference.ToString());
                }
                communicator.initializationData().logger.trace(communicator.traceLevels().locationCat, s.ToString());
            }
        }

        private Request
        getAdapterRequest(Reference reference)
        {
            if (reference.getCommunicator().traceLevels().location >= 1)
            {
                var communicator = reference.getCommunicator();
                System.Text.StringBuilder s = new System.Text.StringBuilder();
                s.Append("searching for adapter by id\nadapter = ");
                s.Append(reference.getAdapterId());
                communicator.initializationData().logger.trace(communicator.traceLevels().locationCat, s.ToString());
            }

            lock (this)
            {
                Request request;
                if (_adapterRequests.TryGetValue(reference.getAdapterId(), out request))
                {
                    return request;
                }

                request = new AdapterRequest(this, reference);
                _adapterRequests.Add(reference.getAdapterId(), request);
                return request;
            }
        }

        private Request
        getObjectRequest(Reference reference)
        {
            if (reference.getCommunicator().traceLevels().location >= 1)
            {
                var communicator = reference.getCommunicator();
                System.Text.StringBuilder s = new System.Text.StringBuilder();
                s.Append("searching for well-known object\nwell-known proxy = ");
                s.Append(reference.ToString());
                communicator.initializationData().logger.trace(communicator.traceLevels().locationCat, s.ToString());
            }

            lock (this)
            {
                Request request;
                if (_objectRequests.TryGetValue(reference.getIdentity(), out request))
                {
                    return request;
                }

                request = new ObjectRequest(this, reference);
                _objectRequests.Add(reference.getIdentity(), request);
                return request;
            }
        }

        private void
        finishRequest(Reference reference, List<Reference> wellKnownRefs, Ice.IObjectPrx proxy, bool notRegistered)
        {
            if (proxy == null || proxy.IceReference.isIndirect())
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

            if (!reference.isWellKnown())
            {
                if (proxy != null && !proxy.IceReference.isIndirect())
                {
                    // Cache the adapter endpoints.
                    _table.addAdapterEndpoints(reference.getAdapterId(), proxy.IceReference.getEndpoints());
                }
                else if (notRegistered) // If the adapter isn't registered anymore, remove it from the cache.
                {
                    _table.removeAdapterEndpoints(reference.getAdapterId());
                }

                lock (this)
                {
                    Debug.Assert(_adapterRequests.ContainsKey(reference.getAdapterId()));
                    _adapterRequests.Remove(reference.getAdapterId());
                }
            }
            else
            {
                if (proxy != null && !proxy.IceReference.isWellKnown())
                {
                    // Cache the well-known object reference.
                    _table.addObjectReference(reference.getIdentity(), proxy.IceReference);
                }
                else if (notRegistered) // If the well-known object isn't registered anymore, remove it from the cache.
                {
                    _table.removeObjectReference(reference.getIdentity());
                }

                lock (this)
                {
                    Debug.Assert(_objectRequests.ContainsKey(reference.getIdentity()));
                    _objectRequests.Remove(reference.getIdentity());
                }
            }
        }

        private readonly Ice.LocatorPrx _locator;
        private Ice.LocatorRegistryPrx _locatorRegistry;
        private readonly LocatorTable _table;
        private readonly bool _background;

        private Dictionary<string, Request> _adapterRequests = new Dictionary<string, Request>();
        private Dictionary<Ice.Identity, Request> _objectRequests = new Dictionary<Ice.Identity, Request>();
    }

    public sealed class LocatorManager
    {
        private struct LocatorKey
        {
            public LocatorKey(Ice.LocatorPrx prx)
            {
                Reference r = prx.IceReference;
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

            public override int GetHashCode()
            {
                int h = 5381;
                HashUtil.hashAdd(ref h, _id);
                HashUtil.hashAdd(ref h, _encoding);
                return h;
            }

            private Ice.Identity _id;
            private Ice.EncodingVersion _encoding;
        }

        internal LocatorManager(Ice.Properties properties)
        {
            _table = new Dictionary<Ice.LocatorPrx, LocatorInfo>();
            _locatorTables = new Dictionary<LocatorKey, LocatorTable>();
            _background = properties.getPropertyAsInt("Ice.BackgroundLocatorCacheUpdates") > 0;
        }

        internal void destroy()
        {
            lock (this)
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
        public LocatorInfo get(LocatorPrx loc)
        {
            if (loc == null)
            {
                throw new System.ArgumentNullException(nameof(loc));
            }

            //
            // The locator can't be located.
            //
            LocatorPrx locator = loc.Clone(clearLocator: true);

            //
            // TODO: reap unused locator info objects?
            //
            lock (this)
            {
                LocatorInfo info;
                if (!_table.TryGetValue(locator, out info))
                {
                    //
                    // Rely on locator identity for the adapter table. We want to
                    // have only one table per locator (not one per locator
                    // proxy).
                    //
                    LocatorTable? table = null;
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

        private Dictionary<Ice.LocatorPrx, LocatorInfo> _table;
        private Dictionary<LocatorKey, LocatorTable> _locatorTables;
        private readonly bool _background;
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
            lock (this)
            {
                _adapterEndpointsTable.Clear();
                _objectTable.Clear();
            }
        }

        internal EndpointI[] getAdapterEndpoints(string adapter, int ttl, out bool cached)
        {
            if (ttl == 0) // Locator cache disabled.
            {
                cached = false;
                return null;
            }

            lock (this)
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
            lock (this)
            {
                _adapterEndpointsTable[adapter] =
                    new EndpointTableEntry(Time.currentMonotonicTimeMillis(), endpoints);
            }
        }

        internal EndpointI[] removeAdapterEndpoints(string adapter)
        {
            lock (this)
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

        internal Reference getObjectReference(Ice.Identity id, int ttl, out bool cached)
        {
            if (ttl == 0) // Locator cache disabled.
            {
                cached = false;
                return null;
            }

            lock (this)
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
            lock (this)
            {
                _objectTable[id] = new ReferenceTableEntry(Time.currentMonotonicTimeMillis(), reference);
            }
        }

        internal Reference removeObjectReference(Ice.Identity id)
        {
            lock (this)
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

        private bool checkTTL(long time, int ttl)
        {
            Debug.Assert(ttl != 0);
            if (ttl < 0) // TTL = infinite
            {
                return true;
            }
            else
            {
                return Time.currentMonotonicTimeMillis() - time <= ((long)ttl * 1000);
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

        private Dictionary<string, EndpointTableEntry> _adapterEndpointsTable;
        private Dictionary<Ice.Identity, ReferenceTableEntry> _objectTable;
    }

}
