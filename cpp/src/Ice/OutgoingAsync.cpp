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

#include <Ice/OutgoingAsync.h>
#include <Ice/Object.h>
#include <Ice/Connection.h>
#include <Ice/Reference.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(OutgoingAsync* p) { p->__incRef(); }
void IceInternal::decRef(OutgoingAsync* p) { p->__decRef(); }

void IceInternal::incRef(Object_ice_invoke* p) { p->__incRef(); }
void IceInternal::decRef(Object_ice_invoke* p) { p->__decRef(); }

IceInternal::OutgoingAsync::OutgoingAsync() :
    _is(0),
    _os(0)
{
}

IceInternal::OutgoingAsync::~OutgoingAsync()
{
    delete _is;
    delete _os;
}

void
IceInternal::OutgoingAsync::__setup(const ConnectionPtr& connection, const ReferencePtr& ref,
				    const string& operation, OperationMode mode, const Context& context)
{
    _connection = connection;
    _compress = ref->compress;
    assert(!_is && !_os);
    _is = new BasicStream(ref->instance);
    _os = new BasicStream(ref->instance);

    _connection->prepareRequest(_os);
    
    ref->identity.__write(_os);
    _os->write(ref->facet);
    _os->write(operation);
    _os->write(static_cast<Byte>(mode));
    _os->writeSize(Int(context.size()));
    Context::const_iterator p;
    for(p = context.begin(); p != context.end(); ++p)
    {
	_os->write(p->first);
	_os->write(p->second);
    }
    
    //
    // Input and output parameters are always sent in an
    // encapsulation, which makes it possible to forward requests as
    // blobs.
    //
    _os->startWriteEncaps();
}

void
IceInternal::OutgoingAsync::__invoke()
{
    _connection->incUsageCount();
    try
    {
	_connection->sendAsyncRequest(this, _compress);
    }
    catch(...)
    {
	_connection->decUsageCount();
	throw;
    }
}

void
IceInternal::OutgoingAsync::__finished(BasicStream& is)
{
    try
    {
	_is->swap(is);
	Byte status;
	_is->read(status);

	switch(static_cast<DispatchStatus>(status))
	{
	    case DispatchOK:
	    {
		//
		// Input and output parameters are always sent in an
		// encapsulation, which makes it possible to forward
		// oneway requests as blobs.
		//
		_is->startReadEncaps();
		__response(true);
		break;
	    }
	    
	    case DispatchUserException:
	    {
		//
		// Input and output parameters are always sent in an
		// encapsulation, which makes it possible to forward
		// oneway requests as blobs.
		//
		_is->startReadEncaps();
		__response(false);
		break;
	    }
	    
	    case DispatchObjectNotExist:
	    {
		ObjectNotExistException ex(__FILE__, __LINE__);
		ex.id.__read(_is);
		_is->read(ex.facet);
		_is->read(ex.operation);
		ice_exception(ex);
		break;
	    }

	    case DispatchFacetNotExist:
	    {
		FacetNotExistException ex(__FILE__, __LINE__);
		ex.id.__read(_is);
		_is->read(ex.facet);
		_is->read(ex.operation);
		ice_exception(ex);
		break;
	    }
	    
	    case DispatchOperationNotExist:
	    {
		OperationNotExistException ex(__FILE__, __LINE__);
		ex.id.__read(_is);
		_is->read(ex.facet);
		_is->read(ex.operation);
		ice_exception(ex);
		break;
	    }
	    
	    case DispatchUnknownException:
	    {
		UnknownException ex(__FILE__, __LINE__);
		_is->read(ex.unknown);
		ice_exception(ex);
		break;
	    }
	    
	    case DispatchUnknownLocalException:
	    {
		UnknownLocalException ex(__FILE__, __LINE__);
		_is->read(ex.unknown);
		ice_exception(ex);
		break;
	    }
	    
	    case DispatchUnknownUserException:
	    {
		UnknownUserException ex(__FILE__, __LINE__);
		_is->read(ex.unknown);
		ice_exception(ex);
		break;
	    }
	    
	    default:
	    {
		ice_exception(UnknownReplyStatusException(__FILE__, __LINE__));
		break;
	    }
	}
    }
    catch(const Exception& ex)
    {
	if(_os->instance()->properties()->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
	{
	    Warning out(_os->instance()->logger());
	    out << "Ice::Exception raised by AMI callback:\n" << ex;
	}
	_connection->decUsageCount();
	return;
    }
    catch(const std::exception& ex)
    {
	if(_os->instance()->properties()->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
	{
	    Warning out(_os->instance()->logger());
	    out << "std::exception raised by AMI callback:\n" << ex.what();
	}
	_connection->decUsageCount();
	return;
    }
    catch(...)
    {
	if(_os->instance()->properties()->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
	{
	    Warning out(_os->instance()->logger());
	    out << "unknown exception raised by AMI callback";
	}
	_connection->decUsageCount();
	return;
    }

    _connection->decUsageCount();
}

void
IceInternal::OutgoingAsync::__finished(const LocalException& ex)
{
    try
    {
	ice_exception(ex);
    }
    catch(const Exception& ex)
    {
	if(_os->instance()->properties()->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
	{
	    Warning out(_os->instance()->logger());
	    out << "Ice::Exception raised by AMI callback:\n" << ex;
	}
	_connection->decUsageCount();
	return;
    }
    catch(const std::exception& ex)
    {
	if(_os->instance()->properties()->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
	{
	    Warning out(_os->instance()->logger());
	    out << "std::exception raised by AMI callback:\n" << ex.what();
	}
	_connection->decUsageCount();
	return;
    }
    catch(...)
    {
	if(_os->instance()->properties()->getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
	{
	    Warning out(_os->instance()->logger());
	    out << "unknown exception raised by AMI callback";
	}
	_connection->decUsageCount();
	return;
    }

    _connection->decUsageCount();
}

BasicStream*
IceInternal::OutgoingAsync::__is()
{
    return _is;
}

BasicStream*
IceInternal::OutgoingAsync::__os()
{
    return _os;
}

void
Ice::Object_ice_invoke::__response(bool ok) // ok == true means no user exception.
{
    vector<Byte> outParams;
    try
    {
	BasicStream* __is = this->__is();
	Int sz = __is->getReadEncapsSize();
	__is->readBlob(outParams, sz);
    }
    catch(const Exception& ex)
    {
	ice_exception(ex);
	return;
    }
    ice_response(ok, outParams);
}
