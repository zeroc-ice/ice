// **********************************************************************
//
// Copyright (c) 2002
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

#include <Ice/Direct.h>
#include <Ice/ObjectAdapterI.h> // For inc/decDirectCount().
#include <Ice/ServantManager.h>
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
    ObjectAdapterI* adapter = dynamic_cast<ObjectAdapterI*>(_current.adapter.get());
    assert(adapter);

    ServantManagerPtr servantManager = adapter->getServantManager();
    assert(servantManager);

    adapter->incDirectCount();
    try
    {
	_servant = servantManager->findServant(_current.id);
	
	if(!_servant && !_current.id.category.empty())
	{
	    _locator = servantManager->findServantLocator(_current.id.category);
	    if(_locator)
	    {
		_servant = _locator->locate(_current, _cookie);
	    }
	}
	
	if(!_servant)
	{
	    _locator = servantManager->findServantLocator("");
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
		adapter->decDirectCount();
		throw;
	    }
	}

	adapter->decDirectCount();
	throw;
    }
}

IceInternal::Direct::~Direct()
{
    ObjectAdapterI* adapter = dynamic_cast<ObjectAdapterI*>(_current.adapter.get());
    assert(adapter);

    if(_locator && _servant)
    {
	try
	{
	    _locator->finished(_current, _servant, _cookie);
	}
	catch(...)
	{
	    adapter->decDirectCount();
	    throw;
	}
    }

    adapter->decDirectCount();
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
