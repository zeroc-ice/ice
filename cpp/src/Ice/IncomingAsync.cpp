// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/IncomingAsync.h>
#include <Ice/ServantLocator.h>
#include <Ice/Object.h>
#include <Ice/ConnectionI.h>
#include <Ice/LocalException.h>
#include <Ice/Protocol.h>
#include <Ice/Instance.h>
#include <Ice/Properties.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(IncomingAsync* p) { p->__incRef(); }
void IceInternal::decRef(IncomingAsync* p) { p->__decRef(); }

void IceInternal::incRef(AMD_Object_ice_invoke* p) { p->__incRef(); }
void IceInternal::decRef(AMD_Object_ice_invoke* p) { p->__decRef(); }

void IceInternal::incRef(AMD_Array_Object_ice_invoke* p) { p->__incRef(); }
void IceInternal::decRef(AMD_Array_Object_ice_invoke* p) { p->__decRef(); }

IceInternal::IncomingAsync::IncomingAsync(Incoming& in) :
    IncomingBase(in),
    _instanceCopy(_os.instance()),
    _connectionCopy(_connection)
{
}

void
IceInternal::IncomingAsync::__response(bool ok)
{
    try
    {
	if(_locator && _servant)
	{
	    _locator->finished(_current, _servant, _cookie);
	}

	if(_response)
	{
	    _os.endWriteEncaps();
	    
	    if(ok)
	    {
		*(_os.b.begin() + headerSize + 4) = static_cast<Byte>(DispatchOK);
	    }
	    else
	    {
		*(_os.b.begin() + headerSize + 4) = static_cast<Byte>(DispatchUserException);
	    }

	    _connection->sendResponse(&_os, _compress);
	}
	else
	{
	    _connection->sendNoResponse();
	}
    }
    catch(const LocalException& ex)
    {
	_connection->exception(ex);
    }
    catch(const std::exception& ex)
    {
	UnknownException uex(__FILE__, __LINE__);
	uex.unknown = string("std::exception: ") + ex.what();
	_connection->exception(uex);
    }
    catch(...)
    {
	UnknownException uex(__FILE__, __LINE__);
	uex.unknown = "unknown c++ exception";
	_connection->exception(uex);
    }
}

void
IceInternal::IncomingAsync::__exception(const Exception& exc)
{
    try
    {
	if(_locator && _servant)
	{
	    _locator->finished(_current, _servant, _cookie);
	}

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
	    
	    if(_os.instance()->initializationData().properties->
	    		getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
	    {
		__warning(ex);
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
		
		//
		// For compatibility with the old FacetPath.
		//
		if(ex.facet.empty())
		{
		    _os.write(static_cast<string*>(0), static_cast<string*>(0));
		}
		else
		{
		    _os.write(&ex.facet, &ex.facet + 1);
		}
		
		_os.write(ex.operation);

		_connection->sendResponse(&_os, _compress);
	    }
	    else
	    {
		_connection->sendNoResponse();
	    }
	}
	catch(const UnknownLocalException& ex)
	{
	    if(_os.instance()->initializationData().properties->
	    		getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	    {
		__warning(ex);
	    }
	    
	    if(_response)
	    {
		_os.endWriteEncaps();
		_os.b.resize(headerSize + 4); // Dispatch status position.
		_os.write(static_cast<Byte>(DispatchUnknownLocalException));
		_os.write(ex.unknown);
		_connection->sendResponse(&_os, _compress);
	    }
	    else
	    {
		_connection->sendNoResponse();
	    }
	}
	catch(const UnknownUserException& ex)
	{
	    if(_os.instance()->initializationData().properties->
	    		getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	    {
		__warning(ex);
	    }

	    if(_response)
	    {
		_os.endWriteEncaps();
		_os.b.resize(headerSize + 4); // Dispatch status position.
		_os.write(static_cast<Byte>(DispatchUnknownUserException));
		_os.write(ex.unknown);
		_connection->sendResponse(&_os, _compress);
	    }
	    else
	    {
		_connection->sendNoResponse();
	    }
	}
	catch(const UnknownException& ex)
	{
	    if(_os.instance()->initializationData().properties->
	    		getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	    {
		__warning(ex);
	    }

	    if(_response)
	    {
		_os.endWriteEncaps();
		_os.b.resize(headerSize + 4); // Dispatch status position.
		_os.write(static_cast<Byte>(DispatchUnknownException));
		_os.write(ex.unknown);
		_connection->sendResponse(&_os, _compress);
	    }
	    else
	    {
		_connection->sendNoResponse();
	    }
	}
	catch(const LocalException& ex)
	{
	    if(_os.instance()->initializationData().properties->
	    		getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	    {
		__warning(ex);
	    }

	    if(_response)
	    {
		_os.endWriteEncaps();
		_os.b.resize(headerSize + 4); // Dispatch status position.
		_os.write(static_cast<Byte>(DispatchUnknownLocalException));
		ostringstream str;
		str << ex;
		_os.write(str.str());
		_connection->sendResponse(&_os, _compress);
	    }
	    else
	    {
		_connection->sendNoResponse();
	    }
	}
	catch(const UserException& ex)
	{
	    if(_os.instance()->initializationData().properties->
	    		getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	    {
		__warning(ex);
	    }

	    if(_response)
	    {
		_os.endWriteEncaps();
		_os.b.resize(headerSize + 4); // Dispatch status position.
		_os.write(static_cast<Byte>(DispatchUnknownUserException));
		ostringstream str;
		str << ex;
		_os.write(str.str());
		_connection->sendResponse(&_os, _compress);
	    }
	    else
	    {
		_connection->sendNoResponse();
	    }
	}
	catch(const Exception& ex)
	{
	    if(_os.instance()->initializationData().properties->
	    		getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	    {
		__warning(ex);
	    }

	    if(_response)
	    {
		_os.endWriteEncaps();
		_os.b.resize(headerSize + 4); // Dispatch status position.
		_os.write(static_cast<Byte>(DispatchUnknownException));
		ostringstream str;
		str << ex;
		_os.write(str.str());
		_connection->sendResponse(&_os, _compress);
	    }
	    else
	    {
		_connection->sendNoResponse();
	    }
	}
    }
    catch(const LocalException& ex)
    {
	_connection->exception(ex);
    }
    catch(const std::exception& ex)
    {
	UnknownException uex(__FILE__, __LINE__);
	uex.unknown = string("std::exception: ") + ex.what();
	_connection->exception(uex);
    }
    catch(...)
    {
	UnknownException uex(__FILE__, __LINE__);
	uex.unknown = "unknown c++ exception";
	_connection->exception(uex);
    }
}

void
IceInternal::IncomingAsync::__exception(const std::exception& ex)
{
    try
    {
	if(_os.instance()->initializationData().properties->
		getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	{
	    __warning(string("std::exception: ") + ex.what());
	}
	
	if(_response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownException));
	    ostringstream str;
	    str << "std::exception: " << ex.what();
	    _os.write(str.str());
	    _connection->sendResponse(&_os, _compress);
	}
	else
	{
	    _connection->sendNoResponse();
	}
    }
    catch(const LocalException& ex)
    {
	_connection->exception(ex);
    }
    catch(const std::exception& ex)
    {
	UnknownException uex(__FILE__, __LINE__);
	uex.unknown = string("std::exception: ") + ex.what();
	_connection->exception(uex);
    }
    catch(...)
    {
	UnknownException uex(__FILE__, __LINE__);
	uex.unknown = "unknown c++ exception";
	_connection->exception(uex);
    }
}

void
IceInternal::IncomingAsync::__exception()
{
    try
    {
	if(_os.instance()->initializationData().properties->
		getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	{
	    __warning("unknown c++ exception");
	}
	
	if(_response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownException));
	    string reason = "unknown c++ exception";
	    _os.write(reason);
	    _connection->sendResponse(&_os, _compress);
	}
	else
	{
	    _connection->sendNoResponse();
	}
    }
    catch(const LocalException& ex)
    {
	_connection->exception(ex);
    }
    catch(const std::exception& ex)
    {
	UnknownException uex(__FILE__, __LINE__);
	uex.unknown = string("std::exception: ") + ex.what();
	_connection->exception(uex);
    }
    catch(...)
    {
	UnknownException uex(__FILE__, __LINE__);
	uex.unknown = "unknown c++ exception";
	_connection->exception(uex);
    }
}

IceAsync::Ice::AMD_Object_ice_invoke::AMD_Object_ice_invoke(Incoming& in) :
    IncomingAsync(in)
{
}

void
IceAsync::Ice::AMD_Object_ice_invoke::ice_response(bool ok, const vector<Byte>& outParams)
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

IceAsync::Ice::AMD_Array_Object_ice_invoke::AMD_Array_Object_ice_invoke(Incoming& in) :
    IncomingAsync(in)
{
}

void
IceAsync::Ice::AMD_Array_Object_ice_invoke::ice_response(bool ok, const pair<const Byte*, const Byte*>& outParams)
{
    try
    {
	__os()->writeBlob(outParams.first, static_cast<Int>(outParams.second - outParams.first));
    }
    catch(const LocalException& ex)
    {
	__exception(ex);
	return;
    }
    __response(ok);
}

void
IceAsync::Ice::AMD_Array_Object_ice_invoke::ice_exception(const Exception& ex)
{
    __exception(ex);
}

void
IceAsync::Ice::AMD_Array_Object_ice_invoke::ice_exception(const std::exception& ex)
{
    __exception(ex);
}

void
IceAsync::Ice::AMD_Array_Object_ice_invoke::ice_exception()
{
    __exception();
}
