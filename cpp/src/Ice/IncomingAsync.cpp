// **********************************************************************
//
// Copyright (c) 2003
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
#include <Ice/Connection.h>
#include <Ice/LocalException.h>
#include <Ice/Protocol.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(IncomingAsync* p) { p->__incRef(); }
void IceInternal::decRef(IncomingAsync* p) { p->__decRef(); }

void IceInternal::incRef(AMD_Object_ice_invoke* p) { p->__incRef(); }
void IceInternal::decRef(AMD_Object_ice_invoke* p) { p->__decRef(); }

IceInternal::IncomingAsync::IncomingAsync(Incoming& in) :
    IncomingBase(in),
    _finished(false),
    _instanceCopy(_is.instance()),
    _connectionCopy(_connection)
{
}

void
IceInternal::IncomingAsync::__response(bool ok)
{
    assert(!_finished);
    _finished = true;

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

    __finishInvoke(true);
}

void
IceInternal::IncomingAsync::__exception(const Exception& exc)
{
    assert(!_finished);
    _finished = true;

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

	__warning(ex);

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
    }
    catch(const LocalException& ex)
    {
	__warning(ex);

	if(_response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownLocalException));
	    ostringstream str;
	    str << ex;
	    _os.write(str.str());
	}
    }
    catch(const UserException& ex)
    {
	__warning(ex);

	if(_response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownUserException));
	    ostringstream str;
	    str << ex;
	    _os.write(str.str());
	}
    }
    catch(const Exception& ex)
    {
	__warning(ex);

	if(_response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownException));
	    ostringstream str;
	    str << ex;
	    _os.write(str.str());
	}
    }

    __finishInvoke(false);
}

void
IceInternal::IncomingAsync::__exception(const std::exception& ex)
{
    assert(!_finished);
    _finished = true;

    if(_response)
    {
	_os.endWriteEncaps();
	_os.b.resize(headerSize + 4); // Dispatch status position.
	_os.write(static_cast<Byte>(DispatchUnknownException));
	ostringstream str;
	str << "std::exception: " << ex.what();
	_os.write(str.str());
    }

    __finishInvoke(false);
}

void
IceInternal::IncomingAsync::__exception()
{
    assert(!_finished);
    _finished = true;

    if(_response)
    {
	_os.endWriteEncaps();
	_os.b.resize(headerSize + 4); // Dispatch status position.
	_os.write(static_cast<Byte>(DispatchUnknownException));
	string reason = "unknown c++ exception";
	_os.write(reason);
    }

    __finishInvoke(false);
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
