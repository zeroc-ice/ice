//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using IceInternal;

namespace Ice
{
    public sealed class LocatorInfo
    {
        public interface IGetEndpointsCallback
        {
            void SetEndpoints(Endpoint[] endpoints, bool cached);
            void SetException(LocalException ex);
        }

        private class RequestCallback
        {
            private readonly Reference _ref;
            private readonly int _ttl;
            private readonly IGetEndpointsCallback? _callback;

            public RequestCallback(Reference reference, int ttl, IGetEndpointsCallback? cb)
            {
                _ref = reference;
                _ttl = ttl;
                _callback = cb;
            }

            public void Response(LocatorInfo locatorInfo, IObjectPrx? proxy)
            {
                Endpoint[]? endpoints = null;
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
                        if (_ref.getCommunicator().TraceLevels.location >= 1)
                        {
                            locatorInfo.Trace("retrieved adapter for well-known object from locator, " +
                                              "adding to locator cache", _ref, r);
                        }
                        locatorInfo.GetEndpoints(r, _ref, _ttl, _callback);
                        return;
                    }
                }

                if (_ref.getCommunicator().TraceLevels.location >= 1)
                {
                    locatorInfo.GetEndpointsTrace(_ref, endpoints, false);
                }
                if (_callback != null)
                {
                    _callback.SetEndpoints(endpoints ?? Array.Empty<Endpoint>(), false);
                }
            }

            public void Exception(LocatorInfo locatorInfo, Exception exc)
            {
                try
                {
                    locatorInfo.GetEndpointsException(_ref, exc); // This throws.
                }
                catch (LocalException ex)
                {
                    if (_callback != null)
                    {
                        _callback.SetException(ex);
                    }
                }
            }
        }

        private abstract class Request
        {
            protected readonly LocatorInfo _locatorInfo;
            protected readonly Reference _ref;

            private readonly List<RequestCallback> _callbacks = new List<RequestCallback>();
            private Exception? _exception;
            private IObjectPrx? _proxy;
            private bool _response;
            private bool _sent;
            private readonly List<Reference> _wellKnownRefs = new List<Reference>();

            internal Request(LocatorInfo locatorInfo, Reference reference)
            {
                _locatorInfo = locatorInfo;
                _ref = reference;
                _sent = false;
                _response = false;
            }

            internal void AddCallback(Reference reference, Reference? wellKnownRef, int ttl, IGetEndpointsCallback? cb)
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
                            Send();
                        }
                        return;
                    }
                }

                if (_response)
                {
                    callback.Response(_locatorInfo, _proxy);
                }
                else
                {
                    Debug.Assert(_exception != null);
                    callback.Exception(_locatorInfo, _exception);
                }
            }

            internal void Response(IObjectPrx proxy)
            {
                lock (this)
                {
                    _locatorInfo.FinishRequest(_ref, _wellKnownRefs, proxy, false);
                    _response = true;
                    _proxy = proxy;
                    Monitor.PulseAll(this);
                }
                foreach (RequestCallback callback in _callbacks)
                {
                    callback.Response(_locatorInfo, proxy);
                }
            }

            internal void Exception(Exception ex)
            {
                lock (this)
                {
                    _locatorInfo.FinishRequest(_ref, _wellKnownRefs, null, ex is UserException);
                    _exception = ex;
                    Monitor.PulseAll(this);
                }
                foreach (RequestCallback callback in _callbacks)
                {
                    callback.Exception(_locatorInfo, ex);
                }
            }

            protected internal abstract void Send();
        }

        private class ObjectRequest : Request
        {
            internal ObjectRequest(LocatorInfo locatorInfo, Reference reference) : base(locatorInfo, reference)
            {
            }

            protected internal override void
            Send()
            {
                try
                {
                    _locatorInfo.Locator.FindObjectByIdAsync(_ref.getIdentity()).ContinueWith(
                        (Task<IObjectPrx> p) =>
                        {
                            try
                            {
                                Response(p.Result);
                            }
                            catch (AggregateException ex)
                            {
                                Debug.Assert(ex.InnerException is Ice.Exception);
                                Exception((Ice.Exception)ex.InnerException);
                            }
                        });
                }
                catch (Ice.Exception ex)
                {
                    Exception(ex);
                }
            }
        }

        private class AdapterRequest : Request
        {
            internal AdapterRequest(LocatorInfo locatorInfo, Reference reference) : base(locatorInfo, reference)
            {
            }

            protected internal override void
            Send()
            {
                try
                {
                    _locatorInfo.Locator.FindAdapterByIdAsync(_ref.getAdapterId()).ContinueWith(
                        (Task<IObjectPrx> p) =>
                        {
                            try
                            {
                                Response(p.Result);
                            }
                            catch (AggregateException ex)
                            {
                                Debug.Assert(ex.InnerException is Ice.Exception);
                                Exception((Ice.Exception)ex.InnerException);
                            }
                        });
                }
                catch (Ice.Exception ex)
                {
                    Exception(ex);
                }
            }
        }

        internal LocatorInfo(ILocatorPrx locator, LocatorTable table, bool background)
        {
            Locator = locator;
            _table = table;
            _background = background;
        }

        public void Destroy()
        {
            lock (this)
            {
                _locatorRegistry = null;
                _table.Clear();
            }
        }

        public override bool Equals(object? obj)
        {
            if (ReferenceEquals(this, obj))
            {
                return true;
            }

            return !(obj is LocatorInfo rhs) ? false : Locator.Equals(rhs.Locator);
        }

        public override int GetHashCode() => Locator.GetHashCode();

        // No synchronization necessary, _locator is immutable.
        public ILocatorPrx Locator { get; set; }

        public ILocatorRegistryPrx? GetLocatorRegistry()
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
            ILocatorRegistryPrx locatorRegistry = Locator.GetRegistry();
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

        public void GetEndpoints(Reference reference, int ttl, IGetEndpointsCallback callback) =>
            GetEndpoints(reference, null, ttl, callback);

        public void
        GetEndpoints(Reference reference, Reference? wellKnownRef, int ttl, IGetEndpointsCallback? callback)
        {
            Debug.Assert(reference.isIndirect());
            Endpoint[]? endpoints = null;
            bool cached;
            if (!reference.isWellKnown())
            {
                endpoints = _table.GetAdapterEndpoints(reference.getAdapterId(), ttl, out cached);
                if (!cached)
                {
                    if (_background && endpoints != null)
                    {
                        GetAdapterRequest(reference).AddCallback(reference, wellKnownRef, ttl, null);
                    }
                    else
                    {
                        GetAdapterRequest(reference).AddCallback(reference, wellKnownRef, ttl, callback);
                        return;
                    }
                }
            }
            else
            {
                Reference? r = _table.GetObjectReference(reference.getIdentity(), ttl, out cached);
                if (!cached)
                {
                    if (_background && r != null)
                    {
                        GetObjectRequest(reference).AddCallback(reference, null, ttl, null);
                    }
                    else
                    {
                        GetObjectRequest(reference).AddCallback(reference, null, ttl, callback);
                        return;
                    }
                }

                Debug.Assert(r != null);
                if (!r.isIndirect())
                {
                    endpoints = r.getEndpoints();
                }
                else if (!r.isWellKnown())
                {
                    if (reference.getCommunicator().TraceLevels.location >= 1)
                    {
                        Trace("found adapter for well-known object in locator cache", reference, r);
                    }
                    GetEndpoints(r, reference, ttl, callback);
                    return;
                }
            }

            Debug.Assert(endpoints != null);
            if (reference.getCommunicator().TraceLevels.location >= 1)
            {
                GetEndpointsTrace(reference, endpoints, true);
            }
            if (callback != null)
            {
                callback.SetEndpoints(endpoints, true);
            }
        }

        public void ClearCache(Reference rf)
        {
            Debug.Assert(rf.isIndirect());
            if (!rf.isWellKnown())
            {
                Endpoint[]? endpoints = _table.RemoveAdapterEndpoints(rf.getAdapterId());

                if (endpoints != null && rf.getCommunicator().TraceLevels.location >= 2)
                {
                    Trace("removed endpoints for adapter from locator cache", rf, endpoints);
                }
            }
            else
            {
                Reference? r = _table.RemoveObjectReference(rf.getIdentity());
                if (r != null)
                {
                    if (!r.isIndirect())
                    {
                        if (rf.getCommunicator().TraceLevels.location >= 2)
                        {
                            Trace("removed endpoints for well-known object from locator cache", rf, r.getEndpoints());
                        }
                    }
                    else if (!r.isWellKnown())
                    {
                        if (rf.getCommunicator().TraceLevels.location >= 2)
                        {
                            Trace("removed adapter for well-known object from locator cache", rf, r);
                        }
                        ClearCache(r);
                    }
                }
            }
        }

        private void Trace(string msg, Reference r, Endpoint[] endpoints)
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

            r.getCommunicator().Logger.trace(r.getCommunicator().TraceLevels.locationCat, s.ToString());
        }

        private void Trace(string msg, Reference r, Reference resolved)
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

            r.getCommunicator().Logger.trace(r.getCommunicator().TraceLevels.locationCat, s.ToString());
        }

        private void GetEndpointsException(Reference reference, System.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch (AdapterNotFoundException ex)
            {
                Communicator communicator = reference.getCommunicator();
                if (communicator.TraceLevels.location >= 1)
                {
                    System.Text.StringBuilder s = new System.Text.StringBuilder();
                    s.Append("adapter not found\n");
                    s.Append("adapter = " + reference.getAdapterId());
                    communicator.Logger.trace(communicator.TraceLevels.locationCat, s.ToString());
                }

                throw new NotRegisteredException(ex)
                {
                    kindOfObject = "object adapter",
                    id = reference.getAdapterId()
                };
            }
            catch (ObjectNotFoundException ex)
            {
                Communicator communicator = reference.getCommunicator();
                if (communicator.TraceLevels.location >= 1)
                {
                    System.Text.StringBuilder s = new System.Text.StringBuilder();
                    s.Append("object not found\n");
                    s.Append("object = " + reference.getIdentity().ToString(communicator.ToStringMode));
                    communicator.Logger.trace(communicator.TraceLevels.locationCat, s.ToString());
                }

                throw new NotRegisteredException(ex)
                {
                    kindOfObject = "object",
                    id = reference.getIdentity().ToString(communicator.ToStringMode)
                };
            }
            catch (NotRegisteredException)
            {
                throw;
            }
            catch (LocalException ex)
            {
                Communicator communicator = reference.getCommunicator();
                if (communicator.TraceLevels.location >= 1)
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
                    communicator.Logger.trace(communicator.TraceLevels.locationCat, s.ToString());
                }
                throw;
            }
            catch (System.Exception)
            {
                Debug.Assert(false);
            }
        }

        private void GetEndpointsTrace(Reference reference, Endpoint[]? endpoints, bool cached)
        {
            if (endpoints != null && endpoints.Length > 0)
            {
                if (cached)
                {
                    if (reference.isWellKnown())
                    {
                        Trace("found endpoints for well-known proxy in locator cache", reference, endpoints);
                    }
                    else
                    {
                        Trace("found endpoints for adapter in locator cache", reference, endpoints);
                    }
                }
                else
                {
                    if (reference.isWellKnown())
                    {
                        Trace("retrieved endpoints for well-known proxy from locator, adding to locator cache",
                              reference, endpoints);
                    }
                    else
                    {
                        Trace("retrieved endpoints for adapter from locator, adding to locator cache",
                              reference, endpoints);
                    }
                }
            }
            else
            {
                Communicator communicator = reference.getCommunicator();
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
                communicator.Logger.trace(communicator.TraceLevels.locationCat, s.ToString());
            }
        }

        private Request GetAdapterRequest(Reference reference)
        {
            if (reference.getCommunicator().TraceLevels.location >= 1)
            {
                Communicator communicator = reference.getCommunicator();
                System.Text.StringBuilder s = new System.Text.StringBuilder();
                s.Append("searching for adapter by id\nadapter = ");
                s.Append(reference.getAdapterId());
                communicator.Logger.trace(communicator.TraceLevels.locationCat, s.ToString());
            }

            lock (this)
            {
                if (_adapterRequests.TryGetValue(reference.getAdapterId(), out Request request))
                {
                    return request;
                }

                request = new AdapterRequest(this, reference);
                _adapterRequests.Add(reference.getAdapterId(), request);
                return request;
            }
        }

        private Request GetObjectRequest(Reference reference)
        {
            if (reference.getCommunicator().TraceLevels.location >= 1)
            {
                Communicator communicator = reference.getCommunicator();
                System.Text.StringBuilder s = new System.Text.StringBuilder();
                s.Append("searching for well-known object\nwell-known proxy = ");
                s.Append(reference.ToString());
                communicator.Logger.trace(communicator.TraceLevels.locationCat, s.ToString());
            }

            lock (this)
            {
                if (_objectRequests.TryGetValue(reference.getIdentity(), out Request request))
                {
                    return request;
                }

                request = new ObjectRequest(this, reference);
                _objectRequests.Add(reference.getIdentity(), request);
                return request;
            }
        }

        private void
        FinishRequest(Reference reference, List<Reference> wellKnownRefs, IObjectPrx? proxy, bool notRegistered)
        {
            if (proxy == null || proxy.IceReference.isIndirect())
            {
                //
                // Remove the cached references of well-known objects for which we tried
                // to resolved the endpoints if these endpoints are empty.
                //
                foreach (Reference r in wellKnownRefs)
                {
                    _table.RemoveObjectReference(r.getIdentity());
                }
            }

            if (!reference.isWellKnown())
            {
                if (proxy != null && !proxy.IceReference.isIndirect())
                {
                    // Cache the adapter endpoints.
                    _table.AddAdapterEndpoints(reference.getAdapterId(), proxy.IceReference.getEndpoints());
                }
                else if (notRegistered) // If the adapter isn't registered anymore, remove it from the cache.
                {
                    _table.RemoveAdapterEndpoints(reference.getAdapterId());
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
                    _table.AddObjectReference(reference.getIdentity(), proxy.IceReference);
                }
                else if (notRegistered) // If the well-known object isn't registered anymore, remove it from the cache.
                {
                    _table.RemoveObjectReference(reference.getIdentity());
                }

                lock (this)
                {
                    Debug.Assert(_objectRequests.ContainsKey(reference.getIdentity()));
                    _objectRequests.Remove(reference.getIdentity());
                }
            }
        }

        private ILocatorRegistryPrx? _locatorRegistry;
        private readonly LocatorTable _table;
        private readonly bool _background;

        private readonly Dictionary<string, Request> _adapterRequests = new Dictionary<string, Request>();
        private readonly Dictionary<Identity, Request> _objectRequests = new Dictionary<Identity, Request>();
    }

    public sealed partial class Communicator
    {
        private readonly struct LocatorKey : IEquatable<LocatorKey>
        {
            public LocatorKey(ILocatorPrx prx)
            {
                _id = prx.Identity;
                _encoding = prx.EncodingVersion;
            }

            public bool Equals(LocatorKey other) => _id.Equals(other._id) && _encoding.Equals(other._encoding);

            public override bool Equals(object obj) => (obj is LocatorKey other) && Equals(other);

            public override int GetHashCode()
            {
                int h = 5381;
                HashUtil.hashAdd(ref h, _id);
                HashUtil.hashAdd(ref h, _encoding);
                return h;
            }

            private readonly Identity _id;
            private readonly EncodingVersion _encoding;
        }
        //
        // Returns locator info for a given locator. Automatically creates
        // the locator info if it doesn't exist yet.
        //
        internal LocatorInfo GetLocatorInfo(ILocatorPrx loc)
        {
            if (loc == null)
            {
                throw new ArgumentNullException(nameof(loc));
            }

            //
            // The locator can't be located.
            //
            ILocatorPrx locator = loc.Clone(clearLocator: true);

            //
            // TODO: reap unused locator info objects?
            //
            lock (_locatorInfoMap)
            {
                if (!_locatorInfoMap.TryGetValue(locator, out LocatorInfo info))
                {
                    //
                    // Rely on locator identity for the adapter table. We want to
                    // have only one table per locator (not one per locator
                    // proxy).
                    //
                    var key = new LocatorKey(locator);
                    if (!_locatorTableMap.TryGetValue(key, out LocatorTable table))
                    {
                        table = new LocatorTable();
                        _locatorTableMap[key] = table;
                    }

                    info = new LocatorInfo(locator, table, _backgroundLocatorCacheUpdates);
                    _locatorInfoMap[locator] = info;
                }

                return info;
            }
        }

        private readonly Dictionary<ILocatorPrx, LocatorInfo> _locatorInfoMap = new Dictionary<ILocatorPrx, LocatorInfo>();
        private readonly Dictionary<LocatorKey, LocatorTable> _locatorTableMap = new Dictionary<LocatorKey, LocatorTable>();
        private readonly bool _backgroundLocatorCacheUpdates;
    }

    internal sealed class LocatorTable
    {
        internal void Clear()
        {
            lock (this)
            {
                _adapterEndpointsTable.Clear();
                _objectTable.Clear();
            }
        }

        internal Endpoint[]? GetAdapterEndpoints(string adapter, int ttl, out bool cached)
        {
            if (ttl == 0) // Locator cache disabled.
            {
                cached = false;
                return null;
            }

            lock (this)
            {
                if (_adapterEndpointsTable.TryGetValue(adapter, out (long Time, Endpoint[] Endpoints) entry))
                {
                    cached = CheckTTL(entry.Time, ttl);
                    return entry.Endpoints;

                }
                cached = false;
                return null;
            }
        }

        internal void AddAdapterEndpoints(string adapter, Endpoint[] endpoints)
        {
            lock (this)
            {
                _adapterEndpointsTable[adapter] = (Time.currentMonotonicTimeMillis(), endpoints);
            }
        }

        internal Endpoint[]? RemoveAdapterEndpoints(string adapter)
        {
            lock (this)
            {
                if (_adapterEndpointsTable.TryGetValue(adapter, out (long Time, Endpoint[] Endpoints) entry))
                {
                    _adapterEndpointsTable.Remove(adapter);
                    return entry.Endpoints;
                }
                return null;
            }
        }

        internal Reference? GetObjectReference(Identity id, int ttl, out bool cached)
        {
            if (ttl == 0) // Locator cache disabled.
            {
                cached = false;
                return null;
            }

            lock (this)
            {
                if (_objectTable.TryGetValue(id, out (long Time, Reference Reference) entry))
                {
                    cached = CheckTTL(entry.Time, ttl);
                    return entry.Reference;
                }
                cached = false;
                return null;
            }
        }

        internal void AddObjectReference(Identity id, Reference reference)
        {
            lock (this)
            {
                _objectTable[id] = (Time.currentMonotonicTimeMillis(), reference);
            }
        }

        internal Reference? RemoveObjectReference(Identity id)
        {
            lock (this)
            {
                if (_objectTable.TryGetValue(id, out (long Time, Reference Reference) entry))
                {
                    _objectTable.Remove(id);
                    return entry.Reference;
                }
                return null;
            }
        }

        private bool CheckTTL(long time, int ttl)
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

        private readonly struct TableEntry<T>
        {
            public TableEntry(long time, T value)
            {
                Time = time;
                Value = value;
            }

            public readonly long Time;
            public readonly T Value;
        }

        private readonly Dictionary<string, (long Time, Endpoint[] Endpoints)> _adapterEndpointsTable =
            new Dictionary<string, (long Time, Endpoint[] Endpoints)>();
        private readonly Dictionary<Identity, (long Time, Reference Reference)> _objectTable =
            new Dictionary<Identity, (long Time, Reference Reference)>();
    }

}
