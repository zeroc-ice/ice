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
#include <Ice/Proxy.h>
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
    Current current;
    Byte gotProxy;
    _is.read(gotProxy);
    if (gotProxy)
    {
	_is.read(current.proxy);
	current.identity = current.proxy->ice_getIdentity();
	current.facet = current.proxy->ice_getFacet();
    }
    else
    {
	_is.read(current.identity);
	_is.read(current.facet);
    }
    _is.read(current.operation);
    Int sz;
    _is.read(sz);
    while (sz--)
    {
	pair<string, string> pair;
	_is.read(pair.first);
	_is.read(pair.second);
	current.context.insert(current.context.end(), pair);
    }

    BasicStream::Container::size_type statusPos = _os.b.size();
    _os.write(static_cast<Byte>(0));

    //
    // Input and output parameters are always sent in an
    // encapsulation, which makes it possible to forward oneway
    // requests as blobs.
    //
    _is.startReadEncaps();
    _os.startWriteEncaps();

    ObjectPtr servant;
    ServantLocatorPtr locator;
    LocalObjectPtr cookie;
	
    try
    {
	servant = _adapter->identityToServant(current.identity);
	DispatchStatus status;

	if (!servant)
	{
	    string::size_type pos = current.identity.find('#');
	    if (pos != string::npos)
	    {
		locator = _adapter->findServantLocator(current.identity.substr(0, pos));
		if (locator)
		{
		    servant = locator->locate(_adapter, current, cookie);
		}
	    }
	}

	if (!servant)
	{
	    locator = _adapter->findServantLocator("");
	    if (locator)
	    {
		servant = locator->locate(_adapter, current, cookie);
	    }
	}

	if (!servant)
	{
	    status = DispatchObjectNotExist;
	}
	else
	{
	    if (!current.facet.empty())
	    {
		ObjectPtr facetServant = servant->ice_findFacet(current.facet);
		if (!facetServant)
		{
		    status = DispatchFacetNotExist;
		}
		else
		{
		    status = facetServant->__dispatch(*this, current);
		}
	    }
	    else
	    {
		status = servant->__dispatch(*this, current);
	    }
	}

	if (locator && servant)
	{
	    locator->finished(_adapter, current, servant, cookie);
	}
	
	_is.endReadEncaps();
	_os.endWriteEncaps();
	
	if (status != DispatchOK && status != DispatchUserException)
	{
	    _os.b.resize(statusPos);
	    _os.write(static_cast<Byte>(status));
	}
	else
	{
	    *(_os.b.begin() + statusPos) = static_cast<Byte>(status);
	}
    }
    catch (const LocationForward& ex)
    {
	if (locator && servant)
	{
	    locator->finished(_adapter, current, servant, cookie);
	}

	_is.endReadEncaps();
	_os.endWriteEncaps();

	_os.b.resize(statusPos);
	_os.write(static_cast<Byte>(DispatchLocationForward));
	_os.write(ex._prx);
    }
    catch (const LocalException& ex)
    {
	if (locator && servant)
	{
	    locator->finished(_adapter, current, servant, cookie);
	}

	_is.endReadEncaps();
	_os.endWriteEncaps();

	_os.b.resize(statusPos);
	_os.write(static_cast<Byte>(DispatchUnknownLocalException));

	ex.ice_throw();
    }
    catch (const UserException& ex)
    {
	if (locator && servant)
	{
	    locator->finished(_adapter, current, servant, cookie);
	}

	_is.endReadEncaps();
	_os.endWriteEncaps();

	_os.b.resize(statusPos);
	_os.write(static_cast<Byte>(DispatchUnknownUserException));

	ex.ice_throw();
    }
    catch (...)
    {
	if (locator && servant)
	{
	    locator->finished(_adapter, current, servant, cookie);
	}

	_is.endReadEncaps();
	_os.endWriteEncaps();

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
