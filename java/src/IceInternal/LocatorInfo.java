// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
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
	cached.value = false;

	if(ref.adapterId.length() > 0)
	{
	    endpoints = _adapterTable.get(ref.adapterId);
	    if(endpoints == null)
	    {
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
	    }
	    else
	    {
		cached.value = true;
	    }

	    if(endpoints != null)
	    {
		if(ref.instance.traceLevels().location >= 1)
		{
		    StringBuffer s = new StringBuffer();
		    if(cached.value)
			s.append("found object in local locator table\n");
		    else
			s.append("found object in locator\n");
		    s.append("identity = " + Ice.Util.identityToString(ref.identity) + "\n");
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
    addEndpoints(Reference ref, IceInternal.Endpoint[] endpoints)
    {
	assert(endpoints.length > 0);

	if(ref.adapterId.length() > 0)
	{
	    _adapterTable.add(ref.adapterId, endpoints);

	    if(ref.instance.traceLevels().location >= 2)
	    {
		String s = "added adapter to local locator table\nadapter = " + ref.adapterId;
		ref.instance.logger().trace(ref.instance.traceLevels().locationCat, s);
	    }
	}
    }

    public void
    removeEndpoints(Reference ref)
    {
	if(ref.adapterId.length() > 0)
	{
	    _adapterTable.remove(ref.adapterId);

	    if(ref.instance.traceLevels().location >= 2)
	    {
		String s = "removed adapter from local locator table\nadapter = " + ref.adapterId;
		ref.instance.logger().trace(ref.instance.traceLevels().locationCat, s);
	    }
	}
    }

    private Ice.LocatorPrx _locator;
    private Ice.LocatorRegistryPrx _locatorRegistry;
    private LocatorAdapterTable _adapterTable;
};

