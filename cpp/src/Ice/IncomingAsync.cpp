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
#include <Ice/ObjectAdapterI.h> // We need ObjectAdapterI, not ObjectAdapter, because of inc/decUsageCount().
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
    _current(in._current),
    _servant(in._servant),
    _locator(in._locator),
    _cookie(in._cookie),
    _connection(in._connection),
    _compress(in._compress),
    _instance(in._is.instance()),
    _is(_instance),
    _os(_instance)
{
    _is.swap(in._is);
    _os.swap(in._os);
}

void
IceInternal::IncomingAsync::__response(bool ok)
{
    finishInvoke();

    if(_connection) // Response expected?
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

	_connection->sendResponse(&_os, _compress);
    }
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
	finishInvoke();

	if(_connection) // Response expected?
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
	    
	    //
            // Write the data from the exception if set so that a
            // RequestFailedException can override the information
            // from _current.
	    //
	    if(!ex.id.name.empty())
	    {
		ex.id.__write(&_os);
	    }
	    else
	    {
		_current.id.__write(&_os);
	    }

	    if(!ex.facet.empty())
	    {
		_os.write(ex.facet);
	    }
	    else
	    {
		_os.write(_current.facet);
	    }

	    if(ex.operation.empty())
	    {
		_os.write(ex.operation);
	    }
	    else
	    {
		_os.write(_current.operation);
	    }

	    _connection->sendResponse(&_os, _compress);
	}

/*
	if(_instance->properties()->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
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

	    Warning out(_instance->logger());
	    out << "dispatch exception:\n" << ex;
	}
*/
    }
    catch(const LocalException& ex)
    {
	finishInvoke();

	if(_connection) // Response expected?
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownLocalException));
	    ostringstream str;
	    str << ex;
	    _os.write(str.str());

	    _connection->sendResponse(&_os, _compress);
	}
/*
	if(_instance->properties()->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	{
	    ostringstream str;
	    str << ex;
	    warning("dispatch exception: unknown local exception:", str.str());
	}
*/
    }
    catch(const UserException& ex)
    {
	finishInvoke();

	if(_connection) // Response expected?
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownUserException));
	    ostringstream str;
	    str << ex;
	    _os.write(str.str());

	    _connection->sendResponse(&_os, _compress);
	}

/*
	if(_instance->properties()->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	{
	    ostringstream str;
	    str << ex;
	    warning("dispatch exception: unknown user exception:", str.str());
	}
*/
    }
    catch(const Exception& ex)
    {
	finishInvoke();

	if(_connection) // Response expected?
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownException));
	    ostringstream str;
	    str << ex;
	    _os.write(str.str());

	    _connection->sendResponse(&_os, _compress);
	}

/*
	if(_instance->properties()->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	{
	    ostringstream str;
	    str << ex;
	    warning("dispatch exception: unknown exception:", str.str());
	}
*/
    }
}

void
IceInternal::IncomingAsync::__fatal(const LocalException& ex)
{
    if(_connection)
    {
	_connection->exception(ex);
    }
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
    
    _is.endReadEncaps();
}

Ice::AMD_Object_ice_invoke::AMD_Object_ice_invoke(Incoming& in) :
    IncomingAsync(in)
{
}

void
Ice::AMD_Object_ice_invoke::ice_response(bool ok, const ::std::vector< ::Ice::Byte>& outParams)
{
    try
    {
	__os()->writeBlob(outParams);
    }
    catch(const LocalException& ex)
    {
	ice_exception(ex);
	return;
    }
    
    __response(ok);
}

void
Ice::AMD_Object_ice_invoke::ice_exception(const Exception& ex)
{
    __exception(ex);
}
