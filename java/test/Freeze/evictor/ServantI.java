// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

public final class ServantI extends Test.Servant
{
    ServantI()
    {
    }

    ServantI(RemoteEvictorI remoteEvictor, Freeze.Evictor evictor, int value)
    {
        _remoteEvictor = remoteEvictor;
        _evictor = evictor;
        this.value = value;
    }

    void
    init(RemoteEvictorI remoteEvictor, Freeze.Evictor evictor)
    {	
        _remoteEvictor = remoteEvictor;
        _evictor = evictor;

	String[] facets = ice_facets(null);
	for(int i = 0; i < facets.length; i++)
	{
	    Ice.Object o = ice_findFacet(facets[i]);
	    if(o instanceof ServantI)
	    {
		((ServantI) o).init(remoteEvictor, evictor);
	    }
	    else
	    {
		assert(o instanceof FacetI);
		((FacetI) o).init(remoteEvictor, evictor);
	    }
	}    
    }

    public void
    destroy(Ice.Current current)
    {
        _evictor.destroyObject(current.id);
    }

    public synchronized int
    getValue(Ice.Current current)
    {
        return value;
    }

    public synchronized void
    setValue(int value, Ice.Current current)
    {
        this.value = value;
    }

    public void
    setValueAsync_async(Test.AMD_Servant_setValueAsync __cb, int value, Ice.Current current)
    {
        _setValueAsyncCB = __cb;
        _setValueAsyncValue = value;
    }

    public void
    releaseAsync(Ice.Current current)
    {
        if(_setValueAsyncCB != null)
        {
	    synchronized(this)
	    {
		value = _setValueAsyncValue;
	    }
            _setValueAsyncCB.ice_response();
            _setValueAsyncCB = null;
        }
    }

    public void
    addFacet(String name, String data, Ice.Current current)
	throws Test.AlreadyRegisteredException
    {
	String[] facetPath = new String[current.facet.length + 1];
	System.arraycopy(current.facet, 0, facetPath, 0, current.facet.length);
	facetPath[facetPath.length - 1] = name;

	FacetI facet = new FacetI(_remoteEvictor, _evictor, value, data);
	
	try
	{
	    _evictor.addFacet(current.id, facetPath, facet);
	}
	catch(Ice.AlreadyRegisteredException ex)
	{
	    throw new Test.AlreadyRegisteredException();
	}
    }

    public void
    removeFacet(String name, Ice.Current current)
	throws Test.NotRegisteredException
    {
	String[] facetPath = new String[current.facet.length + 1];
	System.arraycopy(current.facet, 0, facetPath, 0, current.facet.length);
	facetPath[facetPath.length - 1] = name;
	try
	{
	    _evictor.removeFacet(current.id, facetPath);
	}
	catch(Ice.NotRegisteredException ex)
	{
	    throw new Test.NotRegisteredException();
	}
   
    }

    public void
    removeAllFacets(Ice.Current current)
    {
	_evictor.removeAllFacets(current.id);
    }
    
    public void
    __write(IceInternal.BasicStream os)
    {
        _remoteEvictor.setLastSavedValue(value);
        super.__write(os);
    }

    public void
    __marshal(Ice.Stream os, boolean marshalFacets)
    {
        _remoteEvictor.setLastSavedValue(value);
        super.__marshal(os, marshalFacets);
    }

    RemoteEvictorI _remoteEvictor;
    Freeze.Evictor _evictor;
    Test.AMD_Servant_setValueAsync _setValueAsyncCB;
    int _setValueAsyncValue;
}
