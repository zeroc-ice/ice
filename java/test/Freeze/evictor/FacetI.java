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

public final class FacetI extends Test.Facet
{
    FacetI()
    {
	_servant = new ServantI();
    }

    FacetI(RemoteEvictorI remoteEvictor, Freeze.Evictor evictor, int value, String d)
    {
	_servant = new ServantI(remoteEvictor, evictor, 0);
	_servant.value = value;
	data = d;
    }
    
    void
    init(RemoteEvictorI remoteEvictor, Freeze.Evictor evictor)
    {	
        _servant._remoteEvictor = remoteEvictor;
        _servant._evictor = evictor;

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
	_servant.destroy(current);
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
	_servant.setValueAsync_async(__cb, value, current);
    }

    public void
    releaseAsync(Ice.Current current)
    {
        if(_servant._setValueAsyncCB != null)
        {
	    synchronized(this)
	    {
		value = _servant._setValueAsyncValue;
	    }
            _servant._setValueAsyncCB.ice_response();
            _servant._setValueAsyncCB = null;
        }
    }

    public void
    addFacet(String name, String data, Ice.Current current)
	throws Test.AlreadyRegisteredException
    {
	_servant.addFacet(name, data, current);
    }

    public void
    removeFacet(String name, Ice.Current current)
	throws Test.NotRegisteredException
    {
	_servant.removeFacet(name, current);
    }

    public void
    removeAllFacets(Ice.Current current)
    {
	_servant.removeAllFacets(current);
    }
    
    public void
    __write(IceInternal.BasicStream os)
    {
        _servant._remoteEvictor.setLastSavedValue(value);
        super.__write(os);
    }

    public void
    __marshal(Ice.Stream os, boolean marshalFacets)
    {
        _servant._remoteEvictor.setLastSavedValue(value);
        super.__marshal(os, marshalFacets);
    }

    public synchronized String
    getData(Ice.Current current)
    {
	return data;
    }

    public synchronized void
    setData(String d, Ice.Current current)
    {
	data = d;
    }

    private ServantI _servant;
}
