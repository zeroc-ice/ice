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

namespace IceInternal
{

    using System.Diagnostics;

    public sealed class Direct
    {
	public Direct(Ice.Current current)
	{
	    _current = current;
	    
	    Ice.ObjectAdapterI adapter = (Ice.ObjectAdapterI)_current.adapter;
	    Debug.Assert(adapter != null);
	    
	    //
	    // Must call incDirectCount() first, because it checks for
	    // adapter deactivation, and prevents deactivation completion
	    // until decDirectCount() is called. This is important,
	    // because getServantManager() may not be called afer
	    // deactivation completion.
	    //
	    adapter.incDirectCount();
	    
	    ServantManager servantManager = adapter.getServantManager();
	    Debug.Assert(servantManager != null);
	    
	    try
	    {
		_servant = servantManager.findServant(_current.id);
		
		if(_servant == null && _current.id.category.Length > 0)
		{
		    _locator = servantManager.findServantLocator(_current.id.category);
		    if(_locator != null)
		    {
			_servant = _locator.locate(_current, out _cookie);
		    }
		}
		
		if(_servant == null)
		{
		    _locator = servantManager.findServantLocator("");
		    if(_locator != null)
		    {
			_servant = _locator.locate(_current, out _cookie);
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
		
		if(_current.facet.Count > 0)
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
	    catch(System.Exception ex)
	    {
		try
		{
		    if(_locator != null && _servant != null)
		    {
			_locator.finished(_current, _servant, _cookie);
		    }
		    throw ex;
		}
		finally
		{
		    adapter.decDirectCount();
		}
	    }
	}
	
	public void destroy()
	{
	    Ice.ObjectAdapterI adapter = (Ice.ObjectAdapterI)_current.adapter;
	    Debug.Assert(adapter != null);
	    
	    try
	    {
		if(_locator != null && _servant != null)
		{
		    _locator.finished(_current, _servant, _cookie);
		}
	    }
	    finally
	    {
		adapter.decDirectCount();
	    }
	}
	
	public Ice.Object facetServant()
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
	
	private Ice.Current _current;
	private Ice.Object _servant;
	private Ice.Object _facetServant;
	private Ice.ServantLocator _locator;
	private Ice.LocalObject _cookie;
    }

}
