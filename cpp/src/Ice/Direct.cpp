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
#include <Ice/ObjectAdapterI.h> // We need ObjectAdapterI, not ObjectAdapter, because of inc/decUsageCount().
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
    dynamic_cast<ObjectAdapterI*>(_current.adapter.get())->incUsageCount();

    try
    {
	_servant = _current.adapter->identityToServant(_current.id);
    
	if(!_servant && !_current.id.category.empty())
	{
	    _locator = _current.adapter->findServantLocator(_current.id.category);
	    if(_locator)
	    {
		_servant = _locator->locate(_current, _cookie);
	    }
	}

	if(!_servant)
	{
	    _locator = _current.adapter->findServantLocator("");
	    if(_locator)
	    {
		_servant = _locator->locate(_current, _cookie);
	    }
	}
	
	if(!_servant)
	{
	    ObjectNotExistException ex(__FILE__, __LINE__);
	    ex.id = _current.id;
	    ex.facet = _current.facet;
	    ex.operation = _current.operation;
	    throw ex;
	}

	if(!_current.facet.empty())
	{
	    _facetServant = _servant->ice_findFacetPath(_current.facet, 0);
	    if(!_facetServant)
	    {
		FacetNotExistException ex(__FILE__, __LINE__);
		ex.id = _current.id;
		ex.facet = _current.facet;
		ex.operation = _current.operation;
		throw ex;
	    }
	}
    }
    catch(...)
    {
	if(_locator && _servant)
	{
	    try
	    {
		_locator->finished(_current, _servant, _cookie);
	    }
	    catch(...)
	    {
		dynamic_cast<ObjectAdapterI*>(_current.adapter.get())->decUsageCount();
		throw;
	    }
	}

	dynamic_cast<ObjectAdapterI*>(_current.adapter.get())->decUsageCount();
	throw;
    }
}

IceInternal::Direct::~Direct()
{
    if(_locator && _servant)
    {
	try
	{
	    _locator->finished(_current, _servant, _cookie);
	}
	catch(...)
	{
	    dynamic_cast<ObjectAdapterI*>(_current.adapter.get())->decUsageCount();
	    throw;
	}
    }

    dynamic_cast<ObjectAdapterI*>(_current.adapter.get())->decUsageCount();
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
