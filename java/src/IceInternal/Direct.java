// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public final class Direct
{
    public
    Direct(Ice.Current current)
    {
        _current = current;

	((Ice.ObjectAdapterI)(_current.adapter)).incUsageCount();

        try
        {
            _servant = _current.adapter.identityToServant(_current.id);

            if(_servant == null && _current.id.category.length() > 0)
            {
                _locator = _current.adapter.findServantLocator(_current.id.category);
                if(_locator != null)
                {
                    _cookie = new Ice.LocalObjectHolder(); // Lazy creation
                    _servant = _locator.locate(_current, _cookie);
                }
            }

            if(_servant == null)
            {
                _locator = _current.adapter.findServantLocator("");
                if(_locator != null)
                {
                    _cookie = new Ice.LocalObjectHolder(); // Lazy creation
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
		((Ice.ObjectAdapterI)(_current.adapter)).decUsageCount();
	    }
        }
    }

    public void
    destroy()
    {
	try
	{
	    if(_locator != null && _servant != null)
	    {
		_locator.finished(_current, _servant, _cookie.value);
	    }
	}
	finally
	{
	    ((Ice.ObjectAdapterI)(_current.adapter)).decUsageCount();
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

    private Ice.Current _current;
    private Ice.Object _servant;
    private Ice.Object _facetServant;
    private Ice.ServantLocator _locator;
    private Ice.LocalObjectHolder _cookie;
}
