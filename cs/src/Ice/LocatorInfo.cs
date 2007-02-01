// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections;
using System.Diagnostics;

namespace IceInternal
{

    public sealed class LocatorInfo
    {
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

        public EndpointI[] getEndpoints(IndirectReference @ref, int ttl, out bool cached)
        {
            Debug.Assert(@ref.getEndpoints().Length == 0);
            
            EndpointI[] endpoints = null;
            Ice.ObjectPrx obj = null;
            cached = true;
            string adapterId = @ref.getAdapterId();
            Ice.Identity identity = @ref.getIdentity();
            
            try
            {
                if(adapterId.Length > 0)
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
                        obj = _locator.findAdapterById(adapterId);
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

                        obj = _locator.findObjectById(identity);
                    }
                    
                    bool endpointsCached = true;
                    if(obj != null)
                    {
                        Reference r = ((Ice.ObjectPrxHelperBase)obj).reference__();
                        if(r is DirectReference)
                        {
                            DirectReference odr = (DirectReference)r;
                            endpointsCached = false;
                            endpoints = odr.getEndpoints();
                        }
                        else
                        {
                            IndirectReference oir = (IndirectReference)r;
                            if(oir.getAdapterId().Length > 0)
                            {
                                endpoints = getEndpoints(oir, ttl, out endpointsCached);
                            }
                        }
                    }
                    
                    if(!objectCached && endpoints != null && endpoints.Length > 0)
                    {
                        _table.addProxy(identity, obj);
                    }

                    cached = objectCached || endpointsCached;
                }
            }
            catch(Ice.AdapterNotFoundException ex)
            {
                if(@ref.getInstance().traceLevels().location >= 1)
                {
                    System.Text.StringBuilder s = new System.Text.StringBuilder();
                    s.Append("adapter not found\n");
                    s.Append("adapter = " + adapterId);
                    @ref.getInstance().initializationData().logger.trace(
                        @ref.getInstance().traceLevels().locationCat, s.ToString());
                }

                Ice.NotRegisteredException e = new Ice.NotRegisteredException(ex);
                e.kindOfObject = "object adapter";
                e.id = adapterId;
                throw e;
            }
            catch(Ice.ObjectNotFoundException ex)
            {
                if(@ref.getInstance().traceLevels().location >= 1)
                {
                    System.Text.StringBuilder s = new System.Text.StringBuilder();
                    s.Append("object not found\n");
                    s.Append("object = " + @ref.getInstance().identityToString(identity));
                    @ref.getInstance().initializationData().logger.trace(
                        @ref.getInstance().traceLevels().locationCat, s.ToString());
                }

                Ice.NotRegisteredException e = new Ice.NotRegisteredException(ex);
                e.kindOfObject = "object";
                e.id = @ref.getInstance().identityToString(identity);
                throw e;
            }
            catch(Ice.NotRegisteredException)
            {
                throw;
            }
            catch(Ice.LocalException ex)
            {
                if(@ref.getInstance().traceLevels().location >= 1)
                {
                    System.Text.StringBuilder s = new System.Text.StringBuilder();
                    s.Append("couldn't contact the locator to retrieve adapter endpoints\n");
                    if(adapterId.Length > 0)
                    {
                        s.Append("adapter = " + adapterId + "\n");
                    }
                    else
                    {
                        s.Append("object = " + @ref.getInstance().identityToString(identity) + "\n");
                    }
                    s.Append("reason = " + ex);
                    @ref.getInstance().initializationData().logger.trace(
                        @ref.getInstance().traceLevels().locationCat, s.ToString());
                }
            }
            
            if(@ref.getInstance().traceLevels().location >= 1)
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
                    System.Text.StringBuilder s = new System.Text.StringBuilder();
                    s.Append("no endpoints configured for ");
                    if(adapterId.Length > 0)
                    {
                        s.Append("adapter\n");
                        s.Append("adapter = " + adapterId);
                    }
                    else
                    {
                        s.Append("object\n");
                        s.Append("object = " + @ref.getInstance().identityToString(identity));
                    }
                    @ref.getInstance().initializationData().logger.trace(
                        @ref.getInstance().traceLevels().locationCat, s.ToString());
                }
            }
            
            return endpoints == null ? new EndpointI[0] : endpoints;
        }
        
        public void clearObjectCache(IndirectReference rf)
        {
            if(rf.getAdapterId().Length == 0 && rf.getEndpoints().Length == 0)
            {
                Ice.ObjectPrx obj = _table.removeProxy(rf.getIdentity());
                if(obj != null)
                {
                    if(((Ice.ObjectPrxHelperBase)obj).reference__() is IndirectReference)
                    {
                        IndirectReference oir = (IndirectReference)((Ice.ObjectPrxHelperBase)obj).reference__();
                        if(oir.getAdapterId().Length > 0)
                        {
                            clearCache(oir);
                        }
                    }
                    else
                    {
                        if(rf.getInstance().traceLevels().location >= 2)
                        {
                            trace("removed endpoints from locator table",
                                  rf, ((Ice.ObjectPrxHelperBase)obj).reference__().getEndpoints());
                        }
                    }
                }
            }
        }
        
        public void clearCache(IndirectReference rf)
        {
            if(rf.getAdapterId().Length > 0)
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
                    if(((Ice.ObjectPrxHelperBase)obj).reference__() is IndirectReference)
                    {
                        IndirectReference oir = (IndirectReference)((Ice.ObjectPrxHelperBase)obj).reference__();
                        if(oir.getAdapterId().Length > 0)
                        {
                            clearCache(oir);
                        }
                    }
                    else
                    {
                        if(rf.getInstance().traceLevels().location >= 2)
                        {
                            trace("removed endpoints from locator table",
                                  rf, ((Ice.ObjectPrxHelperBase)obj).reference__().getEndpoints());
                        }
                    }
                }
            }
        }
        
        private void trace(string msg, IndirectReference r, EndpointI[] endpoints)
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
        
        private readonly Ice.LocatorPrx _locator;
        private Ice.LocatorRegistryPrx _locatorRegistry;
        private readonly LocatorTable _table;
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
                _adapterEndpointsTable[adapter] = new EndpointTableEntry(System.DateTime.Now.Ticks / 10000, endpoints);
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
                _objectTable[id] = new ProxyTableEntry(System.DateTime.Now.Ticks / 10000, proxy);
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
                return System.DateTime.Now.Ticks / 10000 - time <= ((long)ttl * 1000);
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
