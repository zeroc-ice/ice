// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
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
#include <Ice/Exception.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::Direct::Direct(const ObjectAdapterPtr& adapter, const ReferencePtr& ref, const char* operation) :
    _adapter(adapter)
{
    _current.identity = ref->identity;
    _current.facet = ref->facet;
    _current.operation = operation;

    try
    {
	_servant = _adapter->identityToServant(_current.identity);
    
	if (!_servant)
	{
	    string::size_type pos = _current.identity.find('#');
	    if (pos != string::npos)
	    {
		_locator = _adapter->findServantLocator(_current.identity.substr(0, pos));
		if (_locator)
		{
		    _servant = _locator->locate(_adapter, _current, _cookie);
		}
	    }
	}

	if (!_servant)
	{
	    _locator = _adapter->findServantLocator("");
	    if (_locator)
	    {
		_servant = _locator->locate(_adapter, _current, _cookie);
	    }
	}
	
	if (_servant && !ref->facet.empty())
	{
	    _facetServant = _servant->ice_findFacet(ref->facet);
	    if (!_facetServant)
	    {
		throw FacetNotExistException(__FILE__, __LINE__);
	    }
	}
    }
    catch(...)
    {
	if (_locator && _servant)
	{
	    _locator->finished(_adapter, _current, _servant, _cookie);
	}
	throw;
    }

    if(!_servant)
    {
	throw ObjectNotExistException(__FILE__, __LINE__);
    }
}

IceInternal::Direct::~Direct()
{
    if (_locator && _servant)
    {
	_locator->finished(_adapter, _current, _servant, _cookie);
    }
}

const ObjectPtr&
IceInternal::Direct::facetServant()
{
    if (_facetServant)
    {
	return _facetServant;
    }
    else
    {
	return _servant;
    }
}
