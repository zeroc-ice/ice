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

#include <Ice/Incoming.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/ServantLocator.h>
#include <Ice/ServantManager.h>
#include <Ice/Object.h>
#include <Ice/Connection.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/Properties.h>
#include <Ice/IdentityUtil.h>
#include <Ice/LoggerUtil.h>
#include <Ice/StringUtil.h>
#include <Ice/Protocol.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::IncomingBase::IncomingBase(Instance* instance, Connection* connection, 
					const ObjectAdapterPtr& adapter,
					bool response, Byte compress) :
    _response(response),
    _compress(compress),
    _is(instance),
    _os(instance),
    _connection(connection)
{
    _current.adapter = adapter;
}

IceInternal::IncomingBase::IncomingBase(IncomingBase& in) :
    _current(in._current),
    _servant(in._servant),
    _locator(in._locator),
    _cookie(in._cookie),
    _response(in._response),
    _compress(in._compress),
    _is(in._is.instance()),
    _os(in._os.instance()),
    _connection(in._connection)
{
    _is.swap(in._is);
    _os.swap(in._os);
}

void
IceInternal::IncomingBase::__finishInvoke(bool success)
{
    if(_locator && _servant)
    {
	_locator->finished(_current, _servant, _cookie);
    }
    
    if(success)
    {
	_is.endReadEncaps();
    }
    else
    {
	_is.skipReadEncaps();
    }
    
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

void
IceInternal::IncomingBase::__warning(const Exception& ex) const
{
    ostringstream str;
    str << ex;
    __warning(str.str());
}

void
IceInternal::IncomingBase::__warning(const string& msg) const
{
    if(_os.instance()->properties()->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
    {
	Warning out(_os.instance()->logger());
	
	out << "dispatch exception: " << msg;
	out << "\nidentity: " << _current.id;
	out << "\nfacet: ";
	vector<string>::const_iterator p = _current.facet.begin();
	while(p != _current.facet.end())
	{
	    out << encodeString(*p++, "/");
	    if(p != _current.facet.end())
	    {
		out << '/';
	    }
	}
	out << "\noperation: " << _current.operation;
    }
}

IceInternal::Incoming::Incoming(Instance* instance, Connection* connection, 
				const ObjectAdapterPtr& adapter,
				bool response, Byte compress) :
    IncomingBase(instance, connection, adapter, response, compress)
{
}

void
IceInternal::Incoming::invoke(const ServantManagerPtr& servantManager)
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
	pair<const string, string> pr;
	_is.read(const_cast<string&>(pr.first));
	_is.read(pr.second);
	_current.ctx.insert(_current.ctx.end(), pr);
    }

    _is.startReadEncaps();

    if(_response)
    {
	assert(_os.b.size() == headerSize + 4); // Dispatch status position.
	_os.write(static_cast<Byte>(0));
	_os.startWriteEncaps();
    }

    DispatchStatus status;

    //
    // Don't put the code above into the try block below. Exceptions
    // in the code above are considered fatal, and must propagate to
    // the caller of this operation.
    //

    try
    {
	if(servantManager)
	{
	    _servant = servantManager->findServant(_current.id);
	    
	    if(!_servant && !_current.id.category.empty())
	    {
		_locator = servantManager->findServantLocator(_current.id.category);
		if(_locator)
		{
		    _servant = _locator->locate(_current, _cookie);
		}
	    }
	    
	    if(!_servant)
	    {
		_locator = servantManager->findServantLocator("");
		if(_locator)
		{
		    _servant = _locator->locate(_current, _cookie);
		}
	    }
	}
	    
	if(!_servant)
	{
	    status = DispatchObjectNotExist;
	}
	else
	{
	    if(!_current.facet.empty())
	    {
		ObjectPtr facetServant = _servant->ice_findFacetPath(_current.facet, 0);
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
		status = _servant->__dispatch(*this, _current);
	    }

	    //
	    // In case of an asynchronous dispatch, _is is now empty,
	    // because an IncomingAsync has adopted this Incoming.
	    //
	    if(_is.b.empty())
	    {
		//
		// If this was an asynchronous dispatch, we're done
		// here.  We do *not* call __finishInvoke(), because the
		// call is not finished yet.
		//
		assert(status == DispatchOK);
		return;
	    }
	}
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

	__finishInvoke(false);
	return;
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

	__finishInvoke(false);
	return;
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

	__finishInvoke(false);
	return;
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

	__finishInvoke(false);
	return;
    }
    catch(const std::exception& ex)
    {
	__warning(string("std::exception: ") + ex.what());

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
	return;
    }
    catch(...)
    {
	__warning("unknown c++ exception");

	if(_response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownException));
	    string reason = "unknown c++ exception";
	    _os.write(reason);
	}

	__finishInvoke(false);
	return;
    }

    //
    // Don't put the code below into the try block above. Exceptions
    // in the code below are considered fatal, and must propagate to
    // the caller of this operation.
    //

    if(_response)
    {
	_os.endWriteEncaps();
	
	if(status != DispatchOK && status != DispatchUserException)
	{
	    assert(status == DispatchObjectNotExist ||
		   status == DispatchFacetNotExist ||
		   status == DispatchOperationNotExist);
	    
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(status));
	    
	    _current.id.__write(&_os);
	    _os.write(_current.facet);
	    _os.write(_current.operation);
	}
	else
	{
	    *(_os.b.begin() + headerSize + 4) = static_cast<Byte>(status); // Dispatch status position.
	}
    }

    __finishInvoke(status == DispatchOK || status == DispatchUserException);
}
