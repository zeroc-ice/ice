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
    Direct(Ice.ObjectAdapter adapter, Ice.Current current)
    {
        _adapter = adapter;
        _current = current;

        try
        {
            _servant = _adapter.identityToServant(_current.id);

            if(_servant == null && _current.id.category.length() > 0)
            {
                _locator = _adapter.findServantLocator(_current.id.category);
                if(_locator != null)
                {
                    _cookie = new Ice.LocalObjectHolder(); // Lazy creation
                    _servant = _locator.locate(_adapter, _current, _cookie);
                }
            }

            if(_servant == null)
            {
                _locator = _adapter.findServantLocator("");
                if(_locator != null)
                {
                    _cookie = new Ice.LocalObjectHolder(); // Lazy creation
                    _servant = _locator.locate(_adapter, _current, _cookie);
                }
            }

            if(_servant != null && _current.facet.length > 0)
            {
                _facetServant = _servant.ice_findFacet(_current.facet[0]);
                if(_facetServant == null)
                {
		    Ice.FacetNotExistException ex = new Ice.FacetNotExistException();
		    ex.facet = _current.facet;
		    throw ex;
                }
            }
        }
        catch(RuntimeException ex)
        {
            if(_locator != null && _servant != null)
            {
                _locator.finished(_adapter, _current, _servant, _cookie.value);
            }
            throw ex;
        }

        if(_servant == null)
        {
            Ice.ObjectNotExistException ex = new Ice.ObjectNotExistException();
	    ex.id = _current.id;
	    throw ex;
        }
    }

    public void
    destroy()
    {
        if(_locator != null && _servant != null)
        {
            _locator.finished(_adapter, _current, _servant, _cookie.value);
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

    private Ice.ObjectAdapter _adapter;
    private Ice.Current _current;
    private Ice.Object _servant;
    private Ice.Object _facetServant;
    private Ice.ServantLocator _locator;
    private Ice.LocalObjectHolder _cookie;
}
