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

IceInternal::Direct::Direct(const Current& current) :
    _current(current)
{
    try
    {
	_servant = current.adapter->identityToServant(_current.id);
    
	if(!_servant && !_current.id.category.empty())
	{
	    _locator = current.adapter->findServantLocator(_current.id.category);
	    if(_locator)
	    {
		_servant = _locator->locate(_current, _cookie);
	    }
	}

	if(!_servant)
	{
	    _locator = current.adapter->findServantLocator("");
	    if(_locator)
	    {
		_servant = _locator->locate(_current, _cookie);
	    }
	}
	
	if(_servant && !_current.facet.empty())
	{
	    _facetServant = _servant->ice_findFacetPath(_current.facet, 0);
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
	    _locator->finished(_current, _servant, _cookie);
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
	_locator->finished(_current, _servant, _cookie);
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
