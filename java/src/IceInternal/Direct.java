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

public final class Direct
{
    public
    Direct(Ice.ObjectAdapter adapter, Reference ref, String operation)
    {
        _adapter = adapter;
        _reference = ref;
        _operation = operation;

        try
        {
            _servant = _adapter.identityToServant(_reference.identity);

            if (_servant == null)
            {
                int pos = _reference.identity.indexOf('#');
                if (pos != -1)
                {
                    _locator = _adapter.findServantLocator(
                        _reference.identity.substring(0, pos));
                    if (_locator != null)
                    {
                        _servant = _locator.locate(_adapter,
                                                   _reference.identity,
                                                   _operation,
                                                   _cookie);
                    }
                }
            }

            if (_servant == null)
            {
                _locator = _adapter.findServantLocator("");
                if (_locator != null)
                {
                        _servant = _locator.locate(_adapter,
                                                   _reference.identity,
                                                   _operation,
                                                   _cookie);
                }
            }

            if (_servant != null && ref.facet.length() > 0)
            {
                _facetServant = _servant.ice_findFacet(ref.facet);
                if (_facetServant == null)
                {
                    throw new Ice.FacetNotExistException();
                }
            }
        }
        finally
        {
            if (_locator != null && _servant != null)
            {
                _locator.finished(_adapter, _reference.identity, _operation,
                                  _servant, _cookie.value);
            }
        }

        if (_servant == null)
        {
            throw new Ice.ObjectNotExistException();
        }
    }

    protected void
    finalize()
        throws Throwable
    {
        if (_locator != null && _servant != null)
        {
            _locator.finished(_adapter, _reference.identity, _operation,
                              _servant, _cookie.value);
        }
    }

    public Ice.Object
    facetServant()
    {
        if (_facetServant != null)
        {
            return _facetServant;
        }
        else
        {
            return _servant;
        }
    }

    private Ice.ObjectAdapter _adapter;
    private Reference _reference;
    private String _operation;
    private Ice.Object _servant;
    private Ice.Object _facetServant;
    private Ice.ServantLocator _locator;
    private Ice.LocalObjectHolder _cookie = new Ice.LocalObjectHolder();
}
