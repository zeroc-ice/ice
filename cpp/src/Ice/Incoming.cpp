// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Incoming.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/ServantLocator.h>
#include <Ice/Object.h>
#include <Ice/Exception.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::Incoming::Incoming(const InstancePtr& instance, const ObjectAdapterPtr& adapter) :
    _adapter(adapter),
    _is(instance),
    _os(instance)
{
}

void
IceInternal::Incoming::invoke()
{
    string identity;
    _is.read(identity);
    string facet;
    _is.read(facet);
    string operation;
    _is.read(operation);

    BasicStream::Container::size_type statusPos = _os.b.size();

    ObjectPtr servant;
    ServantLocatorPtr locator;
    LocalObjectPtr cookie;
    try
    {
	//
	// Input parameters are always sent in an encapsulation,
	// which makes it possible to forward oneway requests as
	// blobs.
	//
	_is.startReadEncaps();

	servant = _adapter->identityToServant(identity);

	if (!servant)
	{
	    string::size_type pos = identity.find('#');
	    if (pos != string::npos)
	    {
		locator = _adapter->findServantLocator(identity.substr(0, pos));
		if (locator)
		{
		    servant = locator->locate(_adapter, identity, operation, cookie);
		}
	    }
	}

	if (!servant)
	{
	    locator = _adapter->findServantLocator("");
	    if (locator)
	    {
		servant = locator->locate(_adapter, identity, operation, cookie);
	    }
	}

	if(!servant)
	{
	    _os.write(static_cast<Byte>(DispatchObjectNotExist));
	}
	else
	{
	    if (!facet.empty())
	    {
		ObjectPtr facetServant = servant->ice_findFacet(facet);
		if (!facetServant)
		{
		    _os.write(static_cast<Byte>(DispatchFacetNotExist));
		}
		else
		{
		    _os.write(static_cast<Byte>(DispatchOK));
		    DispatchStatus status = facetServant->__dispatch(*this, identity, facet, operation);
		    _is.checkReadEncaps();
		    *(_os.b.begin() + statusPos) = static_cast<Byte>(status);
		}
	    }
	    else
	    {
		_os.write(static_cast<Byte>(DispatchOK));
		DispatchStatus status = servant->__dispatch(*this, identity, facet, operation);
		_is.checkReadEncaps();
		*(_os.b.begin() + statusPos) = static_cast<Byte>(status);
	    }
	}

	_is.endReadEncaps();
	if (locator && servant)
	{
	    locator->finished(_adapter, identity, operation, servant, cookie);
	}
    }
    catch (const LocationForward& ex)
    {
	_is.endReadEncaps();
	if (locator && servant)
	{
	    locator->finished(_adapter, identity, operation, servant, cookie);
	}
	_os.b.resize(statusPos);
	_os.write(static_cast<Byte>(DispatchLocationForward));
	_os.write(ex._prx);
	return;
    }
    catch (const LocalException& ex)
    {
	_is.endReadEncaps();
	if (locator && servant)
	{
	    locator->finished(_adapter, identity, operation, servant, cookie);
	}
	_os.b.resize(statusPos);
	_os.write(static_cast<Byte>(DispatchUnknownLocalException));
	ex.ice_throw();
    }
    catch (const UserException& ex)
    {
	_is.endReadEncaps();
	if (locator && servant)
	{
	    locator->finished(_adapter, identity, operation, servant, cookie);
	}
	_os.b.resize(statusPos);
	_os.write(static_cast<Byte>(DispatchUnknownUserException));
	ex.ice_throw();
    }
    catch (...)
    {
	_is.endReadEncaps();
	if (locator && servant)
	{
	    locator->finished(_adapter, identity, operation, servant, cookie);
	}
	_os.b.resize(statusPos);
	_os.write(static_cast<Byte>(DispatchUnknownException));
	throw UnknownException(__FILE__, __LINE__);
    }
}

BasicStream*
IceInternal::Incoming::is()
{
    return &_is;
}

BasicStream*
IceInternal::Incoming::os()
{
    return &_os;
}
