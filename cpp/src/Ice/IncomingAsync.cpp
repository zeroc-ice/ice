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

#include <Ice/IncomingAsync.h>
#include <Ice/ServantLocator.h>
#include <Ice/Object.h>
#include <Ice/Incoming.h>
#include <Ice/Connection.h>
#include <Ice/LocalException.h>
//#include <Ice/Instance.h>
//#include <Ice/Properties.h>
//#include <Ice/LoggerUtil.h>
#include <Ice/Protocol.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(IncomingAsync* p) { p->__incRef(); }
void IceInternal::decRef(IncomingAsync* p) { p->__decRef(); }

void IceInternal::incRef(AMD_Object_ice_invoke* p) { p->__incRef(); }
void IceInternal::decRef(AMD_Object_ice_invoke* p) { p->__decRef(); }

IceInternal::IncomingAsync::IncomingAsync(Incoming& in) :
    _instance(in._is.instance()),
    _current(in._current),
    _servant(in._servant),
    _locator(in._locator),
    _cookie(in._cookie),
    _connection(in._connection),
    _response(in._response),
    _compress(in._compress),
    _is(_instance),
    _os(_instance)
{
    _is.swap(in._is);
    _os.swap(in._os);
}

void
IceInternal::IncomingAsync::__response(bool ok)
{
    if(_response)
    {
	_os.endWriteEncaps();

	if(ok)
	{
	    *(_os.b.begin() + headerSize + 4) = static_cast<Byte>(DispatchOK); // Dispatch status position.
	}
	else
	{
	    *(_os.b.begin() + headerSize + 4) = static_cast<Byte>(DispatchUserException); // Dispatch status position.
	}
    }

    finishInvoke();
}

void
IceInternal::IncomingAsync::__exception(const Exception& exc)
{
    try
    {
	exc.ice_throw();
    }
    catch(RequestFailedException& ex)
    {
	if(ex.id.name.empty())
	{
	    ex.id = _current.id;
	}
	
	if(ex.facet.empty() && !_current.facet.empty())
	{
	    ex.facet = _current.facet;
	}
	
	if(ex.operation.empty() && !_current.operation.empty())
	{
	    ex.operation = _current.operation;
	}

	if(_response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    if(dynamic_cast<ObjectNotExistException*>(&ex))
	    {
		_os.write(static_cast<Byte>(DispatchObjectNotExist));
	    }
	    else if(dynamic_cast<FacetNotExistException*>(&ex))
	    {
		_os.write(static_cast<Byte>(DispatchFacetNotExist));
	    }
	    else if(dynamic_cast<OperationNotExistException*>(&ex))
	    {
		_os.write(static_cast<Byte>(DispatchOperationNotExist));
	    }
	    else
	    {
		assert(false);
	    }
	    ex.id.__write(&_os);
	    _os.write(ex.facet);
	    _os.write(ex.operation);
	}

	finishInvoke();
    }
    catch(const LocalException& ex)
    {
	if(_response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownLocalException));
	    ostringstream str;
	    str << ex;
	    _os.write(str.str());
	}

	finishInvoke();
    }
    catch(const UserException& ex)
    {
	if(_response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownUserException));
	    ostringstream str;
	    str << ex;
	    _os.write(str.str());
	}

	finishInvoke();
    }
    catch(const Exception& ex)
    {
	if(_response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownException));
	    ostringstream str;
	    str << ex;
	    _os.write(str.str());
	}

	finishInvoke();
    }
}

void
IceInternal::IncomingAsync::__exception(const std::exception& ex)
{
    if(_response)
    {
	_os.endWriteEncaps();
	_os.b.resize(headerSize + 4); // Dispatch status position.
	_os.write(static_cast<Byte>(DispatchUnknownException));
	ostringstream str;
	str << "std::exception: " << ex.what();
	_os.write(str.str());
    }

    finishInvoke();
}

void
IceInternal::IncomingAsync::__exception()
{
    if(_response)
    {
	_os.endWriteEncaps();
	_os.b.resize(headerSize + 4); // Dispatch status position.
	_os.write(static_cast<Byte>(DispatchUnknownException));
	string reason = "unknown c++ exception";
	_os.write(reason);
    }

    finishInvoke();
}

BasicStream*
IceInternal::IncomingAsync::__is()
{
    return &_is;
}

BasicStream*
IceInternal::IncomingAsync::__os()
{
    return &_os;
}

void
IceInternal::IncomingAsync::finishInvoke()
{
    if(_locator && _servant)
    {
	_locator->finished(_current, _servant, _cookie);
    }

    _is.endReadEncaps();

    //
    // Send a response if necessary. If we don't need to send a
    // response, we still need to tell the connection that we're
    // finished with dispatching.
    //
    if(_response)
    {
	_connection->sendResponse(&_os, _compress);
    }
    else
    {
	_connection->sendNoResponse();
    }
}

IceAsync::Ice::AMD_Object_ice_invoke::AMD_Object_ice_invoke(Incoming& in) :
    IncomingAsync(in)
{
}

void
IceAsync::Ice::AMD_Object_ice_invoke::ice_response(bool ok, const ::std::vector< ::Ice::Byte>& outParams)
{
    try
    {
	__os()->writeBlob(outParams);
    }
    catch(const LocalException& ex)
    {
	__exception(ex);
	return;
    }
    
    __response(ok);
}

void
IceAsync::Ice::AMD_Object_ice_invoke::ice_exception(const Exception& ex)
{
    __exception(ex);
}

void
IceAsync::Ice::AMD_Object_ice_invoke::ice_exception(const std::exception& ex)
{
    __exception(ex);
}

void
IceAsync::Ice::AMD_Object_ice_invoke::ice_exception()
{
    __exception();
}
