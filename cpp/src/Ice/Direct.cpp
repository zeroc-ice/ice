// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Direct.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/ServantLocator.h>
#include <Ice/Reference.h>
#include <Ice/Object.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::Direct::Direct(const ObjectAdapterPtr& adapter, const Current& current) :
    _adapter(adapter),
    _current(current)
{
    try
    {
	_servant = _adapter->identityToServant(_current.id);
    
	if(!_servant && !_current.id.category.empty())
	{
	    _locator = _adapter->findServantLocator(_current.id.category);
	    if(_locator)
	    {
		_servant = _locator->locate(_adapter, _current, _cookie);
	    }
	}

	if(!_servant)
	{
	    _locator = _adapter->findServantLocator("");
	    if(_locator)
	    {
		_servant = _locator->locate(_adapter, _current, _cookie);
	    }
	}
	
	if(_servant && !_current.facet.empty())
	{
	    _facetServant = _servant->ice_findFacet(_current.facet.front());
	    if(!_facetServant)
	    {
		FacetNotExistException ex(__FILE__, __LINE__);
		ex.facet = _current.facet;
		throw ex;
	    }
	}
    }
    catch(...)
    {
	if(_locator && _servant)
	{
	    _locator->finished(_adapter, _current, _servant, _cookie);
	}
	throw;
    }

    if(!_servant)
    {
	ObjectNotExistException ex(__FILE__, __LINE__);
	ex.id = _current.id;
	throw ex;
    }
}

IceInternal::Direct::~Direct()
{
    if(_locator && _servant)
    {
	_locator->finished(_adapter, _current, _servant, _cookie);
    }
}

const ObjectPtr&
IceInternal::Direct::facetServant()
{
    if(_facetServant)
    {
	return _facetServant;
    }
    else
    {
	return _servant;
    }
}
