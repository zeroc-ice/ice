// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

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
            response(LocatorInfo locatorInfo, Ice.ObjectPrx proxy)
            {
                EndpointI[] endpoints = null;
                if(proxy != null)
                {
                    Reference r = ((Ice.ObjectPrxHelperBase)proxy).reference__();
                    if(_ref.isWellKnown() && !Protocol.isSupported(_ref.getEncoding(), r.getEncoding()))
                    {
                        //
                        // If a well-known proxy and the returned
                        // proxy encoding isn't supported, we're done:
                        // there's no compatible endpoint we can use.
                        //
                    }
                    else if(!r.isIndirect())
                    {
                        endpoints = r.getEndpoints();
                    }
                    else if(_ref.isWellKnown() && !r.isWellKnown())
                    {
                        //
                        // We're resolving the endpoints of a well-known object and the proxy returned
                        // by the locator is an indirect proxy. We now need to resolve the endpoints
                        // of this indirect proxy.
                        //
                        locatorInfo.getEndpoints(r, _ref, _ttl, _callback);
                        return;
                    }
                }

                if(_ref.getInstance().traceLevels().location >= 1)
                {
                    locatorInfo.getEndpointsTrace(_ref, endpoints, false);
                }
                if(_callback != null)
                {
                    _callback.setEndpoints(endpoints == null ? new EndpointI[0] : endpoints, false);
                }
            }

            public void
            exception(LocatorInfo locatorInfo, Ice.Exception exc)
            {
                try
                {
                    locatorInfo.getEndpointsException(_ref, exc); // This throws.
                }
                catch(Ice.LocalException ex)
                {
                    if(_callback != null)
                    {
                        _callback.setException(ex);
                    }
                }
            }

            public
            RequestCallback(Reference @ref, int ttl, GetEndpointsCallback cb)
            {
                _ref = @ref;
                _ttl = ttl;
                _callback = cb;
            }

            readonly Reference _ref;
            readonly int _ttl;
            readonly GetEndpointsCallback _callback;
        }

        private abstract class Request
        {
            public void
            addCallback(Reference @ref, Reference wellKnownRef, int ttl, GetEndpointsCallback cb)
            {
                RequestCallback callback = new RequestCallback(@ref, ttl, cb);
                lock(this)
                {
                    if(!_response && _exception == null)
                    {
                        _callbacks.Add(callback);
                        if(wellKnownRef != null)
                        {
                            // This request is to resolve the endpoints of a cached well-known object ref
                            _wellKnownRefs.Add(wellKnownRef);
                        }
                        if(!_sent)
                        {
                            _sent = true;
                            send();
                        }
                        return;
                    }
                }

                if(_response)
                {
                    callback.response(_locatorInfo, _proxy);
                }
                else
                {
                    Debug.Assert(_exception != null);
                    callback.exception(_locatorInfo, _exception);
                }
            }

            public EndpointI[]
            getEndpoints(Reference @ref, Reference wellKnownRef, int ttl, out bool cached)
            {
                lock(this)
                {
                    if(!_response || _exception == null)
                    {
                        if(wellKnownRef != null)
                        {
                            // This request is to resolve the endpoints of a cached well-known object ref
                            _wellKnownRefs.Add(wellKnownRef);
                        }
                        if(!_sent)
                        {
                            _sent = true;
                            send();
                        }

                        while(!_response && _exception == null)
                        {
                            System.Threading.Monitor.Wait(this);
                        }
                    }

                    if(_exception != null)
                    {
                        _locatorInfo.getEndpointsException(@ref, _exception); // This throws.
                    }

                    Debug.Assert(_response);
                    EndpointI[] endpoints = null;
                    if(_proxy != null)
                    {
                        Reference r = ((Ice.ObjectPrxHelperBase)_proxy).reference__();
                        if(!r.isIndirect())
                        {
                            endpoints = r.getEndpoints();
                        }
                        else if(@ref.isWellKnown() && !r.isWellKnown())
                        {
                            //
                            // We're resolving the endpoints of a well-known object and the proxy returned
                            // by the locator is an indirect proxy. We now need to resolve the endpoints
                            // of this indirect proxy.
                            //
                            return _locatorInfo.getEndpoints(r, @ref, ttl, out cached);
                        }
                    }

                    cached = false;
                    if(_ref.getInstance().traceLevels().location >= 1)
                    {
                        _locatorInfo.getEndpointsTrace(@ref, endpoints, false);
                    }
                    return endpoints == null ? new EndpointI[0] : endpoints;
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
                lock(this)
                {
                    _locatorInfo.finishRequest(_ref, _wellKnownRefs, proxy, false);
                    _response = true;
                    _proxy = proxy;
                    System.Threading.Monitor.PulseAll(this);
                }
                foreach(RequestCallback callback in _callbacks)
                {
                    callback.response(_locatorInfo, proxy);
                }
            }

            public void
            exception(Ice.Exception ex)
            {
                lock(this)
                {
                    _locatorInfo.finishRequest(_ref, _wellKnownRefs, null, ex is Ice.UserException);
                    _exception = ex;
                    System.Threading.Monitor.PulseAll(this);
                }
                foreach(RequestCallback callback in _callbacks)
                {
                    callback.exception(_locatorInfo, ex);
                }
            }

            protected abstract void send();

            readonly protected LocatorInfo _locatorInfo;
            readonly protected Reference _ref;

            private List<RequestCallback> _callbacks = new List<RequestCallback>();
            private List<Reference> _wellKnownRefs = new List<Reference>();
            private bool _sent;
            private bool _response;
            private Ice.ObjectPrx _proxy;
            private Ice.Exception _exception;
        }

        private class ObjectRequest : Request
        {
            public ObjectRequest(LocatorInfo locatorInfo, Reference @ref) : base(locatorInfo, @ref)
            {
            }

            override protected void
            send()
            {
                try
                {
                    _locatorInfo.getLocator().begin_findObjectById(_ref.getIdentity()).whenCompleted(
                        this.response, this.exception);
                }
                catch(Ice.Exception ex)
                {
                    exception(ex);
                }
            }
        }

        private class AdapterRequest : Request
        {
            public AdapterRequest(LocatorInfo locatorInfo, Reference @ref) : base(locatorInfo, @ref)
            {
            }

            override protected void
            send()
            {
                try
                {
                    _locatorInfo.getLocator().begin_findAdapterById(_ref.getAdapterId()).whenCompleted(
                        this.response, this.exception);
                }
                catch(Ice.Exception ex)
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
            lock(this)
            {
                _locatorRegistry = null;
                _table.clear();
            }
        }

        public override bool Equals(object obj)
        {
            if(object.ReferenceEquals(this, obj))
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
            lock(this)
            {
                if(_locatorRegistry != null)
                {
                    return _locatorRegistry;
                }
            }

            //
            // Do not make locator calls from within sync.
            //
            Ice.LocatorRegistryPrx locatorRegistry = _locator.getRegistry();
            if(locatorRegistry == null)
            {
                return null;
            }

            lock(this)
            {
                //
                // The locator registry can't be located. We use ordered
                // endpoint selection in case the locator returned a proxy
                // with some endpoints which are prefered to be tried first.
                //
                _locatorRegistry = (Ice.LocatorRegistryPrx)locatorRegistry.ice_locator(null).ice_endpointSelection(
                    Ice.EndpointSelectionType.Ordered);
                return _locatorRegistry;
            }
        }

        public EndpointI[]
        getEndpoints(Reference @ref, int ttl, out bool cached)
        {
            return getEndpoints(@ref, null, ttl, out cached);
        }

        public EndpointI[]
        getEndpoints(Reference @ref, Reference wellKnownRef, int ttl, out bool cached)
        {
            Debug.Assert(@ref.isIndirect());
            EndpointI[] endpoints = null;
            cached = false;
            if(!@ref.isWellKnown())
            {
                endpoints = _table.getAdapterEndpoints(@ref.getAdapterId(), ttl, out cached);
                if(!cached)
                {
                    if(_background && endpoints != null)
                    {
                        getAdapterRequest(@ref).addCallback(@ref, wellKnownRef, ttl, null);
                    }
                    else
                    {
                        return getAdapterRequest(@ref).getEndpoints(@ref, wellKnownRef, ttl, out cached);
                    }
                }
            }
            else
            {
                Reference r = _table.getObjectReference(@ref.getIdentity(), ttl, out cached);
                if(!cached)
                {
                    if(_background && r != null)
                    {
                        getObjectRequest(@ref).addCallback(@ref, null, ttl, null);
                    }
                    else
                    {
                        return getObjectRequest(@ref).getEndpoints(@ref, null, ttl, out cached);
                    }
                }

                if(!r.isIndirect())
                {
                    endpoints = r.getEndpoints();
                }
                else if(!r.isWellKnown())
                {
                    return getEndpoints(r, @ref, ttl, out cached);
                }
            }

            Debug.Assert(endpoints != null);
            cached = true;
            if(@ref.getInstance().traceLevels().location >= 1)
            {
                getEndpointsTrace(@ref, endpoints, true);
            }
            return endpoints;
        }

        public void
        getEndpoints(Reference @ref, int ttl, GetEndpointsCallback callback)
        {
            getEndpoints(@ref, null, ttl, callback);
        }

        public void
        getEndpoints(Reference @ref, Reference wellKnownRef, int ttl, GetEndpointsCallback callback)
        {
            Debug.Assert(@ref.isIndirect());
            EndpointI[] endpoints = null;
            bool cached = false;
            if(!@ref.isWellKnown())
            {
                endpoints = _table.getAdapterEndpoints(@ref.getAdapterId(), ttl, out cached);
                if(!cached)
                {
                    if(_background && endpoints != null)
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
                if(!cached)
                {
                    if(_background && r != null)
                    {
                        getObjectRequest(@ref).addCallback(@ref, null, ttl, null);
                    }
                    else
                    {
                        getObjectRequest(@ref).addCallback(@ref, null, ttl, callback);
                        return;
                    }
                }

                if(!r.isIndirect())
                {
                    endpoints = r.getEndpoints();
                }
                else if(!r.isWellKnown())
                {
                    getEndpoints(r, @ref, ttl, callback);
                    return;
                }
            }

            Debug.Assert(endpoints != null);
            if(@ref.getInstance().traceLevels().location >= 1)
            {
                getEndpointsTrace(@ref, endpoints, true);
            }
            if(callback != null)
            {
                callback.setEndpoints(endpoints, true);
            }
        }

        public void clearCache(Reference rf)
        {
            Debug.Assert(rf.isIndirect());
            if(!rf.isWellKnown())
            {
                EndpointI[] endpoints = _table.removeAdapterEndpoints(rf.getAdapterId());

                if(endpoints != null && rf.getInstance().traceLevels().location >= 2)
                {
                    trace("removed endpoints from locator table\n", rf, endpoints);
                }
            }
            else
            {
                Reference r = _table.removeObjectReference(rf.getIdentity());
                if(r != null)
                {
                    if(!r.isIndirect())
                    {
                        if(rf.getInstance().traceLevels().location >= 2)
                        {
                            trace("removed endpoints from locator table", rf, r.getEndpoints());
                        }
                    }
                    else if(!r.isWellKnown())
                    {
                        clearCache(r);
                    }
                }
            }
        }

        private void trace(string msg, Reference r, EndpointI[] endpoints)
        {
            System.Text.StringBuilder s = new System.Text.StringBuilder();
            s.Append(msg + "\n");
            if(r.getAdapterId().Length > 0)
            {
                s.Append("adapter = " + r.getAdapterId() + "\n");
            }
            else
            {
                s.Append("object = " + r.getInstance().identityToString(r.getIdentity()) + "\n");
            }

            s.Append("endpoints = ");
            int sz = endpoints.Length;
            for (int i = 0; i < sz; i++)
            {
                s.Append(endpoints[i].ToString());
                if(i + 1 < sz)
                {
                    s.Append(":");
                }
            }

            r.getInstance().initializationData().logger.trace(r.getInstance().traceLevels().locationCat, s.ToString());
        }

        private void getEndpointsException(Reference @ref, System.Exception exc)
        {
            try
            {
                throw exc;
            }
            catch(Ice.AdapterNotFoundException ex)
            {
                Instance instance = @ref.getInstance();
                if(instance.traceLevels().location >= 1)
                {
                    System.Text.StringBuilder s = new System.Text.StringBuilder();
                    s.Append("adapter not found\n");
                    s.Append("adapter = " + @ref.getAdapterId());
                    instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.ToString());
                }

                Ice.NotRegisteredException e = new Ice.NotRegisteredException(ex);
                e.kindOfObject = "object adapter";
                e.id = @ref.getAdapterId();
                throw e;
            }
            catch(Ice.ObjectNotFoundException ex)
            {
                Instance instance = @ref.getInstance();
                if(instance.traceLevels().location >= 1)
                {
                    System.Text.StringBuilder s = new System.Text.StringBuilder();
                    s.Append("object not found\n");
                    s.Append("object = " + instance.identityToString(@ref.getIdentity()));
                    instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.ToString());
                }

                Ice.NotRegisteredException e = new Ice.NotRegisteredException(ex);
                e.kindOfObject = "object";
                e.id = instance.identityToString(@ref.getIdentity());
                throw e;
            }
            catch(Ice.NotRegisteredException)
            {
                throw;
            }
            catch(Ice.LocalException ex)
            {
                Instance instance = @ref.getInstance();
                if(instance.traceLevels().location >= 1)
                {
                    System.Text.StringBuilder s = new System.Text.StringBuilder();
                    s.Append("couldn't contact the locator to retrieve adapter endpoints\n");
                    if(@ref.getAdapterId().Length > 0)
                    {
                        s.Append("adapter = " + @ref.getAdapterId() + "\n");
                    }
                    else
                    {
                        s.Append("object = " + instance.identityToString(@ref.getIdentity()) + "\n");
                    }
                    s.Append("reason = " + ex);
                    instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.ToString());
                }
                throw;
            }
            catch(System.Exception)
            {
                Debug.Assert(false);
            }
        }

        private void getEndpointsTrace(Reference @ref, EndpointI[] endpoints, bool cached)
        {
            if(endpoints != null && endpoints.Length > 0)
            {
                if(cached)
                {
                    trace("found endpoints in locator table", @ref, endpoints);
                }
                else
                {
                    trace("retrieved endpoints from locator, adding to locator table", @ref, endpoints);
                }
            }
            else
            {
                Instance instance = @ref.getInstance();
                System.Text.StringBuilder s = new System.Text.StringBuilder();
                s.Append("no endpoints configured for ");
                if(@ref.getAdapterId().Length > 0)
                {
                    s.Append("adapter\n");
                    s.Append("adapter = " + @ref.getAdapterId());
                }
                else
                {
                    s.Append("object\n");
                    s.Append("object = " + instance.identityToString(@ref.getIdentity()));
                }
                instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.ToString());
            }
        }

        private Request
        getAdapterRequest(Reference @ref)
        {
            if(@ref.getInstance().traceLevels().location >= 1)
            {
                Instance instance = @ref.getInstance();
                System.Text.StringBuilder s = new System.Text.StringBuilder();
                s.Append("searching for adapter by id\nadapter = ");
                s.Append(@ref.getAdapterId());
                instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.ToString());
            }

            lock(this)
            {
                Request request;
                if(_adapterRequests.TryGetValue(@ref.getAdapterId(), out request))
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
            if(@ref.getInstance().traceLevels().location >= 1)
            {
                Instance instance = @ref.getInstance();
                System.Text.StringBuilder s = new System.Text.StringBuilder();
                s.Append("searching for object by id\nobject = ");
                s.Append(instance.identityToString(@ref.getIdentity()));
                instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.ToString());
            }

            lock(this)
            {
                Request request;
                if(_objectRequests.TryGetValue(@ref.getIdentity(), out request))
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
            if(proxy == null || @base.reference__().isIndirect())
            {
                //
                // Remove the cached references of well-known objects for which we tried
                // to resolved the endpoints if these endpoints are empty.
                //
                foreach(Reference r in wellKnownRefs)
                {
                    _table.removeObjectReference(r.getIdentity());
                }
            }

            if(!@ref.isWellKnown())
            {
                if(proxy != null && !@base.reference__().isIndirect())
                {
                    // Cache the adapter endpoints.
                    _table.addAdapterEndpoints(@ref.getAdapterId(), @base.reference__().getEndpoints());
                }
                else if(notRegistered) // If the adapter isn't registered anymore, remove it from the cache.
                {
                    _table.removeAdapterEndpoints(@ref.getAdapterId());
                }

                lock(this)
                {
                    Debug.Assert(_adapterRequests.ContainsKey(@ref.getAdapterId()));
                    _adapterRequests.Remove(@ref.getAdapterId());
                }
            }
            else
            {
                if(proxy != null && !@base.reference__().isWellKnown())
                {
                    // Cache the well-known object reference.
                    _table.addObjectReference(@ref.getIdentity(), @base.reference__());
                }
                else if(notRegistered) // If the well-known object isn't registered anymore, remove it from the cache.
                {
                    _table.removeObjectReference(@ref.getIdentity());
                }

                lock(this)
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

        private Dictionary<string, Request> _adapterRequests = new Dictionary<string, Request>();
        private Dictionary<Ice.Identity, Request> _objectRequests = new Dictionary<Ice.Identity, Request>();
    }

    public sealed class LocatorManager
    {
        struct LocatorKey
        {
            public LocatorKey(Ice.LocatorPrx prx)
            {
                Reference r = ((Ice.ObjectPrxHelperBase)prx).reference__();
                _id = r.getIdentity();
                _encoding = r.getEncoding();
            }

            public override bool Equals(object o)
            {
                LocatorKey k = (LocatorKey)o;
                if(!k._id.Equals(_id))
                {
                    return false;
                }
                if(!k._encoding.Equals(_encoding))
                {
                    return false;
                }
                return true;
            }

            public override int GetHashCode()
            {
                int h = 5381;
                IceInternal.HashUtil.hashAdd(ref h, _id);
                IceInternal.HashUtil.hashAdd(ref h, _encoding);
                return h;
            }

            private Ice.Identity _id;
            private Ice.EncodingVersion _encoding;
        };

        internal LocatorManager(Ice.Properties properties)
        {
            _table = new Dictionary<Ice.LocatorPrx, LocatorInfo>();
            _locatorTables = new Dictionary<LocatorKey, LocatorTable>();
            _background = properties.getPropertyAsInt("Ice.BackgroundLocatorCacheUpdates") > 0;
        }

        internal void destroy()
        {
            lock(this)
            {
                foreach(LocatorInfo info in _table.Values)
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
            if(loc == null)
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

            lock(this)
            {
                LocatorInfo info = null;
                if(!_table.TryGetValue(locator, out info))
                {
                    //
                    // Rely on locator identity for the adapter table. We want to
                    // have only one table per locator (not one per locator
                    // proxy).
                    //
                    LocatorTable table = null;
                    LocatorKey key = new LocatorKey(locator);
                    if(!_locatorTables.TryGetValue(key, out table))
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

    sealed class LocatorTable
    {
        internal LocatorTable()
        {
            _adapterEndpointsTable = new Dictionary<string, EndpointTableEntry>();
            _objectTable = new Dictionary<Ice.Identity, ReferenceTableEntry>();
        }

        internal void clear()
        {
            lock(this)
            {
                _adapterEndpointsTable.Clear();
                _objectTable.Clear();
            }
        }

        internal IceInternal.EndpointI[] getAdapterEndpoints(string adapter, int ttl, out bool cached)
        {
            if(ttl == 0) // Locator cache disabled.
            {
                cached = false;
                return null;
            }

            lock(this)
            {
                EndpointTableEntry entry = null;
                if(_adapterEndpointsTable.TryGetValue(adapter, out entry))
                {
                    cached = checkTTL(entry.time, ttl);
                    return entry.endpoints;

                }
                cached = false;
                return null;
            }
        }

        internal void addAdapterEndpoints(string adapter, IceInternal.EndpointI[] endpoints)
        {
            lock(this)
            {
                _adapterEndpointsTable[adapter] =
                    new EndpointTableEntry(Time.currentMonotonicTimeMillis(), endpoints);
            }
        }

        internal IceInternal.EndpointI[] removeAdapterEndpoints(string adapter)
        {
            lock(this)
            {
                EndpointTableEntry entry = null;
                if(_adapterEndpointsTable.TryGetValue(adapter, out entry))
                {
                    _adapterEndpointsTable.Remove(adapter);
                    return entry.endpoints;
                }
                return null;
            }
        }

        internal Reference getObjectReference(Ice.Identity id, int ttl, out bool cached)
        {
            if(ttl == 0) // Locator cache disabled.
            {
                cached = false;
                return null;
            }

            lock(this)
            {
                ReferenceTableEntry entry = null;
                if(_objectTable.TryGetValue(id, out entry))
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
            lock(this)
            {
                _objectTable[id] = new ReferenceTableEntry(Time.currentMonotonicTimeMillis(), reference);
            }
        }

        internal Reference removeObjectReference(Ice.Identity id)
        {
            lock(this)
            {
                ReferenceTableEntry entry = null;
                if(_objectTable.TryGetValue(id, out entry))
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
            if(ttl < 0) // TTL = infinite
            {
                return true;
            }
            else
            {
                return Time.currentMonotonicTimeMillis() - time <= ((long)ttl * 1000);
            }
        }

        sealed private class EndpointTableEntry
        {
            public EndpointTableEntry(long time, IceInternal.EndpointI[] endpoints)
            {
                this.time = time;
                this.endpoints = endpoints;
            }

            public long time;
            public IceInternal.EndpointI[] endpoints;
        }

        sealed private class ReferenceTableEntry
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
