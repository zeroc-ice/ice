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

#include <Ice/Incoming.h>
#include <Ice/ObjectAdapterI.h> // We need ObjectAdapterI, not ObjectAdapter, because of inc/decUsageCount().
#include <Ice/ServantLocator.h>
#include <Ice/Object.h>
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

IceInternal::Incoming::Incoming(const InstancePtr& instance, const ObjectAdapterPtr& adapter,
				Connection* connection, bool compress) :
    _connection(connection),
    _compress(compress),
    _is(instance),
    _os(instance)
{
    _current.adapter = adapter;
}

bool
IceInternal::Incoming::invoke()
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

    if(_connection) // Response expected?
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
	if(_current.adapter)
	{
	    dynamic_cast<ObjectAdapterI*>(_current.adapter.get())->incUsageCount();

	    _servant = _current.adapter->identityToServant(_current.id);
	    
	    if(!_servant && !_current.id.category.empty())
	    {
		_locator = _current.adapter->findServantLocator(_current.id.category);
		if(_locator)
		{
		    _servant = _locator->locate(_current, _cookie);
		}
	    }
	    
	    if(!_servant)
	    {
		_locator = _current.adapter->findServantLocator("");
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

	    if(status == DispatchAsync)
	    {
		//
		// This was a asynchronous dispatch, we're done here.
		// We do *not* call finishInvoke(), because the call is
		// not finished yet.
		//
		return true; // Asynchronous dispatch.
	    }
	}
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
	}

	if(_os.instance()->properties()->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
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

	    Warning out(_os.instance()->logger());
	    out << "dispatch exception:\n" << ex;
	}
	
	return false; // Regular, non-asynchronous dispatch.
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
	}

	if(_os.instance()->properties()->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	{
	    ostringstream str;
	    str << ex;
	    warning("dispatch exception: unknown local exception:", str.str());
	}
	
	return false; // Regular, non-asynchronous dispatch.
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
	}

	if(_os.instance()->properties()->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	{
	    ostringstream str;
	    str << ex;
	    warning("dispatch exception: unknown user exception:", str.str());
	}

	return false; // Regular, non-asynchronous dispatch.
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
	}

	if(_os.instance()->properties()->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	{
	    ostringstream str;
	    str << ex;
	    warning("dispatch exception: unknown exception:", str.str());
	}

	return false; // Regular, non-asynchronous dispatch.
    }
    catch(const std::exception& ex)
    {
	finishInvoke();

	if(_connection) // Response expected?
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownException));
	    ostringstream str;
	    str << "std::exception: " << ex.what();
	    _os.write(str.str());
	}

	if(_os.instance()->properties()->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	{
	    warning("dispatch exception: unknown std::exception:", ex.what());
	}

	return false; // Regular, non-asynchronous dispatch.
    }
    catch(...)
    {
	finishInvoke();

	if(_connection) // Response expected?
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownException));
	    string reason = "unknown c++ exception";
	    _os.write(reason);
	}

	if(_os.instance()->properties()->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	{
	    warning("dispatch exception: unknown c++ exception:", "");
	}

	return false; // Regular, non-asynchronous dispatch.
    }

    //
    // Don't put the code below into the try block above. Exceptions
    // in the code below are considered fatal, and must propagate to
    // the caller of this operation.
    //

    finishInvoke();

    if(_connection) // Response expected?
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

    return false; // Regular, non-asynchronous dispatch.
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

void
IceInternal::Incoming::finishInvoke()
{
    if(_locator && _servant)
    {
	try
	{
	    _locator->finished(_current, _servant, _cookie);
	}
	catch(...)
	{
	    if(_current.adapter)
	    {
		dynamic_cast<ObjectAdapterI*>(_current.adapter.get())->decUsageCount();
	    }
	    throw;
	}
    }
    
    if(_current.adapter)
    {
	dynamic_cast<ObjectAdapterI*>(_current.adapter.get())->decUsageCount();
    }
    
    _is.endReadEncaps();
}

void
IceInternal::Incoming::warning(const string& msg, const string& ex) const
{
    Warning out(_os.instance()->logger());

    out << msg;
    if(!ex.empty())
    {
	out << "\n" << ex;
    }
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
