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

package IceInternal;

public final class Direct
{
    public
    Direct(Ice.Current current)
    {
        _current = current;

	Ice.ObjectAdapterI adapter = (Ice.ObjectAdapterI)_current.adapter;
	assert(adapter != null);
	
	//
	// Must call incDirectCount() first, because it checks for
	// adapter deactivation, and prevents deactivation completion
	// until decDirectCount() is called. This is important,
	// because getServantManager() may not be called afer
	// deactivation completion.
	//
	adapter.incDirectCount();

	ServantManager servantManager = adapter.getServantManager();
	assert(servantManager != null);

        try
        {
	    _servant = servantManager.findServant(_current.id);
	    
	    if(_servant == null && _current.id.category.length() > 0)
	    {
		_locator = servantManager.findServantLocator(_current.id.category);
		if(_locator != null)
		{
		    _cookie = new Ice.LocalObjectHolder(); // Lazy creation.
		    _servant = _locator.locate(_current, _cookie);
		}
	    }
	    
	    if(_servant == null)
	    {
		_locator = servantManager.findServantLocator("");
		if(_locator != null)
		{
		    _cookie = new Ice.LocalObjectHolder(); // Lazy creation.
		    _servant = _locator.locate(_current, _cookie);
		}
	    }
	    
	    if(_servant == null)
	    {
		Ice.ObjectNotExistException ex = new Ice.ObjectNotExistException();
		ex.id = _current.id;
		ex.facet = _current.facet;
		ex.operation = _current.operation;
		throw ex;
	    }
	    
            if(_current.facet.length > 0)
            {
                _facetServant = _servant.ice_findFacetPath(_current.facet, 0);
                if(_facetServant == null)
                {
		    Ice.FacetNotExistException ex = new Ice.FacetNotExistException();
		    ex.id = _current.id;
		    ex.facet = _current.facet;
		    ex.operation = _current.operation;
		    throw ex;
                }
            }
        }
        catch(RuntimeException ex)
        {
	    try
	    {
		if(_locator != null && _servant != null)
		{
		    _locator.finished(_current, _servant, _cookie.value);
		}
		throw ex;
	    }
	    finally
	    {
		adapter.decDirectCount();
	    }
        }
    }

    public void
    destroy()
    {
	Ice.ObjectAdapterI adapter = (Ice.ObjectAdapterI)_current.adapter;
	assert(adapter != null);
	
	try
	{
	    if(_locator != null && _servant != null)
	    {
		_locator.finished(_current, _servant, _cookie.value);
	    }
	}
	finally
	{
	    adapter.decDirectCount();
	}
    }

    public Ice.Object
    facetServant()
    {
        if(_facetServant != null)
        {
            return _facetServant;
        }
        else
        {
            return _servant;
        }
    }

    private final Ice.Current _current;
    private Ice.Object _servant;
    private Ice.Object _facetServant;
    private Ice.ServantLocator _locator;
    private Ice.LocalObjectHolder _cookie;
}
