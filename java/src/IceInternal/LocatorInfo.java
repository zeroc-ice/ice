// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public final class LocatorInfo
{
    LocatorInfo(Ice.LocatorPrx locator, LocatorAdapterTable adapterTable)
    {
	_locator = locator;
	_adapterTable = adapterTable;
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
	}
	
	return _locatorRegistry;
    }

    public IceInternal.Endpoint[]
    getEndpoints(Reference ref, Ice.BooleanHolder cached)
    {
	IceInternal.Endpoint[] endpoints = null;

	if(ref.adapterId.length() > 0)
	{
	    cached.value = true;
	
	    endpoints = _adapterTable.get(ref.adapterId);
	    if(endpoints == null)
	    {
		cached.value = false;

		//
		// Search the adapter in the location service if we didn't
		// find it in the cache.
		//
		try
		{
		    Ice.ObjectPrx object = _locator.findAdapterByName(ref.adapterId);
		    if(object != null)
		    {
			endpoints = ((Ice.ObjectPrxHelper)object).__reference().endpoints;
		    }
		}
		catch(Ice.LocalException ex)
		{
		    //
		    // Ignore. The proxy will most likely get empty
		    // endpoints and raise a NoEndpointException().
		    //
		}
		
		if(endpoints != null && endpoints.length > 0)
		{
		    _adapterTable.add(ref.adapterId, endpoints);
		}
	    }

	    if(endpoints != null && endpoints.length > 0)
	    {
		if(ref.instance.traceLevels().location >= 1)
		{
		    StringBuffer s = new StringBuffer();
		    if(cached.value)
			s.append("found endpoints in locator table\n");
		    else
			s.append("retrieved endpoints from locator, adding to locator table\n");
		    s.append("adapter = " + ref.adapterId + "\n");
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
	    }
	}

	return endpoints == null ? new IceInternal.Endpoint[0] : endpoints;
    }

    public void
    clearCache(Reference ref)
    {
	if(ref.adapterId.length() > 0)
	{
	    IceInternal.Endpoint[] endpoints = _adapterTable.remove(ref.adapterId);

	    if(ref.instance.traceLevels().location >= 2)
	    {
		StringBuffer s = new StringBuffer();
		s.append("removed endpoints from locator table\n");
		s.append("adapter = " + ref.adapterId + "\n");
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
	}
    }

    private Ice.LocatorPrx _locator;
    private Ice.LocatorRegistryPrx _locatorRegistry;
    private LocatorAdapterTable _adapterTable;
};

