// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
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
    current.adapter = _adapter;
    current.id.__read(&_is);
    _is.read(current.facet);
    _is.read(current.operation);
    Byte b;
    _is.read(b);
    current.mode = static_cast<OperationMode>(b);
    Int sz;
    _is.readSize(sz);
    while(sz--)
    {
	pair<string, string> pr;
	_is.read(pr.first);
	_is.read(pr.second);
	current.ctx.insert(current.ctx.end(), pr);
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
	    servant = _adapter->identityToServant(current.id);
	    
	    if(!servant && !current.id.category.empty())
	    {
		locator = _adapter->findServantLocator(current.id.category);
		if(locator)
		{
		    servant = locator->locate(current, cookie);
		}
	    }
	    
	    if(!servant)
	    {
		locator = _adapter->findServantLocator("");
		if(locator)
		{
		    servant = locator->locate(current, cookie);
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
		ObjectPtr facetServant = servant->ice_findFacetPath(current.facet, 0);
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
	    locator->finished(current, servant, cookie);
	}
	
	_is.endReadEncaps();
	if(response)
	{
	    _os.endWriteEncaps();

	    if(status != DispatchOK && status != DispatchUserException)
	    {
		assert(status == DispatchObjectNotExist ||
		       status == DispatchFacetNotExist ||
		       status == DispatchOperationNotExist);
		       
		_os.b.resize(statusPos);
		_os.write(static_cast<Byte>(status));

		current.id.__write(&_os);
		_os.write(current.facet);
		_os.write(current.operation);
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
	    locator->finished(current, servant, cookie);
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
    catch(const RequestFailedException& ex)
    {
	if(locator && servant)
	{
	    assert(_adapter);
	    locator->finished(current, servant, cookie);
	}

	_is.endReadEncaps();
	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(statusPos);
	    if(dynamic_cast<const ObjectNotExistException*>(&ex))
	    {
		_os.write(static_cast<Byte>(DispatchObjectNotExist));
	    }
	    else if(dynamic_cast<const FacetNotExistException*>(&ex))
	    {
		_os.write(static_cast<Byte>(DispatchFacetNotExist));
	    }
	    else if(dynamic_cast<const OperationNotExistException*>(&ex))
	    {
		_os.write(static_cast<Byte>(DispatchOperationNotExist));
	    }
	    else
	    {
		assert(false);
	    }

            // Not current.id.__write(_os), so that the identity
            // can be overwritten.
	    ex.id.__write(&_os);
	    // Not _os.write(current.facet), so that the facet can
	    // be overwritten.
	    _os.write(ex.facet);
	    // Not _os.write(current.operation), so that the operation
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
	    locator->finished(current, servant, cookie);
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
	    locator->finished(current, servant, cookie);
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
    catch(...)
    {
	if(locator && servant)
	{
	    assert(_adapter);
	    locator->finished(current, servant, cookie);
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
