// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Incoming.h>
#include <IceE/ServantManager.h>
#include <IceE/Object.h>
#include <IceE/Connection.h>
#include <IceE/LocalException.h>
#include <IceE/Instance.h>
#include <IceE/Properties.h>
#include <IceE/IdentityUtil.h>
#include <IceE/LoggerUtil.h>
#include <IceE/Protocol.h>
#include <IceE/StringUtil.h>
#include <IceE/ObjectAdapter.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::Incoming::Incoming(Instance* inst, Connection* con, BasicStream& is, const ObjectAdapterPtr& adapter) :
    _os(inst, inst->messageSizeMax()),
    _is(is),
    _connection(con)
{
    setAdapter(adapter);

    _current.con = con;
    _current.adapter = adapter.get();
}

void
IceInternal::Incoming::setAdapter(const Ice::ObjectAdapterPtr& adapter)
{
    _adapter = adapter;
    if(_adapter)
    {
	_servantManager = _adapter->getServantManager().get();
	if(!_servantManager)
	{
	    _adapter = 0;
	}
    }
    else
    {
	_servantManager = 0;
    }
}

void
IceInternal::Incoming::invoke(bool response, Int requestId)
{
    assert(_adapter && _servantManager);

    _current.requestId = requestId;

    //
    // Clear the context from the previous invocation.
    //
    _current.ctx.clear();

    //
    // Read the current.
    //
    //_current.id.__read(&_is);
    _is.read(_current.id.name); // Directly read name for performance reasons.
    _is.read(_current.id.category); // Directly read category for performance reasons.

    //
    // For compatibility with the old FacetPath. Note that we don't use
    // the stream read vector method for performance reasons.
    //
//     vector<string> facetPath;
//     _is.read(facetPath);
//     string facet;
//     if(!facetPath.empty())
//     {
// 	if(facetPath.size() > 1)
// 	{
// 	    throw MarshalException(__FILE__, __LINE__);
// 	}
// 	facet.swap(facetPath[0]);
//     }
//     _current.facet.swap(facet);
    Int sz;
    _is.readSize(sz);
    if(sz > 0)
    {
	if(sz > 1)
	{
	    throw MarshalException(__FILE__, __LINE__);
	}
	_is.read(_current.facet);
    }
    else
    {
	_current.facet.clear();
    }

    _is.read(_current.operation);

    Byte b;
    _is.read(b);
    _current.mode = static_cast<OperationMode>(b);

    _is.readSize(sz);
    while(sz--)
    {
	pair<const string, string> pr;
	_is.read(const_cast<string&>(pr.first));
	_is.read(pr.second);
	_current.ctx.insert(_current.ctx.end(), pr);
    }

    _is.startReadEncaps();

    if(response)
    {
	assert(_os.b.size() == headerSize + 4); // Dispatch status position.
	_os.write(static_cast<Byte>(0));
	_os.startWriteEncaps();
    }

    // Initialize status to some value, to keep the compiler happy.
    DispatchStatus status = DispatchOK;

    //
    // Don't put the code above into the try block below. Exceptions
    // in the code above are considered fatal, and must propagate to
    // the caller of this operation.
    //

    try
    {
	Ice::ObjectPtr servant;
	if(_servantManager)
	{
	    servant = _servantManager->findServant(_current.id, _current.facet);
	}
	    
	if(!servant)
	{
	    if(_servantManager && _servantManager->hasServant(_current.id))
	    {
	        status = DispatchFacetNotExist;
	    }
	    else
	    {
	        status = DispatchObjectNotExist;
	    }
	}
	else
	{
	    status = servant->__dispatch(*this, _current);
	}
    }
    catch(RequestFailedException& ex)
    {
	_is.endReadEncaps();

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

	if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
	{
	    __warning(ex);
	}

	if(response)
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
	    
	    _connection->sendResponse(&_os);
	}
	else
	{
	    _connection->sendNoResponse();
	}
	
	return;
    }
    catch(const UnknownLocalException& ex)
    {
	_is.endReadEncaps();

	if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	{
	    __warning(ex);
	}

	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownLocalException));
	    _os.write(ex.unknown);
	    _connection->sendResponse(&_os);
	}
	else
	{
	    _connection->sendNoResponse();
	}

	return;
    }
    catch(const UnknownUserException& ex)
    {
	_is.endReadEncaps();

	if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	{
	    __warning(ex);
	}

	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownUserException));
	    _os.write(ex.unknown);
	    _connection->sendResponse(&_os);
	}
	else
	{
	    _connection->sendNoResponse();
	}

	return;
    }
    catch(const UnknownException& ex)
    {
	_is.endReadEncaps();

	if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	{
	    __warning(ex);
	}

	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownException));
	    _os.write(ex.unknown);
	    _connection->sendResponse(&_os);
	}
	else
	{
	    _connection->sendNoResponse();
	}

	return;
    }
    catch(const LocalException& ex)
    {
	_is.endReadEncaps();

	if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	{
	    __warning(ex);
	}

	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownLocalException));
	    _os.write(ex.toString());
	    _connection->sendResponse(&_os);
	}
	else
	{
	    _connection->sendNoResponse();
	}

	return;
    }
    catch(const UserException& ex)
    {
	_is.endReadEncaps();

	if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	{
	    __warning(ex);
	}

	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownUserException));
	    _os.write(ex.toString());
	    _connection->sendResponse(&_os);
	}
	else
	{
	    _connection->sendNoResponse();
	}

	return;
    }
    catch(const Exception& ex)
    {
	_is.endReadEncaps();

	if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	{
	    __warning(ex);
	}

	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownException));
	    _os.write(ex.toString());
	    _connection->sendResponse(&_os);
	}
	else
	{
	    _connection->sendNoResponse();
	}

	return;
    }
    catch(const std::exception& ex)
    {
	_is.endReadEncaps();

	if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	{
	    __warning(string("std::exception: ") + ex.what());
	}

	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownException));
	    string msg = string("std::exception: ") + ex.what();
	    _os.write(msg);
	    _connection->sendResponse(&_os);
	}
	else
	{
	    _connection->sendNoResponse();
	}

	return;
    }
    catch(...)
    {
	_is.endReadEncaps();

	if(_os.instance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
	{
	    __warning("unknown c++ exception");
	}

	if(response)
	{
	    _os.endWriteEncaps();
	    _os.b.resize(headerSize + 4); // Dispatch status position.
	    _os.write(static_cast<Byte>(DispatchUnknownException));
	    _os.write(string("unknown c++ exception"));
	    _connection->sendResponse(&_os);
	}
	else
	{
	    _connection->sendNoResponse();
	}

	return;
    }

    //
    // Don't put the code below into the try block above. Exceptions
    // in the code below are considered fatal, and must propagate to
    // the caller of this operation.
    //
    
    _is.endReadEncaps();

    if(response)
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

	    //
	    // For compatibility with the old FacetPath.
	    //
	    if(_current.facet.empty())
	    {
		_os.write(static_cast<string*>(0), static_cast<string*>(0));
	    }
	    else
	    {
		_os.write(&_current.facet, &_current.facet + 1);
	    }

	    _os.write(_current.operation);
	}
	else
	{
	    *(_os.b.begin() + headerSize + 4) = static_cast<Byte>(status); // Dispatch status position.
	}

	_connection->sendResponse(&_os);
    }
    else
    {
	_connection->sendNoResponse();
    }
}

void
IceInternal::Incoming::__warning(const Exception& ex) const
{
    __warning(ex.toString());
}

void
IceInternal::Incoming::__warning(const string& msg) const
{
    Warning out(_os.instance()->initializationData().logger);
    out << "dispatch exception: " << msg;
    out << "\nidentity: " << identityToString(_current.id);
    out << "\nfacet: " << IceUtil::escapeString(_current.facet, "");
    out << "\noperation: " << _current.operation;
}

