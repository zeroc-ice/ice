// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
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

	public Endpoint[] getEndpoints(Reference r, out bool cached)
	{
	    Debug.Assert(r.endpoints.Length == 0);
	    
	    Endpoint[] endpoints = null;
	    Ice.ObjectPrx obj = null;
	    cached = true;
	    
	    try
	    {
		if(r.adapterId.Length > 0)
		{
		    endpoints = _table.getAdapterEndpoints(r.adapterId);
		    if(endpoints == null)
		    {
			cached = false;
			
			//
			// Search the adapter in the location service if we didn't
			// find it in the cache.
			//
			obj = _locator.findAdapterById(r.adapterId);
			if(obj != null)
			{
			    endpoints = ((Ice.ObjectPrxHelperBase)obj).__reference().endpoints;
			    
			    if(endpoints != null && endpoints.Length > 0)
			    {
				_table.addAdapterEndpoints(r.adapterId, endpoints);
			    }
			}
		    }
		}
		else
		{
		    obj = _table.getProxy(r.identity);
		    if(obj == null)
		    {
			cached = false;
		    
			obj = _locator.findObjectById(r.identity);
		    }
		    
		    if(obj != null)
		    {
			if(((Ice.ObjectPrxHelperBase) obj).__reference().endpoints.Length > 0)
			{
			    endpoints = ((Ice.ObjectPrxHelperBase)obj).__reference().endpoints;
			}
			else if(((Ice.ObjectPrxHelperBase)obj).__reference().adapterId.Length > 0)
			{
			    endpoints = getEndpoints(((Ice.ObjectPrxHelperBase)obj).__reference(), out cached);
			}
		    }
		    
		    if(!cached && endpoints != null && endpoints.Length > 0)
		    {
			_table.addProxy(r.identity, obj);
		    }
		}
	    }
	    catch(Ice.AdapterNotFoundException ex)
	    {
		Ice.NotRegisteredException e = new Ice.NotRegisteredException(ex);
		e.kindOfObject = "object adapter";
		e.id = r.adapterId;
		throw e;
	    }
	    catch(Ice.ObjectNotFoundException ex)
	    {
		Ice.NotRegisteredException e = new Ice.NotRegisteredException(ex);
		e.kindOfObject = "object";
		e.id = Ice.Util.identityToString(r.identity);
		throw e;
	    }
	    catch(Ice.NotRegisteredException ex)
	    {
		throw ex;
	    }
	    catch(Ice.LocalException ex)
	    {
		if(r.instance.traceLevels().location >= 1)
		{
		    System.Text.StringBuilder s = new System.Text.StringBuilder();
		    s.Append("couldn't contact the locator to retrieve adapter endpoints\n");
		    if(r.adapterId.Length > 0)
		    {
			s.Append("adapter = " + r.adapterId + "\n");
		    }
		    else
		    {
			s.Append("object = " + Ice.Util.identityToString(r.identity) + "\n");
		    }
		    s.Append("reason = " + ex);
		    r.instance.logger().trace(r.instance.traceLevels().locationCat, s.ToString());
		}
	    }
	    
	    if(r.instance.traceLevels().location >= 1 && endpoints != null && endpoints.Length > 0)
	    {
		if(cached)
		{
		    trace("found endpoints in locator table", r, endpoints);
		}
		else
		{
		    trace("retrieved endpoints from locator, adding to locator table", r, endpoints);
		}
	    }
	    
	    return endpoints == null ? new Endpoint[0] : endpoints;
	}
	
	public void clearObjectCache(Reference rf)
	{
	    if(rf.adapterId.Length == 0 && rf.endpoints.Length == 0)
	    {
		Ice.ObjectPrx obj = _table.removeProxy(rf.identity);
		if(obj != null && rf.instance.traceLevels().location >= 2)
		{
		    Reference r = ((Ice.ObjectPrxHelperBase)obj).__reference();
		    if(r.endpoints.Length > 0)
		    {
			trace("removed endpoints from locator table", rf, r.endpoints);
		    }
		}
	    }
	}
	
	public void clearCache(Reference rf)
	{
	    if(rf.adapterId.Length > 0)
	    {
		Endpoint[] endpoints = _table.removeAdapterEndpoints(rf.adapterId);
		
		if(endpoints != null && rf.instance.traceLevels().location >= 2)
		{
		    trace("removed endpoints from locator table\n", rf, endpoints);
		}
	    }
	    else if(rf.endpoints.Length == 0)
	    {
		Ice.ObjectPrx obj = _table.removeProxy(rf.identity);
		if(obj != null)
		{
		    Reference r = ((Ice.ObjectPrxHelperBase)obj).__reference();
		    if(r.adapterId.Length > 0)
		    {
			clearCache(r);
		    }
		    else if(r.endpoints.Length > 0)
		    {
			if(rf.instance.traceLevels().location >= 2)
			{
			    trace("removed endpoints from locator table", rf, r.endpoints);
			}
		    }
		}
	    }
	}
	
	private void trace(string msg, Reference r, Endpoint[] endpoints)
	{
	    System.Text.StringBuilder s = new System.Text.StringBuilder();
	    s.Append(msg + "\n");
	    if(r.adapterId.Length > 0)
	    {
		s.Append("adapter = " + r.adapterId + "\n");
	    }
	    else
	    {
		s.Append("object = " + Ice.Util.identityToString(r.identity) + "\n");
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
	    
	    r.instance.logger().trace(r.instance.traceLevels().locationCat, s.ToString());
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
	
        internal IceInternal.Endpoint[] getAdapterEndpoints(string adapter)
        {
            lock(this)
            {
                return (IceInternal.Endpoint[])_adapterEndpointsTable[adapter];
            }
        }
	
        internal void addAdapterEndpoints(string adapter, IceInternal.Endpoint[] endpoints)
        {
            lock(this)
            {
                _adapterEndpointsTable[adapter] = endpoints;
            }
        }
	
        internal IceInternal.Endpoint[] removeAdapterEndpoints(string adapter)
        {
            lock(this)
            {
                IceInternal.Endpoint[] endpoints = (IceInternal.Endpoint[])_adapterEndpointsTable[adapter];
                _adapterEndpointsTable.Remove(adapter);
                return endpoints;
            }
        }
	
        internal Ice.ObjectPrx getProxy(Ice.Identity id)
        {
            lock(this)
            {
                return (Ice.ObjectPrx)_objectTable[id];
            }
        }
	
        internal void addProxy(Ice.Identity id, Ice.ObjectPrx proxy)
        {
            lock(this)
            {
                _objectTable[id] = proxy;
            }
        }
	
        internal Ice.ObjectPrx removeProxy(Ice.Identity id)
        {
            lock(this)
            {
                Ice.ObjectPrx obj = (Ice.ObjectPrx)_objectTable[id];
                _objectTable.Remove(id);
                return obj;
            }
        }
	
        private Hashtable _adapterEndpointsTable;
        private Hashtable _objectTable;
    }

}
