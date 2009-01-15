// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
        private interface RequestCallback
        {
            void response(LocatorInfo locatorInfo, Ice.ObjectPrx proxy);
            void exception(LocatorInfo locatorInfo, Ice.Exception ex);
        }

        private abstract class Request
        {
            public void 
            addCallback(RequestCallback callback)
            {
                lock(this)
                {
                    if(_response)
                    {
                        callback.response(_locatorInfo, _proxy);
                        return;
                    }
                    else if(_exception != null)
                    {
                        callback.exception(_locatorInfo, _exception);
                        return;
                    }
                    
                    _callbacks.Add(callback);
                    
                    if(!_sent)
                    {
                        _sent = true;
                        try
                        {
                            send();
                        }
                        catch(Ice.Exception ex)
                        {
                            exception(ex);
                        }
                    }
                }
            }

            public Ice.ObjectPrx 
            getProxy()
            {
                lock(this)
                {
                    if(_response)
                    {
                        return _proxy;
                    }
                    else if(_exception != null)
                    {
                        throw _exception;
                    }
                    
                    if(!_sent)
                    {
                        _sent = true;
                        try
                        {
                            send();
                        }
                        catch(Ice.Exception ex)
                        {
                            exception(ex);
                        }
                    }

                    while(!_response && _exception == null)
                    {
                        Monitor.Wait(this);
                    }

                    if(_exception != null)
                    {
                        throw _exception;
                    }
                    Debug.Assert(_response);
                    return _proxy;
                }
            }

            protected 
            Request(LocatorInfo locatorInfo)
            {
                _locatorInfo = locatorInfo;
                _sent = false;
                _response = false;
            }

            public void 
            response(Ice.ObjectPrx proxy)
            {
                lock(this)
                {
                    _response = true;
                    _proxy = proxy;
                    foreach(RequestCallback c in _callbacks)
                    {
                        c.response(_locatorInfo, proxy);
                    }
                    Monitor.PulseAll(this);
                }
            }

            public void 
            exception(Ice.Exception ex)
            {
                lock(this)
                {
                    _exception = ex;
                    foreach(RequestCallback c in _callbacks)
                    {
                        c.exception(_locatorInfo, ex);
                    }
                    Monitor.PulseAll(this);
                }
            }

            protected abstract void send();

            protected readonly LocatorInfo _locatorInfo;

            private List<RequestCallback> _callbacks = new List<RequestCallback>();
            private bool _sent;
            private bool _response;
            private Ice.ObjectPrx _proxy;
            private Ice.Exception _exception;
        }

        private class ObjectRequest : Request
        {
            private class AMICallback : Ice.AMI_Locator_findObjectById
            {
                public AMICallback(Request request, LocatorInfo locatorInfo, Ice.Identity id)
                {
                    _request = request;
                    _locatorInfo = locatorInfo;
                    _id = id;
                }

                override public void
                ice_response(Ice.ObjectPrx proxy)
                {
                    _locatorInfo.removeObjectRequest(_id);
                    _request.response(proxy);
                }

                override public void
                ice_exception(Ice.Exception ex)
                {
                    _locatorInfo.removeObjectRequest(_id);
                    _request.exception(ex);
                }

                private readonly Request _request;
                private readonly LocatorInfo _locatorInfo;
                private readonly Ice.Identity _id;
            }

            public ObjectRequest(LocatorInfo locatorInfo, Ice.Identity id) : base(locatorInfo)
            {
                _id = id;
            }

            override protected void 
            send()
            {
                _locatorInfo.getLocator().findObjectById_async(new AMICallback(this, _locatorInfo, _id), _id);
            }

            private readonly Ice.Identity _id;
        }

        private class AdapterRequest : Request
        {
            private class AMICallback : Ice.AMI_Locator_findAdapterById
            {
                public AMICallback(Request request, LocatorInfo locatorInfo, string id)
                {
                    _request = request;
                    _locatorInfo = locatorInfo;
                    _id = id;
                }

                override public void
                ice_response(Ice.ObjectPrx proxy)
                {
                    _locatorInfo.removeAdapterRequest(_id);
                    _request.response(proxy);
                }

                override public void
                ice_exception(Ice.Exception ex)
                {
                    _locatorInfo.removeAdapterRequest(_id);
                    _request.exception(ex);
                }

                private readonly Request _request;
                private readonly LocatorInfo _locatorInfo;
                private readonly string _id;
            }

            public AdapterRequest(LocatorInfo locatorInfo, string id) : base(locatorInfo)
            {
                _id = id;
            }

            override protected void
            send()
            {
                _locatorInfo.getLocator().findAdapterById_async(new AMICallback(this, _locatorInfo, _id), _id);
            }

            private readonly string _id;
        }

        private class ObjectRequestCallback : RequestCallback
        {
            public void
            response(LocatorInfo locatorInfo, Ice.ObjectPrx obj)
            {
                locatorInfo.getWellKnownObjectEndpoints(_reference, obj, _ttl, false, _callback);
            }

            public void
            exception(LocatorInfo locatorInfo, Ice.Exception ex)
            {
                if(ex is Ice.CollocationOptimizationException)
                {
                    try
                    {
                        bool cached;
                        _callback.setEndpoints(locatorInfo.getEndpoints(_reference, _ttl, out cached), cached);
                    }
                    catch(Ice.LocalException e)
                    {
                        _callback.setException(e);
                    }
                }
                else
                {
                    locatorInfo.getEndpointsException(_reference, ex, _callback);
                }
            }

            public 
            ObjectRequestCallback(Reference @ref, int ttl, GetEndpointsCallback cb)
            {
                _reference = @ref;
                _ttl = ttl;
                _callback = cb;
            }

            private readonly Reference _reference;
            private readonly int _ttl;
            private readonly GetEndpointsCallback _callback;
        }

        private class AdapterRequestCallback : RequestCallback
        {
            public void
            response(LocatorInfo locatorInfo, Ice.ObjectPrx obj)
            {
                EndpointI[] endpoints = null;
                if(obj != null)
                {
                    endpoints = ((Ice.ObjectPrxHelperBase)obj).reference__().getEndpoints();
                    if(endpoints.Length > 0)
                    {
                        locatorInfo.getTable().addAdapterEndpoints(_reference.getAdapterId(), endpoints);
                    }
                }

                if(_reference.getInstance().traceLevels().location >= 1)
                {
                    locatorInfo.getEndpointsTrace(_reference, endpoints, false);
                }

                if(endpoints == null)
                {
                    _callback.setEndpoints(new EndpointI[0], false);
                }
                else
                {
                    _callback.setEndpoints(endpoints, false);
                }
            }

            public void
            exception(LocatorInfo locatorInfo, Ice.Exception ex)
            {
                if(ex is Ice.CollocationOptimizationException)
                {
                    try
                    {
                        bool cached;
                        _callback.setEndpoints(locatorInfo.getEndpoints(_reference, _ttl, out cached), cached);
                    }
                    catch(Ice.LocalException e)
                    {
                        _callback.setException(e);
                    }
                }
                else
                {
                    locatorInfo.getEndpointsException(_reference, ex, _callback);
                }
            }

            public 
            AdapterRequestCallback(Reference @ref, int ttl, GetEndpointsCallback callback)
            {
                _reference = @ref;
                _ttl = ttl;
                _callback = callback;
            }

            private readonly Reference _reference;
            private readonly int _ttl;
            private readonly GetEndpointsCallback _callback;
        }

        public interface GetEndpointsCallback
        {
            void setEndpoints(EndpointI[] endpoints, bool cached);
            void setException(Ice.LocalException ex);
        }

        internal LocatorInfo(Ice.LocatorPrx locator, LocatorTable table)
        {
            _locator = locator;
            _table = table;
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
                if(_locatorRegistry == null) // Lazy initialization
                {
                    _locatorRegistry = _locator.getRegistry();
                    
                    //
                    // The locator registry can't be located.
                    //
                    _locatorRegistry = Ice.LocatorRegistryPrxHelper.uncheckedCast(_locatorRegistry.ice_locator(null));
                }
                
                return _locatorRegistry;
            }
        }

        public EndpointI[] getEndpoints(Reference @ref, int ttl, out bool cached)
        {
            Debug.Assert(@ref.isIndirect());

            EndpointI[] endpoints = null;
            Ice.ObjectPrx obj = null;
            cached = true;
            string adapterId = @ref.getAdapterId();
            Ice.Identity identity = @ref.getIdentity();
            
            try
            {
                if(!@ref.isWellKnown())
                {
                    endpoints = _table.getAdapterEndpoints(adapterId, ttl);
                    if(endpoints == null)
                    {
                        cached = false;
                        
                        if(@ref.getInstance().traceLevels().location >= 1)
                        {
                            System.Text.StringBuilder s = new System.Text.StringBuilder();
                            s.Append("searching for adapter by id\n");
                            s.Append("adapter = " + adapterId);
                            @ref.getInstance().initializationData().logger.trace(
                                @ref.getInstance().traceLevels().locationCat, s.ToString());
                        }

                        //
                        // Search the adapter in the location service if we didn't
                        // find it in the cache.
                        //
                        Request request = getAdapterRequest(adapterId);
                        obj = request.getProxy();
                        if(obj != null)
                        {
                            endpoints = ((Ice.ObjectPrxHelperBase)obj).reference__().getEndpoints();
                            
                            if(endpoints != null && endpoints.Length > 0)
                            {
                                _table.addAdapterEndpoints(adapterId, endpoints);
                            }
                        }
                    }
                }
                else
                {
                    bool objectCached = true;
                    obj = _table.getProxy(identity, ttl);
                    if(obj == null)
                    {
                        objectCached = false;
                    
                        if(@ref.getInstance().traceLevels().location >= 1)
                        {
                            System.Text.StringBuilder s = new System.Text.StringBuilder();
                            s.Append("searching for object by id\n");
                            s.Append("object = " + @ref.getInstance().identityToString(identity));
                            @ref.getInstance().initializationData().logger.trace(
                                @ref.getInstance().traceLevels().locationCat, s.ToString());
                        }

                        Request request = getObjectRequest(identity);
                        obj = request.getProxy();
                    }
                    
                    bool endpointsCached = true;
                    if(obj != null)
                    {
                        Reference r = ((Ice.ObjectPrxHelperBase)obj).reference__();
                        if(!r.isIndirect())
                        {
                            endpointsCached = false;
                            endpoints = r.getEndpoints();
                        }
                        else if(!r.isWellKnown())
                        {
                            endpoints = getEndpoints(r, ttl, out endpointsCached);
                        }
                    }
                    
                    if(!objectCached && endpoints != null && endpoints.Length > 0)
                    {
                        _table.addProxy(identity, obj);
                    }

                    cached = objectCached || endpointsCached;
                }
            }
            catch(System.Exception ex)
            {
                getEndpointsException(@ref, ex);
            }

            if(@ref.getInstance().traceLevels().location >= 1)
            {
                getEndpointsTrace(@ref, endpoints, cached);
            }       

            return endpoints == null ? new EndpointI[0] : endpoints;
        }

        public void getEndpoints(Reference @ref, int ttl, GetEndpointsCallback callback)
        {
            Debug.Assert(@ref.isIndirect());

            string adapterId = @ref.getAdapterId();
            Ice.Identity identity = @ref.getIdentity();
            Instance instance = @ref.getInstance();
            if(!@ref.isWellKnown())
            {
                EndpointI[] endpoints = _table.getAdapterEndpoints(adapterId, ttl);
                if(endpoints == null)
                {
                    if(instance.traceLevels().location >= 1)
                    {
                        System.Text.StringBuilder s = new System.Text.StringBuilder();
                        s.Append("searching for adapter by id\n");
                        s.Append("adapter = " + adapterId);
                        instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.ToString());
                    }

                    //
                    // Search the adapter in the location service if we didn't
                    // find it in the cache.
                    //
                    Request request = getAdapterRequest(adapterId);
                    request.addCallback(new AdapterRequestCallback(@ref, ttl, callback));
                    return;
                }
                else
                {
                    if(instance.traceLevels().location >= 1)
                    {
                        getEndpointsTrace(@ref, endpoints, true);
                    }
                    callback.setEndpoints(endpoints, true);
                    return;
                }
            }
            else
            {
                Ice.ObjectPrx obj = _table.getProxy(identity, ttl);
                if(obj == null)
                {
                    if(instance.traceLevels().location >= 1)
                    {
                        System.Text.StringBuilder s = new System.Text.StringBuilder();
                        s.Append("searching for object by id\n");
                        s.Append("object = " + instance.identityToString(identity));
                        instance.initializationData().logger.trace(instance.traceLevels().locationCat, s.ToString());
                    }

                    Request request = getObjectRequest(identity);
                    request.addCallback(new ObjectRequestCallback(@ref, ttl, callback));
                    return;
                }
                else
                {
                    getWellKnownObjectEndpoints(@ref, obj, ttl, true, callback);
                    return;
                }
            }
        }

        public void clearObjectCache(Reference rf)
        {
            Debug.Assert(rf.isIndirect());
            if(rf.getAdapterId().Length == 0)
            {
                Ice.ObjectPrx obj = _table.removeProxy(rf.getIdentity());
                if(obj != null)
                {
                    Reference r = ((Ice.ObjectPrxHelperBase)obj).reference__();
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
                Ice.ObjectPrx obj = _table.removeProxy(rf.getIdentity());
                if(obj != null)
                {
                    Reference r = ((Ice.ObjectPrxHelperBase)obj).reference__();
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

        private void getEndpointsException(Reference @ref, System.Exception exc, GetEndpointsCallback callback)
        {
            try
            {
                getEndpointsException(@ref, exc);
            }
            catch(Ice.LocalException ex)
            {
                callback.setException(ex);
            }
            catch(System.Exception)
            {
                Debug.Assert(false);
            }
        }

        private class GetWellKnownObjectEndpointsCallback : GetEndpointsCallback
        {
            internal GetWellKnownObjectEndpointsCallback(LocatorInfo info, Reference @ref, Ice.ObjectPrx obj,
                                                         bool objectCached, GetEndpointsCallback callback)
            {
                _info = info;
                _ref = @ref;
                _obj = obj;
                _objectCached = objectCached;
                _callback = callback;
            }

            public void setEndpoints(EndpointI[] endpoints, bool endpointsCached)
            {
                if(!_objectCached && endpoints != null && endpoints.Length > 0)
                {
                    _info._table.addProxy(_ref.getIdentity(), _obj);
                }

                if(_ref.getInstance().traceLevels().location >= 1)
                {
                    _info.getEndpointsTrace(_ref, endpoints, _objectCached || endpointsCached);
                }

                _callback.setEndpoints(endpoints, _objectCached || endpointsCached);
            }

            public void setException(Ice.LocalException ex)
            {
                _callback.setException(ex);
            }

            private LocatorInfo _info;
            private Reference _ref;
            private Ice.ObjectPrx _obj;
            private bool _objectCached;
            private GetEndpointsCallback _callback;
        }

        private void getWellKnownObjectEndpoints(Reference @ref, Ice.ObjectPrx obj, int ttl,
                                                 bool objectCached, GetEndpointsCallback callback)
        {
            EndpointI[] endpoints = null;
            if(obj != null)
            {
                Reference r = ((Ice.ObjectPrxHelperBase)obj).reference__();
                if(!r.isIndirect())
                {
                    endpoints = r.getEndpoints();
                }
                else if(!r.isWellKnown())
                {
                    getEndpoints(r, ttl,
                                 new GetWellKnownObjectEndpointsCallback(this, @ref, obj, objectCached, callback));
                    return;
                }
            }
            
            if(!objectCached && endpoints != null && endpoints.Length > 0)
            {
                _table.addProxy(@ref.getIdentity(), obj);
            }

            if(@ref.getInstance().traceLevels().location >= 1)
            {
                getEndpointsTrace(@ref, endpoints, objectCached);
            }

            if(endpoints == null)
            {
                callback.setEndpoints(new EndpointI[0], false);
            }
            else
            {
                callback.setEndpoints(endpoints, objectCached);
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
        getAdapterRequest(string id)
        {
            lock(this)
            {
                Request request;
                if(_adapterRequests.TryGetValue(id, out request))
                {
                    return request;
                }
                
                request = new AdapterRequest(this, id);
                _adapterRequests.Add(id, request);
                return request;
            }
        }

        private void
        removeAdapterRequest(string id)
        {
            lock(this)
            {
                Debug.Assert(_adapterRequests.ContainsKey(id));
                _adapterRequests.Remove(id);
            }
        }

        private Request
        getObjectRequest(Ice.Identity id)
        {
            lock(this)
            {
                Request request;
                if(_objectRequests.TryGetValue(id, out request))
                {
                    return request;
                }
                
                request = new ObjectRequest(this, id);
                _objectRequests.Add(id, request);
                return request;
            }
        }

        private void
        removeObjectRequest(Ice.Identity id)
        {
            lock(this)
            {
                Debug.Assert(_objectRequests.ContainsKey(id));
                _objectRequests.Remove(id);
            }
        }

        private LocatorTable
        getTable()
        {
            return _table;
        }

        private readonly Ice.LocatorPrx _locator;
        private Ice.LocatorRegistryPrx _locatorRegistry;
        private readonly LocatorTable _table;

        private Dictionary<string, Request> _adapterRequests = new Dictionary<string, Request>();
        private Dictionary<Ice.Identity, Request> _objectRequests = new Dictionary<Ice.Identity, Request>();
    }

    public sealed class LocatorManager
    {
        internal LocatorManager()
        {
            _table = new Hashtable();
            _locatorTables = new Hashtable();
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
                LocatorInfo info = (LocatorInfo)_table[locator];
                if(info == null)
                {
                    //
                    // Rely on locator identity for the adapter table. We want to
                    // have only one table per locator (not one per locator
                    // proxy).
                    //
                    LocatorTable table = (LocatorTable)_locatorTables[locator.ice_getIdentity()];
                    if(table == null)
                    {
                        table = new LocatorTable();
                        _locatorTables[locator.ice_getIdentity()] = table;
                    }
                    
                    info = new LocatorInfo(locator, table);
                    _table[locator] = info;
                }
                
                return info;
            }
        }
        
        private Hashtable _table;
        private Hashtable _locatorTables;
    }

    sealed class LocatorTable
    {
        internal LocatorTable()
        {
            _adapterEndpointsTable = new Hashtable();
            _objectTable = new Hashtable();
        }
        
        internal void clear()
        {
            lock(this)
            {
                _adapterEndpointsTable.Clear();
                _objectTable.Clear();
            }
        }
        
        internal IceInternal.EndpointI[] getAdapterEndpoints(string adapter, int ttl)
        {
            if(ttl == 0) // Locator cache disabled.
            {
                return null;
            }

            lock(this)
            {
                EndpointTableEntry entry = (EndpointTableEntry)_adapterEndpointsTable[adapter];
                if(entry != null && checkTTL(entry.time, ttl))
                {
                    return entry.endpoints;
                }
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
                EndpointTableEntry entry = (EndpointTableEntry)_adapterEndpointsTable[adapter];
                _adapterEndpointsTable.Remove(adapter);
                return entry != null ? entry.endpoints : null;
            }
        }
        
        internal Ice.ObjectPrx getProxy(Ice.Identity id, int ttl)
        {
            if(ttl == 0) // Locator cache disabled.
            {
                return null;
            }

            lock(this)
            {
                ProxyTableEntry entry = (ProxyTableEntry)_objectTable[id];
                if(entry != null && checkTTL(entry.time, ttl))
                {
                    return entry.proxy;
                }
                return null;
            }
        }
        
        internal void addProxy(Ice.Identity id, Ice.ObjectPrx proxy)
        {
            lock(this)
            {
                _objectTable[id] = new ProxyTableEntry(Time.currentMonotonicTimeMillis(), proxy);
            }
        }
        
        internal Ice.ObjectPrx removeProxy(Ice.Identity id)
        {
            lock(this)
            {
                ProxyTableEntry entry = (ProxyTableEntry)_objectTable[id];
                _objectTable.Remove(id);
                return entry != null ? entry.proxy : null;
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

        sealed private class ProxyTableEntry
        {
            public ProxyTableEntry(long time, Ice.ObjectPrx proxy)
            {
                this.time = time;
                this.proxy = proxy;
            }

            public long time;
            public Ice.ObjectPrx proxy;
        }

        private Hashtable _adapterEndpointsTable;
        private Hashtable _objectTable;
    }

}
