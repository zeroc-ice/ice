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

public class ServantI implements Test._ServantOperations
{
    ServantI(Test.Servant tie)
    {
	_tie = tie;
    }

    ServantI(Test.Servant tie, RemoteEvictorI remoteEvictor, Freeze.Evictor evictor, int value)
    {
	_tie = tie;
        _remoteEvictor = remoteEvictor;
        _evictor = evictor;
        _tie.value = value;
    }

    void
    init(RemoteEvictorI remoteEvictor, Freeze.Evictor evictor)
    {	
        _remoteEvictor = remoteEvictor;
        _evictor = evictor;

	String[] facets = _tie.ice_facets(null);
	for(int i = 0; i < facets.length; i++)
	{
	    Ice.Object o = _tie.ice_findFacet(facets[i]);
	    if(o instanceof Test._ServantTie)
	    {
		ServantI servant = (ServantI) ((Test._ServantTie) o).ice_delegate();
		servant.init(remoteEvictor, evictor);
	    }
	    else
	    {
		assert(o instanceof Test._FacetTie);
		FacetI facet = (FacetI) ((Test._FacetTie) o).ice_delegate();
		facet.init(remoteEvictor, evictor);
	    }
	}    
    }

    public void
    destroy(Ice.Current current)
    {
        _evictor.destroyObject(current.id);
    }

    public int
    getValue(Ice.Current current)
    {
	synchronized(_tie)
	{
	    return _tie.value;
	}
    }

    public void
    setValue(int value, Ice.Current current)
    {
	synchronized(_tie)
	{
	    _tie.value = value;
	}
    }

    public void
    setValueAsync_async(Test.AMD_Servant_setValueAsync __cb, int value, Ice.Current current)
    {
	synchronized(_tie)
	{
	    _setValueAsyncCB = __cb;
	    _setValueAsyncValue = value;
	}
    }

    public void
    releaseAsync(Ice.Current current)
    {
	synchronized(_tie)
	{
	    if(_setValueAsyncCB != null)
	    {
		_tie.value = _setValueAsyncValue;
		_setValueAsyncCB.ice_response();
		_setValueAsyncCB = null;
	    }
        }
    }

    public void
    addFacet(String name, String data, Ice.Current current)
	throws Test.AlreadyRegisteredException
    {
	String[] facetPath = new String[current.facet.length + 1];
	System.arraycopy(current.facet, 0, facetPath, 0, current.facet.length);
	facetPath[facetPath.length - 1] = name;

	Server.FacetTie tie = new Server.FacetTie();
	tie.ice_delegate(new FacetI(tie, _remoteEvictor, _evictor, 0, data));

	try
	{
	    _evictor.addFacet(current.id, facetPath, tie);
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

    void setLastSavedValue()
    {
	synchronized(_tie)
	{
	    _remoteEvictor.setLastSavedValue(_tie.value);
	}
    }
    
    protected RemoteEvictorI _remoteEvictor;
    protected Freeze.Evictor _evictor;
    protected Test.AMD_Servant_setValueAsync _setValueAsyncCB;
    protected int _setValueAsyncValue;
    protected Test.Servant _tie;
}
