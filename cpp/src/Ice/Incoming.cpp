// **********************************************************************
//
// Copyright (c) 2001
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

#include <Ice/Incoming.h>
#include <Ice/ObjectAdapterI.h> // We need ObjectAdapterI, not ObjectAdapter, because of inc/decUsageCount().
#include <Ice/ServantLocator.h>
#include <Ice/Object.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::Incoming::Incoming(const InstancePtr& instance, const ObjectAdapterPtr& adapter) :
    _is(instance),
    _os(instance)
{
    _current.adapter = adapter;
    if(_current.adapter)
    {
	dynamic_cast<ObjectAdapterI*>(_current.adapter.get())->incUsageCount();
    }
}

IceInternal::Incoming::~Incoming()
{
    if(_current.adapter)
    {
	dynamic_cast<ObjectAdapterI*>(_current.adapter.get())->decUsageCount();
    }
}

void
IceInternal::Incoming::invoke(bool response)
{
    //
    // Read the current.
    //
    _current.id.__read(&_is);
    _is.read(_current.facet);
    _is.read(_current.operation);
    Byte b;
    _is.read(b);
    _current.mode = static_cast<OperationMode>(b);
    Int sz;
    _is.readSize(sz);
    while(sz--)
    {
	pair<string, string> pr;
	_is.read(pr.first);
	_is.read(pr.second);
	_current.ctx.insert(_current.ctx.end(), pr);
    }

    _is.startReadEncaps();

    BasicStream::Container::size_type statusPos;
    if(response)
    {
	statusPos = _os.b.size();
	_os.write(static_cast<Byte>(0));
	_os.startWriteEncaps();
    }
    else
    {
	statusPos = 0; // Initialize, to keep the compiler happy.
    }

    ObjectPtr servant;
    ServantLocatorPtr locator;
    LocalObjectPtr cookie;
    DispatchStatus status;

    //
    // Don't put the code above into the try block below. Exceptions
    // in the code above are considered fatal, and must propagate to
    // the caller of this operation.
    //

    try
    {
	if(_current.adapter)
	{
	    servant = _current.adapter->identityToServant(_current.id);
	    
	    if(!servant && !_current.id.category.empty())
	    {
		locator = _current.adapter->findServantLocator(_current.id.category);
		if(locator)
		{
		    servant = locator->locate(_current, cookie);
		}
	    }
	    
	    if(!servant)
	    {
		locator = _current.adapter->findServantLocator("");
		if(locator)
		{
		    servant = locator->locate(_current, cookie);
		}
	    }
	}
	    
	if(!servant)
	{
	    status = DispatchObjectNotExist;
	}
	else
	{
	    if(!_current.facet.empty())
	    {
		ObjectPtr facetServant = servant->ice_findFacetPath(_current.facet, 0);
		if(!facetServant)
		{
		    status = DispatchFacetNotExist;
		}
		else
		{
		    status = facetServant->__dispatch(*this, _current);
		}
	    }
	    else
	    {
		status = servant->__dispatch(*this, _current);
	    }
	}
    }
    catch(const LocationForward& ex)
    {
	if(locator && servant)
	{
	    locator->finished(_current, servant, cookie);
	}

	_is.endReadEncaps();

	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(statusPos);
	    _os.write(static_cast<Byte>(DispatchLocationForward));
	    _os.write(ex._prx);
	}

	return;
    }
    catch(const RequestFailedException& ex)
    {
	if(locator && servant)
	{
	    locator->finished(_current, servant, cookie);
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
            // Write the data from the exception, not from _current,
            // so that a RequestFailedException can override the
            // information from _current.
	    ex.id.__write(&_os);
	    _os.write(ex.facet);
	    _os.write(ex.operation);
	}

	return;
    }
    catch(const LocalException& ex)
    {
	if(locator && servant)
	{
	    locator->finished(_current, servant, cookie);
	}

	_is.endReadEncaps();

	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(statusPos);
	    _os.write(static_cast<Byte>(DispatchUnknownLocalException));
	    ostringstream str;
	    str << ex;
	    _os.write(str.str());
	}

	return;
    }
    catch(const UserException& ex)
    {
	if(locator && servant)
	{
	    locator->finished(_current, servant, cookie);
	}

	_is.endReadEncaps();

	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(statusPos);
	    _os.write(static_cast<Byte>(DispatchUnknownUserException));
	    ostringstream str;
	    str << ex;
	    _os.write(str.str());
	}

	return;
    }
    catch(const Exception& ex)
    {
	if(locator && servant)
	{
	    locator->finished(_current, servant, cookie);
	}

	_is.endReadEncaps();

	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(statusPos);
	    _os.write(static_cast<Byte>(DispatchUnknownException));
	    ostringstream str;
	    str << ex;
	    _os.write(str.str());
	}

	return;
    }
    catch(const std::exception& ex)
    {
	if(locator && servant)
	{
	    locator->finished(_current, servant, cookie);
	}
	
	_is.endReadEncaps();

	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(statusPos);
	    _os.write(static_cast<Byte>(DispatchUnknownException));
	    ostringstream str;
	    str << "std::exception: " << ex.what();
	    _os.write(str.str());
	}

	return;
    }
    catch(...)
    {
	if(locator && servant)
	{
	    locator->finished(_current, servant, cookie);
	}

	_is.endReadEncaps();

	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(statusPos);
	    _os.write(static_cast<Byte>(DispatchUnknownException));
	    string reason = "unknown c++ exception";
	    _os.write(reason);
	}

	return;
    }


    //
    // Don't put the code below into the try block above. Exceptions
    // in the code below are considered fatal, and must propagate to
    // the caller of this operation.
    //

    if(locator && servant)
    {
	locator->finished(_current, servant, cookie);
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
	    
	    _current.id.__write(&_os);
	    _os.write(_current.facet);
	    _os.write(_current.operation);
	}
	else
	{
	    *(_os.b.begin() + statusPos) = static_cast<Byte>(status);
	}
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
