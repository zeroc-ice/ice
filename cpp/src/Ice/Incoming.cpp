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
#include <Ice/LocalException.h>

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
IceInternal::Incoming::invoke(bool response)
{
    Current current;
    current.identity.__read(&_is);
    _is.read(current.facet);
    _is.read(current.operation);
    _is.read(current.nonmutating);
    Int sz;
    _is.readSize(sz);
    while(sz--)
    {
	pair<string, string> pair;
	_is.read(pair.first);
	_is.read(pair.second);
	current.context.insert(current.context.end(), pair);
    }

    BasicStream::Container::size_type statusPos = 0; // Initialize, to keep the compiler happy.
    if(response)
    {
	statusPos = _os.b.size();
	_os.write(static_cast<Byte>(0));
    }

    //
    // Input and output parameters are always sent in an
    // encapsulation, which makes it possible to forward requests as
    // blobs.
    //
    _is.startReadEncaps();
    if(response)
    {
	_os.startWriteEncaps();
    }

    ObjectPtr servant;
    ServantLocatorPtr locator;
    LocalObjectPtr cookie;
	
    try
    {
	if(_adapter)
	{
	    servant = _adapter->identityToServant(current.identity);
	    
	    if(!servant && !current.identity.category.empty())
	    {
		locator = _adapter->findServantLocator(current.identity.category);
		if(locator)
		{
		    servant = locator->locate(_adapter, current, cookie);
		}
	    }
	    
	    if(!servant)
	    {
		locator = _adapter->findServantLocator("");
		if(locator)
		{
		    servant = locator->locate(_adapter, current, cookie);
		}
	    }
	}
	    
	DispatchStatus status;

	if(!servant)
	{
	    status = DispatchObjectNotExist;
	}
	else
	{
	    if(!current.facet.empty())
	    {
		ObjectPtr facetServant = servant->ice_findFacet(current.facet);
		if(!facetServant)
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

	if(locator && servant)
	{
	    assert(_adapter);
	    locator->finished(_adapter, current, servant, cookie);
	}
	
	_is.endReadEncaps();
	if(response)
	{
	    _os.endWriteEncaps();

	    if(status != DispatchOK && status != DispatchUserException)
	    {
		_os.b.resize(statusPos);
		_os.write(static_cast<Byte>(status));

		if(status == DispatchObjectNotExist)
		{
		    current.identity.__write(&_os);
		}
		else if(status == DispatchFacetNotExist)
		{
		    _os.write(current.facet);
		}
		else if(status == DispatchOperationNotExist)
		{
		    _os.write(current.operation);
		}
	    }
	    else
	    {
		*(_os.b.begin() + statusPos) = static_cast<Byte>(status);
	    }
	}
    }
    catch(const LocationForward& ex)
    {
	if(locator && servant)
	{
	    assert(_adapter);
	    locator->finished(_adapter, current, servant, cookie);
	}

	_is.endReadEncaps();
	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(statusPos);
	    _os.write(static_cast<Byte>(DispatchLocationForward));
	    _os.write(ex._prx);
	}
    }
    catch(const ObjectNotExistException& ex)
    {
	if(locator && servant)
	{
	    assert(_adapter);
	    locator->finished(_adapter, current, servant, cookie);
	}

	_is.endReadEncaps();
	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(statusPos);
	    _os.write(static_cast<Byte>(DispatchObjectNotExist));
            // Not current.identity.__write(_os), so that the identity
            // can be overwritten.
	    ex.identity.__write(&_os);
	}

	// Rethrow, so that the caller can print a warning.
	ex.ice_throw();
    }
    catch(const FacetNotExistException& ex)
    {
	if(locator && servant)
	{
	    assert(_adapter);
	    locator->finished(_adapter, current, servant, cookie);
	}

	_is.endReadEncaps();
	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(statusPos);
	    _os.write(static_cast<Byte>(DispatchFacetNotExist));
	    // Not _os.write(current.facet), so that the identity can
	    // be overwritten.
	    _os.write(ex.facet);
	}

	// Rethrow, so that the caller can print a warning.
	ex.ice_throw();
    }
    catch(const OperationNotExistException& ex)
    {
	if(locator && servant)
	{
	    assert(_adapter);
	    locator->finished(_adapter, current, servant, cookie);
	}

	_is.endReadEncaps();
	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(statusPos);
	    _os.write(static_cast<Byte>(DispatchOperationNotExist));
	    // Not _os.write(current.operation), so that the identity
	    // can be overwritten.
	    _os.write(ex.operation);
	}

	// Rethrow, so that the caller can print a warning.
	ex.ice_throw();
    }
    catch(const LocalException& ex)
    {
	if(locator && servant)
	{
	    assert(_adapter);
	    locator->finished(_adapter, current, servant, cookie);
	}

	_is.endReadEncaps();
	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(statusPos);
	    _os.write(static_cast<Byte>(DispatchUnknownLocalException));
	}

	// Rethrow, so that the caller can print a warning.
	ex.ice_throw();
    }
    catch(const UserException& ex)
    {
	if(locator && servant)
	{
	    assert(_adapter);
	    locator->finished(_adapter, current, servant, cookie);
	}

	_is.endReadEncaps();
	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(statusPos);
	    _os.write(static_cast<Byte>(DispatchUnknownUserException));
	}

	// Rethrow, so that the caller can print a warning.
	ex.ice_throw();
    }
    catch (...)
    {
	if(locator && servant)
	{
	    assert(_adapter);
	    locator->finished(_adapter, current, servant, cookie);
	}

	_is.endReadEncaps();
	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(statusPos);
	    _os.write(static_cast<Byte>(DispatchUnknownException));
	}

	// Rethrow, so that the caller can print a warning.
	throw;
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
