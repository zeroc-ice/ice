// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


package IceInternal;

public final class LocatorInfo
{
    LocatorInfo(Ice.LocatorPrx locator, LocatorTable table)
    {
	_locator = locator;
	_table = table;
    }

    synchronized public void
    destroy()
    {
	_locatorRegistry = null;
	_table.clear();
    }

    public boolean
    equals(java.lang.Object obj)
    {
        LocatorInfo rhs = null;
        try
        {
            rhs = (LocatorInfo)obj;
        }
        catch (ClassCastException ex)
        {
            return false;
        }

        return _locator.equals(rhs._locator);
    }

    public Ice.LocatorPrx
    getLocator()
    {
	//
	// No synchronization necessary, _locator is immutable.
	//
	return _locator;
    }

    public synchronized Ice.LocatorRegistryPrx
    getLocatorRegistry()
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

    public Endpoint[]
    getEndpoints(Reference ref, Ice.BooleanHolder cached)
    {
	assert(ref.endpoints.length == 0);

	Endpoint[] endpoints = null;
	Ice.ObjectPrx object = null;
	cached.value = true;
	
	try
	{
	    if(ref.adapterId.length() > 0)
	    {
		endpoints = _table.getAdapterEndpoints(ref.adapterId);
		if(endpoints == null)
		{
		    cached.value = false;
		    
		    //
		    // Search the adapter in the location service if we didn't
		    // find it in the cache.
		    //
		    object = _locator.findAdapterById(ref.adapterId);
		    if(object != null)
		    {
			endpoints = ((Ice.ObjectPrxHelperBase)object).__reference().endpoints;
			
			if(endpoints != null && endpoints.length > 0)
			{
			    _table.addAdapterEndpoints(ref.adapterId, endpoints);
			}
		    }
		}
	    }
	    else
	    {
		object = _table.getProxy(ref.identity);
		if(object == null)
		{
		    cached.value = false;

		    object = _locator.findObjectById(ref.identity);
		}

		if(object != null)
		{
		    if(((Ice.ObjectPrxHelperBase)object).__reference().endpoints.length > 0)
		    {
			endpoints = ((Ice.ObjectPrxHelperBase)object).__reference().endpoints;
		    }
		    else if(((Ice.ObjectPrxHelperBase)object).__reference().adapterId.length() > 0)
		    {
			endpoints = getEndpoints(((Ice.ObjectPrxHelperBase)object).__reference(), cached);
		    }
		}
		
		if(!cached.value && endpoints != null && endpoints.length > 0)
		{
		    _table.addProxy(ref.identity, object);
		}
	    }
	}
	catch(Ice.AdapterNotFoundException ex)
	{
	    Ice.NotRegisteredException e = new Ice.NotRegisteredException();
	    e.kindOfObject = "object adapter";
	    e.id = ref.adapterId;
	    throw e;
	}
	catch(Ice.ObjectNotFoundException ex)
	{
	    Ice.NotRegisteredException e = new Ice.NotRegisteredException();
	    e.kindOfObject = "object";
	    e.id = Ice.Util.identityToString(ref.identity);
	    throw e;
	}
	catch(Ice.NotRegisteredException ex)
	{
	    throw ex;
	}
	catch(Ice.LocalException ex)
	{
	    if(ref.instance.traceLevels().location >= 1)
	    {
		StringBuffer s = new StringBuffer();
		s.append("couldn't contact the locator to retrieve adapter endpoints\n");
		if(ref.adapterId.length() > 0)
		{
		    s.append("adapter = " + ref.adapterId + "\n");
		}
		else
		{
		    s.append("object = " + Ice.Util.identityToString(ref.identity) + "\n");
		}
		s.append("reason = " + ex);
		ref.instance.logger().trace(ref.instance.traceLevels().locationCat, s.toString());
	    }
	}

	if(ref.instance.traceLevels().location >= 1 && endpoints != null && endpoints.length > 0)
	{
	    if(cached.value)
	    {
		trace("found endpoints in locator table", ref, endpoints);
	    }
	    else
	    {
		trace("retrieved endpoints from locator, adding to locator table", ref, endpoints);
	    }
	}

	return endpoints == null ? new Endpoint[0] : endpoints;
    }

    public void
    clearObjectCache(Reference ref)
    {
	if(ref.adapterId.length() == 0 && ref.endpoints.length == 0)
	{
	    Ice.ObjectPrx object = _table.removeProxy(ref.identity);
	    if(object != null && ref.instance.traceLevels().location >= 2)
	    {
		Reference r = ((Ice.ObjectPrxHelperBase)object).__reference();
		if(r.endpoints.length > 0)
		{
		    trace("removed endpoints from locator table", ref, r.endpoints);
		}
	    }
	}
    }
    
    public void
    clearCache(Reference ref)
    {
	if(ref.adapterId.length() > 0)
	{
	    Endpoint[] endpoints = _table.removeAdapterEndpoints(ref.adapterId);

	    if(endpoints != null && ref.instance.traceLevels().location >= 2)
	    {
		trace("removed endpoints from locator table\n", ref, endpoints);
	    }
	}
	else if(ref.endpoints.length == 0)
	{
	    Ice.ObjectPrx object = _table.removeProxy(ref.identity);
	    if(object != null)
	    {
		Reference r = ((Ice.ObjectPrxHelperBase)object).__reference();
		if(r.adapterId.length() > 0)
		{
		    clearCache(r);
		}
		else if(r.endpoints.length > 0)
		{
		    if(ref.instance.traceLevels().location >= 2)
		    {
			trace("removed endpoints from locator table", ref, r.endpoints);
		    }
		}
	    }
	}
    }

    private void
    trace(String msg, Reference ref, Endpoint[] endpoints)
    {
	StringBuffer s = new StringBuffer();
	s.append(msg + "\n");
	if(ref.adapterId.length() > 0)
	{
	    s.append("adapter = " + ref.adapterId + "\n");
	}
	else
	{
	    s.append("object = " + Ice.Util.identityToString(ref.identity) + "\n");
	}

	s.append("endpoints = ");
	final int sz = endpoints.length;
	for(int i = 0; i < sz; i++)
	{
	    s.append(endpoints[i].toString());
	    if(i + 1 < sz)
		s.append(":");
	}

	ref.instance.logger().trace(ref.instance.traceLevels().locationCat, s.toString());
    }

    private final Ice.LocatorPrx _locator;
    private Ice.LocatorRegistryPrx _locatorRegistry;
    private final LocatorTable _table;
}

